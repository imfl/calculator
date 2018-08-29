// 18/08/29 = Wed

#include <cmath>
#include <deque>
#include <functional>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <stack>
#include <string>
#include <utility>
#include <vector>

using namespace std;

constexpr double pi() { return std::atan(1) * 4; }

map<string, function<double(double, double)>> binops = {
    {"+", [](double a, double b){ return a + b; }},
    {"-", [](double a, double b){ return a - b; }},
    {"*", [](double a, double b){ return a * b; }},
    {"/", [](double a, double b){ return a / b; }},
    {"^", [](double a, double b){ return pow(a, b); }}
};

map<string, function<double(double)>> unaops = {
    {"pre_unary_minus",     [](double a){ return -a; }},
    {"pre_unary_sine",      [](double a){ return sin(a); }},
    {"pre_unary_cosine",    [](double a){ return cos(a); }},
    {"post_unary_degree",   [](double a){ return a * pi() / 180; }}
};

map<string, string> pre_unary_names = {
    {"-",   "pre_unary_minus"},
    {"sin", "pre_unary_sine"},
    {"cos", "pre_unary_cosine"}
};

map<string, string> post_unary_names = {
    {"d",   "post_unary_degree"}
};

vector<set<string>> precedence = {
    {"^", "post_unary_degree"},
    {"pre_unary_minus",
     "pre_unary_sine",
     "pre_unary_cosine"},
    {"*", "/"},
    {"+", "-"}
};

enum char_type { other, alpha, digit };

char_type char_type_of(char c)
{
    if(isdigit(c))
        return digit;
    if(isalpha(c))
        return alpha;
    return other;
}

string separate_alpha_digit(string s){
  string t = "";
  char_type last = other;
    for(auto c : s) {
        if ((last == alpha && char_type_of(c) == digit) ||
            (last == digit && char_type_of(c) == alpha))
            t.push_back(' ');
        t.push_back(c);
        last = char_type_of(c);
    }
    return t;
}

string separate_symbol(string s) {
    string t = "";
    for(auto c : s) {
        if (!isalnum(c) && c != '.') {
            t.push_back(' ');
            t.push_back(c);
            t.push_back(' ');
        }
        else
            t.push_back(c);
    }
    return t;
}


bool isnum(const string & s)
{
    return !s.empty() & s.find_first_not_of(".0123456789") == string::npos &
           (s.find('.') == string::npos ||
            s.find('.', s.find('.') + 1) == string::npos);
}

template<typename S, typename T, typename V>
bool isin(const T & t, const map<S, V> & m)
{
    return m.find(S(t)) != m.end();
}


double eval(stack<string> && expr);

double evaluate(string str)
{
    str = separate_alpha_digit(str);
    str = separate_symbol(str);
    str = "( " + str + " )";

    stack<string> expr;
    istringstream iss(str);
    string s;

    while (iss >> s) {
        if (s != ")")
            expr.push(s);
        else {
            stack<string> to_eval;
            while (expr.top() != "(") {
                to_eval.push(std::move(expr.top()));
                expr.pop();
            }
            expr.pop();

            double res = eval(std::move(to_eval));
            if (res >= 0)
                expr.push(to_string(res));
            else {
                expr.push("-");
                expr.push(to_string(-res));
            }
        }
    }
    return eval(std::move(expr));
}

double eval(stack<string> && expr)
{
    deque<double> num;
    deque<string> oper;
    string s;
    bool expect_number_or_pre_unary = true;
    bool met_unary = false;

    while (!expr.empty()) {
        s = expr.top();
        if (expect_number_or_pre_unary) {
            // met a number
            if (isnum(s)) {
                num.push_back(stod(s));
                met_unary = false;
            }
            // met a prefix unary
            else {
                oper.push_back(pre_unary_names[s]);
                met_unary = true;
            }
        }
        else {
            // met a postfix unary
            if (isin(s, post_unary_names)) {
                oper.push_back(post_unary_names[s]);
                met_unary = true;
            }
            // met a binary
            else {
                oper.push_back(s);
                met_unary = false;
            }
        }
        if (!met_unary)
            expect_number_or_pre_unary = !expect_number_or_pre_unary;
        expr.pop();
    }

    for (auto it = precedence.cbegin(); it != precedence.cend(); ++it) {
        deque<double> rest_num;
        deque<string> rest_oper;
        int i = 0;
        for (; !oper.empty(); oper.pop_front()) {
            string op = oper.front();
            // op is in current precedence level
            if (it->find(op) != it->end()) {
                // op is unary
                if (op.find("unary") != string::npos) {
                    double a = num.front();
                    num.pop_front();
                    num.push_front(unaops[op](a));
                }
                // op is binary
                else {
                    double a = num.front();
                    num.pop_front();
                    double b = num.front();
                    num.pop_front();
                    num.push_front(binops[op](a, b));
                }
            }
            // op not in current precedence level
            else {
                // save op to deal with it later
                rest_oper.push_back(op);
                // if op is not prefix unary:
                //      save number to deal with it later
                if (op.find("pre_unary") == string::npos) {
                    rest_num.push_back(num.front());
                    num.pop_front();
                }
            }
        }
        rest_num.push_back(num.front());
        num = rest_num;
        oper = rest_oper;
    }
    return num.front();
}

int main()
{
    string s;
    while (getline(cin, s)) {
        cout << s << " = " << evaluate(s) << endl;
    }
}