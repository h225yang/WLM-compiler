#include "cfglr.h"
#include <sstream>
#include <fstream>
#include <stdexcept>
#include <algorithm>
using namespace std;

struct ParseTree {
    string rule;
    string lexeme;
    list<ParseTree> children;

    void printTree() {
        for(auto tree : children) {
            tree.printTree();
        }
        cout << rule << endl;
    }
};


LR1::LR1(std::istream& lr1_file) {

    // Construct of LR1

    // begin construct of CFG
    // read terminal
    int numberOfLines;
    lr1_file >> numberOfLines;
    lr1_file.ignore();
    for (int i = 0; i < numberOfLines; ++i) {
        string term;
        getline(lr1_file, term);
        this->terminal.insert(term);
    }
    // read non_term
    lr1_file >> numberOfLines;
    lr1_file.ignore();
    for (int i = 0; i < numberOfLines; ++i) {
        string non_term;
        getline(lr1_file, non_term);
        this->non_terminal.insert(non_term);
    }
    // read start_symbol
    string start;
    lr1_file >> start;
    if(this->non_terminal.count(start) == 0) {
        cerr << "ERROR: start symbol not in non terminal" << endl;
    }
    this->start_symbol = start;
    // read production_rule
    lr1_file >> numberOfLines;
    lr1_file.ignore();
    for (int i = 0; i < numberOfLines; ++i) {
        string rule;
        getline(lr1_file, rule);
        this->production_rule.push_back(rule);
        if(rule.compare(0,start_symbol.length(), start_symbol) == 0) {
            root_number = i;
        }
    }

    if(root_number == -1) {
        cerr << "ERROR: check start rule" << endl;
    }

    // end construct of CFG

    // begin construct of LR1 machine
    // first number of states is useless
    lr1_file >> numberOfLines;
    lr1_file >> numberOfLines;
    for (int i = 0; i < numberOfLines; ++i) {
        int state1;
        string symbol;
        string action;
        int state2_or_rule;

        lr1_file >> state1;
        lr1_file >> symbol;
        lr1_file >> action;
        lr1_file >> state2_or_rule;

        this->parser.push_back(make_tuple(state1, symbol, action, state2_or_rule));
    }
    // end of construction of LR1

    // take target sequence
    lr1_file.ignore();

}


tuple<int, string, string, int> LR1::parser_search(int state, string curr_str) {
    for (auto this_tuple : parser) {
        if(state == get<0>(this_tuple) && curr_str == get<1>(this_tuple)) {
            return this_tuple;
        }
    }
    throw std::logic_error("ERROR in search");
};


void LR1::push_root(ParseTree& curr_tree, stringstream& ss) {
    if(non_terminal.count(curr_tree.rule) == 1) { // rule
        getline(ss, curr_tree.rule);

        stringstream temp_ss{curr_tree.rule};
        string temp_str;
        temp_ss >> temp_str; // skip lhs

        while(temp_ss >> temp_str) {
            //cout << ">>>>>" << temp_str << "<<<<<" << endl;
            ParseTree temp_tree;
            temp_tree.rule = temp_str;
            push_root(temp_tree, ss);
            curr_tree.children.push_back(temp_tree);
        }
    } else {
        getline(ss, curr_tree.rule);
    }
}


void LR1::read_rightmost() {
    ParseTree final;

    stringstream ss{target_sequence};

    final.rule = "start";
    push_root(final ,ss);

    final.printTree();
}


void LR1::take_target_sequence(std::istream& in) {
    getline(in, this->target_sequence, '\x1A');
}


int main() {
    fstream oracal{"./wlmparse_table.lr1"};
    LR1 thisLR1(oracal);
    thisLR1.take_target_sequence(std::cin);

    thisLR1.read_rightmost();

}
