//
// Created by 909 DL on 2026/1/16.
// Template file, Should not be compiled

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
        this->command = node.child("command").text().as_string();
        this->path = node.child("path").text().as_string();
    }
    ~OneLineTaskWithName() override = default;

    static int create(const int index, pugi::xml_node & root)
    {

        return EXIT_SUCCESS;
    }

    int rend(const int line, const bool choose) override
    {

        return 1;
    }

    int update_score(const std::string& input) override
    {

        return 0;
    }

    std::string get_output() override
    {
        return std::format("");
    }

    pugi::xml_node save() override
    {
        pugi::xml_node node_task;

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

[[maybe_unused]] static TaskTypeRegistrar<OneLineTaskWithName> registrar("template");
[[maybe_unused]] static UserTaskTypeRegistrar user_registrar("template",&OneLineTaskWithName::create);
