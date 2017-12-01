/**
 * interpreter.cc - interperter
 *
 * Author: Bao Hexing <HexingB@qq.com>
 * Created: 25 November 2017
 *
 * Copyright © 2017, Bao Hexing. All Rights Reserved.
 */

#include <iostream>
#include <readline/readline.h>
#include <readline/history.h>
#include "parser.hh"

using namespace ttl;

int main(int argc, char ** argv) {
    char * line = NULL;

    Parser::Init();
    while (true) {
        line = readline("> ");
        if (strcmp(line, "quit") == 0) {
            break;
        }

        Parser p;
        bool ret = p.Create(line);
        if (ret == false) {
            std::cerr << "Error to create ast: " << p.ErrorMsg() << std::endl;
            std::string msg;
            p.ErrorContext(msg);
            std::cerr << msg << std::endl;
        } else {
            double result = p.Evaluate();
            std::cout << result << std::endl;
        }
    }
    return 0;
}
