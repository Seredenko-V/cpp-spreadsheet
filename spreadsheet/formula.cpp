#include "formula.h"

#include "FormulaAST.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <sstream>
#include <string>

//using namespace std::literals;

using namespace std;

// ==================================================================

FormulaError::FormulaError(Category category)
    : category_(category) {
}

FormulaError::Category FormulaError::GetCategory() const {
    return category_;
}

bool FormulaError::operator==(FormulaError rhs) const {
    return category_ == rhs.category_;
}

std::string_view FormulaError::ToString() const {
    switch(category_) {
        case Category::Div0:
            return "#DIV/0!"sv;
        case Category::Ref:
            return "#REF!"sv;
        case Category::Value:
            return "#VALUE!"sv;
    }
    return ""sv;
}

std::ostream& operator<<(std::ostream& output, FormulaError fe) {
    return output << fe.ToString();
}

// ==================================================================

namespace {
class Formula : public FormulaInterface {
public:
    // учитывается случай синтаксически некорректной формулы
    explicit Formula(std::string expression)
    try : ast_(ParseFormulaAST(expression)) {
    } catch (const std::exception& e) {
        std::throw_with_nested(FormulaException(e.what()));
    }

    Value Evaluate(const SheetInterface& sheet) const override {
        // Получить значение ячейки. Если ячейка текстовая, то будет
        // предпринята попытка конвертации к числу
        const ArgumentCell arg = [&sheet](const Position& pos) -> double {
            if (!pos.IsValid()) {
                throw FormulaError(FormulaError::Category::Ref);
            }
            const CellInterface* cell = sheet.GetCell(pos);
            if (!cell) {
                return 0;
            }

            if (holds_alternative<double>(cell->GetValue())) {
                return get<double>(cell->GetValue());
            }
            // попытка конвертации к числу
            if (holds_alternative<string>(cell->GetValue())) {
                string text_cell = get<string>(cell->GetValue());
                double result = 0;
                if (!text_cell.empty()) {
                    std::istringstream in(text_cell);
                    if (!(in >> result) || !in.eof()) {
                        throw FormulaError(FormulaError::Category::Value);
                    }
                }
                return result;
            }
            throw FormulaError(get<FormulaError>(cell->GetValue()));
        };

        try {
            return ast_.Execute(arg);
        } catch (const FormulaError& except) {
            return except;
        }
    }

    std::vector<Position> GetReferencedCells() const override {
        std::vector<Position> cells;
        const std::forward_list<Position>& list_cells = ast_.GetCells();
        cells.reserve(distance(list_cells.begin(), list_cells.end()));

        for (const Position& cell : list_cells) {
            if (cell.IsValid()) {
                cells.push_back(cell);
            }
        }

        // удаление повторяющихся ячеек
        cells.resize(std::unique(cells.begin(), cells.end()) - cells.begin());
        return cells;
    }

    std::string GetExpression() const override {
        std::ostringstream out;
        ast_.PrintFormula(out);
        return out.str();
    }

private:
    FormulaAST ast_;
};
}  // namespace

std::unique_ptr<FormulaInterface> ParseFormula(std::string expression) {
    return std::make_unique<Formula>(std::move(expression));
}
