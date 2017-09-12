#ifndef BSTR_H_INCLUDED
#define BSTR_H_INCLUDED
#include <vector>


std::string bleft(const std::string &sourceStr, size_t index)
{
    std::string outputStr;
    for(size_t i=0 ; i<=index ; ++i)
        outputStr += sourceStr[i];
    return outputStr;
}
std::string bright(const std::string &sourceStr,size_t indexbegin)
{
    std::string outputStr;
    for(size_t i=indexbegin ; i<sourceStr.length() ; ++i)
        outputStr += sourceStr[i];
    return outputStr;
}
std::string bmid(const std::string &sourceStr,size_t indexbegin, size_t indexend, bool inclusivity = false)
{
    std::string outputStr;
    if(inclusivity)
    {
        for(size_t i=indexbegin ; i<=indexend ; ++i)
            outputStr += sourceStr[i];
    }
    else
    {
        for(size_t i=indexbegin+1 ; i<indexend ; ++i)
            outputStr += sourceStr[i];
    }
    return outputStr;
}
std::string filter(const std::string &sourceStr, const std::string filterOut)
{
    std::string outputStr = sourceStr;
    int loc = sourceStr.find(filterOut);
    while(loc != -1)
    {
        loc = sourceStr.find(filterOut,loc + 1);
        if(loc != -1)
        {
            outputStr = outputStr.replace(loc,filterOut.length(),"");
        }
    }
    return outputStr;
}
std::vector<std::string> split(const std::string &sourceStr, const char delimiter, bool check_delimiters = true)
// This function will take a string reference and return a parsed vector of the string based on a delimiter
// the check delimiters parameter will tell the function whether or not the source string contains delimiters enclosed with quotes
{
    std::vector<std::string> splitVec;
    std::string temp;
    if (!check_delimiters)
    {
        for(size_t i=0 ; i<sourceStr.length() ; ++i)
        {
            if(sourceStr[i] != delimiter)
            {
                temp += sourceStr[i];
            }
            else
            {
                splitVec.push_back(temp);
                temp.clear();
            }
        }
        splitVec.push_back(temp);
        return splitVec;
    }
    else
    {
        for(size_t i=0 ; i<sourceStr.length() ; ++i)
        {
            if(sourceStr[i]!='"')
            {
                if(sourceStr[i] != delimiter)
                {
                    temp += sourceStr[i];
                }
                else
                {
                    splitVec.push_back(temp);
                    temp.clear();
                }
            }
            else
            {
                ++i;
                while(sourceStr[i]!='"')
                {
                    if(sourceStr[i] != '"')
                    {
                        temp += sourceStr[i];
                        ++i;
                    }
                    else
                    {
                        splitVec.push_back(temp);
                        temp.clear();
                    }
                }
            }
        }
        splitVec.push_back(temp);
        return splitVec;
    }
}
std::string splice(const std::string &sourceStr, const std::string leftStr, const std::string rightStr)
{
     size_t l = sourceStr.find(leftStr) + leftStr.length();
     size_t r = sourceStr.find(rightStr);
     std::string outputStr;
     for(size_t i=l ; i<r ; ++i)
     {
        outputStr+=sourceStr[i];
     }
     return outputStr;
}


#endif // BSTR_H_INCLUDED
