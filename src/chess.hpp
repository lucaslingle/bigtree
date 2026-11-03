#pragma once
#include <iostream>
#include <array>
#include <string>
#include "state.hpp"

class ChessState : public AbstractState<int, 64 * 64, 64> {
public:
    ChessState();
    ChessState(const ChessState &state, std::size_t action);
    std::ostream& print(std::ostream& os) const override;
    bool is_valid(std::size_t action) const override;
    void is_valid_thrower(std::size_t action) const override;
    Status compute_status() const override;
};
