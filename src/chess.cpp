#include <cstdlib>
#include <stdexcept>
#include "chess.hpp"

ChessState::ChessState()
: AbstractState<int, 64 * 64, 64>() {
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

    int start_location = action / 64;
    int end_location = action % 64;
    board[end_location] = board[start_location];
    board[start_location] = 0;

    turn = !get_turn();
    status = compute_status();
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
    int end_piece = board[end_location];
    // blank square of chessboard
    if (start_piece == 0)
        return false;
    // black piece but white turn, can't move that piece
    if (start_piece < 0 && turn)
        return false;
    // white piece but black turn, can't move that piece
    if (start_piece > 0 && !turn)
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
        // must actually move
        if (abs_d_row == 0 && abs_d_col == 0)
            return false;
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
        // at most one square in each direction
        if (abs_d_row > 1 || abs_d_col > 1)
            return false;
        // can't land on own piece
        if ((start_piece > 0 && end_piece > 0) || (start_piece < 0 && end_piece < 0))
            return false;
    }
    // check for castling
    // check for en passant
    // check for promotion
    // check for capture
    // check for check
    // check for checkmate
    // check for stalemate
    // check for threefold repetition

    return true;
}

void ChessState::is_valid_thrower(std::size_t action) const {
    // if (get_status() != InProgress)
    //     throw std::runtime_error("Game is not in progress.");
    // if (action >= get_num_actions())
    //     throw std::invalid_argument("Invalid action.");
    // if (board[0 * 7 + action] != 0)
    //     throw std::invalid_argument("Invalid action.");
}

Status ChessState::compute_status() const {
    return InProgress;
    // TODO: Implement this
}
