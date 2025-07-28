//
// Created by 909 DL on 2025/7/20.
//

#ifndef CPPCOMMANDMARK_H
#define CPPCOMMANDMARK_H
#include <string>

enum MODE
{
    ERROR=0,
    MARK=1,
    CHECK=2,
    DELETE=3,
    HELP=4,
};


MODE parameter_processor(int argc, const char * * argv, std::string &str, long* id);

int error_handle(char * error_description);

int mark(const std::string & command);

int delete_mark(long id);

int help();

int choose(std::string & command, int& index);

int leave_text_in_terminal(const char *text_to_leave);

int cpp_command_mark(int argc,const char** argv);

#endif //CPPCOMMANDMARK_H
