//
// Created by 909 DL on 2025/10/22.
//

#pragma once
#include <string>
#include <vector>
#include <pugixml.hpp>

std::string operator*(const std::string& lhs, int rhs);

void bold_mvprintw(int line, int column, const std::string& text, int max_length, const std::vector<int>& bold_list);

std::string get_path();

int leave_text_in_terminal(const char* text_to_leave);

void print_doc(const pugi::xml_document& doc, const std::string& title);