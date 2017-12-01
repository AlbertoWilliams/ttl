/**
 * tokenizer.h - tokenizer
 *
 * Author: Bao Hexing <HexingB@qq.com>
 * Created: 13 November 2017
 *
 * Copyright © 2017, Bao Hexing. All Rights Reserved.
 */

#ifndef TTL_TOKENIZER_H
#define TTL_TOKENIZER_H

namespace ttl {

    struct Token {
        long token_type;
        const char * token_pos;
        int token_length;
    };

    class Tokenizer {
    public:
        Tokenizer(const char * buffer);
        void NextToken(Token & t);
        bool Expect(long type);
        // push back the token(after this, call NextToken will return 't')
        bool PushBack(const Token& t);
        void Reset(const char * buffer);
        unsigned int ProcessedLength() const;
        int Context(std::string& c) const;

    private:
        void TwoCharSymbolToken(Token& token,
                                long type,
                                const char * beginning,
                                const char expected);
        void GetToken(Token & t, long type, const char * pos, int length);

    public:
        const static long TOKEN_EOL = '\0';
        const static long TOKEN_BANG = '!';
        const static long TOKEN_MOD = '%';
        const static long TOKEN_LEFT_BANANA = '(';
        const static long TOKEN_RIGHT_BANANA = ')';
        const static long TOKEN_MUL = '*';
        const static long TOKEN_ADD = '+';
        const static long TOKEN_SUB = '-';
        const static long TOKEN_DIV = '/';
        const static long TOKEN_SEMICOLON = ';';
        const static long TOKEN_LT = '<';
        const static long TOKEN_ASSIGN = '=';
        const static long TOKEN_GT = '>';
        const static long TOKEN_LEFT_TORUS = '{';
        const static long TOKEN_RIGHT_TORUS = '}';
        const static long TOKEN_NEQ = ('!' << 8) + '=';
        const static long TOKEN_MOD_ASSIGN = ('%' << 8) + '=';
        const static long TOKEN_AND = ('&' << 8) + '&';
        const static long TOKEN_MUL_ASSIGN = ('*' << 8) + '=';
        const static long TOKEN_ADD_ASSIGN = ('+' << 8) + '=';
        const static long TOKEN_SUB_ASSIGN = ('-' << 8) + '=';
        const static long TOKEN_DIV_ASSIGN = ('/' << 8) + '=';
        const static long TOKEN_LE = ('<' << 8) + '=';
        const static long TOKEN_EQ = ('=' << 8) + '=';
        const static long TOKEN_GE = ('>' << 8) + '=';
        const static long TOKEN_OR = ('|' << 8) + '|';

        const static long TOKEN_NAME = 1 << 16;
        const static long TOKEN_NUM = 1 << 17;

    private:
        const char * buffer_;
        const char * pos_;
    };

}

#endif
