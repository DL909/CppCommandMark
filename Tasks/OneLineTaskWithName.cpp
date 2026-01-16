//
// Created by 909 DL on 2026/1/16.
//

#include <ncurses.h>
#include <pugixml.hpp>
#include "../Util/config_and_constants.h"
#include "Task.h"
#include "TaskTypeRegistrar.h"
#include "../Util/util.h"

class OneLineTaskWithName final : public Task
{
public:

    OneLineTaskWithName(const int index_, const pugi::xml_node& node) : Task(index_, node)
    {
        this->name = node.child("name").text().as_string();
        this->command = node.child("command").text().as_string();
        this->path = node.child("path").text().as_string();
        this->name_match_number = 0;
    }
    ~OneLineTaskWithName() override = default;

    static int create(const int index, pugi::xml_node & root)
    {
        std::cout << "enter name:" << std::endl;
        std::string name;
        std::getline(std::cin, name);
        if (name.empty())
        {
            error_messages.emplace_back(std::format("Error: invalid"));
            return EXIT_FAILURE;
        }
        std::cout << "enter command:" << std::endl;
        std::string command;
        std::getline(std::cin, command);
        if (command.empty())
        {
            error_messages.emplace_back(std::format("Error: invalid"));
            return EXIT_FAILURE;
        }
        std::cout << "enter path(" << get_path() << "):" << std::endl;
        std::string path;
        std::getline(std::cin, path);
        if (path.empty())
        {
            path = get_path();
        }
        std::cout << std::format("Adding one line task with name: \nname: {}\ncommand: {}\npath:    {}\nconfirm(Y/n):", name, command, path) << std::endl;
        const int c = std::cin.get();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        if (!(c == 'y' || c == '\n' || c == 'Y' || c == '\r'))
        {
            error_messages.emplace_back(std::format("Error: user canceled"));
            return EXIT_FAILURE;
        }
        pack(index, name, command, path, root);
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
        const int name_cols = COLS - 6 - path_cols - 4 - (config.verbose_flag ? score_long : 0);
        const int name_start = 2 + (config.verbose_flag ? score_long : 0);
        const int path_start = name_cols + 10 + (config.verbose_flag ? score_long : 0);
        if (config.verbose_flag) { mvprintw(line, 2, "%ld", this -> score); }
        bold_mvprintw(line,name_start,this->name,name_cols,range(this->name_match_number));
        bold_mvprintw(line,path_start,this->path,path_cols,{});
        if (choose)
        {
            attroff(A_REVERSE);
        }
        return 1;
    }

    int update_score(const std::string& input) override
    {
        if (input == this->name)
        {
            this->score = INT32_MAX;
        }else
        {
            this->score = 0;
            this->name_match_number = 0;
            for (int i=0;i<std::min(input.length(),this->name.length());i++)
            {
                if (input.at(i) == this->name.at(i))
                {
                    score += 20;
                    this->name_match_number++;
                }else
                {
                    break;
                }
            }
        }
        return 0;
    }

    std::string get_output() override
    {
        return std::format("cd \"{}\";{}",path,command);
    }

    pugi::xml_node save() override
    {
        pugi::xml_node node_task;
        node_task.set_name("task");
        pugi::xml_node node_type = node_task.append_child("type");
        node_type.text().set("one_line_task_with_name");
        pugi::xml_node node_index = node_task.append_child("index");
        node_index.text().set(this->index);
        pugi::xml_node data = node_task.append_child("data");
        pugi::xml_node node_name = data.append_child("name");
        node_name.text().set(this->name);
        pugi::xml_node node_command = data.append_child("command");
        node_command.text().set(this->command);
        pugi::xml_node node_path = data.append_child("path");
        node_path.text().set(this->path);

        return node_task;
    }



private:
    std::string name;
    std::string command;
    std::string path;
    int name_match_number;

    static void pack(const int _index, const std::string& _name, const std::string& _command, const std::string& _path, pugi::xml_node & _root)
    {
        pugi::xml_node task = _root.append_child(_command);
        task.set_name("task");
        pugi::xml_node type = task.append_child("type");
        type.text().set("one_line_task_with_name");
        pugi::xml_node index = task.append_child("index");
        index.text().set(_index);
        pugi::xml_node data = task.append_child("data");
        pugi::xml_node name = data.append_child("name");
        name.text().set(_name);
        pugi::xml_node command = data.append_child("command");
        command.text().set(_command);
        pugi::xml_node path = data.append_child("path");
        path.text().set(_path);
    }
};

[[maybe_unused]] static TaskTypeRegistrar<OneLineTaskWithName> registrar("one_line_task_with_name");
[[maybe_unused]] static UserTaskTypeRegistrar user_registrar("one_line_task_with_name",&OneLineTaskWithName::create);