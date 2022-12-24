#pragma once

#include "cell.h"
#include "common.h"

#include <functional>
#include <unordered_map>

class Sheet : public SheetInterface {
public:
    ~Sheet();

    void SetCell(Position pos, std::string text) override;

    const CellInterface* GetCell(Position pos) const override;
    CellInterface* GetCell(Position pos) override;

    void ClearCell(Position pos) override;

    // получение размера печатной области
    Size GetPrintableSize() const override;

    void PrintValues(std::ostream& output) const override;
    void PrintTexts(std::ostream& output) const override;

private:
    struct PositionHasher {
        size_t operator()(const Position& position) const {
            return std::hash<int>()(position.col) + Position::MAX_ROWS * std::hash<int>()(position.row);
        }
    };

    // определяет границы области печати
    std::pair<Position, Position> FindPrintArea() const;

private:
    std::unordered_map<Position, std::unique_ptr<Cell>, PositionHasher> sheet_;
};
