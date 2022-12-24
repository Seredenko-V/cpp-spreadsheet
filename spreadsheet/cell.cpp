#include "cell.h"
#include "sheet.h"

#include <cassert>
#include <iostream>
#include <string>
#include <optional>
#include <stdexcept>
#include <stack>

using namespace std;

// =========================================================

class Cell::Impl {
public:
    virtual string GetText() const = 0;
    virtual Value GetValue() const = 0;
    virtual void CacheInvalidate() {}
    virtual vector<Position> GetReferencedCells() const {
        return {};
    }
};

class Cell::EmptyImpl : public Impl {
public:
    string GetText() const override {
        return {};
    }

    Value GetValue() const override {
        return ""s;
    }
};

class Cell::TextImpl : public Impl {
public:
    TextImpl(string&& text)
        : text_(move(text)) {
        if (text_.empty()) {
            throw logic_error("TextImpl cannot be empty"s);
        }
    }

    string GetText() const override {
        return text_;
    }

    Value GetValue() const override {
        if (text_.front() == ESCAPE_SIGN) {
            return string(text_.begin() + 1, text_.end());
        }
        return text_;
    }

private:
    string text_;
};

class Cell::FormulaImpl : public Impl {
public:
    FormulaImpl(string&& text, const SheetInterface& sheet)
        // т.к. "=" нужно отсеять на данном этапе
        : formula_(ParseFormula(text.substr(1)))
        , sheet_(sheet) {
    }

    string GetText() const override {
        return FORMULA_SIGN + formula_->GetExpression();
    }

    Value GetValue() const override {
        FormulaInterface::Value result;

        if (!cache_.has_value()) {
            cache_ = formula_->Evaluate(sheet_);
        }
        result = cache_.value();

        if (holds_alternative<double>(result)) {
            return get<double>(result);
        }
        return get<FormulaError>(result);
    }

    void CacheInvalidate() override {
        cache_.reset();
    }

    vector<Position> GetReferencedCells() const override {
        return formula_->GetReferencedCells();
    }

private:
    unique_ptr<FormulaInterface> formula_;
    const SheetInterface& sheet_;
    mutable optional<FormulaInterface::Value> cache_;
};

// =========================================================

Cell::Cell(Sheet& sheet)
    : impl_(std::make_unique<EmptyImpl>())
    , sheet_(sheet) {
}

Cell::~Cell(){}

void Cell::Set(string text) {
    unique_ptr<Impl> tmp_impl;
    if (text.empty()) {
        tmp_impl = make_unique<EmptyImpl>();
    } else if (text.size() > 1 && text.front() == FORMULA_SIGN) {
        tmp_impl = make_unique<FormulaImpl>(move(text), sheet_);
    } else {
        tmp_impl = make_unique<TextImpl>(move(text));
    }

    if (IsCircularDependency(*tmp_impl)) {
        throw CircularDependencyException("The new value of the cell \"" + tmp_impl->GetText() +
                                          "\" creates a cyclic dependency"s);
    }
    impl_ = move(tmp_impl);

    // кладем в перечень зависимых ячееек родителей (от которых зависим) текущую ячейку
    for (const Position& pos : impl_->GetReferencedCells()) {
        Cell* cell = static_cast<Cell*>(sheet_.GetCell(pos));
        // если ячейки не существует, то создаем пустую
        if (!cell) {
           sheet_.SetCell(pos, ""s);
           cell = static_cast<Cell*>(sheet_.GetCell(pos));
        }

        cell->descendants_.insert(this);
    }
    CacheInvalidate();
}

void Cell::Clear() {
    impl_ = make_unique<EmptyImpl>();
}

Cell::Value Cell::GetValue() const {
    return impl_->GetValue();
}
std::string Cell::GetText() const {
    return impl_->GetText();
}

vector<Position> Cell::GetReferencedCells() const {
    return impl_->GetReferencedCells();
}

// =========================================================

void Cell::CacheInvalidate() {
    impl_->CacheInvalidate();
    for (Cell* cell : descendants_) {
        cell->CacheInvalidate();
    }
}

bool Cell::IsCircularDependency(const Impl& impl) const {
    // для обеспечения поиска зависимых ячеек за О(1)
    unordered_set<const Cell*> dependent_cells;
    for (const Position& pos : impl.GetReferencedCells()) {
        dependent_cells.insert(static_cast<Cell*>(sheet_.GetCell(pos)));
    }

    stack<const Cell*> to_visit; // стек для обхода ячеек
    to_visit.push(this); // добавление текущей ячейки
    unordered_set<const Cell*> already_visited; // посещенные ячейки

    // Depth First Search (DFS) поиск в глубину
    while (!to_visit.empty()) {
        const Cell* current_cell = to_visit.top();
        to_visit.pop();

        // если текущая ячейка присутствует среди зависимых от нее
        if (dependent_cells.count(current_cell)) {
            return true;
        }

        // запись ячеек, зависимых от текущей
        for (const Cell* cell : current_cell->descendants_) {
            to_visit.push(cell);
        }
    }

    return false;
}
