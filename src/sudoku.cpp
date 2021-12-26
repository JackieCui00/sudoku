#include "sudoku.h"

std::string Sudoku::ToString()
{
    std::stringstream ss;

    const std::string head = "  | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 ";
    const std::string lineSeperator = std::string(head.size(), '-');

    ss << head;

    uint64_t rowIndex = 0;
    for (const auto& row : mData)
    {
        ss << std::endl << lineSeperator << std::endl;
        ss << ++rowIndex << " ";

        for (const auto& cell : row)
        {
            ss << "| " << cell.ValueString() << " ";
        }
    }

    return ss.str();
}

bool Sudoku::VerifyRow(const uint64_t index)
{
    std::bitset<10> bits;
    bits.set(0);

    const auto& row = mData.at(index);
    for (const auto& cell : row)
    {
        if (!cell.IsSet())
        {
            return false;
        }

        const uint64_t value = cell.GetValue();
        ASSERT(value >= 1 && value <= 9);
        bits.set(value);
    }

    return bits.all();
}

bool Sudoku::VerifyColumn(const uint64_t index)
{
    std::bitset<10> bits;
    bits.set(0);

    for (const auto& row : mData)
    {
        const auto& cell = row.at(index);
        if (!cell.IsSet())
        {
            return false;
        }

        const uint64_t value = cell.GetValue();
        ASSERT(value >=1 && value <= 9);
        bits.set(value);
    }

    return bits.all();
}

bool Sudoku::VerifySquare(const uint64_t index)
{
    std::bitset<10> bits;
    bits.set(0);

    ASSERT(index <= 8);
    const uint64_t rowStart = (index / 3) * 3;
    const uint64_t columnStart = (index % 3) * 3;

    for (uint64_t i = 0; i < 3; ++i)
    {
        const auto& row = mData.at(rowStart + i);
        for (uint64_t j = 0; j < 3; ++j)
        {
            const auto& cell = row.at(columnStart + j);
            if (!cell.IsSet())
            {
                return false;
            }
            const uint64_t value = cell.GetValue();
            ASSERT(value >= 1 && value <= 9);
            bits.set(value);
        }
    }

    return bits.all();
}

bool Sudoku::Verify()
{
    for (uint64_t i = 0; i < 9; ++i)
    {
        if (this->VerifyRow(i) \
                && this->VerifyColumn(i) \
                && this->VerifySquare(i))
        {
            continue;
        }

        return false;
    }

    return true;
}

void Sudoku::ClearCandidate(const uint64_t rowIndex,
                    const uint64_t columnIndex,
                    const uint64_t value)
{
    { // clear row
        auto& row = mData.at(rowIndex);
        for (auto& cell : row)
        {
            cell.RemoveCandidate(value);
        }
    }

    {
        for (auto& row : mData)
        {
            auto& cell = row.at(columnIndex);
            cell.RemoveCandidate(value);
        }
    }

    {
        const uint64_t rowStart = (rowIndex / 3) * 3;
        const uint64_t columnStart = (columnIndex / 3) * 3;
        for (uint64_t i = 0; i < 3; ++i)
        {
            auto& row = mData.at(rowStart + i);
            for (uint64_t j = 0; j < 3; ++j)
            {
                auto& cell = row.at(columnStart + j);
                cell.RemoveCandidate(value);
            }
        }
    }
}

static std::mt19937_64 RandomGenerator;

std::unique_ptr<Sudoku> RandomSudokuGenerator::Generate()
{
    std::unique_ptr<Sudoku> result;
    for (uint64_t times = 0; ; ++times)
    {
        result = std::make_unique<Sudoku>();
        bool success = true;
        for (auto iter = result->begin(); iter != result->end(); ++iter)
        {
            auto cell = *iter;
            const std::vector<uint64_t>& candidate = cell->GetCandidates();
            if (candidate.size() == 0)
            {
                success = false;
                break;
            }
            const uint64_t value = candidate.at(RandomGenerator() % candidate.size());

            const auto index = iter.GetRowColumnIndex();
            result->SetCell(index.first, index.second, value);
        }

        if (success)
        {
            // std::cout << "Success after " << times << " times." << std::endl;
            return result;
        }
    }
}

InputSudokuGenerator::InputSudokuGenerator(const std::string& file) : mFile(file)
{
    if (mFile.empty())
    { // use std::cin
        return;
    }

    mInput = std::ifstream(mFile.c_str());
}

InputSudokuGenerator::~InputSudokuGenerator()
{
    if (mFile.empty())
    {
        // use std::cin
        return;
    }

    mInput.close();
}

std::unique_ptr<Sudoku> InputSudokuGenerator::Generate()
{
    std::istream* in = nullptr;
    if (mFile.empty())
    {
        in = &std::cin;
    }
    else
    {
        in = &mInput;
    }

    auto result = std::make_unique<Sudoku>();
    for (auto iter = result->begin(); iter != result->end(); ++iter)
    {
        int value = 100;
        if (!in->good())
        {
            std::cerr << "Fail to input sudoku, file:<" << mFile << ">";
            return nullptr;
        }
        *in >> value;

        if (value < 1 || value > 9)
        {
            continue;
        }

        auto cell = *iter;
        cell->SetColor(Color::RED);

        const auto index = iter.GetRowColumnIndex();
        result->SetCell(index.first, index.second, value);

        ASSERT(cell->IsSet());
    }

    return result;
}

void IterateSudokuSolver::interactive(Sudoku* sudoku)
{
    while (std::cin.good())
    {
        std::cout << "> ";

        std::string line;
        std::getline(std::cin, line);
        if (line.empty())
        {
            continue;
        }

        std::stringstream ss(line);

        std::string s;
        ss >> s;

        if (s == "continue" || s == "c")
        {
            return;
        }
        else if (s == "query" || s == "q")
        {
            int rowIndex = 0;
            int columnIndex = 0;
            ss >> rowIndex >> columnIndex;
            if (rowIndex < 1 || rowIndex > 9
                    || columnIndex < 1 || columnIndex> 9)
            {
                std::cerr << "Invalid query index:<" << rowIndex
                    << ", " << columnIndex << ">" << std::endl;
                continue;
            }

            auto cell = sudoku->GetCell(rowIndex - 1, columnIndex - 1);
            std::cout << std::make_pair(rowIndex, columnIndex) << ": "
                << cell->ToString() << std::endl;
        }
        else if (s == "row" || s == "r")
        {
            int rowIndex = 0;
            ss >> rowIndex;
            if (rowIndex < 1 || rowIndex > 9)
            {
                std::cerr << "Invalid rowIndex:" << rowIndex << std::endl;
                continue;
            }

            for (uint64_t columnIndex = 1; columnIndex <= 9; ++ columnIndex)
            {
                auto cell = sudoku->GetCell(rowIndex - 1, columnIndex - 1);
                std::cout << std::make_pair(rowIndex, columnIndex)
                    << ": " << cell->ToString() << std::endl;
            }
        }
        else if (s == "column" || s == "c")
        {
            int columnIndex = 0;
            ss >> columnIndex;
            if (columnIndex < 1 || columnIndex > 9)
            {
                std::cerr << "Invalid columnIndex:" << columnIndex << std::endl;
                continue;
            }

            for (uint64_t rowIndex = 1; rowIndex <= 9; ++rowIndex)
            {
                auto cell = sudoku->GetCell(rowIndex - 1, columnIndex - 1);
                std::cout << std::make_pair(rowIndex, columnIndex)
                    << ": " << cell->ToString() << std::endl;
            }
        }
        else if (s == "square" || s == "s")
        {
            int squareIndex = 0;
            ss >> squareIndex;
            if (squareIndex < 1 || squareIndex > 9)
            {
                std::cerr << "Invalid squareIndex:" << squareIndex << std::endl;
                continue;
            }

            --squareIndex;
            const uint64_t rowStart = (squareIndex / 3) * 3;
            const uint64_t columnStart = (squareIndex % 3) * 3;
            for (uint64_t i = 0; i < 3; ++i)
            {
                for (uint64_t j = 0; j < 3; ++j)
                {
                    const uint64_t rowIndex = rowStart + i + 1;
                    const uint64_t columnIndex = columnStart + j + 1;
                    auto cell = sudoku->GetCell(rowIndex - 1, columnIndex - 1);
                    std::cout << std::make_pair(rowIndex, columnIndex)
                        << ": " << cell->ToString() << std::endl;
                }
            }
        }
        else if (s == "all" || s == "a")
        {
            for (uint64_t rowIndex = 1; rowIndex <=9; ++rowIndex)
            {
                for (uint64_t columnIndex = 1; columnIndex <=9; ++ columnIndex)
                {
                    auto cell = sudoku->GetCell(rowIndex - 1, columnIndex - 1);
                    std::cout << std::make_pair(rowIndex, columnIndex)
                        << ": " << cell->ToString() << std::endl;
                }
            }
        }
        else
        {
            std::cout << "Avaiable commands:" << std::endl
                        << "\tcontinue|c" << std::endl
                        << "\tquery|q <row> <column>" << std::endl
                        << "\trow|r <row>" << std::endl
                        << "\tcolumn|c <column>" << std::endl
                        << "\tsquare|s <square>" << std::endl
                        << "\tall|a" << std::endl;
        }
    }

    exit(0);
}

void IterateSudokuSolver::Solve(Sudoku* sudoku)
{
    for (int i = 1; ; ++i)
    {
        this->interactive(sudoku);

        bool hasProgress = false;
        const Cell* lastCellWithCandidate = nullptr;
        std::pair<uint64_t, uint64_t> lastCellWithCandidatePosition;
        std::vector<const Cell*> toClearCell;

        for (auto iter = sudoku->begin(); iter != sudoku->end(); ++iter)
        {
            auto cell = *iter;

            if (cell->IsSet())
            {
                continue;
            }

            std::vector<uint64_t> candidates = cell->GetCandidates();
            if (candidates.empty())
            {
                std::cout << "Failed, Final state:\n";
                cell->SetColor(Color::YELLOW);
                std::cout << sudoku->ToString() << std::endl;
                return;
            }

            if (candidates.size() == 1)
            {
                cell->SetColor(Color::GREEN);
                sudoku->SetCell(iter.GetRowColumnIndex(), candidates.at(0));
                toClearCell.push_back(cell);
                hasProgress = true;
            }
            else
            {
                lastCellWithCandidate = cell;
                lastCellWithCandidatePosition = iter.GetRowColumnIndex();
            }
        }

        if (!hasProgress)
        {
            auto candidates = lastCellWithCandidate->GetCandidates();
            std::cout << "Random setting cell, candidates:" << candidates
                << ", choosing:" << candidates.at(0) << std::endl;
            sudoku->SetCell(lastCellWithCandidatePosition, candidates.at(0));
            lastCellWithCandidate->SetColor(Color::YELLOW);
            toClearCell.push_back(lastCellWithCandidate);
        }

        std::cout << "Iterate Times:" << i << "\n"
            << sudoku->ToString() << std::endl;

        if (sudoku->IsAllSet())
        {
            std::cout << "Done! Iterate " << i << " Times" << std::endl;
            break;
        }

        for (auto& cell : toClearCell)
        {
            cell->SetColor(Color::RED);
        }
    }
}
