//
// Created by 909 DL on 2025/10/17.
//

#include <pugixml.hpp>
#include <fmt/format.h>
#include <ncurses.h>

#include "../Util/config_and_constants.h"
#include "fuzzy_match.h"
#include "Task.h"
#include "TaskTypeRegistrar.h"
#include "../Util/util.h"

class OneLineTask final : public Task
{
public:

    OneLineTask(const int index_, const pugi::xml_node& node) : Task(index_, node)
    {
        this->command = node.child("command").text().as_string();
        this->path = node.child("path").text().as_string();

    }
    ~OneLineTask() override = default;

    static int create(const int index, pugi::xml_node & root)
    {
        std::cout << "enter command:" << std::endl;
        std::string command;
        std::getline(std::cin, command);
        if (command.empty())
        {
            error_messages.emplace_back(fmt::format("Error: invalid"));
            return EXIT_FAILURE;
        }
        std::cout << "enter path(" << get_path() << "):" << std::endl;
        std::string path;
        std::getline(std::cin, path);
        if (path.empty())
        {
            path = get_path();
        }
        std::cout << fmt::format("Adding one line task: \ncommand: {}\npath:    {}\nconfirm(Y/n):", command, path) << std::endl;
        const int c = std::cin.get();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        if (!(c == 'y' || c == '\n' || c == 'Y' || c == '\r'))
        {
            error_messages.emplace_back(fmt::format("Error: user canceled"));
            return EXIT_FAILURE;
        }
        pack(index, command, path, root);
        return EXIT_SUCCESS;
    }

    int rend(const int line, const bool choose) override
    {
        if (choose) {
            attron(A_REVERSE); // 开启反显
        }
        mvprintw(line, 0, "%s", (static_cast<std::string>(" ") * COLS).c_str());
        if (choose)
        {
            mvprintw(line, 0, "* ");
        }
        const int path_cols = ((COLS - 6) - (COLS - 6) % 3) / 3;
        const int command_cols = COLS - 6 - path_cols - 4 - (config.verbose_flag ? score_long : 0);
        const int command_start = 2 + (config.verbose_flag ? score_long : 0);
        const int path_start = command_cols + 10 + (config.verbose_flag ? score_long : 0);
        if (config.verbose_flag) { mvprintw(line, 2, "%ld", this -> score); }
        bold_mvprintw(line,command_start,this->command,command_cols,command_list);
        bold_mvprintw(line,path_start,this->path,path_cols,path_list);
        if (choose)
        {
            attroff(A_REVERSE);
        }
        return 1;
    }

    int update_score(const std::string& input) override
    {
        bool command_flag = true;
        bool backslash_flag = false;
        std::string p_command;
        std::string p_path;
        for (const char c : input)
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


        const int command_score = fuzzy_match(p_command.c_str(), this->command.c_str());

        const node * p = match_end;
        int i = -1;
        command_list = {};

        while (p != nullptr)
        {
            i += p->number + 1;
            command_list.push_back(i);
            p = p->next;
        }

        recursive_delete(match_end);

        const int path_score = fuzzy_match(p_path.c_str(), this->path.c_str());

        p = match_end;
        i = -1;
        path_list = {};

        while (p != nullptr)
        {
            i += p->number + 1;
            path_list.push_back(i);
            p = p->next;
        }

        recursive_delete(match_end);

        if (command_score == INT32_MIN || path_score == INT32_MIN) {
            this->score = INT32_MIN;
        }
        else
        {
            this->score = command_score + path_score;
        }
        return 0;
    }

    std::string get_output() override
    {
        return fmt::format("cd \"{}\";{}",path,command);
    }

    pugi::xml_node save() override
    {
        pugi::xml_node node_task;
        node_task.set_name("task");
        pugi::xml_node node_type = node_task.append_child("type");
        node_type.text().set("one_line_task");
        pugi::xml_node node_index = node_task.append_child("index");
        node_index.text().set(this->index);
        pugi::xml_node data = node_task.append_child("data");
        pugi::xml_node node_command = data.append_child("command");
        node_command.text().set(this->command);
        pugi::xml_node node_path = data.append_child("path");
        node_path.text().set(this->path);
        return node_task;
    }



private:
    std::string command;
    std::string path;
    std::vector<int> command_list;
    std::vector<int> path_list;
    static void pack(const int _index, const std::string& _command, const std::string& _path, pugi::xml_node & _root)
    {
        pugi::xml_node task = _root.append_child(_command);
        task.set_name("task");
        pugi::xml_node type = task.append_child("type");
        type.text().set("one_line_task");
        pugi::xml_node index = task.append_child("index");
        index.text().set(_index);
        pugi::xml_node data = task.append_child("data");
        pugi::xml_node command = data.append_child("command");
        command.text().set(_command);
        pugi::xml_node path = data.append_child("path");
        path.text().set(_path);
    }
};

[[maybe_unused]] static TaskTypeRegistrar<OneLineTask> registrar("one_line_task");
[[maybe_unused]] static UserTaskTypeRegistrar user_registrar("one_line_task",&OneLineTask::create);



