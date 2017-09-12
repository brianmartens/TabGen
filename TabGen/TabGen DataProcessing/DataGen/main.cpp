#include <iostream>
#include <vector>
#include <functional>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <random>
#include <chrono>

  int sum(std::vector<int> input, size_t start, size_t stop)
  {
      int summation = 0;
      for(size_t i=start ; i<=stop ; ++i)
          summation += input.at(i);
      return summation;
  }

int main()
{
    // random number engine, seed, and distributions
    auto seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    std::mt19937 mt_rand(seed);
    std::uniform_real_distribution<double> _0_to_1(0.0,1.0);
    std::uniform_int_distribution<int> _0_to_10(0,10);
    std::uniform_int_distribution<int> _1_to_2(1,2);
    std::uniform_int_distribution<int> _0_or_1(0,1);
    std::uniform_int_distribution<int> _1_to_3(1,3);
    std::uniform_int_distribution<int> _1_to_5(1,5);
    std::uniform_int_distribution<int> _1_to_7(1,7);
    std::uniform_int_distribution<int> _1_to_6(1,6);
    std::uniform_int_distribution<int> _1_to_4(1,4);
    std::uniform_int_distribution<int> _1_to_9(1,9);
    std::uniform_int_distribution<int> _1_to_15(1,15);
    std::uniform_int_distribution<int> _1_to_10(1,10);

    std::ofstream dataset;
    dataset.precision(10);
    std::remove("data.csv");
    dataset.open("data.csv");
    std::vector<std::string> variables;
    std::vector<int> case_i;
    std::string linestr = "";
    for(size_t i=0 ; i<10 ; ++i)
        variables.push_back("q1_" + std::to_string(i));std::string a = "1,2,,3,4,5";
    for(size_t i=0 ; i<5 ; i++)
        variables.push_back("q" + std::to_string(2 + i));
    for(size_t i=0 ; i<15 ; i++)
        variables.push_back("q7_" + std::to_string(i));
    variables.push_back("q8");
    variables.push_back("q9");
    variables.push_back("q10");
    for(size_t i=0 ; i<8 ; ++i)
        variables.push_back("q11_" + std::to_string(i));
    for(size_t i=0 ; i<5 ; i++)
        variables.push_back("q" + std::to_string(12+i));
    for(size_t i=0 ; i<8 ; ++i){
        for(size_t j=0 ; j<7 ; j++)
            variables.push_back("q17_" + std::to_string(i) + '_' + std::to_string(j));
    }
    variables.push_back("year");
    variables.push_back("gender");
    variables.push_back("age");
    variables.push_back("income");
    variables.push_back("cosize");
    variables.push_back("tenure");
    variables.push_back("weight");
    for(size_t i=0 ; i<variables.size() ; ++i)
    {
        if(i < variables.size() - 1)
            linestr += variables[i] + ',';
        else
            linestr += variables[i];
    }
    dataset << linestr << std::endl;
    for(size_t i=0 ; i<250000 ; ++i)
    {
        linestr.clear();
        case_i.clear();
        for(size_t j=0 ; j<10 ; ++j) // q1 vars(0-9)
            case_i.push_back( _0_to_10(mt_rand));
        case_i.push_back(_1_to_7(mt_rand)); // q2 vars(10)
        if(case_i[10] <= 3)
            case_i.push_back(_1_to_5(mt_rand)); // q3 vars(11)
        else
            case_i.push_back(-1);
        if(case_i[11] < 3 && case_i[11] != -1)
            case_i.push_back(_1_to_3(mt_rand)); // q4 vars(12)
        else
            case_i.push_back(-1);
        if(case_i[12] > 1)
            case_i.push_back(_1_to_2(mt_rand)); // q5 vars(13)
        else
            case_i.push_back(-1);
        if(case_i[13] == 1)
            case_i.push_back(_0_to_10(mt_rand)); // q6 vars(14)
        else
            case_i.push_back(-1);
        for(size_t j=0 ; j<15 ; ++j) // q7 vars(15-29)
            case_i.push_back(_0_or_1(mt_rand));
        case_i.push_back(_1_to_10(mt_rand)); // q8 vars(30)
        if(case_i[30] > 5)
            case_i.push_back(_1_to_15(mt_rand)); // q9 vars(31)
        else
            case_i.push_back(-1);
        int valid[] = {1,2,3,5,9,13,14,15};
        if(std::binary_search(std::begin(valid), std::end(valid), case_i[31]))
            case_i.push_back(_1_to_2(mt_rand)); // q10 vars(32)
        else
            case_i.push_back(-1);

        //q11
        if(sum(case_i,15,29) >= 8)
        {
            std::array<int,8> rankset = {0,0,0,0,0,0,0,0};
            int subsetsize = _1_to_5(mt_rand);
            int subset[subsetsize];
            for(int j=1 ; j<=subsetsize ; ++j)
                subset[j-1] = j;
            for(int j=0 ; j<8 ; ++j){
                if(j<subsetsize)
                    rankset[j] = subset[j];
                else
                    rankset[j] = -1;
            }
            std::random_shuffle(std::begin(rankset), std::end(rankset));
            for(size_t j=0 ; j<8 ; ++j)
                case_i.push_back(rankset[j]);
        }
        else
        {
            for(size_t j=0 ; j<8 ; ++j)
                case_i.push_back(-1);
        }
        case_i.push_back(_1_to_4(mt_rand)); //q12
        if(case_i[41] == 2  || case_i[41] == 1)
            case_i.push_back(_1_to_7(mt_rand)); // q13
        else
            case_i.push_back(-1);
        if(case_i[42] >= 1 && case_i[42] < 4)
            case_i.push_back(_1_to_3(mt_rand)); // q14
        else
            case_i.push_back(-1);
        if(case_i[43] == 2 || case_i[43] == 3)
            case_i.push_back(_1_to_2(mt_rand)); // q15
        else
            case_i.push_back(-1);
        if(case_i[44] == 1)
            case_i.push_back(_0_to_10(mt_rand)); // q16
        else
            case_i.push_back(-1);
        for(size_t j=0 ; j<8 ; ++j){
            for(size_t k=0 ; k<7 ; k++)
                case_i.push_back(_1_to_7(mt_rand)); // q17
        }
        if(i<50000){
            case_i.push_back(4);
        }else if(i<100000){
            case_i.push_back(3);
        }else if(i<150000){
            case_i.push_back(2);
        }else if(i<200000){
            case_i.push_back(1);
        }else if(i<250000){
            case_i.push_back(0);
        }
        case_i.push_back(_1_to_2(mt_rand)); // gender
        case_i.push_back(_1_to_5(mt_rand)); // age
        case_i.push_back(_1_to_7(mt_rand)); // income
        case_i.push_back(_1_to_6(mt_rand)); // cosize
        case_i.push_back(_1_to_4(mt_rand)); // tenure
        for(size_t j=0 ; j<case_i.size() ; ++j)
        {
            if(case_i[j] == -1)
                linestr += ',';
            else
                linestr += std::to_string(case_i[j]) + ',';
        }
        linestr += std::to_string(_0_to_1(mt_rand) + .5); // weight
        dataset << linestr << std::endl;
    }
    return 0;
}


