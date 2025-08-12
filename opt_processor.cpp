//
// Created by 909 DL on 25-8-2.
//

/*
 * Copyright (C) 2025 DL909 - This file has been modified from its original version.
 *
 * Licensed under the MIT License.
 * https://opensource.org/licenses/MIT
 */

#include "opt_processor.h"

int processor_error;

int opt_index = 1;
int opt_index_of_current=1;
int arg_index = 2;

int process(const int argc, const char** argv, const std::vector<option>& options)
{
    int result = 0;
    while (opt_index < argc)
    {
        if (argv[opt_index][0] == '-')
        {
            if (argv[opt_index][1] == '-')
            {
                result = 1;
                for (auto & option : options)
                {
                    if (option.long_name == argv[opt_index]+2)
                    {
                        if (option.has_arg)
                        {
                            if (arg_index < argc)
                            {
                                if ((processor_error = option.processor(argv[arg_index],option.place_to_save))!=0)
                                {
                                    result = 3;
                                    break;
                                }
                                result = 0;
                                opt_index = arg_index + 1;
                                arg_index = opt_index + 1;
                                break;
                            }
                            result = 2;
                            break;
                        }
                        if ((processor_error = option.processor(nullptr,option.place_to_save))!=0)
                        {
                            result = 3;
                            break;
                        }
                        result = 0;
                        opt_index++;
                        arg_index++;
                        break;
                    }
                }
                if (result != 0)
                {
                    break;
                }
            }else
            {
                if (argv[opt_index][opt_index_of_current] == 0)
                {
                    opt_index = arg_index ;
                    arg_index = opt_index + 1;
                    opt_index_of_current = 1;
                }else
                {
                    const char short_arg = argv[opt_index][opt_index_of_current];
                    result = 1;
                    for (auto & option : options)
                    {
                        if (short_arg == option.short_name)
                        {
                            if (option.has_arg)
                            {
                                if (arg_index < argc)
                                {
                                    if ((processor_error = option.processor(argv[arg_index],option.place_to_save))!=0)
                                    {
                                        result = 3;
                                        break;
                                    }
                                    opt_index_of_current++;
                                    arg_index++;
                                    result = 0;
                                    break;
                                }
                                result = 2;
                                break;
                            }
                            if ((processor_error = option.processor(nullptr,option.place_to_save))!=0)
                            {
                                result = 3;
                                break;
                            }
                            result = 0;
                            opt_index_of_current++;
                            arg_index++;
                            break;
                        }
                    }
                    if (result != 0)
                    {
                        break;
                    }
                }
            }
        }else
        {
            result = 4;
            break;
        }
    }

    return result;
}

