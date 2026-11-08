#ifndef PROBLEMS_MINI_H
#define PROBLEMS_MINI_H

#include "../problem.hpp"

#include <string>
#include <vector>
#include <set>
#include <map>
#include <utility>

class MiniProblem : public Problem {
public:
    void initialize_parser(cxxopts::Options &options) override;
    bool is_chosen_problem(const cxxopts::ParseResult &args) override;
    int run(const cxxopts::ParseResult &args) override;

private:
    std::vector<std::string> states;
    std::vector<char> alphabet;
    std::string startState;
    std::set<std::string> finalStates;

    // Determinisztikus átmenetek: (állapot, szimbólum) -> célállapot
    std::map<std::pair<std::string, char>, std::string> transitions;

    // Segédfüggvény a "q12" -> 12 kinyeréséhez
    int extract_number(const std::string& s);
};

#endif // PROBLEMS_MINI_H