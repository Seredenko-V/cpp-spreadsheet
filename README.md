# Электронная таблица
Дипломный проект в рамках [курса](https://practicum.yandex.ru/cpp/?from=catalog) Яндекс Практикума.  
Является аналогом [Google таблиц](https://docs.google.com/spreadsheets) или [Excel](https://www.microsoft.com/ru-ru/microsoft-365/excel).  
Код лексического и синтаксического анализаторов, а также код для обхода дерева разбора на С++ генерируется при помощи
[ANTLR](https://www.antlr.org/).  

## Ячейки
Существует три типа ячеек: формульная, текстовая и пустая. В процессе работы тип ячейки может изменяться динамически, в зависимости от 
вновь установленного содержимого.  
Все ячейки имеют следующие методы:
1. `void Set(std::string)` - установить содержимое ячейки.
2. `void Clear()` -  очистить содержимое ячейки. Вызов этого метода равносилен созданию пустой ячейки.
3. `Value GetValue() const` - получить значение ячейки. Значениями могут являться `std::string`, `double` или `FormulaError`.
4. `std::string GetText() const` - получить текстовое содержимое ячейки.

### Формульная
Ячейка является формульной, если её текст начинается со знака `=`. Пробелы перед знаком `=` **не** игнорируются, то есть ячейка, 
заданная текстом: `" =1+2"`, не считается формульной. Содержимое после знака `=` является выражением формулы. При этом текст, 
который содержит только знак `=`, формулой не считается, а считается текстом.

### Текстовая
Все ячейки, кроме формульных, трактуются как текстовые. Обычно для них результат метода `GetValue()` совпадает с результатом 
метода `GetText()`. Кроме случая, когда текст начинается с символа `'` (апостроф). Тогда в `GetValue()` этот символ отсутствует. 
Это нужно, если мы хотим начать текст со знака `=`, но не хотим, чтобы он интерпретировался как формула.  
Например, ячейка задана строкой `"'=1+2"`, тогда
* `GetText()` от неё `"'=1+2"`;
* `GetValue()` от неё `"=1+2"`.

### Пустая
Создаётся при передаче в метод `Set(std::string)` пустой строки. Также может быть получена путём вызова метода `Clear()` 
от непустой ячейки. Методы `GetText()` и `GetValue()` от неё возвращают пустые строки.

## Таблица
* Хранит в себе ячейки `Cell`. Формулы в ячейках могут содержать числа и ссылки на другие ячейки.
* Доступ к ячейкам осуществляется при помощи индексов, которые представляют собой строки вида `B1`, `RD7`.
Например, индексу `А1` соответствует позиция `(0, 0)`, а индексу `AB15` — позиция `(14, 27)`.

### Минимальная печатная область
Для отображения таблицы необходимо знать размер её минимальной печатной области. Это минимальная прямоугольная область с 
вершиной в ячейке `A1`, содержащая все непустые ячейки. Она нужна чтобы не печатать всю таблицу, если её большая часть состоит
из пустых ячеек.
![Минимальная печатная область](https://raw.githubusercontent.com/Seredenko-V/cpp-spreadsheet/main/images/print-area.png?token=GHSAT0AAAAAAB7SDXXWS433JRMQU4426BYWZKQUBLQ "Минимальная печатная область")    

### Методы, применяемые ко всей таблице
1. `Size GetPrintableSize() const` - определяет размер минимальной печатной области. Ячейки из одной строки разделены 
табуляцией `\t`, в конце строки присутствует символ перевода строки `\n`.
2. `void PrintTexts(std::ostream&) const` - выводит текстовые представления ячеек. Это текст, который пользователь 
задал в методе `Set(std::string)`.
![PrintTexts](https://raw.githubusercontent.com/Seredenko-V/cpp-spreadsheet/main/images/PrintTexts.png?token=GHSAT0AAAAAAB7SDXXX23DZPZ3UXA2HIQSKZKQUBXQ "PrintTexts")
3. `void PrintValues(std::ostream&) const` - выводит значения ячеек — строки, числа или `FormulaError`.
![PrintValues](https://raw.githubusercontent.com/Seredenko-V/cpp-spreadsheet/main/images/PrintValues.png?token=GHSAT0AAAAAAB7SDXXXPXBGEZLPI5XDQ7PEZKQUCDA "PrintValues")

### Методы, применяемые к ячейке по индексу
1. `SetCell(Position, std::string)` - задаёт содержимое ячейки по индексу `Position`. Если ячейки не существует, то будет 
создана пустая, с последующим вызовом метода `Cell::Set(std::string)`. Этот метод установит нужный тип ячейки в зависимости 
от содержимого строки.
2. `Cell* GetCell(Position pos)` - константный и неконстантный геттеры, которые возвращают указатель на ячейку, 
расположенную по индексу `pos`. Если ячейка пуста, возвращают `nullptr`. Последующий вызов `GetCell()` для этой ячейки 
вернёт `nullptr`. При этом может измениться размер минимальной печатной области. Если в примере выше удалить ячейку `В5`, 
минимальная печатная область поменяется и будет выглядеть так:
![Минимальная печатная область](https://raw.githubusercontent.com/Seredenko-V/cpp-spreadsheet/main/images/print-area2.png?token=GHSAT0AAAAAAB7SDXXWPWSTXUPIOH2KJKJKZKQUCLQ "Минимальная печатная область")   

## Системные требования
* CMake 3.8
* GCC 10.0
* C++17
* ANTLR 4.11.0

## Настройка окружения
* Руководство по установке ANTLR4 можно найти [здесь](https://github.com/antlr/antlr4/blob/master/doc/getting-started.md). 
Если возникают трудности при работе в windows, можно также попробовать 
[это руководство](https://www.notion.so/ANTLR4-windows-0addba24c5d74810a964eac8420f272a?pvs=4).

* Для компиляции кода понадобится библиотека ANTLR4 C++ Runtime. Скачайте архив `antlr4-cpp-runtime*.zip` 
из раздела [Download](https://www.antlr.org/download.html), расположенный в **C++ Target**.

* Далее необходимо собрать проект согласно структуре
```
spreadsheet/
 ├── antlr4_runtime/
 │   └── Содержимое архива antlr4-cpp-runtime*.zip.
 ├── build/
 ├── antlr-4.11.1-complete.jar
 ├── CMakeLists.txt
 ├── FindANTLR.cmake
 ├── Formula.g4
 ├── Остальные файлы проекта
 └── ...
```
**Обратите внимание**, что версия архива `antlr-4.11.1-complete.jar` может отличаться от актуальной. 
 В CMakeLists.txt замените версию JAR-файла на актуальную.
 
## Тесты и примеры использования
Примеры использования таблицы являются тестами и находятся в `tests.cpp`. Отдельный пример вынесен в `main.cpp`.

## UML-диаграммы
### Модуль `domain`
![domain-UML](https://raw.githubusercontent.com/Seredenko-V/cpp-spreadsheet/7ca0a3f16eff909cb652069fa4b94329d7d2fd95/images/domain-modul-UML.svg?token=AWESMY2LHGGRK6AAWE6SRGDFKCQPO "domain-UML")
### Модуль `formula`
![formula-UML](https://raw.githubusercontent.com/Seredenko-V/cpp-spreadsheet/7ca0a3f16eff909cb652069fa4b94329d7d2fd95/images/formula-modul-UML.svg?token=AWESMY5PA7RX2XLIRS6LCFDFKCQSM "formula-UML")
### Решение полностью
![solution-UML](https://raw.githubusercontent.com/Seredenko-V/cpp-spreadsheet/7ca0a3f16eff909cb652069fa4b94329d7d2fd95/images/solution-UML.svg?token=AWESMY66AYBU7DSRHIC2IRDFKCQVQ "solution-UML") 
