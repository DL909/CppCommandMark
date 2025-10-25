//
// Created by 909 DL on 2025/10/22.
//

#include "util.h"

#include <filesystem>
#include <iostream>
#include <ncurses.h>
#include <unistd.h>
#include <fmt/format.h>
#include <sys/ioctl.h>
#include <sys/ttycom.h>

#include "config_and_constants.h"

std::string operator*(const std::string& lhs, const int rhs)
{
    std::string temp;
    for (int i = 0; i < rhs; i++)
    {
        temp += lhs;
    }
    return temp;
}

void bold_mvprintw(const int line, const int column, const std::string& text, const int max_length, const std::vector<int>& bold_list)
{
    move(line, column);
    int index_of_list = 0;
    if (text.length() > max_length)
    {
        for (int i = 0; i<max_length - 3; i++)
        {
            if (index_of_list < bold_list.size() && bold_list[index_of_list] == i)
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
            if (index_of_list < bold_list.size() && bold_list[index_of_list] == i)
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

void print_doc(const pugi::xml_document& doc, const std::string& title) {
    std::cout << "--- " << title << " ---" << std::endl;
    doc.save(std::cout, "  ");
    std::cout << std::endl;
}