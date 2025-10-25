//
// Created by 909 DL on 2025/10/16.
//

#pragma once
#include <iostream>
#include <map>
#include <string>
#include <functional>
#include <memory>
// ReSharper disable once CppUnusedIncludeDirective
#include <pugixml.hpp>
#include <ranges>

/** \brief introduction of a Task class
 *  \note this is an ABC, inherit it as an actual task
 */
class Task
{
public:

    /// index is unique for each task, and provided as an input when instantiated
    const int index;

    /// score is how a task matching the pattern, and will be updated when pattern is changed through task::update_score
    long score;

    /// a task is instantiated with a unique index and a group of info provided
    Task(const int index_, const pugi::xml_node &) : index(index_), score(100)
    {
    }
    ;

    virtual ~Task() = default;



    /** a task should render itself in a ncurses context through task::rend
     * \param line given which line the render start(at the bottom of the rend part)
     * \param choose true if this task is chosen by user
     * \return return an int as how many lines did the task use
     */
    virtual int rend(int line, bool choose) = 0;


    /** a task should update itself through task::update_score
     *
     * this method get called when user input and changed the pattern
     *
     * \param input provide the pattern as a whole string reference
     * \return return an int to reflect if the command runs correct(0 if everything goes right)
     *
     *
    */
    virtual int update_score(const std::string& input) = 0;


    /**
     * chosen task will finally output itself as a string into shell
     * @return the string of command
     */
    virtual std::string get_output() = 0;

    /**
     * task will be saved using a pugi::xml_node
     * @return a pugi::xml_node saved its type, index and data
     */
    virtual pugi::xml_node save() = 0;

    /**
     * create a static std::optional<pugi::xml_node> create(int index) and register it to allow users
     * create a task manually
     */
    /**
     * \brief create a new task
     * A task need to get its data inside this method
     * will be called under a normal state, and need to init curses by itself and close curses when exit
     * @param index provided task index
     * @return pugi::xml_node of this task or std::nullopt for error
     */
};

/** \brief Factory for multi-type task
 *
 */
class TaskFactory
{
public:
    /**
     * the Creator is the function signature of the instantiating method of Task
     */
    using Creator = std::function<std::unique_ptr<Task>(int,pugi::xml_node)>;

    /**
     *
     * @param name the name of the task type in definition e.g. one_line_task
     * @param index index for instantiating task
     * @param node xml_node for instantiating task
     * @return a unique_ptr of the task created
     */
    static std::unique_ptr<Task> create(const std::string& name, const int index, const pugi::xml_node & node)
    {
        auto& registry = getRegistry();
        if (const auto it = registry.find(name); it != registry.end()) {
            return it->second(index, node); // 调用存储的创建函数
        }
        return nullptr;
    }

    /**
     * helping method, you shouldn't use it
     * @param name the name of the task type, it is agreed to use the UnderScoreCase e.g. one_line_task
     * @param creator the instantiating method
     */
    static void regist(const std::string& name, Creator creator) {
        getRegistry()[name] = std::move(creator);
    }

    /**
     * get all task type's name for choose
     * @return list of task type's name
     */
    static std::vector<std::string> get_task_type_list()
    {
        std::vector<std::string> type_list;
        type_list.reserve(getRegistry().size());
        for (const auto& key : getRegistry() | std::views::keys) {
            type_list.push_back(key);
        }
        return type_list;
    }
private:
    static std::map<std::string,Creator> & getRegistry()
    {
        static std::map<std::string,Creator> registry;
        return registry;
    }
};

/**
 * \brief Factory for creating multi-type task by user
 */
class UserTaskFactory
{
public:
    /**
     * the Creator is the function signature
     */
    using Creator = std::function<int(int,pugi::xml_node&)>;

    /**
     *
     * @param name the name of the task type in definition e.g. one_line_task
     * @param index index for instantiating task
     * @param root root of xml doc
     * @return a pugi::xml_node or std::nullptr for error
     */
    static int create(const std::string& name, const int index,pugi::xml_node root)
    {
        auto& registry = getRegistry();
        if (const auto it = registry.find(name); it != registry.end()) {
            return it->second(index,root); // 调用存储的创建函数
        }
        return EXIT_FAILURE;
    }

    /**
     * helping method, you shouldn't use it
     * @param name the name of the task type, it is agreed to use the UnderScoreCase e.g. one_line_task
     * @param creator the instantiating method
     */
    static void regist(const std::string& name, Creator creator) {
        getRegistry()[name] = std::move(creator);
    }

    /**
     * get all task type's name for choose
     * @return list of task type's name
     */
    static std::vector<std::string> get_user_create_task_type_list()
    {
        std::vector<std::string> type_list;
        type_list.reserve(getRegistry().size());
        for (const auto& key : getRegistry() | std::views::keys) {
            type_list.push_back(key);
        }
        return type_list;
    }
private:
    static std::map<std::string,Creator> & getRegistry()
    {
        static std::map<std::string,Creator> registry;
        return registry;
    }
};
