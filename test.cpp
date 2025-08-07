//
// Created by 909 DL on 2025/7/20.
//

#include <iostream>

#include "opt_processor.h"

int foo1(const char * param, void * place_to_save)
{
    std::cout << param << std::endl;
    * static_cast<int*>(place_to_save) = 42;
    return 0;
};
int foo2(const char * param, void * place_to_save)
{
    * static_cast<int*>(place_to_save) = 41;
    return 0;
}

int main(const int argc, const char** argv)
{
    std::vector<option> options = {};
    int temp = 0;

    options.push_back(option("test",'t',true,foo1,static_cast<void*>(&temp)));
    options.push_back(option("est",'e',false,foo2,static_cast<void*>(&temp)));
    std::cout << process(argc, argv, options) << std::endl;
    std::cout << temp << std::endl;
    return 0;
}
