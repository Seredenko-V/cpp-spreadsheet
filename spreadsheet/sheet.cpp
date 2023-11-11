#include "sheet.h"
#include "cell.h"
#include "common.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>

using namespace std;

std::pair<Position, Position> Sheet::FindPrintArea() const {
    Position min_pos{0, 0};
    Position max_pos{0, 0};

    for (const auto& [key, _] : sheet_) {
        min_pos.row = min(min_pos.row, key.row);
        min_pos.col = min(min_pos.col, key.col);

        max_pos.row = max(max_pos.row, key.row);
        max_pos.col = max(max_pos.col, key.col);
    }
    return {min_pos, max_pos};
}

Sheet::~Sheet() {}

void Sheet::SetCell(Position pos, string text) {
    if(!pos.IsValid()) {
        throw InvalidPositionException("The position ("s + to_string(pos.row) + ',' + to_string(pos.col)
                                       + ") is incorrect"s);
    }
    if (!sheet_.count(pos)) {
        // создаем ячейку
        sheet_[pos] = make_unique<Cell>(*this);
    }
    sheet_[pos]->Set(text);
}

const CellInterface* Sheet::GetCell(Position pos) const {
    if(!pos.IsValid()) {
        throw InvalidPositionException("The position ("s + to_string(pos.row) + ',' + to_string(pos.col)
                                       + ") is incorrect"s);
    }
    return sheet_.count(pos) ? sheet_.at(pos).get() : nullptr;
}

CellInterface* Sheet::GetCell(Position pos) {
    if(!pos.IsValid()) {
        throw InvalidPositionException("The position ("s + to_string(pos.row) + ',' + to_string(pos.col)
                                       + ") is incorrect"s);
    }
    return sheet_.count(pos) ? sheet_.at(pos).get() : nullptr;
}

void Sheet::ClearCell(Position pos) {
    if (!pos.IsValid()) {
        throw InvalidPositionException("The position ("s + to_string(pos.row) + ',' + to_string(pos.col)
                                       + ") is incorrect"s);
    }
    if (sheet_.count(pos)) {
        sheet_.erase(pos);
    }
}

Size Sheet::GetPrintableSize() const {
    if (sheet_.empty()) {
        return Size{0, 0};
    }

    auto [min_pos, max_pos] = FindPrintArea();
    return Size{max_pos.row - min_pos.row + 1, max_pos.col - min_pos.col + 1};
}

void Sheet::PrintValues(std::ostream& output) const {
    if (sheet_.empty()) {
        return;
    }

    // т.к. у unordered_map нет установленного порядка элементов
    auto [min_pos, max_pos] = FindPrintArea();
    for (int row = min_pos.row; row <= max_pos.row; ++row) {
        for (int col = min_pos.col; col <= max_pos.col; ++col) {
            Position current_pos{row, col};
            const CellInterface* current_cell = GetCell(current_pos);

            if (current_cell) {
                //проверка содержимого ячейки
                if (holds_alternative<double>(current_cell->GetValue())) {
                    output << get<double>(current_cell->GetValue());
                } else if(holds_alternative<string>(current_cell->GetValue())) {
                    output << get<string>(current_cell->GetValue());
                } else {
                    output << get<FormulaError>(current_cell->GetValue());
                }
            }

            if (col == max_pos.col) {
                continue;
            }
            output << '\t';
        }
        output << '\n';
    }
}

void Sheet::PrintTexts(std::ostream& output) const {
    if (sheet_.empty()) {
        return;
    }

    // т.к. у unordered_map нет установленного порядка элементов
    auto [min_pos, max_pos] = FindPrintArea();
    for (int row = min_pos.row; row <= max_pos.row; ++row) {
        for (int col = min_pos.col; col <= max_pos.col; ++col) {
            Position current_pos{row, col};
            const CellInterface* current_cell = GetCell(current_pos);
            if (current_cell) {
                output << current_cell->GetText();
            }
            if (col == max_pos.col) {
                continue;
            }
            output << '\t';
        }
        output << '\n';
    }
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}
