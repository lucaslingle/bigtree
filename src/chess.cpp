#include <stdexcept>
#include "chess.hpp"

ChessState::ChessState()
: AbstractState<int, 64 * 73, 64>() {
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

    int start_location = action / 73;
    int command = action % 73;
    // queen like movement
    if (command < 56) {
        int direction = command / 7;
        int magnitude = 1 + (command % 7);
        // north
        if (direction == 0) {
            int row, col;
            for (int i = 1; i != magnitude + 1; ++i) {
                row = start_location / 8 + i;
                col = start_location % 8;
                if (board[row * 8 + col] != 0)
                    throw std::invalid_argument("Invalid move.");
            }
            if (row < 0 || row >= 8 || col < 0 || col >= 8)
                throw std::invalid_argument("Invalid move.");
            board[row * 8 + col] = board[start_location];
            board[start_location] = 0;
            return;
        }
    }

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
    // if (get_status() != InProgress)
    //     return false;
    // if (action >= get_num_actions())
    //     return false;
    // if (board[0 * 7 + action] != 0)
    //     return false;
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
