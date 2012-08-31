#ifndef __MI_INTERPRETER_CHARANALYZER_H__
#define __MI_INTERPRETER_CHARANALYZER_H__

namespace MI
{

class CCharAnalyzer
{
public:
    inline bool isSpace(char c)const
    {
        return c == ' ' || c == '\t' || c == '\r';// || c == '\n';
    }
    inline bool isAlphabet(char c)const
    {
        return 'A' <= c && c <= 'Z' || 'a' <= c && c <= 'z';
    }
    inline bool isNumber(char c)const
    {
        return '0' <= c && c <= '9';
    }
    inline bool isIdentifierChar(char c)const
    {
        return isAlphabet(c) || isNumber(c) || c == '_';
    }

    inline bool isEOL(char c)const
    {
        return c == '\0' || c == '\n';
    }

    inline bool isSJIS1st(unsigned char c)const
    {
        return true;
    }

    inline char convertEscapeSequence(char c)const
    {
        char ret=0;
        switch(c)
        {
        case 'n':
            ret = '\n';
            break;
        case 'r':
            ret = '\r';
            break;
        case 't':
            ret = '\t';
            break;
        case '"':
            ret = '"';
            break;
        default:
            ret = c;
            break;
        }
        return ret;
    }
};



}

#endif

