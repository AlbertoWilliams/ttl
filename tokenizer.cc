/**
 * tokenizer.cc - tokenizer
 *
 * Author: Bao Hexing <HexingB@qq.com>
 * Created: 13 November 2017
 *
 * Copyright © 2017, Bao Hexing. All Rights Reserved.
 */

#include <iostream>
#include "tokenizer.hh"

namespace ttl {

    Tokenizer::Tokenizer(const char * buffer)
        : buffer_(buffer), pos_(buffer) {}

    void Tokenizer::Reset(const char * buffer) {
        buffer_ = buffer;
        pos_ = buffer;
    }

    unsigned int Tokenizer::ProcessedLength() const {
        return pos_ - buffer_;
    }

    int Tokenizer::Context(std::string& c) const {
        const char * end = pos_;
        for (int i = 0; i < 40 && (*end) != '\0' && (*end) != '\n'; ++i, ++end)
            ;

        const char * start = pos_;
        for (int i = 0; i < 40 && start != buffer_; ++i, --start)
            ;

        c = std::string(start, end - start);
        return pos_ - start;
    }

    void Tokenizer::GetToken(Token & t, long type, const char * pos, int length) {
        t.token_type = type;
        t.token_pos = pos;
        t.token_length = length;
        return;
    }

    bool Tokenizer::PushBack(const Token& t) {
        pos_ -= t.token_length;
        return pos_ >= buffer_;
    }

    void Tokenizer::TwoCharSymbolToken(Token& token,
                                       long type,
                                       const char * beginning,
                                       const char expected) {
        if (beginning != pos_) {
            return GetToken(token, type, beginning, pos_ - beginning);
        } else {
            GetToken(token, long(*pos_), pos_, 1);
            pos_++;
            if (*pos_ == expected) {
                token.token_type <<= 8;
                token.token_type += expected;
                pos_++;
                token.token_length = 2;
            }
        }
    }

    /**
     * get next one token.
     *
     * NOTE:
     *     0. whitespace are skiped, so caller don't need to trim the token;
     *     1. new line "\n" is processed, so caller don't bother to process this;
     */
    void Tokenizer::NextToken(Token& token) {
        long type = TOKEN_NAME;
        for (const char * beginning = pos_; true ; ++pos_) {
            switch (*pos_) {
            case ' ':
            case '\t':
                if (beginning == pos_) {
                    beginning = pos_ + 1;
                } else {
                    return GetToken(token, type, beginning, pos_ - beginning);
                }
                break;
            case '\0':
                if (beginning != pos_) {
                    return GetToken(token, type, beginning, pos_ - beginning);
                } else {
                    return GetToken(token, TOKEN_EOL, pos_, 1);
                }
            case '\n':
                if (beginning != pos_) {
                    return GetToken(token, type, beginning, pos_ - beginning);
                } else {
                    beginning = pos_ + 1;
                }
                break;
            case ';':
            case '(':
            case ')':
            case '{':
            case '}':
            case ',':
                if (beginning != pos_) {
                    return GetToken(token, type, beginning, pos_ - beginning);
                } else {
                    GetToken(token, long(*pos_), pos_, 1);
                    pos_++;
                    return;
                }
            case '!':
            case '+':
            case '-':
            case '*':
            case '/':
            case '%':
            case '<':
            case '>':
            case '=':
                return TwoCharSymbolToken(token, type, beginning, '=');
            case '&':
                return TwoCharSymbolToken(token, type, beginning, '&');
            case '|':
                return TwoCharSymbolToken(token, type, beginning, '|');
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                if (beginning == pos_) {
                    type = TOKEN_NUM;
                }
                break;
            default:
                break;
            }
        }
    }

    bool Tokenizer::Expect(long type) {
        Token token;
        do {
            NextToken(token);
        } while (token.token_type != TOKEN_EOL && token.token_type != type);
        return token.token_type == type;
    }

}  // ttl
