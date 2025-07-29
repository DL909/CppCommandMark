//
// Created by 909 DL on 2025/7/20.
//

#include "CppCommandMark.h"
#include <iostream>
#include <ncurses.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fstream>
#include <vector>
#include <format>
#include <list>
#include <algorithm>
#include <cstdlib>

#include "fuzzy_match.h"

constexpr bool debug = true;
constexpr int score_long = 20;
std::vector<std::string> error_messages;

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

MODE parameter_processor(int argc, const char** argv, std::string& str, long& id)
{
    if (argc == 1)
    {
        return CHECK;
    }
    if (argc >= 2)
    {
        if (strcmp(argv[1], "-c") == 0 || strcmp(argv[1], "--check") == 0)
        {
            return CHECK;
        }
        if (strcmp(argv[1], "-d") == 0 || strcmp(argv[1], "--delete") == 0)
        {
            return DELETE;
        }
        if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)
        {
            return HELP;
        }
        if (strcmp(argv[1], "-m") == 0 || strcmp(argv[1], "--mark") == 0)
        {
            if (argc == 3)
            {
                str = argv[2];
            }
            else
            {
                str = std::format(
                    "parameter_processor() error: mark mode needs second parameter as command, provided %d", argc - 2);
                return ERROR;
            }
            return MARK;
        }
        return ERROR;
    }
    return ERROR;
}

std::string get_path()
{
    try
    {
        return std::filesystem::current_path().string();
    }
    catch (const std::filesystem::filesystem_error& e)
    {
        error_messages.emplace_back(std::format("get_path() error: {}\n", e.what()));
        return "";
    }
}

int mark(const std::string& command)
{
    const std::string path = get_path();
    if (path.empty())
    {
        error_messages.emplace_back("mark() error: get_path() failed\n");
        return EXIT_FAILURE;
    }
    const std::string file_path = static_cast<std::string>(getenv("HOME")) + "/.command_mark";
    std::ofstream ofstream(file_path, std::ios::app);
    if (!ofstream.is_open())
    {
        error_messages.emplace_back(std::format("mark() error: could not open file {}\n", file_path));
        return EXIT_FAILURE;
    }
    ofstream << command << std::endl << path << std::endl;
    ofstream.close();
    std::cout << "marked command: \n" << command << std::endl << "in directory: \n" << path << std::endl;
    return EXIT_SUCCESS;
}

int delete_mark(long id)
{
    int result = EXIT_SUCCESS;
    const std::string file_path = static_cast<std::string>(getenv("HOME")) + "/.command_mark";
    const std::string bak_file_path = file_path + ".bak";
    if (access(bak_file_path.c_str(),F_OK) == 0)
    {
        if (remove(bak_file_path.c_str()) != 0)
        {
            error_messages.emplace_back(std::format("mark() error: could not remove file {}\n", bak_file_path));
            result = EXIT_FAILURE;
            return result;
        }
    }
    if (rename(file_path.c_str(), bak_file_path.c_str()) != 0)
    {
        error_messages.emplace_back(std::format("mark() error: could not rename file {}\n", file_path));
        result = EXIT_FAILURE;
        return result;
    }
    std::ifstream old_file(bak_file_path);
    if (!old_file.is_open())
    {
        error_messages.emplace_back(std::format("mark() error: could not open file {}\n", bak_file_path));
        result = EXIT_FAILURE;
        return result;
    }
    std::ofstream new_file(file_path);
    if (!new_file.is_open())
    {
        error_messages.emplace_back(std::format("mark() error: could not open file {}\n", file_path));
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
    //TODO
    error_messages.emplace_back(std::format("help() error: not finished\n"));
    return EXIT_FAILURE;
    return EXIT_SUCCESS;
}

std::vector<task> get_task_list()
{
    const std::string file_path = static_cast<std::string>(getenv("HOME")) + "/.command_mark";
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

inline void rend(const int line, const task& task, const bool choose)
{
    mvprintw(line, 0, "%s", (static_cast<std::string>(" ") * COLS).c_str());
    if (choose)
    {
        mvprintw(line, 0, "* ");
    }
    const int path_cols = ((COLS - 6) - (COLS - 6) % 3) / 3;
    const int command_cols = COLS - 6 - path_cols - 4 - (debug ? score_long : 0);
    const int command_start = 2 + (debug ? score_long : 0);
    const int path_start = command_cols + 10 + (debug ? score_long : 0);
    if (debug) { mvprintw(line, 2, "%d", task.score); }
    if (task.command.length() >= command_cols)
    {
        mvprintw(line, command_start, "%s...", task.command.substr(0, command_cols).c_str());
    }
    else
    {
        mvprintw(line, command_start, "%s", task.command.c_str());
        for (int i = command_start + task.command.length(); i < command_start + command_cols; i++)
        {
            mvprintw(line, i, " ");
        };
    }
    if (task.path.length() >= path_cols)
    {
        mvprintw(line, path_start, "...%s",
                 task.path.substr(task.path.length() - path_cols + 3, path_cols - 3).c_str());
    }
    else
    {
        mvprintw(line, path_start, "%s", task.path.c_str());
        for (int i = path_start + task.path.length(); i < path_start + path_cols; i++)
        {
            mvprintw(line, i, " ");
        };
    }
}

inline void sort_tasks(std::vector<task> & tasks)
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
    const int number = std::min(LINES - 3 - (debug ? 3 : 0), static_cast<int>(tasks.size()));
    int index = 0;
    std::string input;
    sort_tasks(tasks);
    for (int i = 0; i < std::min(LINES - 2, static_cast<int>(tasks.size())); i++)
    {
        rend(LINES - 2 - i, tasks[i], choice == i);
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
                if (c <= 126){
                    input.insert(index,std::string(1,static_cast<char>(c)));
                    index++;
                }else if (c == 127 || c == KEY_BACKSPACE)
                {
                    if (index > 0)
                    {
                        input.erase(index -1,1);
                        index--;
                    }else
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

            if (debug)
            {
                mvprintw(0, 0, "%s", (static_cast<std::string>(" ") * COLS).c_str());
                mvprintw(1, 0, "%s", (static_cast<std::string>(" ") * COLS).c_str());
                mvprintw(0, 0, "command : %s", p_command.c_str());
                mvprintw(1, 0, "path    : %s", p_path.c_str());
            }
            break;
        }
        for (int i = 0; i < number; i++)
        {
            rend(LINES - 2 - i, tasks[i], choice == i);
        }
        mvprintw(LINES - 1, 2, "%s", (static_cast<std::string>(" ") * (COLS-2)).c_str());
        mvprintw(LINES - 1, 2, "%s", input.c_str());
        mvprintw(LINES - 1,index + 2,"");

    }

    echo();
    endwin();
    command = std::format("cd \"{}\";{}", tasks[choice].path, tasks[choice].command);
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

        // 使用 ioctl 系统调用和 TIOCSTI 请求
        // TIOCSTI (Terminal I/O Control Simulate Terminal Input)
        // 这个操作会将字符 'c' 插入到终端的输入队列中。
        // 第一个参数 0 (STDIN_FILENO) 代表标准输入，也就是当前终端。
        if (ioctl(STDIN_FILENO, TIOCSTI, &c) < 0)
        {
            // 如果操作失败，打印错误信息并返回
            error_messages.emplace_back("leave_text_in_terminal() error: ioctl TIOCSTI failed");
            return EXIT_FAILURE;
        }
    }
    echo(); //恢复输出
    endwin();
    return EXIT_SUCCESS;
}

int cpp_command_mark(const int argc, const char** argv)
{
    int result = EXIT_SUCCESS;
    std::string param;
    long id;
    switch (parameter_processor(argc, argv, param, id))
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
        result = leave_text_in_terminal(param.c_str());
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
    }
    return result;
}
