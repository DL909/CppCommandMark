//
// Created by 909 DL on 2025/10/18.
//

#pragma once
#include <string>
#include <vector>

enum RESPONSE_MODE
{
    COMMAND = 0,
    TEXT = 1,
};

struct config
{
    bool verbose_flag = false;
    bool directory_flag = false; // given directory rather than using current directory
    bool wait_flag = false;
    std::string path;
    std::string file_path =  static_cast<std::string>(getenv("HOME")) + "/.command_mark.xml";
    RESPONSE_MODE response_mode = COMMAND;
};
constexpr int score_long = 20;
inline std::vector<std::string> error_messages = {};
inline config config;