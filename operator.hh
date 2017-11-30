/**
 * operator.h - operators for abstract syntax tree
 *
 * Author: Bao Hexing <HexingB@qq.com>
 * Created:  7 November 2017
 *
 * Copyright © 2017, Bao Hexing. All Rights Reserved.
 */

#ifndef TTL_OPERATOR_H
#define TTL_OPERATOR_H

#include <vector>
#include <map>
#include <string>
#include <iostream>
#include "common.hh"

namespace ttl {

    class Operator {
    public:

        Operator()  : children_() {}

        ~Operator() {
            for (int i = 0; i < children_.size(); ++i) {
                delete children_[i];
            }
        }

        void AddChild(Operator * child) {
            children_.push_back(child);
        }

        virtual double Evaluate() = 0;

    protected:
        std::vector<Operator*> children_;
    };

    class Module : public Operator {
    public:
        Module(double default_value) : Operator(), variables_(), should_return_(false) {
            variables_.insert(make_pair(std::string("default"), default_value));
            variables_.insert(make_pair(std::string("return"), default_value));

            default_value_ = &(variables_.find("default")->second);
            return_value_ = &(variables_.find("return")->second);
        }

        ~Module() {}

        bool PopLastChild(Operator ** child) {
            if (child == NULL || children_.size() == 0) {
                return false;
            }

            *child = children_.back();
            children_.pop_back();
            return true;
        }

        double * GetReturn() const {
            return return_value_;
        }

        double GetDefault() const {
            return *default_value_;
        }

        double * GetVariable(const std::string& variable_name) {
            std::map<std::string, double>::iterator target = variables_.find(variable_name);
            if (target == variables_.end()) {
                return NULL;
            }
            return &(target->second);
        }

        double * CreateOrGetVariable(const std::string& variable_name) {
            std::pair<std::map<std::string, double>::iterator, bool> result =
                variables_.insert(make_pair(std::string(variable_name), *default_value_));
            return &(result.first->second);
        }

        virtual double Evaluate() {
            double value = *default_value_;
            for (std::vector<Operator*>::iterator it = children_.begin();
                 it != children_.end() && should_return_ == false; ++it) {
                value = (*it)->Evaluate();
            }

            // return 'return_value_' if "return" is called,
            // or return the value of last sentence(for offline tool)
            return should_return_ ? *return_value_ : value;
        }

        void Return(double value) {
            *return_value_ = value;
            should_return_ = true;
        }

    private:
        double * default_value_;
        double * return_value_;
        std::map<std::string, double> variables_;
        bool should_return_;
    };

    class Num : public Operator {
    public:
        Num(double value) : Operator(), value_(value) {}
        virtual double Evaluate() { return value_; }
    private:
        double value_;
    };

    class Variable : public Operator {
    public:
        Variable(double * reference) : reference_(reference) {}
        virtual double Evaluate() {
            return *reference_;
        }
    private:
        double * reference_;
    };

    class Reference : public Operator {
    public:
        Reference(Module * module,
                  const std::string& name,
                  double (*op)(double, double) = assign,
                  bool check_rhs = false)
            : Operator(),
              module_(module),
              reference_(NULL),
              is_return_(false),
              op_(op),
              check_rhs_(check_rhs) {
                  reference_ = module_->CreateOrGetVariable(name);
                  is_return_ = name == "return" ? true : false;
              }

        virtual double Evaluate() {
            if (is_return_) {
                module_->Return(children_[0]->Evaluate());
            } else {
                double lhs = *reference_;
                double rhs = children_[0]->Evaluate();
                *reference_ = (check_rhs_ && rhs == 0) ? module_->GetDefault() : op_(lhs, rhs);
            }
            return *reference_;
        }
    private:
        Module * module_;
        double * reference_;
        bool is_return_;
        bool check_rhs_;
        double (*op_)(double, double);
    };

    class Add : public Operator {
    public:
        virtual double Evaluate() {
            double value = 0.0;
            for (std::vector<Operator *>::iterator it = children_.begin();
                 it != children_.end(); ++it) {
                value += (*it)->Evaluate();
            }
            return value;
        }
    };

    class Negative : public Operator {
    public:
        virtual double Evaluate() {
            return - children_[0]->Evaluate();
        }
    };

    class If : public Operator {
    public:
        virtual double Evaluate() {
            if (children_[0]->Evaluate()) {
                return children_[1]->Evaluate();
            } else {
                return children_[2]->Evaluate();
            }
        }
    };

    class Or : public Operator {
    public:
        virtual double Evaluate() {
            for (std::vector<Operator *>::iterator it = children_.begin();
                 it != children_.end(); ++it) {
                double result = (*it)->Evaluate();
                if (result != 0) {
                    return (double)true;
                }
            }
            return (double)false;
        }
    };

    class And : public Operator {
    public:
        virtual double Evaluate() {
            for (std::vector<Operator *>::iterator it = children_.begin();
                 it != children_.end(); ++it) {
                if ((*it)->Evaluate() == 0) {
                    return (double)false;
                }
            }
            return (double)true;
        }
    };

    class Less : public Operator {
    public:
        virtual double Evaluate() {
            double lhs = children_[0]->Evaluate();
            double rhs = children_[1]->Evaluate();
            bool ret =  lhs < rhs;
            return (double)ret;
        }
    };

    class LessEqual : public Operator {
    public:
        virtual double Evaluate() {
            double lhs = children_[0]->Evaluate();
            double rhs = children_[1]->Evaluate();
            bool ret =  lhs <= rhs;
            return (double)ret;
        }
    };

    class Greater : public Operator {
    public:
        virtual double Evaluate() {
            double lhs = children_[0]->Evaluate();
            double rhs = children_[1]->Evaluate();
            bool ret =  lhs > rhs;
            return (double)ret;
        }
    };

    class GreaterEqual : public Operator {
    public:
        virtual double Evaluate() {
            double lhs = children_[0]->Evaluate();
            double rhs = children_[1]->Evaluate();
            bool ret =  lhs >= rhs;
            return (double)ret;
        }
    };

    class Equal : public Operator {
    public:
        virtual double Evaluate() {
            double lhs = children_[0]->Evaluate();
            double rhs = children_[1]->Evaluate();
            bool ret =  lhs == rhs;
            return (double)ret;
        }
    };

    class NotEqual : public Operator {
    public:
        virtual double Evaluate() {
            double lhs = children_[0]->Evaluate();
            double rhs = children_[1]->Evaluate();
            bool ret =  lhs != rhs;
            return (double)ret;
        }
    };

    class Div : public Operator {
    public:
        Div(double default_value) : Operator(), default_value_(default_value) {}

        virtual double Evaluate() {
            double divisor = children_[1]->Evaluate();
            if (divisor == 0) {
                std::cerr << "Divided by zero. Return default value "
                          << default_value_ << "." << std::endl;
                return default_value_;
            }
            return children_[0]->Evaluate() / divisor;
        }
    private:
        double default_value_;
    };

    class Mul : public Operator {
    public:
        virtual double Evaluate() {
            return children_[0]->Evaluate() * children_[1]->Evaluate();
        }
    };

    class Mod : public Operator {
    public:
        virtual double Evaluate() {
            return (long long)children_[0]->Evaluate() % (long long)children_[1]->Evaluate();
        }
    };

    class Not : public Operator {
    public:
        virtual double Evaluate() {
            return !children_[0]->Evaluate();
        }
    };
}

#endif
