//
// Created by 909 DL on 2025/10/17.
//

#pragma once
#include "Task.h"
template <typename T>

/**
 * \brief registrar for task types
*/
class TaskTypeRegistrar {
public:
    // 构造函数接受注册名
    explicit TaskTypeRegistrar(const std::string& name) {
        // 将一个 lambda 表达式注册到工厂，这个 lambda 知道如何创建 T 的实例
        TaskFactory::regist(name, [](int index, const pugi::xml_node& node) {
            return std::make_unique<T>(index, node);
        });
    }
};

using ConfigCreatorFuncPtr = int(*)(int, pugi::xml_node &);
/**
 * \brief registrar for user create task types
*/
class UserTaskTypeRegistrar {
public:
    // 构造函数接受注册名
    explicit UserTaskTypeRegistrar(const std::string& name, ConfigCreatorFuncPtr func) {
        UserTaskFactory::regist(name, func);
    }
};