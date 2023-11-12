#pragma once

#include "common.h"

#include <functional>
#include <unordered_set>

class Sheet;

class Cell : public CellInterface {
public:
    Cell(Sheet& sheet);
    ~Cell();

    void Set(std::string text);
    void Clear();

    Value GetValue() const override;
    std::string GetText() const override;

    // получение позиций ячеек-родителей (от которых зависим)
    std::vector<Position> GetReferencedCells() const override;

private:
    // возможные типы ячейки
    class Impl;
    class EmptyImpl;
    class TextImpl;
    class FormulaImpl;
    std::unique_ptr<Impl> impl_;
    Sheet& sheet_; // лист, которому принадлежит ячейка
    std::unordered_set<Cell*> descendants_; // зависимые ячейки от текущей

private:
    // проверка на наличие циклическоей зависимости
    bool IsCircularDependency(const Impl& impl) const;

    // очистка кэша текущей и зависимых ячеек
    void CacheInvalidate();
};
