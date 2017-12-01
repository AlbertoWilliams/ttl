/**
 * scorer.cpp - scorer implementation
 *
 * Author: Bao Hexing <HexingB@qq.com>
 * Created:  7 November 2017
 *
 * Copyright © 2017, Bao Hexing. All Rights Reserved.
 */

#include <iostream>
#include <fstream>
#include <string>
#include <string.h> // for strlen
#include <map>
#include <functional>
#include <time.h>
#include "parser.hh"
#include "common.hh"

namespace ttl {

    static const char * const ERROR_MESSAGE[] = {
        "", // 0
        "bad syntax", // 1
        "nested loop", // 2
        "file not readable", // 3
        "variable not defined" // 4
    };

    std::map<std::string, Parser::fn> Parser::name_token_processors_;

    bool Parser::Init() {
        if (name_token_processors_.size() > 0) {
            return true; // has been initialized
        }

        // register name token handlers, such as lr", "lambdamart", ...
        name_token_processors_.insert(make_pair(std::string("include"), &Parser::CreateInclude));
        name_token_processors_.insert(make_pair(std::string("now"), &Parser::CreateNow));

        // ...
        return true;
    }

    Parser::Parser() : Parser(NULL) {}

    Parser::Parser(std::list<std::string> * module_name_stack)
        : ast_tree_(NULL),
          current_token_(),
          tokenizer_(""),
          error_code_(0),
          module_name_stack_(module_name_stack) {
        Init();

        if (module_name_stack_ == NULL) {
            module_name_stack_ = new std::list<std::string>();
        }
    }

    Parser::~Parser() {
        delete ast_tree_;

        // only the top Parser is responsible to release module_name_stack_
        if (module_name_stack_->size() == 0) {
            delete module_name_stack_;
        }
    }

    bool Parser::Create(const char * code) {
        if (code == NULL) {
            error_code_ = 3;
            return false;
        }

        tokenizer_.Reset(code);

        module_name_stack_->push_back("plugin.conf");
        ast_tree_ = new Module(Constants::DEFAULT_RETURN_VALUE);
        CreateModule(Tokenizer::TOKEN_EOL);
        module_name_stack_->pop_back();

        return error_code_ == 0 && ast_tree_ != NULL;
    }

    double Parser::Evaluate() {
        return ast_tree_->Evaluate();
    }

    void Parser::CreateModule(long end_type) {
        do {
            tokenizer_.NextToken(current_token_);

            if (current_token_.token_type == end_type) {
                break;
            }

            CreateSentence();

            if (current_token_.token_type == end_type) {
                break;
            }

            if (current_token_.token_type != Tokenizer::TOKEN_SEMICOLON) {
                error_code_ = 1;
            }
        } while (error_code_ == 0);
        return;
    }

    Module * Parser::CreateTorusModule() {
        if (current_token_.token_type != Tokenizer::TOKEN_LEFT_TORUS) {
            error_code_ = 1;
            return NULL;
        }

        Module * origin_ast = ast_tree_;
        ast_tree_ = new Module(Constants::DEFAULT_RETURN_VALUE);
        CreateModule(Tokenizer::TOKEN_RIGHT_TORUS);

        if (current_token_.token_type != Tokenizer::TOKEN_RIGHT_TORUS) {
            error_code_ = 1;
            delete ast_tree_;
            ast_tree_ = origin_ast;
            return NULL;
        }

        Module * torus_module = ast_tree_;
        ast_tree_ = origin_ast;
        return torus_module;
    }

    void Parser::CreateIf() {
        Module * torus_module = NULL;
        If * if_op = new If();

        do {
            tokenizer_.NextToken(current_token_);
            CreateValue(); // create 'condition'
            if (error_code_ != 0) {
                delete if_op;
                return;
            }

            Operator * condition = NULL;
            if (ast_tree_->PopLastChild(&condition) == false) {
                delete if_op;
                error_code_ = 1;
                return;
            }
            if_op->AddChild(condition);

            torus_module = CreateTorusModule();
            if (torus_module == NULL) {
                delete if_op;
                return;
            }
            if_op->AddChild(torus_module);

            tokenizer_.NextToken(current_token_);
            if (current_token_.token_type != Tokenizer::TOKEN_NAME ||
                strncmp(current_token_.token_pos, "else", strlen("else")) != 0) {
                ast_tree_->AddChild(if_op);
                return; // if ( ... ) { ... }
            }
            tokenizer_.NextToken(current_token_);
        } while (current_token_.token_type == Tokenizer::TOKEN_NAME &&
                 strncmp(current_token_.token_pos, "if", strlen("if")) == 0);

        torus_module = CreateTorusModule();
        if (torus_module == NULL) {
            delete if_op;
            return;
        }

        if_op->AddChild(torus_module);
        ast_tree_->AddChild(if_op);
        tokenizer_.NextToken(current_token_);
        return;
    }

    void Parser::CreateReturn() {
        tokenizer_.NextToken(current_token_);
        CreateValue();
        if (error_code_ != 0) {
            return;
        }

        if (current_token_.token_type != Tokenizer::TOKEN_SEMICOLON) {
            error_code_ = 1;
            return;
        }

        Operator * expr;
        if (ast_tree_->PopLastChild(&expr) == false) {
            error_code_ = 1;
            return;
        }

        Reference * ref = new Reference(ast_tree_, std::string("return"));
        ref->AddChild(expr);
        ast_tree_->AddChild(ref);
        return;
    }

    bool Parser::NestedIncluded(const std::string& filename) {
        for (std::list<std::string>::iterator it = module_name_stack_->begin();
             it != module_name_stack_->end(); ++it) {
            if (*it == filename) {
                return true;
            }
        }
        return false;
    }

    void Parser::CreateInclude() {
        // read "("
        tokenizer_.NextToken(current_token_);
        if (current_token_.token_type != Tokenizer::TOKEN_LEFT_BANANA) {
            error_code_ = 1;
            return;
        }

        // begin to read file name. "/" is considered as file path separator.
        tokenizer_.NextToken(current_token_);
        const char * path_start = current_token_.token_pos;
        while (current_token_.token_type == Tokenizer::TOKEN_NAME ||
               current_token_.token_type == Tokenizer::TOKEN_DIV) {
            tokenizer_.NextToken(current_token_);
        }

        // read ")"
        if (current_token_.token_type != Tokenizer::TOKEN_RIGHT_BANANA) {
            error_code_ = 1;
            return;
        }

        std::string filename = std::string(path_start, current_token_.token_pos - path_start);
        if (NestedIncluded(filename) == true) {
            error_code_ = 2;
            return;
        }

        const char * content = ReadFile(filename);

        module_name_stack_->push_back(filename);
        Parser p(module_name_stack_);
        if (p.Create(content) == false) {
            error_code_ = p.error_code_; // TODO copy the error context
            module_name_stack_->pop_back();
            return;
        }
        module_name_stack_->pop_back();

        ast_tree_->AddChild(p.ast_tree_);
        p.ast_tree_ = NULL;

        tokenizer_.NextToken(current_token_);
    }

    void Parser::CreateNow() {
        time_t time = ::time(NULL);
        tokenizer_.NextToken(current_token_);
        if (current_token_.token_type != Tokenizer::TOKEN_LEFT_BANANA) {
            error_code_ = 1;
            return;
        }

        tokenizer_.NextToken(current_token_);
        if (current_token_.token_type != Tokenizer::TOKEN_RIGHT_BANANA) {
            error_code_ = 1;
            return;
        }

        Num * num = new Num(time);
        ast_tree_->AddChild(num);
        tokenizer_.NextToken(current_token_);
    }

    void Parser::CreateAssign(const std::string& name, double (*op)(double, double), bool check_rhs) {
        tokenizer_.NextToken(current_token_);
        CreateValue();
        if (error_code_ != 0) {
            return;
        }

        Operator * child = NULL;
        if (ast_tree_->PopLastChild(&child) == false) {
            error_code_ = 1;
            return;
        }

        Reference * n = new Reference(ast_tree_, name, op, check_rhs);
        n->AddChild(child);
        ast_tree_->AddChild(n);
        return;
    }

    void Parser::CreateVariable(const std::string &name) {
        double (*op)(double, double) = assign;
        bool check_rhs = false;
        tokenizer_.NextToken(current_token_);
        switch (current_token_.token_type) {
        case Tokenizer::TOKEN_ASSIGN:
            // do not need to check variable name is exists.
            return CreateAssign(name, op, check_rhs);
        case Tokenizer::TOKEN_ADD_ASSIGN:
            op = add;
            break;
        case Tokenizer::TOKEN_SUB_ASSIGN:
            op = sub;
            break;
        case Tokenizer::TOKEN_MUL_ASSIGN:
            op = mul;
            break;
        case Tokenizer::TOKEN_DIV_ASSIGN:
            op = div;
            check_rhs = true;
            break;
        case Tokenizer::TOKEN_MOD_ASSIGN:
            op = mod;
            check_rhs = true;
            break;
        default:
            error_code_ = 4;
            return;
        }

        double * variable = ast_tree_->GetVariable(name);
        if (variable == NULL) {
            error_code_ = 4;
            return;
        }

        return CreateAssign(name, op, check_rhs);
    }

    void Parser::CreateNum() {
        char * endptr = NULL;
        double value = strtod(current_token_.token_pos, &endptr);
        if (endptr != current_token_.token_pos + current_token_.token_length) {
            error_code_ = 1;
            return;
        }

        Num * num = new Num(value);
        ast_tree_->AddChild(num);

        tokenizer_.NextToken(current_token_);
    }

    void Parser::CreateVariableValue(const std::string& name, double * variable) {
        Variable * v = new Variable(variable);
        ast_tree_->AddChild(v);
        tokenizer_.NextToken(current_token_);
    }

    void Parser::ProcessTokenName(const std::string& name) {
        std::map<std::string, fn>::iterator processor = name_token_processors_.find(name);
        if (processor != name_token_processors_.end()) {
            fn f = processor->second;
            return (this->*f)(); // named functions
        }

        double * variable = ast_tree_->GetVariable(name);
        if (variable != NULL) {
            // process variable value.
            return CreateVariableValue(name, variable);
        }

        error_code_ = 4;
        return;
    }

    void Parser::CreateAtom() {
        switch (current_token_.token_type) {
        case Tokenizer::TOKEN_NUM:
            return CreateNum();
        case Tokenizer::TOKEN_NAME:
            {
                std::string name(current_token_.token_pos, current_token_.token_length);
                if (name == "if" || name == "return") {
                    error_code_ = 1;
                    return;
                }

                return ProcessTokenName(name);
            }
        case Tokenizer::TOKEN_LEFT_BANANA:
            {
                tokenizer_.NextToken(current_token_);
                CreateValue();
                if (error_code_ != 0) {
                    return;
                }
                if (current_token_.token_type != Tokenizer::TOKEN_RIGHT_BANANA) {
                    error_code_ = 1;
                    return;
                }
                tokenizer_.NextToken(current_token_);
            }
            break;
        default:
            error_code_ = 1;
        }
    }

    void Parser::CreateRotator() {
        if (current_token_.token_type != Tokenizer::TOKEN_BANG) {
            return CreateAtom();
        }

        tokenizer_.NextToken(current_token_);
        CreateAtom();
        if (error_code_ != 0) {
            return;
        }

        Operator * atom = NULL;
        if (ast_tree_->PopLastChild(&atom) == false) {
            error_code_ = 1;
            return;
        }

        Not * not_op = new Not();
        not_op->AddChild(atom);
        ast_tree_->AddChild(not_op);
    }

    bool Parser::PopTwoFactors(Operator ** lhs, Operator ** rhs) {
        if (ast_tree_->PopLastChild(rhs) == false) {
            error_code_ = 1;
            return false;
        }

        if (ast_tree_->PopLastChild(lhs) == false) {
            error_code_ = 1;
            delete *rhs;
            return false;
        }
        return true;
    }

    void Parser::DivMulMod(op_allocator operator_allocator) {
        tokenizer_.NextToken(current_token_);
        CreateRotator();
        if (error_code_ != 0) {
            return;
        }

        Operator * rhs = NULL;
        Operator * lhs = NULL;
        if (PopTwoFactors(&lhs, &rhs) == false) {
            error_code_ = 1;
            return;
        }

        Operator * op = (this->*operator_allocator)();
        op->AddChild(lhs);
        op->AddChild(rhs);

        ast_tree_->AddChild(op);

        switch (current_token_.token_type) {
        case Tokenizer::TOKEN_DIV: return CreateDiv();
        case Tokenizer::TOKEN_MUL: return CreateMul();
        case Tokenizer::TOKEN_MOD: return CreateMod();
        default:
            return;
        }
    }

    Operator * Parser::AllocateDiv() {
        return new Div(ast_tree_->GetDefault());
    }

    void Parser::CreateDiv() {
        DivMulMod(&Parser::AllocateDiv);
    }

    Operator * Parser::AllocateMul() {
        return new Mul();
    }

    void Parser::CreateMul() {
        DivMulMod(&Parser::AllocateMul);
    }

    Operator * Parser::AllocateMod() {
        return new Mod();
    }

    void Parser::CreateMod() {
        DivMulMod(&Parser::AllocateMod);
    }

    void Parser::CreateFactor() {
        CreateRotator();
        if (error_code_ != 0) {
            return;
        }

        switch (current_token_.token_type) {
        case Tokenizer::TOKEN_DIV: return CreateDiv();
        case Tokenizer::TOKEN_MUL: return CreateMul();
        case Tokenizer::TOKEN_MOD: return CreateMod();
        default:
            return;
        }
    }

    void Parser::CreateNegative() {
        Operator * expr = NULL;
        if (ast_tree_->PopLastChild(&expr) == false) {
            error_code_ = 1;
            return;
        }

        Negative * neg = new Negative();
        neg->AddChild(expr);
        ast_tree_->AddChild(neg);
    }

    void Parser::CreateSymbol() {
        bool negative = false;
        if (current_token_.token_type == Tokenizer::TOKEN_ADD ||
            current_token_.token_type == Tokenizer::TOKEN_SUB) {
            if (current_token_.token_type == Tokenizer::TOKEN_SUB) {
                negative = true;
            }
            tokenizer_.NextToken(current_token_);
        }

        CreateFactor();
        if (error_code_ != 0) {
            return;
        }

        if (negative) {
            CreateNegative();
        }

        if (current_token_.token_type != Tokenizer::TOKEN_ADD &&
            current_token_.token_type != Tokenizer::TOKEN_SUB) {
            return;
        }

        Operator * add = new Add();
        Operator * addend = NULL;
        if (ast_tree_->PopLastChild(&addend) == false) {
            error_code_ = 1;
            delete add;
            return;
        }
        add->AddChild(addend);

        while (current_token_.token_type == Tokenizer::TOKEN_ADD ||
               current_token_.token_type == Tokenizer::TOKEN_SUB) {
            negative = current_token_.token_type == Tokenizer::TOKEN_SUB ? true : false;

            tokenizer_.NextToken(current_token_);
            CreateFactor();
            if (error_code_ != 0) {
                delete add;
                return;
            }

            if (negative) {
                CreateNegative();
            }

            if (ast_tree_->PopLastChild(&addend) == false) {
                error_code_ = 1;
                delete add;
                return;
            }
            add->AddChild(addend);
        }
        ast_tree_->AddChild(add);
    }

    void Parser::CreateCmp() {
        CreateSymbol();
        if (error_code_ != 0) {
            return;
        }

        Operator * symbol = NULL;
        if (ast_tree_->PopLastChild(&symbol) == false) {
            error_code_ = 1;
            return;
        }

        Operator * cmp = NULL;
        switch (current_token_.token_type) {
        case Tokenizer::TOKEN_LT: cmp = new Less(); break;
        case Tokenizer::TOKEN_LE: cmp = new LessEqual(); break;
        case Tokenizer::TOKEN_GT: cmp = new Greater(); break;
        case Tokenizer::TOKEN_GE: cmp = new GreaterEqual(); break;
        case Tokenizer::TOKEN_EQ: cmp = new Equal(); break;
        case Tokenizer::TOKEN_NEQ: cmp = new NotEqual(); break;
        default:
            ast_tree_->AddChild(symbol);
            return;
        }

        cmp->AddChild(symbol);

        tokenizer_.NextToken(current_token_);
        CreateSymbol();
        if (error_code_ != 0) {
            delete cmp;
            return;
        }

        if (ast_tree_->PopLastChild(&symbol) == false) {
            error_code_ = 1;
            delete cmp;
            return;
        }

        cmp->AddChild(symbol);
        ast_tree_->AddChild(cmp);
    }

    void Parser::CreateExpr() {
        CreateCmp();
        if (error_code_ != 0 || current_token_.token_type != Tokenizer::TOKEN_AND) {
            return;
        }

        And * and_operator = new And();
        Operator * cond = NULL;
        if (ast_tree_->PopLastChild(&cond) == false) {
            error_code_ = 1;
            delete and_operator;
            return;
        }
        and_operator->AddChild(cond);

        while (current_token_.token_type == Tokenizer::TOKEN_AND) {
            tokenizer_.NextToken(current_token_);
            CreateCmp();
            if (error_code_ != 0) {
                delete and_operator;
                return;
            }

            if (ast_tree_->PopLastChild(&cond) == false) {
                error_code_ = 1;
                delete and_operator;
                return;
            }
            and_operator->AddChild(cond);
        }
        ast_tree_->AddChild(and_operator);
    }

    void Parser::CreateValue() {
        CreateExpr();
        if (error_code_ != 0 || current_token_.token_type != Tokenizer::TOKEN_OR) {
            return;
        }

        Or * or_operator = new Or();
        Operator * cond = NULL;
        if (ast_tree_->PopLastChild(&cond) == false) {
            error_code_ = 1;
            delete or_operator;
            return;
        }
        or_operator->AddChild(cond);

        while (current_token_.token_type == Tokenizer::TOKEN_OR) { // more conditions
            tokenizer_.NextToken(current_token_);
            CreateExpr();
            if (error_code_ != 0) {
                delete or_operator;
                return;
            }

            if (ast_tree_->PopLastChild(&cond) == false) {
                error_code_ = 1;
                delete or_operator;
                return;
            }
            or_operator->AddChild(cond);
        }
        ast_tree_->AddChild(or_operator);
    }

    void Parser::CreateSentence() {
        switch (current_token_.token_type) {
        case Tokenizer::TOKEN_EOL:
            return;
        case Tokenizer::TOKEN_NAME:
            {
                std::string name(current_token_.token_pos, current_token_.token_length);
                if (name == "if") {
                    return CreateIf();
                } else if (name == "return") {
                    return CreateReturn();
                }

                Token t;
                tokenizer_.NextToken(t);
                switch (t.token_type) {
                case Tokenizer::TOKEN_ASSIGN:       // go through
                case Tokenizer::TOKEN_ADD_ASSIGN:   // go through
                case Tokenizer::TOKEN_SUB_ASSIGN:   // go through
                case Tokenizer::TOKEN_DIV_ASSIGN:   // go through
                case Tokenizer::TOKEN_MUL_ASSIGN:   // go through
                case Tokenizer::TOKEN_MOD_ASSIGN:   // go through
                    tokenizer_.PushBack(t);
                    return CreateVariable(name);
                default:
                    tokenizer_.PushBack(t);
                    break;
                }
                return CreateValue();
            }
        default:
            break;
        }
        return CreateValue();
    }

    const char * Parser::ErrorMsg() const {
        return ERROR_MESSAGE[error_code_];
    }

    void Parser::ErrorContext(std::string& msg) const {
        if (error_code_ != 0) {
            int pos = tokenizer_.Context(msg);
            msg += std::string("\n") + std::string(pos, ' ') + std::string("^");
        }
    }

}  // ttl
