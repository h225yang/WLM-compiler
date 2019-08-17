#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <set>
#include <unordered_map>
#include <exception>
using namespace std;

set<string> non_terminal{
        "start",
        "params",
        "dcl",
        "dcls",
        "expr",
        "factor",
        "pcall",
        "procedures",
        "procedure",
        "statement",
        "statements",
        "term",
        "test"
};


struct ParseTree {
    string kind;
    string lhs;
    string rhs;
    int children_count = 0;
    vector<ParseTree> children;


    ParseTree(string kind, string lhs, string rhs, int count): kind{kind}, lhs{lhs}, rhs{rhs}, children_count{count} {}


    void printTree(string order) {
        if(order == "post") {
            for (auto tree : children) {
                tree.printTree(order);
            }
            cout << lhs << " " << rhs << endl;
        } else {
            cout << lhs << " " << rhs << endl;
            for (auto tree : children) {
                tree.printTree(order);
            }
        }
    }
};

ParseTree read_wlmi(istream& in) {
    string temp;
    getline(in, temp);
    stringstream ss{temp};

    string kind;
    string lhs;
    string rhs;
    int counter = 0;

    ss >> lhs;
    if(non_terminal.count(lhs)) { // non-terminal, root
        kind = "non-term";
        string derivation;
        while (ss >> derivation) {
            counter++;
            rhs += derivation + " ";
        }
        if(!rhs.empty()) {
            rhs.pop_back();
        }
    } else { // terminal, leaf
        kind = "terminal";
        ss >> rhs;
    }
    ParseTree root(kind, lhs, rhs, counter);
    for (int i = 0; i < counter; ++i) {
        root.children.push_back(read_wlmi(in));
    }
    return root;
}



struct Symbol_Table {
    string procedure_name;
    int num_of_var;
    unordered_map<string, int> var_table;

    // copy assignment
    Symbol_Table &operator=(const Symbol_Table &other) {
        procedure_name = other.procedure_name;
        num_of_var = other.num_of_var;
        for (auto it = other.var_table.begin(); it != other.var_table.end(); ++it) {
            var_table[it->first] = it->second;
        }
        return *this;
    }

    ///// test only ////
    void tablePrint() {
        cout << "Procedure name: " << procedure_name << endl;
        for (auto it = var_table.begin(); it != var_table.end(); ++it) {
            cout << it->first << ": " << it->second << endl;
        }
    }
};

void construct_table(ParseTree& tree, Symbol_Table& sym_table) {
    sym_table.var_table.clear();
    if(tree.lhs == "procedure") {
        sym_table.procedure_name = tree.children[1].rhs; // procudure name

        //INT ID LPAREN params RPAREN LBRACE dcls statements RETURN expr SEMI RBRACE

        sym_table.num_of_var = tree.children[3].children_count;
        if(sym_table.num_of_var == 3) sym_table.num_of_var = 2;

        if(sym_table.num_of_var == 0) return;
        if(sym_table.num_of_var >= 1) {
            string var1 = tree.children[3].children[0].children[1].rhs;
            if(sym_table.var_table.count(var1) == 1) {
                throw logic_error("ERROR: duplicate symbol: " + var1 + ", in function: " + sym_table.procedure_name);
            }
            sym_table.var_table[var1]= -4;
        }
        if(sym_table.num_of_var >= 2) {
            string var2 = tree.children[3].children[2].children[1].rhs;
            if(sym_table.var_table.count(var2) == 1) {
                throw logic_error("ERROR: duplicate symbol: " + var2 + ", in function: " + sym_table.procedure_name);
            }
            sym_table.var_table[var2]= -8;
        }

        return;
    }
    throw logic_error("ERROR: invalid tree");
}

struct Function_Table {
    unordered_map<string, int> fun_list; // function_name and # of args
    bool wain_def = false;

    ///// test only ////
    void tablePrint() {
        for (auto it = fun_list.begin(); it != fun_list.end(); ++it) {
            cout << it->first << ": " << it->second << endl;
        }
    }
};

void construct_fn_table(ParseTree &tree, Function_Table &fn_table, Symbol_Table &wain_table) {
    // procedures procedure procedures
    // procedure INT ID LPAREN params RPAREN LBRACE dcls statements RETURN expr SEMI RBRACE
    // must call on tree.lhs == procedures
    if(tree.rhs == "procedure procedures") {
        construct_fn_table(tree.children[1], fn_table, wain_table);
    } else {
        return;
    }

    string fn_name = tree.children[0].children[1].rhs;
    int fn_signature = tree.children[0].children[3].children_count;
    if(fn_signature == 3) fn_signature = 2;

    if(fn_table.fun_list.count(fn_name) == 1) throw logic_error("ERROR: duplicate function: " + fn_name);

    fn_table.fun_list[fn_name] = fn_signature;

    if(fn_name == "wain") {
        fn_table.wain_def = true;
        construct_table(tree.children[0], wain_table);
    }

}



void push(string reg){
    cout << "; pop "<< reg << " to stack" << endl;
    cout << "sw " << reg << ", -4($30)" << endl;
    cout << "sub $30, $30, $4" << endl;
}

void pop(string reg){
    cout << "; pop to "<< reg << " from stack" << endl;
    cout << "add $30, $30, $4" << endl;
    cout << "lw " << reg << ", -4($30)" << endl;
}

void getVar(string var, string target_reg, Symbol_Table table) {
    if(table.var_table.count(var) == 0) throw logic_error("ERROR: var not exist: " + var);
    cout << "; getVar " << var << endl;
    cout << "lw " << target_reg << ", " << table.var_table[var] << "($29)" << endl;
}



// code generation
// $1, $2: params
// $3 = retval
// $4 = 4
// $11 = 1
// $5, $6: temp
// $29: frame
// $30: stack
// $31: return address
void Code(ParseTree& tree, Symbol_Table& table, Function_Table& fn_table) {
    static int var_counter = 1;
    static int loop_counter = 0;
    static int cond_counter = 0;

    if(tree.lhs == "start") {
        Code(tree.children[1], table, fn_table);

    } else if(tree.lhs == "procedures") {
        if(tree.children_count == 0) return; // procedures -> epsilon
        Code(tree.children[0], table, fn_table);
        Code(tree.children[1], table, fn_table);

    } else if(tree.lhs == "procedure") { // INT ID LPAREN params RPAREN LBRACE dcls statements RETURN expr SEMI RBRACE

        static Symbol_Table temp;
        if(tree.children[1].rhs != "wain") {
            temp = table; // copy of wain_table
            static Symbol_Table new_table;
            try {
                construct_table(tree, new_table);
            } catch (logic_error &e) {
                cerr << e.what() << endl;
            }
            table.var_table.clear();
            table = new_table;
        }

        cout << "F" << table.procedure_name << ":" << endl; // assume wain ==> Fwain
        push("$31");

        push("$29");
        push("$5");

        cout << "add $29, $30, $0" << endl;
        push("$1");
        var_counter++;
        push("$2");
        var_counter++;

        Code(tree.children[3], table, fn_table); // params
        Code(tree.children[6], table, fn_table); // dcls
        Code(tree.children[7], table, fn_table); // statements
        Code(tree.children[9], table, fn_table); // expr

        cout << "add $30, $29, $0" << endl; // clear frame

        pop("$5");
        pop("$29");

        pop("$31");
        cout << "jr $31\n" << endl;

        if(tree.children[1].rhs != "wain") {
            table.var_table.clear();
            table = temp;
        }

    } else if(tree.lhs == "params") {
        if(tree.children_count == 0) return; //params
        Code(tree.children[0], table, fn_table); //params dcl
        if(tree.children_count == 3) Code(tree.children[2], table, fn_table); //params dcl COMMA dcl

    } else if(tree.lhs == "dcls") {
        if(tree.children_count == 0) return;
        if(tree.rhs == "dcls dcl BECOMES NUM SEMI") {
            Code(tree.children[0], table, fn_table);
            if(table.var_table.count(tree.children[1].children[1].rhs) == 1) {
                throw logic_error("ERROR: var dcl more than once: " + tree.children[1].children[1].rhs);
            }
            istringstream iss{tree.children[3].rhs};
            long long num;
            long long max = 2147483647ll;
            iss >> num;
            if (num > max || num < 0) throw logic_error("ERROR: number out of range" + tree.rhs);
            cout << "lis $3" << endl;
            cout << ".word " << num << endl;
            push("$3");
            table.var_table[tree.children[1].children[1].rhs] = -4 * var_counter;
            var_counter++;
        }

    } else if(tree.lhs == "dcl") { //INT ID
        return;

    } else if(tree.lhs == "statements") {
        if(tree.children_count == 0) return;
        Code(tree.children[0], table, fn_table); // statements statement
        Code(tree.children[1], table, fn_table);

    } else if(tree.lhs == "expr") {
        if(tree.children_count == 1) {
            Code(tree.children[0], table, fn_table);
        } else if(tree.rhs == "expr PLUS term") {
            Code(tree.children[0],table, fn_table);
            push("$3");
            Code(tree.children[2],table, fn_table);
            pop("$5");
            cout << "add $3, $5, $3" << endl;
        } else if(tree.rhs == "expr MINUS term") {
            Code(tree.children[0], table, fn_table);
            push("$3");
            Code(tree.children[2], table, fn_table);
            pop("$5");
            cout << "sub $3, $5, $3" << endl;
        }

    } else if(tree.lhs == "term") {
        if(tree.children_count == 1) {
            Code(tree.children[0], table, fn_table);
        } else if(tree.rhs == "term STAR factor") {
            Code(tree.children[0], table, fn_table);
            push("$3");
            Code(tree.children[2], table, fn_table);
            pop("$5");
            cout << "mult $3, $5" << endl;
            cout << "mflo $3" << endl;
        } else if(tree.rhs == "term SLASH factor") {
            Code(tree.children[0], table, fn_table);
            push("$3");
            Code(tree.children[2], table, fn_table);
            pop("$5");
            cout << "div $5, $3" << endl;
            cout << "mflo $3" << endl;
        } else if(tree.rhs == "term PCT factor") {
            Code(tree.children[0], table, fn_table);
            push("$3");
            Code(tree.children[2], table, fn_table);
            pop("$5");
            cout << "div $5, $3" << endl;
            cout << "mfhi $3" << endl;
        }

    } else if(tree.lhs == "factor") {
        if(tree.rhs == "NUM") {
            istringstream iss{tree.children[0].rhs};
            long long num;
            long long max = 2147483647ll;
            iss >> num;
            if (num > max || num < 0) throw logic_error("ERROR: number out of range" + tree.rhs);
            cout << "lis $3" << endl;
            cout << ".word " << tree.children[0].rhs << endl;
        } else if(tree.rhs == "ID") {
            getVar(tree.children[0].rhs, "$3", table);
        } else if(tree.rhs == "LPAREN expr RPAREN") {
            Code(tree.children[1], table, fn_table);
        } else if(tree.rhs == "pcall") {
            Code(tree.children[0], table, fn_table);
        }

    } else if(tree.lhs == "pcall") {
        if(tree.rhs == "ID LPAREN RPAREN") {
            if(fn_table.fun_list.count(tree.children[0].rhs) == 0) {
                throw logic_error("ERROR: function not defined " + tree.children[0].rhs);
            }
            if(fn_table.fun_list[tree.children[0].rhs] != 0) {
                throw logic_error("ERROR: function var not matched");
            }
            if(table.var_table.count(tree.children[0].rhs) == 1) {
                throw logic_error("ERROR: function is shadowed by variable: " + tree.children[0].rhs);
            }
            cout << "; pcall to " << tree.children[0].rhs << endl;
            cout << "lis $31" << endl;
            cout << ".word " << "F" << tree.children[0].rhs << endl;
            cout << "jalr $31" << endl;
        } else if(tree.rhs == "ID LPAREN expr RPAREN") {
            if(fn_table.fun_list.count(tree.children[0].rhs) == 0) {
                throw logic_error("ERROR: function not defined " + tree.children[0].rhs);
            }
            if(fn_table.fun_list[tree.children[0].rhs] != 1) {
                throw logic_error("ERROR: function var not matched");
            }
            if(table.var_table.count(tree.children[0].rhs) == 1) {
                throw logic_error("ERROR: function is shadowed by variable: " + tree.children[0].rhs);
            }
            cout << "; pcall to " << tree.children[0].rhs << endl;
            Code(tree.children[2], table, fn_table);
            cout << "add $1, $3, $0" << endl;
            cout << "lis $31" << endl;
            cout << ".word " << "F" << tree.children[0].rhs << endl;
            cout << "jalr $31" << endl;
        } else if(tree.rhs == "ID LPAREN expr COMMA expr RPAREN") {
            if(fn_table.fun_list.count(tree.children[0].rhs) == 0) {
                throw logic_error("ERROR: function not defined " + tree.children[0].rhs);
            }
            if(fn_table.fun_list[tree.children[0].rhs] != 2) {
                throw logic_error("ERROR: function var not matched");
            }
            if(table.var_table.count(tree.children[0].rhs) == 1) {
                throw logic_error("ERROR: function is shadowed by variable: " + tree.children[0].rhs);
            }
            cout << "; pcall to " << tree.children[0].rhs << endl;
            Code(tree.children[2], table, fn_table);
            cout << "add $1, $3, $0" << endl;
            Code(tree.children[4], table, fn_table);
            cout << "add $2, $3, $0" << endl;
            cout << "lis $31" << endl;
            cout << ".word " << "F" << tree.children[0].rhs << endl;
            cout << "jalr $31" << endl;

        }

    } else if(tree.lhs == "statement") {
        if(tree.rhs == "pcall SEMI") { //statement pcall SEMI
            Code(tree.children[0], table, fn_table);
        } else if(tree.rhs == "ID BECOMES expr SEMI") {
            Code(tree.children[2], table, fn_table);
            if(table.var_table.count(tree.children[0].rhs) == 0) {
                throw logic_error("ERROR: var not exist: " + tree.children[0].rhs);
            }
            cout << "sw $3, " << table.var_table[tree.children[0].rhs] << "($29)" << endl;
        } else if(tree.rhs == "WHILE LPAREN test RPAREN LBRACE statements RBRACE") {
            int curr_loop_counter = loop_counter;
            loop_counter++;
            cout << "loop" << curr_loop_counter << ":" << endl;
            Code(tree.children[2], table, fn_table);
            cout << "beq $3, $0, endloop" << curr_loop_counter << endl;
            Code(tree.children[5], table, fn_table);
            cout << "beq $0, $0, loop" << curr_loop_counter << endl;
            cout << "endloop" << curr_loop_counter << ":" << endl;
        } else if(tree.rhs == "IF LPAREN test RPAREN LBRACE statements RBRACE ELSE LBRACE statements RBRACE") {
            int curr_cond_counter = cond_counter;
            cond_counter++;
            Code(tree.children[2], table, fn_table);
            cout << "beq $3, $0, else" << curr_cond_counter << endl;
            Code(tree.children[5], table, fn_table);
            cout << "beq $0, $0, elseif" << curr_cond_counter << endl;
            cout << "else" << curr_cond_counter << ":" << endl;
            Code(tree.children[9], table, fn_table);
            cout << "elseif" << curr_cond_counter << ":" << endl;

        }

    } else if(tree.lhs == "test") {
        if(tree.rhs == "expr LT expr" ) {
            Code(tree.children[0], table, fn_table);
            push("$3");
            Code(tree.children[2], table, fn_table);
            pop("$5");
            cout << "slt $3, $5, $3" << endl;
        } else if(tree.rhs == "expr GE expr") {
            Code(tree.children[0], table, fn_table);
            push("$3");
            Code(tree.children[2], table, fn_table);
            pop("$5");
            cout << "slt $3, $5, $3" << endl;
            cout << "sub $3, $11, $3" << endl;
        } else if(tree.rhs == "expr EQ expr") {
            Code(tree.children[0], table, fn_table);
            push("$3");
            Code(tree.children[2], table, fn_table);
            pop("$5");
            // a-b = 0 <==> a = b
            cout << "sub $3, $3, $5" << endl;
            cout << "beq $3, $0, 1" << endl;
            cout << "add $3, $11, $0" << endl;
            cout << "sub $3, $11, $3" << endl;
        } else if(tree.rhs == "expr NE expr") {
            Code(tree.children[0], table, fn_table);
            push("$3");
            Code(tree.children[2], table, fn_table);
            pop("$5");
            cout << "sub $3, $3, $5" << endl;
            cout << "beq $3, $0, 1" << endl;
            cout << "add $3, $11, $0" << endl;
        } else if(tree.rhs == "expr GT expr") {
            Code(tree.children[0], table, fn_table);
            push("$3");
            Code(tree.children[2], table, fn_table);
            pop("$5");
            cout << "slt $3, $3, $5" << endl;
        } else if(tree.rhs == "expr LE expr") {
            Code(tree.children[0], table, fn_table);
            push("$3");
            Code(tree.children[2], table, fn_table);
            pop("$5");
            cout << "slt $3, $3, $5" << endl;
            cout << "sub $3, $11, $3" << endl;
        }
    }
}



int main() {
    ParseTree root = read_wlmi(cin);
    static Symbol_Table wain_table;
    static Function_Table fn_table;

    // prologue
    cout << "; Prologue" << endl;
    cout << "lis $11" << endl;
    cout << ".word 1" << endl;
    cout << "lis $4" << endl;
    cout << ".word 4" << endl;
    push("$31");
    cout << "lis $31" << endl;
    cout << ".word Fwain" << endl;
    cout << "jalr $31" << endl;
    pop("$31");
    cout << "jr $31\n" << endl;

    // scan and construct function table

    fn_table.fun_list["getchar"] = 0;
    fn_table.fun_list["putchar"] = 1;


    try {
        construct_fn_table(root.children[1], fn_table, wain_table);

        if(!fn_table.wain_def) throw logic_error("ERROR: wain not defined");

        Code(root, wain_table, fn_table);

    } catch (logic_error &e) {
        cerr << e.what() << endl;
    }

    // epilogue
    cout << "; Epilogue" << endl;

    // getchar
    cout << "Fgetchar:" << endl;
    cout << "lis $3" << endl;
    cout << ".word 0xffff0004" << endl;
    cout << "lw $3, 0($3)" << endl;
    cout << "jr $31" << endl;

    cout << endl;

    // putchar
    cout << "Fputchar:" << endl;
    push("$3");
    cout << "lis $3" << endl;
    cout << ".word 0xffff000c" << endl;
    cout << "sw $1, 0($3)" << endl;
    pop("$3");
    cout << "jr $31" << endl;

}

