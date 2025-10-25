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
#include <pugixml.hpp>
#include <memory>
#include <list>

#include "fuzzy_match.h"
#include "Util/opt_processor.h"
#include "Tasks/Task.h"
#include "Util/util.h"


#include "Util/config_and_constants.h"


// parameter flag


enum MODE
{
    ERROR = 0,
    MARK = 1,
    CHECK = 2,
    DELETE = 3,
    HELP = 4,
};

std::vector<int> used_index{};

struct scored_task_type
{
    std::string name;
    int score = 100;
};

inline void sort_scored_types(std::vector<scored_task_type> & types)
{
    std::ranges::sort(types, [](const scored_task_type & a, const scored_task_type & b) {
        return a.score > b.score;
    });
}
int mark(const std::string& command)
{

    std::vector<scored_task_type> types;
    for (const auto& name : UserTaskFactory::get_user_create_task_type_list())
    {
        types.emplace_back(name);
    }
    if (types.empty())
    {
        error_messages.emplace_back("Error: no valid task type");
        return EXIT_FAILURE;
    }

    setlocale(LC_ALL, "");

    // 2. 为ncurses I/O打开/dev/tty
    FILE* tty_fp_out = fopen("/dev/tty", "w");
    FILE* tty_fp_in = fopen("/dev/tty", "r");

    if (!tty_fp_out || !tty_fp_in) {
        std::cerr << "Error: Could not open /dev/tty" << std::endl;
        return -1;
    }

    // 3. 使用newterm()初始化ncurses
    // newterm的第一个参数为NULL，它会从TERM环境变量自动检测终端类型
    SCREEN* term_screen = newterm(nullptr, tty_fp_out, tty_fp_in);
    if (term_screen == nullptr) {
        std::cerr << "Error: newterm() failed" << std::endl;
        fclose(tty_fp_out);
        fclose(tty_fp_in);
        return -1;
    }

    // 设置当前活动的ncurses屏幕
    set_term(term_screen);
    // 4. 设置ncurses模式
    noecho();             // 不回显用户输入
    cbreak();             // 立即获取字符，无需等待回车
    keypad(stdscr, TRUE); // 启用功能键 (如箭头)
    curs_set(1);


    int choice = 0;
    const int number = std::min(LINES - 3 - (config.verbose_flag ? 1 : 0), static_cast<int>(types.size()) - 1);
    std::string input;
    int index=0;
    int result = EXIT_SUCCESS;
    bool continue_flag = true;

    mvprintw(LINES - 1, 0, "> ");
    while (continue_flag)
    {
        if (config.verbose_flag)
        {
            mvprintw(0, 0, "%s", (static_cast<std::string>(" ") * COLS).c_str());
            mvprintw(0, 0, "choice  : %d", choice);
        }
        int i = 0;
        for (const auto & type  : types)
        {
            mvprintw(LINES - 2 - i, 0, "%c %s", choice == i?'*' : ' ',type.name.c_str());
        }
        mvprintw(LINES - 1, 2, "%s", (static_cast<std::string>(" ") * (COLS - 2)).c_str());
        mvprintw(LINES - 1, 2, "%s", input.c_str());
        mvprintw(LINES - 1, index + 2, "");
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
        case 27: // esc
            error_messages.emplace_back("Error: interrupted");
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
            for (auto & [name, score] : types)
            {
                score = fuzzy_match(input.c_str(), name.c_str());
            }
            sort_scored_types(types);
            break;
        }

    }

    echo();
    endwin();
    fclose(tty_fp_out);
    fclose(tty_fp_in);
    if (result != EXIT_SUCCESS)
    {
        return result;
    }
    pugi::xml_document doc;
    doc.load_file(config.file_path.c_str());
    pugi::xml_node root = doc.child("root");
    if (root.empty())
    {
        root = doc.append_child("root");
    }
    if (UserTaskFactory::create(types[choice].name,root.last_child().child("index").text().as_int()+1, root) == EXIT_SUCCESS)
    {
        std::cout << "successfully added task" << std::endl;
        return EXIT_SUCCESS;
    }
    std::cerr << "failed to add task" << std::endl;
    return EXIT_FAILURE;
}

int delete_mark(const long id)
{
    pugi::xml_document doc;
    doc.load_file(config.file_path.c_str());
    pugi::xml_node root = doc.child("root");
    if (root.empty())
    {
        std::cerr << "failed to load xml" << std::endl;
        return EXIT_FAILURE;
    }
    pugi::xml_node current_task = root.child("task");

    while (current_task)
    {
        pugi::xml_node next_task = current_task.next_sibling("task");
        if (current_task.child("index").text().as_int() == id)
        {
            root.remove_child(current_task);
        }
        current_task = next_task;
    }
    return doc.save_file(config.file_path.c_str());
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
    std::cout << "\t--wait(-w): pause program until enter ( for debugging )" << std::endl;

    return EXIT_SUCCESS;
}

std::vector<std::unique_ptr<Task>> get_task_list()
{
    std::vector<std::unique_ptr<Task>> task_list;

    pugi::xml_document doc;
    if (!doc.load_file(config.file_path.c_str()))
    {
        error_messages.emplace_back(fmt::format("get_task_list() error: could not load xml document from {}\n",config.file_path));
    }

    for (pugi::xml_node node : doc.child("root").children())
    {
        std::string name = node.child("type").text().as_string();
        int index = node.child("index").text().as_int();
        task_list.push_back(TaskFactory::create(name,index, node.child("data")));
        used_index.push_back(index);
    }

    return task_list;
}

inline void sort_tasks(std::vector<std::unique_ptr<Task>>& tasks)
{
    std::ranges::sort(tasks, [](const std::unique_ptr<Task>& a, const std::unique_ptr<Task>& b) {
        return a->score == b->score ? a->index > b->index : a->score > b->score;
    });
}

int choose(std::string& command, long& id)
{
    std::vector<std::unique_ptr<Task>> tasks = get_task_list();
    if (tasks.empty())
    {
        error_messages.emplace_back("choose() error: get_task_list() failed");
        return -1;
    }



    setlocale(LC_ALL, "");

    // 2. 为ncurses I/O打开/dev/tty
    FILE* tty_fp_out = fopen("/dev/tty", "w");
    FILE* tty_fp_in = fopen("/dev/tty", "r");

    if (!tty_fp_out || !tty_fp_in) {
        std::cerr << "Error: Could not open /dev/tty" << std::endl;
        return -1;
    }

    // 3. 使用newterm()初始化ncurses
    // newterm的第一个参数为NULL，它会从TERM环境变量自动检测终端类型
    SCREEN* term_screen = newterm(nullptr, tty_fp_out, tty_fp_in);
    if (term_screen == nullptr) {
        std::cerr << "Error: newterm() failed" << std::endl;
        fclose(tty_fp_out);
        fclose(tty_fp_in);
        return -1;
    }

    // 设置当前活动的ncurses屏幕
    set_term(term_screen);
    // 4. 设置ncurses模式
    noecho();             // 不回显用户输入
    cbreak();             // 立即获取字符，无需等待回车
    keypad(stdscr, TRUE); // 启用功能键 (如箭头)
    curs_set(1);


    int choice = 0;
    const int number = std::min(LINES - 3 - (config.verbose_flag ? 1 : 0), static_cast<int>(tasks.size()) - 1);
    std::string input;
    int index=0;
    int result = EXIT_SUCCESS;
    bool continue_flag = true;

    mvprintw(LINES - 1, 0, "> ");
    sort_tasks(tasks);
    while (continue_flag)
    {

        if (config.verbose_flag)
        {
            mvprintw(0, 0, "%s", (static_cast<std::string>(" ") * COLS).c_str());
            mvprintw(0, 0, "choice  : %d", choice);
        }
        int task_index = 0;
        int i = LINES - 2;
        for (const auto & task  : tasks)
        {
            if (i<(config.verbose_flag?1:0))
            {
                break;
            }
            i = i - task->rend(i,choice == task_index);
            task_index ++;
        }
        mvprintw(LINES - 1, 2, "%s", (static_cast<std::string>(" ") * (COLS - 2)).c_str());
        mvprintw(LINES - 1, 2, "%s", input.c_str());
        mvprintw(LINES - 1, index + 2, "");
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
        case 27: // esc
            error_messages.emplace_back(fmt::format("choose() error: interrupted\n"));
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
            for (const auto & task  : tasks)
            {
                task->update_score(input);
            }
            sort_tasks(tasks);
            break;
        }

    }

    echo();
    endwin();
    command = tasks[choice]->get_output();
    id = tasks[choice]->index;
    fclose(tty_fp_out);
    fclose(tty_fp_in);
    return result;
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
    return 0;
}

int p_directory(const char* param, void* place_to_save)
{
    if (config.directory_flag)
    {
        return 1;
    }
    config.directory_flag = true;
    config.path = param;
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
    config.verbose_flag = true;
    return 0;
}

int p_file(const char* param, void* place_to_save)
{
    config.file_path = param;
    return 0;
}

int p_wait(const char* param, void* place_to_save)
{
    config.wait_flag = true;
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
        {"mark", 'm', false, p_mark, nullptr},
        {"help", 'h', false, p_help, nullptr},
        {"directory", '\0', true, p_directory, nullptr},
        {"verbose", 'v', false, p_verbose, nullptr},
        {"file",'f',true,p_file,nullptr},
        {"wait",'w',false,p_wait,nullptr}
    };

    if (process(argc, argv, options)!=0)
    {
        std::cout << "process arg error" << std::endl;
        return EXIT_FAILURE;
    };

    if (config.wait_flag)
    {
        std::cout << getpid() << std::endl;
        getchar();
    }

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
