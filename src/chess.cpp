#include <cstdlib>
#include <stdexcept>
#include "chess.hpp"

ChessState::ChessState()
: AbstractState<int, 64, 64 * 64>() {
    int row = 0;
    board[row * 8 + 3] = -3;
    board[row * 8 + 4] = -2;
    board[row * 8 + 0] = -6;
    board[row * 8 + 7] = -6;
    board[row * 8 + 1] = -5;
    board[row * 8 + 6] = -5;
    board[row * 8 + 2] = -4;
    board[row * 8 + 5] = -4;

    row = 1;
    for (int col = 0; col != 8; ++col)
        board[row * 8 + col] = -1;

    for (row = 2; row != 6; ++row)
        for (int col = 0; col != 8; ++col)
            board[row * 8 + col] = 0;
    
    row = 6;
    for (int col = 0; col != 8; ++col)
        board[row * 8 + col] = 1;

    row = 7;
    board[row * 8 + 3] = 3;
    board[row * 8 + 4] = 2;
    board[row * 8 + 0] = 6;
    board[row * 8 + 7] = 6;
    board[row * 8 + 1] = 5;
    board[row * 8 + 6] = 5;
    board[row * 8 + 2] = 4;
    board[row * 8 + 5] = 4;

    turn = true;
    status = InProgress;
}

ChessState::ChessState(const ChessState &state, std::size_t action)
: AbstractState(state, action) {
    is_valid_thrower(action);
    apply_move(action);
    status = compute_status();
}

// moves the piece (and the rook, when castling) and passes the turn, without validating the move.
// is_valid uses this to look at the position after a move, which it can't do through the public
// constructor: that constructor calls is_valid, which would recurse forever.
void ChessState::apply_move(std::size_t action) {
    int start_location = action / 64;
    int end_location = action % 64;
    int moving_piece = board[start_location];
    board[end_location] = moving_piece;
    board[start_location] = 0;

    // castling: the king moved two squares along its rank, so bring the rook
    // from its corner to the square the king passed over
    if ((moving_piece == 2 || moving_piece == -2)
        && start_location / 8 == end_location / 8
        && std::abs(end_location - start_location) == 2) {
        int row = start_location / 8;
        bool kingside = end_location > start_location;
        int rook_from = row * 8 + (kingside ? 7 : 0);
        int rook_to = (start_location + end_location) / 2;
        board[rook_to] = board[rook_from];
        board[rook_from] = 0;
    }

    turn = !get_turn();
}

std::ostream& ChessState::print(std::ostream &os) const {
    for (int i = 0; i != 8 * 8; ++i) {
        if (i % 8 == 0)
            os << "\n";
        if (board[i] == 0)
            os << " . ";
        else if (board[i] == 1)
            os << "WP ";
        else if (board[i] == 2)
            os << "WK ";
        else if (board[i] == 3)
            os << "WQ ";
        else if (board[i] == 4)
            os << "WB ";
        else if (board[i] == 5)
            os << "WN ";
        else if (board[i] == 6)
            os << "WR ";
        else if (board[i] == -1)
            os << "BP ";
        else if (board[i] == -2)
            os << "BK ";
        else if (board[i] == -3)
            os << "BQ ";
        else if (board[i] == -4)
            os << "BB ";
        else if (board[i] == -5)
            os << "BN ";
        else if (board[i] == -6)
            os << "BR ";
    }
    os << "\n";
    return os;
}

bool ChessState::is_valid(std::size_t action) const {
    if (get_status() != InProgress)
        return false;
    if (action >= get_num_actions())
        return false;

    int start_location = action / 64;
    int end_location = action % 64;
    int start_piece = board[start_location];
    // black piece but white turn, can't move that piece
    if (start_piece < 0 && turn)
        return false;
    // white piece but black turn, can't move that piece
    if (start_piece > 0 && !turn)
        return false;

    // the piece's own movement rules (including castling's required squares)
    if (!is_pseudo_valid(action))
        return false;

    // check for check
    // castling: the king can't castle out of check, or through a square that is attacked
    if ((start_piece == 2 || start_piece == -2)
        && start_location / 8 == end_location / 8
        && std::abs(end_location - start_location) == 2) {
        if (in_check(turn))
            return false;
        ChessState passing_through(*this);
        passing_through.apply_move(start_location * 64 + (start_location + end_location) / 2);
        if (passing_through.in_check(turn))
            return false;
    }
    // no move may leave the mover's own king in check: this stops the king walking into check,
    // and also stops pinned pieces from moving and ignoring a check that's already there
    ChessState next(*this);
    next.apply_move(action);
    if (next.in_check(turn))
        return false;

    // check for promotion

    // TODO: handle these cases which require expanding ChessState
    // check for en passant
    // check for checkmate
    // check for stalemate
    // check for threefold repetition

    return true;
}

// true if the king of the given colour is attacked, i.e. some opponent piece could move onto its square.
// attacks use is_pseudo_valid, not is_valid: a piece still gives check even if moving it would expose
// its own king, and this also keeps is_valid from recursing.
bool ChessState::in_check(bool white) const {
    int own_king = white ? 2 : -2;
    int king_location = -1;
    for (int i = 0; i != 64; ++i)
        if (board[i] == own_king)
            king_location = i;
    if (king_location == -1)
        return false;
    for (int i = 0; i != 64; ++i) {
        bool opponent = white ? (board[i] < 0) : (board[i] > 0);
        if (opponent && is_pseudo_valid(i * 64 + king_location))
            return true;
    }
    return false;
}

// whether the move follows the moving piece's own movement rules, ignoring whose turn it is and check.
bool ChessState::is_pseudo_valid(std::size_t action) const {
    if (action >= get_num_actions())
        return false;

    int start_location = action / 64;
    int end_location = action % 64;
    int start_piece = board[start_location];
    int end_piece = board[end_location];
    // blank square of chessboard
    if (start_piece == 0)
        return false;

    // check for piece conditional moves
    // queen: moves any number of squares along a rank, file, or diagonal
    if (start_piece == 3 || start_piece == -3) {
        int start_row = start_location / 8, start_col = start_location % 8;
        int end_row = end_location / 8, end_col = end_location % 8;
        int d_row = end_row - start_row;
        int d_col = end_col - start_col;
        // must actually move
        if (d_row == 0 && d_col == 0)
            return false;
        // must be straight (rank/file) or diagonal
        bool straight = (d_row == 0 || d_col == 0);
        bool diagonal = (std::abs(d_row) == std::abs(d_col));
        if (!straight && !diagonal)
            return false;
        // can't land on own piece
        if ((start_piece > 0 && end_piece > 0) || (start_piece < 0 && end_piece < 0))
            return false;
        // can't jump over pieces: every square strictly between start and end must be empty
        int step_row = (d_row > 0) - (d_row < 0);
        int step_col = (d_col > 0) - (d_col < 0);
        int row = start_row + step_row, col = start_col + step_col;
        while (row != end_row || col != end_col) {
            if (board[row * 8 + col] != 0)
                return false;
            row += step_row;
            col += step_col;
        }
    }
    // rook: moves any number of squares along a rank or file
    if (start_piece == 6 || start_piece == -6) {
        int start_row = start_location / 8, start_col = start_location % 8;
        int end_row = end_location / 8, end_col = end_location % 8;
        int d_row = end_row - start_row;
        int d_col = end_col - start_col;
        // must actually move
        if (d_row == 0 && d_col == 0)
            return false;
        // must be straight (rank/file)
        if (d_row != 0 && d_col != 0)
            return false;
        // can't land on own piece
        if ((start_piece > 0 && end_piece > 0) || (start_piece < 0 && end_piece < 0))
            return false;
        // can't jump over pieces: every square strictly between start and end must be empty
        int step_row = (d_row > 0) - (d_row < 0);
        int step_col = (d_col > 0) - (d_col < 0);
        int row = start_row + step_row, col = start_col + step_col;
        while (row != end_row || col != end_col) {
            if (board[row * 8 + col] != 0)
                return false;
            row += step_row;
            col += step_col;
        }
    }
    // bishop: moves any number of squares along a diagonal
    if (start_piece == 4 || start_piece == -4) {
        int start_row = start_location / 8, start_col = start_location % 8;
        int end_row = end_location / 8, end_col = end_location % 8;
        int d_row = end_row - start_row;
        int d_col = end_col - start_col;
        // must actually move
        if (d_row == 0 && d_col == 0)
            return false;
        // must be diagonal
        if (std::abs(d_row) != std::abs(d_col))
            return false;
        // can't land on own piece
        if ((start_piece > 0 && end_piece > 0) || (start_piece < 0 && end_piece < 0))
            return false;
        // can't jump over pieces: every square strictly between start and end must be empty
        int step_row = (d_row > 0) - (d_row < 0);
        int step_col = (d_col > 0) - (d_col < 0);
        int row = start_row + step_row, col = start_col + step_col;
        while (row != end_row || col != end_col) {
            if (board[row * 8 + col] != 0)
                return false;
            row += step_row;
            col += step_col;
        }
    }
    // knight: moves in an L-shape (2 squares one way, 1 square the other) and can jump over pieces
    if (start_piece == 5 || start_piece == -5) {
        int start_row = start_location / 8, start_col = start_location % 8;
        int end_row = end_location / 8, end_col = end_location % 8;
        int abs_d_row = std::abs(end_row - start_row);
        int abs_d_col = std::abs(end_col - start_col);
        // must be an L-shape
        if (!((abs_d_row == 2 && abs_d_col == 1) || (abs_d_row == 1 && abs_d_col == 2)))
            return false;
        // can't land on own piece
        if ((start_piece > 0 && end_piece > 0) || (start_piece < 0 && end_piece < 0))
            return false;
    }
    // king: moves exactly one square in any direction (castling handled separately)
    if (start_piece == 2 || start_piece == -2) {
        int start_row = start_location / 8, start_col = start_location % 8;
        int end_row = end_location / 8, end_col = end_location % 8;
        int abs_d_row = std::abs(end_row - start_row);
        int abs_d_col = std::abs(end_col - start_col);
        // must actually move
        if (abs_d_row == 0 && abs_d_col == 0)
            return false;
        // at most one square in each direction, unless castling (two squares along the rank, checked below)
        bool castling_shape = (abs_d_row == 0 && abs_d_col == 2);
        if (!castling_shape && (abs_d_row > 1 || abs_d_col > 1))
            return false;
        // can't land on own piece
        if ((start_piece > 0 && end_piece > 0) || (start_piece < 0 && end_piece < 0))
            return false;
    }
    // pawn: moves straight forward onto an empty square (one square, or two from its starting row),
    // and captures one square diagonally forward. en passant is handled separately.
    if (start_piece == 1 || start_piece == -1) {
        int start_row = start_location / 8, start_col = start_location % 8;
        int end_row = end_location / 8, end_col = end_location % 8;
        // white moves up the board (toward row 0), black moves down (toward row 7)
        int forward = (start_piece > 0) ? -1 : 1;
        int pawn_start_row = (start_piece > 0) ? 6 : 1;
        int d_row = end_row - start_row;
        int d_col = end_col - start_col;
        if (d_col == 0) {
            // straight move: destination must be empty (pawns can't capture straight ahead)
            if (end_piece != 0)
                return false;
            if (d_row == forward) {
                // single step: ok
            } else if (d_row == 2 * forward && start_row == pawn_start_row) {
                // double step: the square jumped over must also be empty
                if (board[(start_row + forward) * 8 + start_col] != 0)
                    return false;
            } else {
                return false;
            }
        } else if (std::abs(d_col) == 1 && d_row == forward) {
            // diagonal move: only allowed as a capture of an opponent piece
            bool captures_opponent = (start_piece > 0) ? (end_piece < 0) : (end_piece > 0);
            if (!captures_opponent)
                return false;
        } else {
            return false;
        }
    }
    // check for castling
    // the king moves two squares toward a rook; the rook's move is implicit.
    // simplification: the king and rook only need to be on their starting squares
    // (we don't track whether either has moved before).
    // TODO: handle if the king has moved before
    if (start_piece == 2 || start_piece == -2) {
        int start_row = start_location / 8, start_col = start_location % 8;
        int end_row = end_location / 8, end_col = end_location % 8;
        int d_col = end_col - start_col;
        if (start_row == end_row && std::abs(d_col) == 2) {
            // king must be on its starting square (white e1, black e8)
            int home_row = (start_piece > 0) ? 7 : 0;
            if (start_row != home_row || start_col != 4)
                return false;
            // own rook must be on the corner square on the side the king moves toward
            int rook_col = (d_col > 0) ? 7 : 0;
            int own_rook = (start_piece > 0) ? 6 : -6;
            if (board[home_row * 8 + rook_col] != own_rook)
                return false;
            // every square between the king and the rook must be empty
            int step_col = (d_col > 0) ? 1 : -1;
            for (int col = start_col + step_col; col != rook_col; col += step_col)
                if (board[home_row * 8 + col] != 0)
                    return false;
        }
    }

    return true;
}

void ChessState::is_valid_thrower(std::size_t action) const {
    if (!is_valid(action))
        throw std::invalid_argument("Invalid action");
}

// called after a move, when `turn` is the side about to move. player one is white (turn == true).
// if the side to move has any valid move, the game goes on. if it has none, it's checkmate when
// that side's king is in check (the other side wins), and stalemate (a tie) when it isn't.
Status ChessState::compute_status() const {
    for (std::size_t action = 0; action != get_num_actions(); ++action)
        if (is_valid(action))
            return InProgress;
    if (in_check(turn))
        return turn ? PlayerTwoWon : PlayerOneWon;
    return Tie;
}
