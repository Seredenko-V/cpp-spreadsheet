#include "common.h"
#include "formula.h"
#include "sheet.h"
#include "test_runner_p.h"

#include <string>
#include <variant>

inline std::ostream& operator<<(std::ostream& output, Size size) {
    return output << "(" << size.rows << ", " << size.cols << ")";
}

inline Position operator"" _pos(const char* str, std::size_t) {
    return Position::FromString(str);
}

void PrintSheet(const std::unique_ptr<SheetInterface>& sheet, std::ostream& out) {
    out << sheet->GetPrintableSize() << std::endl;
    out << "PrintTexts:\n";
    out << "====================================================" << std::endl;
    sheet->PrintTexts(out);
    out << "====================================================" << std::endl;
    out << "PrintValues:\n";
    out << "====================================================" << std::endl;
    sheet->PrintValues(out);
    out << "====================================================" << std::endl;
}

void PrintExample() {
    auto sheet = CreateSheet();
    sheet->SetCell("A1"_pos, "=(1+2)*3");
    sheet->SetCell("B1"_pos, "=1+(2*3)");

    sheet->SetCell("A2"_pos, "some");
    sheet->SetCell("B2"_pos, "text");
    sheet->SetCell("C2"_pos, "here");

    sheet->SetCell("C3"_pos, "'and'");
    sheet->SetCell("D3"_pos, "'here");

    sheet->SetCell("B5"_pos, "=1/0");
    PrintSheet(sheet, std::cout);
}

int main() {
    using namespace tests;
    TestRunner tr;
    RUN_TEST(tr, TestPositionAndStringConversion);
    RUN_TEST(tr, TestPositionToStringInvalid);
    RUN_TEST(tr, TestStringToPositionInvalid);
    RUN_TEST(tr, TestEmpty);
    RUN_TEST(tr, TestInvalidPosition);
    RUN_TEST(tr, TestSetCellPlainText);
    RUN_TEST(tr, TestClearCell);
    RUN_TEST(tr, TestFormulaArithmetic);
    RUN_TEST(tr, TestFormulaReferences);
    RUN_TEST(tr, TestFormulaExpressionFormatting);
    RUN_TEST(tr, TestFormulaReferencedCells);
    RUN_TEST(tr, TestErrorValue);
    RUN_TEST(tr, TestErrorDiv0);
    RUN_TEST(tr, TestEmptyCellTreatedAsZero);
    RUN_TEST(tr, TestFormulaInvalidPosition);
    RUN_TEST(tr, TestPrint);
    RUN_TEST(tr, TestCellReferences);
    RUN_TEST(tr, TestFormulaIncorrect);
    RUN_TEST(tr, TestCellCircularReferences);
    RUN_TEST(tr, TestClearPrint);
    RUN_TEST(tr, TestExample);
    RUN_TEST(tr, TestSetGetCellFormulaZeroDivision);

    PrintExample();
    return 0;
}
