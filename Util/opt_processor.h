//
// Created by 909 DL on 25-8-2.
//
/*
 * Copyright (C) 2025 DL909 - This file has been modified from its original version.
 *
 * Licensed under the MIT License.
 * https://opensource.org/licenses/MIT
 */
#pragma once
#include <string>
#include <vector>

extern int processor_error;
extern int opt_index;
extern int opt_index_of_current;
extern int arg_index;

// every option will be finally processed by its processor
// if option has arg, the arg ( a const char * variable ) will become the first param,
// if option does not have arg, nullptr will become the first param,
// the processor must use its return value to show its status, 0 means success, anything else means error
// if the processor returns anything not 0, it will be writen to variable processor_error and command will exit
// with 3
// else, if function processed all param successfully, it will return 0.
// example
// {"verbose",'v',false...}

struct option
{
    std::string long_name;
    char short_name{};
    bool has_arg{};
    int (*processor)(const char* param, void* place_to_save){};
    void* place_to_save{};
};

// the return value of process() follow these rules:
// 0: success
// 1: no matched option
// 2: no enough arg
// 3: processor error
// 4: too many arg

int process(int argc, const char** argv, const std::vector<option>& options);

