#ifndef BSTR_H_INCLUDED
#define BSTR_H_INCLUDED
#include <vector>
#include "stdarg.h"
#include <mutex>
#include <thread>
#include <algorithm>

int shared_print(std::mutex& mu, const char *__format, ...){
  std::lock_guard<std::mutex> guard(mu);
  register int __retval;
  __builtin_va_list __local_argv;
  __builtin_va_start( __local_argv, __format );
  __retval = __mingw_vprintf( __format, __local_argv );
  __builtin_va_end( __local_argv );
  return __retval;
}

std::string bleft(const std::string &sourceStr, const size_t& index){
    std::string outputStr;
    for(size_t i=0 ; i<=index ; ++i)
        outputStr += sourceStr[i];
    return outputStr;
}

std::string bright(const std::string &sourceStr, const size_t& indexbegin){
    std::string outputStr;
    for(size_t i=indexbegin ; i<sourceStr.length() ; ++i)
        outputStr += sourceStr[i];
    return outputStr;
}

std::string bmid(const std::string &sourceStr, const size_t& indexbegin, const size_t& indexend, bool inclusivity = false){
    std::string outputStr;
    if(inclusivity){
        for(size_t i=indexbegin ; i<=indexend ; ++i)
            outputStr += sourceStr[i];
    }else{
        for(size_t i=indexbegin+1 ; i<indexend ; ++i)
            outputStr += sourceStr[i];
    }
    return outputStr;
}

std::string filter(const std::string &sourceStr, const std::string& filterOut){
    std::string outputStr = sourceStr;
    int loc = sourceStr.find(filterOut);
    while(loc != -1){
        loc = sourceStr.find(filterOut,loc + 1);
        if(loc != -1){
            outputStr = outputStr.replace(loc,filterOut.length(),"");
        }
    }
    return outputStr;
}

std::vector<std::string> split(const std::string& sourceStr, const char& delimiter, bool check_delimiters = true){
    std::vector<std::string> splitVec;
    std::string temp;
    if (!check_delimiters){
        for(size_t i=0 ; i<sourceStr.length() ; ++i){
            if(sourceStr[i] != delimiter){
                temp += sourceStr[i];
            }else{
                splitVec.push_back(temp);
                temp = "";
            }
        }
        splitVec.push_back(temp);
        return splitVec;
    }else{
        for(size_t i=0 ; i<sourceStr.length() ; ++i){
            if(sourceStr[i]!='"'){
                if(sourceStr[i] != delimiter){
                    temp += sourceStr[i];
                }else{
                    splitVec.push_back(temp);
                    temp = "";
                }
            }else{
                ++i;
                while(sourceStr[i]!='"'){
                    if(sourceStr[i] != '"'){
                        temp += sourceStr[i];
                        ++i;
                    }else{
                        splitVec.push_back(temp);
                        temp = "";
                    }
                }
            }
        }
        splitVec.push_back(temp);
        return splitVec;
    }
}

void allocated_split(const std::string& sourceStr, std::vector<std::string>& destination, const char& delimiter, bool check_delimiters = true){
    size_t index = 0;
    std::string temp;
    if (!check_delimiters){
        for(size_t i=0 ; i<sourceStr.length() ; ++i){
            if(sourceStr[i] != delimiter){
                temp += sourceStr[i];
            }else{
                destination[index] = temp;
                ++index;
                temp = "";
            }
        }
        destination[index] = temp;
        ++index;
        temp = "";
    }else{
        for(size_t i=0 ; i<sourceStr.length() ; ++i){
            if(sourceStr[i]!='"'){
                if(sourceStr[i] != delimiter){
                    temp += sourceStr[i];
                }else{
                    destination[index] = temp;
                    ++index;
                    temp = "";
                }
            }else{
                ++i;
                while(sourceStr[i]!='"'){
                    if(sourceStr[i] != '"'){
                        temp += sourceStr[i];
                        ++i;
                    }else{
                        destination[index] = temp;
                        ++index;
                        temp = "";
                    }
                }
            }
        }
        destination[index] = temp;
        ++index;
        temp = "";
    }
}

std::string splice(const std::string& sourceStr, const std::string& leftStr, const std::string& rightStr){
    size_t l = sourceStr.find(leftStr) + leftStr.length();
    size_t r = sourceStr.find(rightStr);
    std::string outputStr;
    if(l!=std::string::npos && r!= std::string::npos){
        for(size_t i=l ; i<r ; ++i){
            outputStr+=sourceStr[i];
        }
        return outputStr;
    }else{
        return outputStr;
    }
}



#endif // BSTR_H_INCLUDED
