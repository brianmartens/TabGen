#ifndef ALGEBRA_H_INCLUDED
#define ALGEBRA_H_INCLUDED
#include "bmath.h"

const bool is_numeric(const std::string& NumberString){
    if(NumberString.size() > 0){ // if value is not empty
        for(size_t i=0 ; i<NumberString.size() ; ++i){
            if(!isdigit(NumberString[i])){
                if(NumberString[i] == '.') continue;
                return false;
            }
        }
        return true;
    }else // we will interpret empty values as being non-numeric to prevent std::stod,std::stoi, etc.. from throwing exception
        return false;
}

bool is_operator(const char& string_char){
    return ((string_char == '*') || (string_char == '/') || (string_char == '-') || (string_char == '+') || (string_char == '^'));
}

double name_or_number(const std::string& formula, const std::vector<std::string>& vars, const std::vector<double>& vals){
    for(size_t i=0 ; i<vars.size() ; ++i){
        if(formula == vars[i])
            return vals[i];
    }
    if(is_numeric(formula))
        return std::atof(formula.c_str());
    return 0.0;
}

float name_or_number(const std::string& formula, const std::vector<std::string>& vars, const std::vector<float>& vals){
    for(size_t i=0 ; i<vars.size() ; ++i){
        if(formula == vars[i])
            return vals[i];
    }
    if(is_numeric(formula))
        return std::atof(formula.c_str());
    return 0.0;
}

size_t name_or_number(const std::string& formula, const std::vector<std::string>& vars, const std::vector<size_t>& vals){
    for(size_t i=0 ; i<vars.size() ; ++i){
        if(formula == vars[i])
            return vals[i];
    }
    if(is_numeric(formula))
        return std::atol(formula.c_str());
    return 0.0;
}

long name_or_number(const std::string& formula, const std::vector<std::string>& vars, const std::vector<long>& vals){
    for(size_t i=0 ; i<vars.size() ; ++i){
        if(formula == vars[i])
            return vals[i];
    }
    if(is_numeric(formula))
        return std::atol(formula.c_str());
    return 0.0;
}

double Algebra(const std::string& formula, const std::vector<std::string>& vars, const std::vector<double>& vals){
    double initial = 0.0;
    size_t opLevel1 = 1;
    size_t opLevel2 = 1;
    size_t plevel = 0;
    size_t i = 0;
    std::string RHS, LHS;
    char op1;
    if((formula.find('-') == std::string::npos) && (formula.find('+') == std::string::npos) && (formula.find('*') == std::string::npos)
        && (formula.find('/') == std::string::npos) && (formula.find('^') == std::string::npos)){
        return name_or_number(formula,vars,vals);
    }
    while(i<formula.size()){
        if(is_operator(formula[i])){
            op1 = formula[i];
            ++i;
            if(LHS.size()>0){
                initial = Algebra(LHS, vars, vals);
                LHS.clear();
            }
            if((op1 == '+') || (op1 == '-')){
                opLevel1 = 1;
            }else if((op1 == '*') || (op1 == '/')){
                opLevel1 = 2;
            }else if(op1 == '^'){
                opLevel1 = 3;
            }
            while(i < formula.size()){
                if(is_operator(formula[i])){
                    if((formula[i] == '-') || (formula[i] == '+')){
                        opLevel2 = 1;
                    }else if((formula[i] == '*') || (formula[i] == '/')){
                        opLevel2 = 2;
                    }else if(formula[i] == '^'){
                        opLevel2 = 3;
                    }
                    if(opLevel1 >= opLevel2){
                        if(op1 == '+'){
                            initial += Algebra(RHS, vars, vals);
                        }else if(op1 == '-'){
                            initial -= Algebra(RHS, vars, vals);
                        }else if(op1 == '*'){
                            initial *= Algebra(RHS, vars, vals);
                        }else if(op1 == '/'){
                            double temp = Algebra(RHS, vars, vals);
                            if(temp != 0.0)
                                initial /= temp;
                            else
                                return 0.0;
                        }else if(op1 == '^'){
                            initial = pow(initial, Algebra(RHS, vars, vals));
                        }
                        RHS.clear();
                        break;
                    }else{
                        RHS += formula[i];
                        ++i;
                    }
                }else if(formula[i] == '('){
                    ++plevel;
                    while(plevel > 0){
                        RHS += formula[i];
                        ++i;
                        if(formula[i] == ')')
                            --plevel;
                        if((i >= formula.size() - 1) && (plevel > 0))
                            return 0.0;
                    }
                }else if(formula[i] == '"'){
                    ++i;
                    while(formula[i] != '"'){
                        RHS += formula[i];
                        ++i;
                    }
                }else{
                    RHS += formula[i];
                    ++i;
                }
            }
        }else if(formula[i] == '('){
            ++plevel;
            ++i;
            while(plevel > 0){
                if(formula[i] == ')')
                    --plevel;
                if((i >= formula.size() - 1) && (plevel > 0))
                    return 0.0;
                if(plevel > 0){
                    LHS += formula[i];
                    ++i;
                }else{
                    ++i;
                    break;
                }
            }
        }else if(formula[i] == '"'){
            ++i;
            while(formula[i] != '"'){
                LHS += formula[i];
                ++i;
            }
        }else{
            LHS += formula[i];
            ++i;
        }
    }
    if(LHS.size() > 0){
        return Algebra(LHS, vars, vals);
    }
    if(RHS.size() > 0){
        if(plevel > 0) return 0.0;
        if(op1 == '+'){
            return initial += Algebra(RHS, vars, vals);
        }else if(op1 == '-'){
            return initial -= Algebra(RHS, vars, vals);
        }else if(op1 == '*'){
            return initial *= Algebra(RHS, vars, vals);
        }else if(op1 == '/'){
            double temp = Algebra(RHS, vars, vals);
            if(temp != 0)
                return initial / temp;
            else
                return 0.0;
        }else if(op1 == '^'){
            return pow(initial, Algebra(RHS, vars, vals));
        }
    }
    return 0.0;
}

float Algebra(const std::string& formula, const std::vector<std::string>& vars, const std::vector<float>& vals){
    float initial = 0.0;
    size_t opLevel1 = 1;
    size_t opLevel2 = 1;
    size_t plevel = 0;
    size_t i = 0;
    std::string RHS, LHS;
    char op1;
    if((formula.find('-') == std::string::npos) && (formula.find('+') == std::string::npos) && (formula.find('*') == std::string::npos)
        && (formula.find('/') == std::string::npos) && (formula.find('^') == std::string::npos)){
        return name_or_number(formula,vars,vals);
    }
    while(i<formula.size()){
        if(is_operator(formula[i])){
            op1 = formula[i];
            ++i;
            if(LHS.size()>0){
                initial = Algebra(LHS, vars, vals);
                LHS.clear();
            }
            if((op1 == '+') || (op1 == '-')){
                opLevel1 = 1;
            }else if((op1 == '*') || (op1 == '/')){
                opLevel1 = 2;
            }else if(op1 == '^'){
                opLevel1 = 3;
            }
            while(i < formula.size()){
                if(is_operator(formula[i])){
                    if((formula[i] == '-') || (formula[i] == '+')){
                        opLevel2 = 1;
                    }else if((formula[i] == '*') || (formula[i] == '/')){
                        opLevel2 = 2;
                    }else if(formula[i] == '^'){
                        opLevel2 = 3;
                    }
                    if(opLevel1 >= opLevel2){
                        if(op1 == '+'){
                            initial += Algebra(RHS, vars, vals);
                        }else if(op1 == '-'){
                            initial -= Algebra(RHS, vars, vals);
                        }else if(op1 == '*'){
                            initial *= Algebra(RHS, vars, vals);
                        }else if(op1 == '/'){
                            float temp = Algebra(RHS, vars, vals);
                            if(temp != 0.0)
                                initial /= temp;
                            else
                                return 0.0;
                        }else if(op1 == '^'){
                            initial = pow(initial, Algebra(RHS, vars, vals));
                        }
                        RHS.clear();
                        break;
                    }else{
                        RHS += formula[i];
                        ++i;
                    }
                }else if(formula[i] == '('){
                    ++plevel;
                    while(plevel > 0){
                        RHS += formula[i];
                        ++i;
                        if(formula[i] == ')')
                            --plevel;
                        if((i >= formula.size() - 1) && (plevel > 0))
                            return 0.0;
                    }
                }else if(formula[i] == '"'){
                    ++i;
                    while(formula[i] != '"'){
                        LHS += formula[i];
                        ++i;
                    }
                }else{
                    RHS += formula[i];
                    ++i;
                }
            }
        }else if(formula[i] == '('){
            ++plevel;
            ++i;
            while(plevel > 0){
                if(formula[i] == ')')
                    --plevel;
                if((i >= formula.size() - 1) && (plevel > 0))
                    return 0.0;
                if(plevel > 0){
                    LHS += formula[i];
                    ++i;
                }else{
                    ++i;
                    break;
                }
            }
        }else if(formula[i] == '"'){
            ++i;
            while(formula[i] != '"'){
                LHS += formula[i];
                ++i;
            }
        }else{
            LHS += formula[i];
            ++i;
        }
    }
    if(LHS.size() > 0){
        return Algebra(LHS, vars, vals);
    }
    if(RHS.size() > 0){
        if(plevel > 0) return 0.0;
        if(op1 == '+'){
            return initial += Algebra(RHS, vars, vals);
        }else if(op1 == '-'){
            return initial -= Algebra(RHS, vars, vals);
        }else if(op1 == '*'){
            return initial *= Algebra(RHS, vars, vals);
        }else if(op1 == '/'){
            float temp = Algebra(RHS, vars, vals);
            if(temp != 0)
                return initial / temp;
            else
                return 0.0;
        }else if(op1 == '^'){
            return pow(initial, Algebra(RHS, vars, vals));
        }
    }
    return 0.0;
}

size_t Algebra(const std::string& formula, const std::vector<std::string>& vars, const std::vector<size_t>& vals){
    size_t initial = 0;
    size_t opLevel1 = 1;
    size_t opLevel2 = 1;
    size_t plevel = 0;
    size_t i = 0;
    std::string RHS, LHS;
    char op1;
    if((formula.find('-') == std::string::npos) && (formula.find('+') == std::string::npos) && (formula.find('*') == std::string::npos)
        && (formula.find('/') == std::string::npos) && (formula.find('^') == std::string::npos)){
        return name_or_number(formula,vars,vals);
    }
    while(i<formula.size()){
        if(is_operator(formula[i])){
            op1 = formula[i];
            ++i;
            if(LHS.size()>0){
                initial = Algebra(LHS, vars, vals);
                LHS.clear();
            }
            if((op1 == '+') || (op1 == '-')){
                opLevel1 = 1;
            }else if((op1 == '*') || (op1 == '/')){
                opLevel1 = 2;
            }else if(op1 == '^'){
                opLevel1 = 3;
            }
            while(i < formula.size()){
                if(is_operator(formula[i])){
                    if((formula[i] == '-') || (formula[i] == '+')){
                        opLevel2 = 1;
                    }else if((formula[i] == '*') || (formula[i] == '/')){
                        opLevel2 = 2;
                    }else if(formula[i] == '^'){
                        opLevel2 = 3;
                    }
                    if(opLevel1 >= opLevel2){
                        if(op1 == '+'){
                            initial += Algebra(RHS, vars, vals);
                        }else if(op1 == '-'){
                            initial -= Algebra(RHS, vars, vals);
                        }else if(op1 == '*'){
                            initial *= Algebra(RHS, vars, vals);
                        }else if(op1 == '/'){
                            size_t temp = Algebra(RHS, vars, vals);
                            if(temp != 0)
                                initial /= temp;
                            else
                                return 0;
                        }else if(op1 == '^'){
                            initial = pow(initial, Algebra(RHS, vars, vals));
                        }
                        RHS.clear();
                        break;
                    }else{
                        RHS += formula[i];
                        ++i;
                    }
                }else if(formula[i] == '('){
                    ++plevel;
                    while(plevel > 0){
                        RHS += formula[i];
                        ++i;
                        if(formula[i] == ')')
                            --plevel;
                        if((i >= formula.size() - 1) && (plevel > 0))
                            return 0;
                    }
                }else if(formula[i] == '"'){
                    ++i;
                    while(formula[i] != '"'){
                        LHS += formula[i];
                        ++i;
                    }
                }else{
                    RHS += formula[i];
                    ++i;
                }
            }
        }else if(formula[i] == '('){
            ++plevel;
            ++i;
            while(plevel > 0){
                if(formula[i] == ')')
                    --plevel;
                if((i >= formula.size() - 1) && (plevel > 0))
                    return 0;
                if(plevel > 0){
                    LHS += formula[i];
                    ++i;
                }else{
                    ++i;
                    break;
                }
            }
        }else if(formula[i] == '"'){
            ++i;
            while(formula[i] != '"'){
                LHS += formula[i];
                ++i;
            }
        }else{
            LHS += formula[i];
            ++i;
        }
    }
    if(LHS.size() > 0){
        return Algebra(LHS, vars, vals);
    }
    if(RHS.size() > 0){
        if(plevel > 0) return 0;
        if(op1 == '+'){
            return initial += Algebra(RHS, vars, vals);
        }else if(op1 == '-'){
            return initial -= Algebra(RHS, vars, vals);
        }else if(op1 == '*'){
            return initial *= Algebra(RHS, vars, vals);
        }else if(op1 == '/'){
            size_t temp = Algebra(RHS, vars, vals);
            if(temp != 0)
                return initial / temp;
            else
                return 0;
        }else if(op1 == '^'){
            return pow(initial, Algebra(RHS, vars, vals));
        }
    }
    return 0;
}

long Algebra(const std::string& formula, const std::vector<std::string>& vars, const std::vector<long>& vals){
    long initial = 0;
    size_t opLevel1 = 1;
    size_t opLevel2 = 1;
    size_t plevel = 0;
    size_t i = 0;
    std::string RHS, LHS;
    char op1;
    if((formula.find('-') == std::string::npos) && (formula.find('+') == std::string::npos) && (formula.find('*') == std::string::npos)
        && (formula.find('/') == std::string::npos) && (formula.find('^') == std::string::npos)){
        return name_or_number(formula,vars,vals);
    }
    while(i<formula.size()){
        if(is_operator(formula[i])){
            op1 = formula[i];
            ++i;
            if(LHS.size()>0){
                initial = Algebra(LHS, vars, vals);
                LHS.clear();
            }
            if((op1 == '+') || (op1 == '-')){
                opLevel1 = 1;
            }else if((op1 == '*') || (op1 == '/')){
                opLevel1 = 2;
            }else if(op1 == '^'){
                opLevel1 = 3;
            }
            while(i < formula.size()){
                if(is_operator(formula[i])){
                    if((formula[i] == '-') || (formula[i] == '+')){
                        opLevel2 = 1;
                    }else if((formula[i] == '*') || (formula[i] == '/')){
                        opLevel2 = 2;
                    }else if(formula[i] == '^'){
                        opLevel2 = 3;
                    }
                    if(opLevel1 >= opLevel2){
                        if(op1 == '+'){
                            initial += Algebra(RHS, vars, vals);
                        }else if(op1 == '-'){
                            initial -= Algebra(RHS, vars, vals);
                        }else if(op1 == '*'){
                            initial *= Algebra(RHS, vars, vals);
                        }else if(op1 == '/'){
                            long temp = Algebra(RHS, vars, vals);
                            if(temp != 0)
                                initial /= temp;
                            else
                                return 0;
                        }else if(op1 == '^'){
                            initial = pow(initial, Algebra(RHS, vars, vals));
                        }
                        RHS.clear();
                        break;
                    }else{
                        RHS += formula[i];
                        ++i;
                    }
                }else if(formula[i] == '('){
                    ++plevel;
                    while(plevel > 0){
                        RHS += formula[i];
                        ++i;
                        if(formula[i] == ')')
                            --plevel;
                        if((i >= formula.size() - 1) && (plevel > 0))
                            return 0;
                    }
                }else if(formula[i] == '"'){
                    ++i;
                    while(formula[i] != '"'){
                        LHS += formula[i];
                        ++i;
                    }
                }else{
                    RHS += formula[i];
                    ++i;
                }
            }
        }else if(formula[i] == '('){
            ++plevel;
            ++i;
            while(plevel > 0){
                if(formula[i] == ')')
                    --plevel;
                if((i >= formula.size() - 1) && (plevel > 0))
                    return 0;
                if(plevel > 0){
                    LHS += formula[i];
                    ++i;
                }else{
                    ++i;
                    break;
                }
            }
        }else if(formula[i] == '"'){
            ++i;
            while(formula[i] != '"'){
                LHS += formula[i];
                ++i;
            }
        }else{
            LHS += formula[i];
            ++i;
        }
    }
    if(LHS.size() > 0){
        return Algebra(LHS, vars, vals);
    }
    if(RHS.size() > 0){
        if(plevel > 0) return 0;
        if(op1 == '+'){
            return initial += Algebra(RHS, vars, vals);
        }else if(op1 == '-'){
            return initial -= Algebra(RHS, vars, vals);
        }else if(op1 == '*'){
            return initial *= Algebra(RHS, vars, vals);
        }else if(op1 == '/'){
            long temp = Algebra(RHS, vars, vals);
            if(temp != 0)
                return initial / temp;
            else
                return 0;
        }else if(op1 == '^'){
            return pow(initial, Algebra(RHS, vars, vals));
        }
    }
    return 0;
}

double Ename_or_number(const std::string& formula, const std::vector<std::string>& vars, const std::vector<std::vector<double> >& vals, const size_t& valindex){
    for(size_t i=0 ; i<vars.size() ; ++i){
        if(formula == vars[i])
            return vals[i][valindex];
    }
    if(is_numeric(formula))
        return std::atof(formula.c_str());
    return 0.0;
}

float Ename_or_number(const std::string& formula, const std::vector<std::string>& vars, const std::vector<std::vector<float> >& vals, const size_t& valindex){
    for(size_t i=0 ; i<vars.size() ; ++i){
        if(formula == vars[i])
            return vals[i][valindex];
    }
    if(is_numeric(formula))
        return std::atof(formula.c_str());
    return 0.0;
}

size_t Ename_or_number(const std::string& formula, const std::vector<std::string>& vars, const std::vector<std::vector<size_t> >& vals, const size_t& valindex){
    for(size_t i=0 ; i<vars.size() ; ++i){
        if(formula == vars[i])
            return vals[i][valindex];
    }
    if(is_numeric(formula))
        return std::atol(formula.c_str());
    return 0;
}

long Ename_or_number(const std::string& formula, const std::vector<std::string>& vars, const std::vector<std::vector<long> >& vals, const size_t& valindex){
    for(size_t i=0 ; i<vars.size() ; ++i){
        if(formula == vars[i])
            return vals[i][valindex];
    }
    if(is_numeric(formula))
        return std::atol(formula.c_str());
    return 0;
}

double C_Algebra(const std::string& formula, const std::vector<std::string>& vars, const std::vector<std::vector<double> >& vals, const size_t& valindex){
    double initial = 0;
    size_t opLevel1 = 1;
    size_t opLevel2 = 1;
    size_t plevel = 0;
    size_t i = 0;
    std::string RHS, LHS;
    char op1;
    if((formula.find('-') == std::string::npos) && (formula.find('+') == std::string::npos) && (formula.find('*') == std::string::npos)
        && (formula.find('/') == std::string::npos) && (formula.find('^') == std::string::npos)){
        return Ename_or_number(formula, vars, vals, valindex);
    }
    while(i<formula.size()){
        if(is_operator(formula[i])){
            op1 = formula[i];
            ++i;
            if(LHS.size()>0){
                initial = C_Algebra(LHS, vars, vals, valindex);
                LHS.clear();
            }
            if((op1 == '+') || (op1 == '-')){
                opLevel1 = 1;
            }else if((op1 == '*') || (op1 == '/')){
                opLevel1 = 2;
            }else if(op1 == '^'){
                opLevel1 = 3;
            }
            while(i < formula.size()){
                if(is_operator(formula[i])){
                    if((formula[i] == '-') || (formula[i] == '+')){
                        opLevel2 = 1;
                    }else if((formula[i] == '*') || (formula[i] == '/')){
                        opLevel2 = 2;
                    }else if(formula[i] == '^'){
                        opLevel2 = 3;
                    }
                    if(opLevel1 >= opLevel2){
                        if(op1 == '+'){
                            initial += C_Algebra(LHS, vars, vals, valindex);
                        }else if(op1 == '-'){
                            initial -= C_Algebra(LHS, vars, vals, valindex);
                        }else if(op1 == '*'){
                            initial *=C_Algebra(LHS, vars, vals, valindex);
                        }else if(op1 == '/'){
                            double temp = C_Algebra(LHS, vars, vals, valindex);
                            if(temp != 0)
                                initial /= temp;
                            else
                                return 0;
                        }else if(op1 == '^'){
                            initial = pow(initial, C_Algebra(LHS, vars, vals, valindex));
                        }
                        RHS.clear();
                        break;
                    }else{
                        RHS += formula[i];
                        ++i;
                    }
                }else if(formula[i] == '('){
                    ++plevel;
                    while(plevel > 0){
                        RHS += formula[i];
                        ++i;
                        if(formula[i] == ')')
                            --plevel;
                        if((i >= formula.size() - 1) && (plevel > 0))
                            return 0;
                    }
                }else if(formula[i] == '"'){
                    ++i;
                    while(formula[i] != '"'){
                        LHS += formula[i];
                        ++i;
                    }
                }else{
                    RHS += formula[i];
                    ++i;
                }
            }
        }else if(formula[i] == '('){
            ++plevel;
            ++i;
            while(plevel > 0){
                if(formula[i] == ')')
                    --plevel;
                if((i >= formula.size() - 1) && (plevel > 0))
                    return 0;
                if(plevel > 0){
                    LHS += formula[i];
                    ++i;
                }else{
                    ++i;
                    break;
                }
            }
        }else if(formula[i] == '"'){
            ++i;
            while(formula[i] != '"'){
                LHS += formula[i];
                ++i;
            }
        }else{
            LHS += formula[i];
            ++i;
        }
    }
    if(LHS.size() > 0){
        return C_Algebra(LHS, vars, vals, valindex);
    }
    if(RHS.size() > 0){
        if(plevel > 0) return 0;
        if(op1 == '+'){
            return initial += C_Algebra(LHS, vars, vals, valindex);
        }else if(op1 == '-'){
            return initial -= C_Algebra(LHS, vars, vals, valindex);
        }else if(op1 == '*'){
            return initial *= C_Algebra(LHS, vars, vals, valindex);
        }else if(op1 == '/'){
            double temp = C_Algebra(LHS, vars, vals, valindex);
            if(temp != 0)
                return initial / temp;
            else
                return 0;
        }else if(op1 == '^'){
            return pow(initial, C_Algebra(LHS, vars, vals, valindex));
        }
    }
    return 0;
}

float C_Algebra(const std::string& formula, const std::vector<std::string>& vars, const std::vector<std::vector<float> >& vals, const size_t& valindex){
    float initial = 0;
    size_t opLevel1 = 1;
    size_t opLevel2 = 1;
    size_t plevel = 0;
    size_t i = 0;
    std::string RHS, LHS;
    char op1;
    if((formula.find('-') == std::string::npos) && (formula.find('+') == std::string::npos) && (formula.find('*') == std::string::npos)
        && (formula.find('/') == std::string::npos) && (formula.find('^') == std::string::npos)){
        return Ename_or_number(formula, vars, vals, valindex);
    }
    while(i<formula.size()){
        if(is_operator(formula[i])){
            op1 = formula[i];
            ++i;
            if(LHS.size()>0){
                initial = C_Algebra(LHS, vars, vals, valindex);
                LHS.clear();
            }
            if((op1 == '+') || (op1 == '-')){
                opLevel1 = 1;
            }else if((op1 == '*') || (op1 == '/')){
                opLevel1 = 2;
            }else if(op1 == '^'){
                opLevel1 = 3;
            }
            while(i < formula.size()){
                if(is_operator(formula[i])){
                    if((formula[i] == '-') || (formula[i] == '+')){
                        opLevel2 = 1;
                    }else if((formula[i] == '*') || (formula[i] == '/')){
                        opLevel2 = 2;
                    }else if(formula[i] == '^'){
                        opLevel2 = 3;
                    }
                    if(opLevel1 >= opLevel2){
                        if(op1 == '+'){
                            initial += C_Algebra(LHS, vars, vals, valindex);
                        }else if(op1 == '-'){
                            initial -= C_Algebra(LHS, vars, vals, valindex);
                        }else if(op1 == '*'){
                            initial *=C_Algebra(LHS, vars, vals, valindex);
                        }else if(op1 == '/'){
                            float temp = C_Algebra(LHS, vars, vals, valindex);
                            if(temp != 0)
                                initial /= temp;
                            else
                                return 0;
                        }else if(op1 == '^'){
                            initial = pow(initial, C_Algebra(LHS, vars, vals, valindex));
                        }
                        RHS.clear();
                        break;
                    }else{
                        RHS += formula[i];
                        ++i;
                    }
                }else if(formula[i] == '('){
                    ++plevel;
                    while(plevel > 0){
                        RHS += formula[i];
                        ++i;
                        if(formula[i] == ')')
                            --plevel;
                        if((i >= formula.size() - 1) && (plevel > 0))
                            return 0;
                    }
                }else if(formula[i] == '"'){
                    ++i;
                    while(formula[i] != '"'){
                        LHS += formula[i];
                        ++i;
                    }
                }else{
                    RHS += formula[i];
                    ++i;
                }
            }
        }else if(formula[i] == '('){
            ++plevel;
            ++i;
            while(plevel > 0){
                if(formula[i] == ')')
                    --plevel;
                if((i >= formula.size() - 1) && (plevel > 0))
                    return 0;
                if(plevel > 0){
                    LHS += formula[i];
                    ++i;
                }else{
                    ++i;
                    break;
                }
            }
        }else if(formula[i] == '"'){
            ++i;
            while(formula[i] != '"'){
                LHS += formula[i];
                ++i;
            }
        }else{
            LHS += formula[i];
            ++i;
        }
    }
    if(LHS.size() > 0){
        return C_Algebra(LHS, vars, vals, valindex);
    }
    if(RHS.size() > 0){
        if(plevel > 0) return 0;
        if(op1 == '+'){
            return initial += C_Algebra(LHS, vars, vals, valindex);
        }else if(op1 == '-'){
            return initial -= C_Algebra(LHS, vars, vals, valindex);
        }else if(op1 == '*'){
            return initial *= C_Algebra(LHS, vars, vals, valindex);
        }else if(op1 == '/'){
            float temp = C_Algebra(LHS, vars, vals, valindex);
            if(temp != 0)
                return initial / temp;
            else
                return 0;
        }else if(op1 == '^'){
            return pow(initial, C_Algebra(LHS, vars, vals, valindex));
        }
    }
    return 0;
}

size_t C_Algebra(const std::string& formula, const std::vector<std::string>& vars, const std::vector<std::vector<size_t> >& vals, const size_t& valindex){
    size_t initial = 0;
    size_t opLevel1 = 1;
    size_t opLevel2 = 1;
    size_t plevel = 0;
    size_t i = 0;
    std::string RHS, LHS;
    char op1;
    if((formula.find('-') == std::string::npos) && (formula.find('+') == std::string::npos) && (formula.find('*') == std::string::npos)
        && (formula.find('/') == std::string::npos) && (formula.find('^') == std::string::npos)){
        return Ename_or_number(formula, vars, vals, valindex);
    }
    while(i<formula.size()){
        if(is_operator(formula[i])){
            op1 = formula[i];
            ++i;
            if(LHS.size()>0){
                initial = C_Algebra(LHS, vars, vals, valindex);
                LHS.clear();
            }
            if((op1 == '+') || (op1 == '-')){
                opLevel1 = 1;
            }else if((op1 == '*') || (op1 == '/')){
                opLevel1 = 2;
            }else if(op1 == '^'){
                opLevel1 = 3;
            }
            while(i < formula.size()){
                if(is_operator(formula[i])){
                    if((formula[i] == '-') || (formula[i] == '+')){
                        opLevel2 = 1;
                    }else if((formula[i] == '*') || (formula[i] == '/')){
                        opLevel2 = 2;
                    }else if(formula[i] == '^'){
                        opLevel2 = 3;
                    }
                    if(opLevel1 >= opLevel2){
                        if(op1 == '+'){
                            initial += C_Algebra(LHS, vars, vals, valindex);
                        }else if(op1 == '-'){
                            initial -= C_Algebra(LHS, vars, vals, valindex);
                        }else if(op1 == '*'){
                            initial *=C_Algebra(LHS, vars, vals, valindex);
                        }else if(op1 == '/'){
                            size_t temp = C_Algebra(LHS, vars, vals, valindex);
                            if(temp != 0)
                                initial /= temp;
                            else
                                return 0;
                        }else if(op1 == '^'){
                            initial = pow(initial, C_Algebra(LHS, vars, vals, valindex));
                        }
                        RHS.clear();
                        break;
                    }else{
                        RHS += formula[i];
                        ++i;
                    }
                }else if(formula[i] == '('){
                    ++plevel;
                    while(plevel > 0){
                        RHS += formula[i];
                        ++i;
                        if(formula[i] == ')')
                            --plevel;
                        if((i >= formula.size() - 1) && (plevel > 0))
                            return 0;
                    }
                }else if(formula[i] == '"'){
                    ++i;
                    while(formula[i] != '"'){
                        LHS += formula[i];
                        ++i;
                    }
                }else{
                    RHS += formula[i];
                    ++i;
                }
            }
        }else if(formula[i] == '('){
            ++plevel;
            ++i;
            while(plevel > 0){
                if(formula[i] == ')')
                    --plevel;
                if((i >= formula.size() - 1) && (plevel > 0))
                    return 0;
                if(plevel > 0){
                    LHS += formula[i];
                    ++i;
                }else{
                    ++i;
                    break;
                }
            }
        }else if(formula[i] == '"'){
            ++i;
            while(formula[i] != '"'){
                LHS += formula[i];
                ++i;
            }
        }else{
            LHS += formula[i];
            ++i;
        }
    }
    if(LHS.size() > 0){
        return C_Algebra(LHS, vars, vals, valindex);
    }
    if(RHS.size() > 0){
        if(plevel > 0) return 0;
        if(op1 == '+'){
            return initial += C_Algebra(LHS, vars, vals, valindex);
        }else if(op1 == '-'){
            return initial -= C_Algebra(LHS, vars, vals, valindex);
        }else if(op1 == '*'){
            return initial *= C_Algebra(LHS, vars, vals, valindex);
        }else if(op1 == '/'){
            size_t temp = C_Algebra(LHS, vars, vals, valindex);
            if(temp != 0)
                return initial / temp;
            else
                return 0;
        }else if(op1 == '^'){
            return pow(initial, C_Algebra(LHS, vars, vals, valindex));
        }
    }
    return 0;
}

long C_Algebra(const std::string& formula, const std::vector<std::string>& vars, const std::vector<std::vector<long> >& vals, const size_t& valindex){
    long initial = 0;
    size_t opLevel1 = 1;
    size_t opLevel2 = 1;
    size_t plevel = 0;
    size_t i = 0;
    std::string RHS, LHS;
    char op1;
    if((formula.find('-') == std::string::npos) && (formula.find('+') == std::string::npos) && (formula.find('*') == std::string::npos)
        && (formula.find('/') == std::string::npos) && (formula.find('^') == std::string::npos)){
        return Ename_or_number(formula, vars, vals, valindex);
    }
    while(i<formula.size()){
        if(is_operator(formula[i])){
            op1 = formula[i];
            ++i;
            if(LHS.size()>0){
                initial = C_Algebra(LHS, vars, vals, valindex);
                LHS.clear();
            }
            if((op1 == '+') || (op1 == '-')){
                opLevel1 = 1;
            }else if((op1 == '*') || (op1 == '/')){
                opLevel1 = 2;
            }else if(op1 == '^'){
                opLevel1 = 3;
            }
            while(i < formula.size()){
                if(is_operator(formula[i])){
                    if((formula[i] == '-') || (formula[i] == '+')){
                        opLevel2 = 1;
                    }else if((formula[i] == '*') || (formula[i] == '/')){
                        opLevel2 = 2;
                    }else if(formula[i] == '^'){
                        opLevel2 = 3;
                    }
                    if(opLevel1 >= opLevel2){
                        if(op1 == '+'){
                            initial += C_Algebra(LHS, vars, vals, valindex);
                        }else if(op1 == '-'){
                            initial -= C_Algebra(LHS, vars, vals, valindex);
                        }else if(op1 == '*'){
                            initial *=C_Algebra(LHS, vars, vals, valindex);
                        }else if(op1 == '/'){
                            long temp = C_Algebra(LHS, vars, vals, valindex);
                            if(temp != 0)
                                initial /= temp;
                            else
                                return 0;
                        }else if(op1 == '^'){
                            initial = pow(initial, C_Algebra(LHS, vars, vals, valindex));
                        }
                        RHS.clear();
                        break;
                    }else{
                        RHS += formula[i];
                        ++i;
                    }
                }else if(formula[i] == '('){
                    ++plevel;
                    while(plevel > 0){
                        RHS += formula[i];
                        ++i;
                        if(formula[i] == ')')
                            --plevel;
                        if((i >= formula.size() - 1) && (plevel > 0))
                            return 0;
                    }
                }else if(formula[i] == '"'){
                    ++i;
                    while(formula[i] != '"'){
                        LHS += formula[i];
                        ++i;
                    }
                }else{
                    RHS += formula[i];
                    ++i;
                }
            }
        }else if(formula[i] == '('){
            ++plevel;
            ++i;
            while(plevel > 0){
                if(formula[i] == ')')
                    --plevel;
                if((i >= formula.size() - 1) && (plevel > 0))
                    return 0;
                if(plevel > 0){
                    LHS += formula[i];
                    ++i;
                }else{
                    ++i;
                    break;
                }
            }
        }else if(formula[i] == '"'){
            ++i;
            while(formula[i] != '"'){
                LHS += formula[i];
                ++i;
            }
        }else{
            LHS += formula[i];
            ++i;
        }
    }
    if(LHS.size() > 0){
        return C_Algebra(LHS, vars, vals, valindex);
    }
    if(RHS.size() > 0){
        if(plevel > 0) return 0;
        if(op1 == '+'){
            return initial += C_Algebra(LHS, vars, vals, valindex);
        }else if(op1 == '-'){
            return initial -= C_Algebra(LHS, vars, vals, valindex);
        }else if(op1 == '*'){
            return initial *= C_Algebra(LHS, vars, vals, valindex);
        }else if(op1 == '/'){
            long temp = C_Algebra(LHS, vars, vals, valindex);
            if(temp != 0)
                return initial / temp;
            else
                return 0;
        }else if(op1 == '^'){
            return pow(initial, C_Algebra(LHS, vars, vals, valindex));
        }
    }
    return 0;
}
#endif // ALGEBRA_H_INCLUDED
