#include <array>
#include <bitset>
#include <cassert>
#include <ctime>
#include <fstream>
#include <iostream>
#include <memory>
#include <random>
#include <sstream>
#include <vector>

#define ASSERT(condition) do { \
    if (!(condition)) { \
        std::cerr << "Assertion Failed!!!! " \
                << "file:" << __FILE__ \
                << ", line:" << __LINE__ \
                << ", condition:" << #condition << std::endl; \
        assert(false); \
    } \
} while (false)

template<typename T>
inline std::ostream& operator<<(std::ostream& os, const std::vector<T>& d)
{
    std::string seperator = "";
    os << "[";
    for (const auto& element : d)
    {
        os << seperator << element;
        seperator = ", ";
    }
    return os << "]";
}

inline std::ostream& operator<<(std::ostream& os,
        const std::pair<uint64_t, uint64_t>& p)
{
    return os << "(" << p.first << ", " << p.second << ")";
}

enum class Color
{
    EMPTY,
    RESET,
    RED,
    GREEN,
    YELLOW,
};

inline const char* TerminalPrefix(const Color c)
{
    switch (c)
    {
    case Color::EMPTY:
        return "";
    case Color::RESET:
        return "\x1b[0m";
    case Color::RED:
        return "\x1b[31m";
    case Color::GREEN:
        return "\x1b[32m";
    case Color::YELLOW:
        return "\x1b[33m";
    default:
        ASSERT(0);
    }

    return "";
}

class Cell
{
public:
    Cell() = default;
    explicit Cell(const uint64_t value) : mValue(value)
    {
        mTaken.set(value);
    }
    ~Cell() = default;

    bool IsSet() const
    {
        return mValue != 0;
    }

    uint64_t GetValue() const
    {
        return mValue;
    }

    void SetValue(const uint64_t value)
    {
        mValue = value;
    }

    std::vector<uint64_t> GetCandidates() const
    {
        std::vector<uint64_t> result;
        for (uint64_t i = 1; i <= 9; ++i)
        {
            if (!mTaken.test(i))
            {
                result.emplace_back(i);
            }
        }

        return result;
    }

    void RemoveCandidate(const uint64_t value)
    {
        ASSERT(value >= 1 && value <= 9);
        mTaken.set(value);
    }

    std::string ToString() const
    {
        std::stringstream ss;
        ss << "Cell{Value:" << this->ValueString()
            << ", Candidates:" << this->GetCandidates();
        return ss.str();
    }

    std::string ValueString() const
    {
        std::stringstream ss;
        ss << TerminalPrefix(mColor) << mValue << TerminalPrefix(Color::RESET);
        return ss.str();
    }

    void ClearColor(const Color c) const
    {
        mColor = Color::EMPTY;
    }

    void SetColor(const Color c) const
    {
        mColor = c;
    }

private:
    mutable Color mColor{Color::EMPTY};

    uint64_t mValue{0};
    std::bitset<10> mTaken{"0000000001"};
};

class Sudoku
{
public:
    Sudoku() = default;
    ~Sudoku() = default;

    std::string ToString();
    bool Verify();

    const Cell* GetCell(const uint64_t rowIndex,
                        const uint64_t columnIndex)
    {
        return &(mData.at(rowIndex).at(columnIndex));
    }

    void SetCell(const std::pair<uint64_t, uint64_t>& index,
            const uint64_t value)
    {
        this->SetCell(index.first, index.second, value);
    }

    void SetCell(const uint64_t rowIndex,
                        const uint64_t columnIndex,
                        const uint64_t value)
    {
        ++mSetCount;
        mData.at(rowIndex).at(columnIndex).SetValue(value);
        this->ClearCandidate(rowIndex, columnIndex, value);
    }

    bool IsAllSet() const
    {
        ASSERT(mSetCount <= 81);
        return mSetCount == 81;
    }

    class Iterator {
    public:
        explicit Iterator(Sudoku* owner) : mOwner(owner) {}
        Iterator(Sudoku* owner, const uint64_t index) : mOwner(owner), mIndex(index) {}
        ~Iterator() = default;

        bool operator!=(const Iterator& other)
        {
            return mOwner != other.mOwner || mIndex != other.mIndex;
        }

        Iterator& operator++()
        {
            ++mIndex;
            return *this;
        }

        const Cell* operator*()
        {
            const auto index = GetRowColumnIndex();
            return &(mOwner->mData.at(index.first).at(index.second));
        }

        std::pair<uint64_t, uint64_t> GetRowColumnIndex()
        {
            return std::make_pair(mIndex / 9, mIndex % 9);
        }

    private:
        Sudoku* mOwner;
        uint64_t mIndex = 0;
    };

    Iterator begin() {
        return Iterator(this);
    }

    Iterator& end() {
        return mEndIterator;
    }

private:
    bool VerifyRow(const uint64_t index);

    bool VerifyColumn(const uint64_t index);

    bool VerifySquare(const uint64_t index);

    void ClearCandidate(const uint64_t rowIndex,
                        const uint64_t columnIndex,
                        const uint64_t value);

private:
    using Row = std::array<Cell, 9>;
    std::array<Row, 9> mData{};
    Iterator mEndIterator{this, 81};
    uint64_t mSetCount{0};
};

class ISudokuSolver
{
public:
    ISudokuSolver() = default;
    virtual ~ISudokuSolver() = default;

    virtual void Solve(Sudoku* sudoku) = 0;
};

class IterateSudokuSolver : ISudokuSolver
{
public:
    IterateSudokuSolver() = default;
    ~IterateSudokuSolver() = default;

    void Solve(Sudoku* sudoku) override;

private:
    void interactive(Sudoku* sudoku);
};

class ISudokuGenerator
{
public:
    ISudokuGenerator() = default;
    virtual ~ISudokuGenerator() = default;

    virtual std::unique_ptr<Sudoku> Generate() = 0;
};

class RandomSudokuGenerator final : public ISudokuGenerator
{
public:
    RandomSudokuGenerator() = default;
    ~RandomSudokuGenerator() = default;

    std::unique_ptr<Sudoku> Generate() override;
};

class InputSudokuGenerator final : public ISudokuGenerator
{
public:
    explicit InputSudokuGenerator(const std::string& file = "");
    ~InputSudokuGenerator();

    std::unique_ptr<Sudoku> Generate() override;

private:
    std::string mFile;
    std::ifstream mInput;
};
