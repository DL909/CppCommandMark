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
#include <pugixml.hpp>
#include <ncurses.h>

#include "Tasks/Task.h"
#include "fuzzy_match.h"
#include "Util/util.h"

void create_xml()
{
    pugi::xml_document doc;
    pugi::xml_node root = doc.append_child("root");
    pugi::xml_node task = root.append_child("task");
    pugi::xml_node type = task.append_child("type");
    type.text().set("one_line_task");
    pugi::xml_node index = task.append_child("index");
    index.text().set(0);
    pugi::xml_node data = task.append_child("data");
    pugi::xml_node command = data.append_child("command");
    command.text().set("ls");
    pugi::xml_node directory = data.append_child("path");
    directory.text().set("/Users/909dl/myProgramming");

    // 保存到文件
    if (doc.save_file("/Users/909dl/.command_mark.xml")) {
        std::cout << "XML saved successfully." << std::endl;
    } else {
        std::cerr << "Failed to save XML file." << std::endl;
    }
}

// test for Task/Task.h TaskFactory::get_task_type_list()
void test1()
{
    for (const auto& t : TaskFactory::get_task_type_list())
    {
        std::cout << t << std::endl;
    }
}

// test for activate curses for several times
void test2()
{
    initscr();
    noecho();
    cbreak();
    mvprintw(0,0,"Hello World");
    char c = getch();
    echo();
    endwin();
    std::cout << static_cast<int>(c) << std::endl;
    c = getchar();
    std::cout << static_cast<int>(c) << std::endl;
    initscr();
    noecho();
    cbreak();
    mvprintw(1,0,"Hello World2");
    c = getch();
    echo();
    endwin();
    std::cout << static_cast<int>(c) << std::endl;
}

void test3()
{
    pugi::xml_document doc;
    doc.load_file("/Users/909dl/myProgramming/programming/cpp/ClionProject/CppCommandMark/test.xml");
    pugi::xml_node root = doc.child("root");
    if (root.empty())
    {
        root = doc.append_child("root");
    }
    root.append_child("task");

    if (UserTaskFactory::create("one_line_task",0, root) == EXIT_SUCCESS)
    {
        print_doc(doc,"title");
    }else
    {
        std::cout << "t" << std::endl;
    }

}



int main() {
    test3();

    return 0;
}

// int main()
// {
//     initscr();
//     noecho();
//     cbreak();
//     mvprintw(0,0,"Hello World");
//     const char c = getch();
//     echo();
//     endwin();
//     std::cout << static_cast<int>(c) << std::endl;
//     return 0;
// }

// int main()
// {
//     std::cout << fuzzy_match("tt","tt t") << std::endl;
//     const node * p = match_end;
//     int index = -1;
//     std::vector<int> array;
//     if (p == nullptr)
//     {
//         std::cout << "t" << std::endl;
//     }
//     while (p != nullptr)
//     {
//         index += p->number + 1;
//         array.push_back(index);
//         p = p->next;
//     }
//     for (const auto i : array)
//     {
//         std::cout << i << std::endl;
//     }
//     recursive_delete(match_end);
//
// }