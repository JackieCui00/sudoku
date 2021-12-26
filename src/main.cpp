#include  "sudoku.h"

int main(int argc, char* argv[])
{
    // RandomGenerator.seed(std::time(nullptr));
    // RandomSudokuGenerator generator;
    // std::cout << "Random:\n" << generator.Generate()->ToString() << std::endl;

    for (int i = 1; i < argc; ++i)
    {
        InputSudokuGenerator inputGenerator(argv[i]);
        auto sudoku = inputGenerator.Generate();
        std::cout << "Input: " << argv[i] << "\n"
            << sudoku->ToString() << std::endl;

        IterateSudokuSolver solver;
        solver.Solve(sudoku.get());
    }

    return 0;
}
