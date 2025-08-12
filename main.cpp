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
#include <ncurses.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fstream>
#include <vector>
#include <fmt/format.h>
#include <algorithm>
#include <cstdlib>
#include <filesystem>

#include <fuzzy_match.h>
#include <list>

#include "opt_processor.h"


constexpr int score_long = 20;
std::vector<std::string> error_messages;

enum RESPONSE_MODE
{
    COMMAND = 0,
    TEXT = 1,
};

// parameter flag
bool verbose_flag = false;
bool directory_flag = false; // given directory rather than using current directory
std::string path;
std::string file_path;
RESPONSE_MODE response_mode = COMMAND;

enum MODE
{
    ERROR = 0,
    MARK = 1,
    CHECK = 2,
    DELETE = 3,
    HELP = 4,
};


class task
{
public:
    int index;
    std::string command;
    std::string path;
    long score;


    task(const int index_, const std::string& command_, const std::string& path_)
    {
        index = index_;
        command = command_;
        path = path_;
        score = 200;
    }
};

std::string get_path()
{
    try
    {
        return std::filesystem::current_path().string();
    }
    catch (const std::filesystem::filesystem_error& e)
    {
        error_messages.emplace_back(fmt::format("get_path() error: {}\n", e.what()));
        return "";
    }
}

int mark(const std::string& command)
{
    if (!directory_flag)
    {
        path = get_path();
    }
    if (path.empty())
    {
        error_messages.emplace_back("mark() error: can't get path\n");
        return EXIT_FAILURE;
    }
    std::cout << fmt::format("command = {}\nin path {}\n type y to agree\n", command, path);
    if (getchar() == 'y')
    {
        if (file_path.empty())
        {
            file_path = static_cast<std::string>(getenv("HOME")) + "/.command_mark";
        }
        std::ofstream ofstream(file_path, std::ios::app);
        if (!ofstream.is_open())
        {
            error_messages.emplace_back(fmt::format("mark() error: could not open file {}\n", file_path));
            return EXIT_FAILURE;
        }
        ofstream << command << std::endl << path << std::endl;
        ofstream.close();
        std::cout << "marked command: \n" << command << std::endl << "in directory: \n" << path << std::endl;
        return EXIT_SUCCESS;
    }
    error_messages.emplace_back("mark() error: user interrupted");
    return EXIT_FAILURE;
}

int delete_mark(long id)
{
    int result = EXIT_SUCCESS;
    if (file_path.empty())
    {
        file_path = static_cast<std::string>(getenv("HOME")) + "/.command_mark";
    }
    const std::string bak_file_path = file_path + ".bak";
    if (access(bak_file_path.c_str(),F_OK) == 0)
    {
        if (remove(bak_file_path.c_str()) != 0)
        {
            error_messages.emplace_back(fmt::format("mark() error: could not remove file {}\n", bak_file_path));
            result = EXIT_FAILURE;
            return result;
        }
    }
    if (rename(file_path.c_str(), bak_file_path.c_str()) != 0)
    {
        error_messages.emplace_back(fmt::format("mark() error: could not rename file {}\n", file_path));
        result = EXIT_FAILURE;
        return result;
    }
    std::ifstream old_file(bak_file_path);
    if (!old_file.is_open())
    {
        error_messages.emplace_back(fmt::format("mark() error: could not open file {}\n", bak_file_path));
        result = EXIT_FAILURE;
        return result;
    }
    std::ofstream new_file(file_path);
    if (!new_file.is_open())
    {
        error_messages.emplace_back(fmt::format("mark() error: could not open file {}\n", file_path));
        result = EXIT_FAILURE;
        return result;
    }
    std::string temp;
    std::string deleted_line1;
    std::string deleted_line2;
    int index = 0;
    while (index < id * 2)
    {
        std::getline(old_file, temp);
        new_file << temp << std::endl;
        index++;
    }
    std::getline(old_file, deleted_line1);
    std::getline(old_file, deleted_line2);
    std::getline(old_file, temp);
    while (!temp.empty())
    {
        new_file << temp << std::endl;
        std::getline(old_file, temp);
    }
    return result;
}

int help()
{
    std::cout << "Cpp Command Mark" << std::endl;
    std::cout << "usage:" << std::endl;
    std::cout << "\t./CppCommandMark <option> [args]" << std::endl;
    std::cout << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "\t--help(-h): show this help" << std::endl;
    std::cout << "\t--check(-c): choose command and use ( default )" << std::endl;
    std::cout << "\t--delete(-d): choose command and delete" << std::endl;
    std::cout << "\t--mark(-x): mark command" << std::endl;
    std::cout << "\t\t--directory(-d): manually choose where the command will run" << std::endl;
    std::cout << "extra options:" << std::endl;
    std::cout << "\t--verbose(-v): show verbose output" << std::endl;
    std::cout << "\t--file(-f): manually choose where the information file is" << std::endl;

    return EXIT_SUCCESS;
}

std::vector<task> get_task_list()
{
    if (file_path.empty())
    {
        file_path = static_cast<std::string>(getenv("HOME")) + "/.command_mark";
    }
    std::ifstream ifstream;
    ifstream.open(file_path);
    if (!ifstream.is_open())
    {
        error_messages.emplace_back("get_task_list() error: could not open file \n");
        return {};
    }
    std::vector<task> command_list;
    std::string temp1;
    std::string temp2;
    int i = 0;
    while (std::getline(ifstream, temp1) && std::getline(ifstream, temp2))
    {
        command_list.emplace_back(i, temp1, temp2);
        i++;
    }
    return command_list;
}

std::string operator*(const std::string& lhs, const int rhs)
{
    std::string temp;
    for (int i = 0; i < rhs; i++)
    {
        temp += lhs;
    }
    return temp;
}
void bold_mvprintw(int line, int column, const std::string& text, int max_length, std::vector<int> list)
{
    move(line, column);
    int index_of_list = 0;
    if (text.length() > max_length)
    {
        for (int i = 0; i<max_length - 3; i++)
        {
            if (index_of_list < list.size() && list[index_of_list] == i)
            {
                attron(A_BOLD);
                index_of_list ++;
                addch(text[i]);
                attroff(A_BOLD);
            }else
            {
                addch(text[i]);
            }
        }
        addch('.');
        addch('.');
        addch('.');
    }else
    {
        for (int i = 0; i<text.length(); i++)
        {
            if (index_of_list < list.size() && list[index_of_list] == i)
            {
                attron(A_BOLD);
                index_of_list ++;
                addch(text[i]);
                attroff(A_BOLD);
            }else
            {
                addch(text[i]);
            }
        }
        for (int i = text.length(); i <max_length; i++)
        {
            addch(' ');
        }
    }
}

void rend(const int line, const task& task, const bool choose, const std::vector<int>& list = {})
{
    mvprintw(line, 0, "%s", (static_cast<std::string>(" ") * COLS).c_str());
    if (choose)
    {
        mvprintw(line, 0, "* ");
    }
    const int path_cols = ((COLS - 6) - (COLS - 6) % 3) / 3;
    const int command_cols = COLS - 6 - path_cols - 4 - (verbose_flag ? score_long : 0);
    const int command_start = 2 + (verbose_flag ? score_long : 0);
    const int path_start = command_cols + 10 + (verbose_flag ? score_long : 0);
    if (verbose_flag) { mvprintw(line, 2, "%ld", task.score); }
    bold_mvprintw(line,command_start,task.command,command_cols,{0,3,7});
    bold_mvprintw(line,path_start,task.path,path_cols,{0,3,7});
}

inline void sort_tasks(std::vector<task>& tasks)
{
    std::ranges::sort(tasks, [](const task& a, const task& b)
    {
        return a.score == b.score ? a.index > b.index : a.score > b.score;
    });
}

void calculate_pattern(const std::string& input, std::string& p_command, std::string& p_path)
{
    bool command_flag = true;
    bool backslash_flag = false;
    p_command = "";
    p_path = "";
    for (char c : input)
    {
        if (!backslash_flag)
        {
            backslash_flag = false;
            if (c == '\\')
            {
                backslash_flag = true;
                continue;
            }
            if (c == ' ' and command_flag)
            {
                command_flag = false;
                continue;
            }
        }
        if (command_flag)
        {
            p_command += c;
        }
        else
        {
            p_path += c;
        }
    }
}

int choose(std::string& command, long& id)
{
    int result = EXIT_SUCCESS;
    std::vector<task> tasks = get_task_list();
    if (tasks.empty())
    {
        error_messages.emplace_back("choose() error: get_task_list() failed");
        return -1;
    }
    std::string p_command;
    std::string p_path;
    setlocale(LC_ALL, "");
    initscr();
    raw();
    keypad(stdscr, TRUE);
    curs_set(0);
    bool continue_flag = true;
    noecho();
    curs_set(1);
    int choice = 0;
    const int number = std::min(LINES - 3 - (verbose_flag ? 3 : 0), static_cast<int>(tasks.size()) - 1);
    int index = 0;
    std::string input;
    sort_tasks(tasks);
    for (int i = 0; i < std::min(LINES - 2, static_cast<int>(tasks.size())); i++)
    {
        rend(LINES - 2 - i, tasks[i], choice == i);
    }
    if (verbose_flag)
    {
        mvprintw(0, 0, "%s", (static_cast<std::string>(" ") * COLS).c_str());
        mvprintw(1, 0, "%s", (static_cast<std::string>(" ") * COLS).c_str());
        mvprintw(2, 0, "%s", (static_cast<std::string>(" ") * COLS).c_str());
        mvprintw(0, 0, "command : %s", p_command.c_str());
        mvprintw(1, 0, "path    : %s", p_path.c_str());
        mvprintw(2, 0, "choice  : %d", choice);
    }
    mvprintw(LINES - 1, 0, "> ");
    while (continue_flag)
    {
        switch (const int c = getch())
        {
        case KEY_UP:
            choice = std::min(choice + 1, number);
            break;
        case KEY_DOWN:
            choice = std::max(choice - 1, 0);
            break;
        case KEY_LEFT:
            index = std::max(index - 1, 0);
            break;
        case KEY_RIGHT:
            index = std::min(index + 1, static_cast<int>(input.length()));
            break;
        case 3: // ctrl + c
            error_messages.emplace_back("choose() error: interrupted\n");
            result = EXIT_FAILURE;
            continue_flag = false;
            continue;
        case '\n':
            continue_flag = false;
            continue;
        default:
            {
                if (c <= 31) // control ascii
                {
                    continue;
                }
                if (c <= 126)
                {
                    input.insert(index, std::string(1, static_cast<char>(c)));
                    index++;
                }
                else if (c == 127 || c == KEY_BACKSPACE)
                {
                    if (index > 0)
                    {
                        input.erase(index - 1, 1);
                        index--;
                    }
                    else
                    {
                        continue;
                    }
                }
            }


            calculate_pattern(input, p_command, p_path);

            for (auto& task : tasks)
            {
                const int command_score = fuzzy_match(p_command.c_str(), task.command.c_str());
                const int path_score = fuzzy_match(p_path.c_str(), task.path.c_str());
                if (command_score == INT32_MIN || path_score == INT32_MIN)
                {
                    task.score = INT32_MIN;
                }
                else
                {
                    task.score = command_score + path_score;
                }
            }
            sort_tasks(tasks);


            break;
        }
        if (verbose_flag)
        {
            mvprintw(0, 0, "%s", (static_cast<std::string>(" ") * COLS).c_str());
            mvprintw(1, 0, "%s", (static_cast<std::string>(" ") * COLS).c_str());
            mvprintw(2, 0, "%s", (static_cast<std::string>(" ") * COLS).c_str());
            mvprintw(0, 0, "command : %s", p_command.c_str());
            mvprintw(1, 0, "path    : %s", p_path.c_str());
            mvprintw(2, 0, "choice  : %d", choice);
        }
        for (int i = 0; i <= number; i++)
        {
            rend(LINES - 2 - i, tasks[i], i == choice);
        }
        mvprintw(LINES - 1, 2, "%s", (static_cast<std::string>(" ") * (COLS - 2)).c_str());
        mvprintw(LINES - 1, 2, "%s", input.c_str());
        //mvcur(LINES -1,LINES-1,index+2,index+2);
        mvprintw(LINES - 1, index + 2, "");
    }

    echo();
    endwin();
    command = fmt::format("cd \"{}\";{}", tasks[choice].path, tasks[choice].command);
    id = tasks[choice].index;
    return result;
}

int leave_text_in_terminal(const char* text_to_leave)
{
    initscr();
    noecho(); //阻止输出
    // 遍历需要留下的字符串中的每一个字符
    for (int i = 0; i < strlen(text_to_leave); ++i)
    {
        char c = text_to_leave[i];
        int t = ioctl(STDIN_FILENO, TIOCSTI, &c);
        if (t < 0)
        {
            error_messages.emplace_back(fmt::format("leave_text_in_terminal() error: ioctl() failed : {}\n",errno));
            return EXIT_FAILURE;
        }
    }
    echo();
    endwin();
    return EXIT_SUCCESS;
}

// parameter processer function

bool mode_flag = false;

MODE mode = CHECK;

int p_check(const char* param, void* place_to_save)
{
    if (mode_flag)
    {
        return 1;
    }
    mode = CHECK;
    return 0;
}

int p_delete(const char* param, void* place_to_save)
{
    if (mode_flag)
    {
        return 1;
    }
    mode = DELETE;
    return 0;
}

int p_mark(const char* param, void* place_to_save)
{
    if (mode_flag)
    {
        return 1;
    }
    mode = MARK;
    *static_cast<std::string*>(place_to_save) = param;
    return 0;
}

int p_directory(const char* param, void* place_to_save)
{
    if (directory_flag)
    {
        return 1;
    }
    directory_flag = true;
    path = param;
    return 0;
}

int p_help(const char* param, void* place_to_save)
{
    if (mode_flag)
    {
        return 1;
    }
    mode = HELP;
    return 0;
}

int p_verbose(const char* param, void* place_to_save)
{
    verbose_flag = true;
    return 0;
}

int p_file(const char* param, void* place_to_save)
{
    file_path = param;
    return 0;
}


int main(const int argc, const char** argv)
{
    int result = EXIT_SUCCESS;
    std::string param;
    long id;
    const std::vector<struct option> options = {
        {"check", 'c', false, p_check, nullptr},
        {"delete", 'd', false, p_delete, nullptr},
        {"mark", 'm', true, p_mark, &param},
        {"help", 'h', false, p_help, nullptr},
        {"directory", 'd', true, p_directory, nullptr},
        {"verbose", 'v', false, p_verbose, nullptr},
        {"file",'f',true,p_file,nullptr},
    };

    process(argc, argv, options);

    switch (mode)
    {
    case ERROR:
        result = EXIT_FAILURE;
        break;
    case MARK:
        return mark(param);
    case CHECK:
        if ((result = choose(param, id)) != EXIT_SUCCESS)
        {
            break;
        }
        if ((result = leave_text_in_terminal(param.c_str())) != EXIT_SUCCESS)
        {
            error_messages.emplace_back(fmt::format("{}", param));
        }
        break;
    case DELETE:
        if ((result = choose(param, id)) != EXIT_SUCCESS)
        {
            break;
        }
        result = delete_mark(id);
        break;
    case HELP:
        result = help();
    }
    if (result != EXIT_SUCCESS)
    {
        for (const auto& error : error_messages)
        {
            std::cout << error;
        }
        std::cout << std::endl;
    }
    return result;
}
