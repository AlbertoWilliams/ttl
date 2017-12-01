/**
 * parser.hh - ast parser
 *
 * Author: Bao Hexing <HexingB@qq.com>
 * Created:  7 November 2017
 *
 * Copyright © 2017, Bao Hexing. All Rights Reserved.
 */

#ifndef TTL_PARSER_H
#define TTL_PARSER_H

#include <istream>
#include <list>
#include <map>
#include <string>
#include "operator.hh"
#include "tokenizer.hh"

namespace ttl {

    class Parser {
    public:
        Parser();
        ~Parser();

        static bool Init();

        // parse code and build ast, return true if no error occurs.
        bool Create(const char * code);

        double Evaluate();

        // return error message if Init() failed, or ""
        const char * ErrorMsg() const;

        void ErrorContext(std::string& msg) const;

    private:
        Parser(std::list<std::string> * module_name_stack);
        void CreateModule(long end_type);
        void CreateSentence();
        Module * CreateTorusModule(); // well, different style, but less code
        void CreateIf();
        void CreateReturn();
        void CreateNow(); // return the number of seconds since epoch
        void CreateAssign(const std::string& name, double (*op)(double, double), bool check_rhs);
        // process variable creation and calculation.
        void CreateVariable(const std::string& name);
        // process variable value.
        void CreateVariableValue(const std::string& name, double * variable);
        void CreateNum();
        void CreateAtom();
        void CreateRotator();
        void CreateDiv();
        void CreateMul();
        void CreateMod();
        void CreateNegative();
        void CreateFactor();
        void CreateSymbol();
        void CreateCmp();
        void CreateExpr();
        void CreateValue();
        void CreateInclude();

        bool NestedIncluded(const std::string& filename);

    private:

        // following are auxiliary methods for "*" "/" and "%"
        bool PopTwoFactors(Operator ** lhs, Operator ** rhs);

        typedef Operator * (Parser::*op_allocator)();
        void DivMulMod(op_allocator operator_allocator);

        Operator * AllocateDiv();
        Operator * AllocateMul();
        Operator * AllocateMod();

        // following are auxiliary methods for TOKNE_NAME
        void ProcessTokenName(const std::string& name);

    private:
        Module * ast_tree_;

        Token current_token_;
        Tokenizer tokenizer_;
        std::size_t error_code_;

        typedef void (Parser::*fn)();
        static std::map<std::string, fn> name_token_processors_;

        // for module nested "include" check;
        // before read a module, push back the module name into this list,
        // after read a module, pop back the module name in this list.
        // every module parser share the same module_name_stack_.
        std::list<std::string> * module_name_stack_;
    };

} // ttl

#endif
