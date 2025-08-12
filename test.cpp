//
// Created by 909 DL on 2025/7/20.
//
/*
 * Copyright (C) 2025 DL909 - This file has been modified from its original version.
 *
 * Licensed under the MIT License.
 * https://opensource.org/licenses/MIT
 */
#include <iostream>

#include "opt_processor.h"
#include "3rd-party/fuzzy-match/fuzzy_match.h"
#include <ncurses.h>


int main(const int argc, const char** argv)
{
    std::cout << fuzzy_match("tttt","?DL909/home/ttestesest") << std::endl;
    const node * p = match_end;
    int index = -1;
    while (p != nullptr)
    {
        index += p->number + 1;
        std::cout << index << std::endl;
        p = p->next;
    }
    recursive_delete(match_end);
    return 0;
}
