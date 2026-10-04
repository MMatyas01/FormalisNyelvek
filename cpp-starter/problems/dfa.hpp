#ifndef PROBLEMS_DFA_H
#define PROBLEMS_DFA_H

#include "../problem.hpp"

#include <string>
#include <vector>
#include <set>
#include <map>

class DFAProblem : public Problem {
public:
    void initialize_parser(cxxopts::Options &options) override;
    bool is_chosen_problem(const cxxopts::ParseResult &args) override;
    int run(const cxxopts::ParseResult &args) override;

private:
    std::vector<std::string> states;
    std::vector<char> alphabet;
    std::string startState;
    std::set<std::string> finalStates;

    std::map<std::pair<std::string, char>, std::string> transitions;
};

#endif // PROBLEMS_DFA_H