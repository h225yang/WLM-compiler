#ifndef CFGLR_H
#define CFGLR_H
#include <iostream>
#include <string>
#include <set>
#include <tuple>
#include <vector>
#include <stack>
#include <list>

struct ParseTree;

// read lr1 file and one sequence to be recognized, output unindented reversed rightmost derivation

class LR1 {
    // CFG part
    std::set<std::string> terminal;
    std::set<std::string> non_terminal;
    std::string start_symbol;
    std::vector<std::string> production_rule;

    int root_number = -1;

    // LR1 machine part
    std::vector<std::tuple<int, std::string, std::string, int>> parser;

    std::string target_sequence;

    std::tuple<int, std::string, std::string, int> parser_search(int state, std::string curr_str);


public:
    LR1(std::istream& lr_file);

    void take_target_sequence(std::istream& in);

    void read_rightmost();

    void push_root(ParseTree& curr_Stack, std::stringstream& ss);
};

#endif
