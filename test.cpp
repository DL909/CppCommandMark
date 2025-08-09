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
    std::cout << "\tmark <command> [options]" << std::endl;
    return 0;
}
