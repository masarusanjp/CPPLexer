#ifndef __INTERPRETER_TOKEN_H__
#define __INTERPRETER_TOKEN_H__

#include <string>

namespace MI {

class CToken
{
public:
    enum{
        TK_CONSTANT,
        TK_LITERAL,
        TK_IDENTIFIER,
        TK_KEYWORD,
        TK_OPERATOR,
        TK_SEPARATOR,
        TK_EOF,
        TK_EOL,
    };
	enum
	{
		CT_DECIMAL,
		CT_BINARY,
		CT_HEX,
	};
    enum{
        OP_EQUAL,
        OP_ASSIGN,                  // 代入(=)
        OP_PLUS,OP_PLUS_SINGLE,     // プラス(+)(計算としての+と符号としての+)
        OP_MINUS,OP_MINUS_SINGLE,   // 計算の-と符号の-
        OP_MUL,OP_DIV,              // 乗算(*)と除算(/)
    };
    enum{
        SP_COMMA,SP_DOT,               // .,
        SP_L_PARENTESIS,SP_R_PARENTSIS,// ()
        SP_L_BRACKET,SP_R_BRACKET,     // []
        SP_L_BRACE,SP_R_BRACE,         // {}
        SP_COLON,SP_SEMICOLON,         // :;
    };
        
    

    int getTokenInt()const{return 0;}
    const std::string& getTokenString()const{return str_;}

    //-- accessor ----------------------------------------------
    int getType()const{return type_;}
    int getSubType()const{return subType_;}

    //-- operator ----------------------------------------------

    //-- construct, destruct -----------------------------------
    CToken(int type,int subType,const char* str,int n) : type_(type),subType_(subType)
    {
        str_.assign(str,n);
    }
    ~CToken(){}
protected:
    std::string str_;
    int type_;
    int subType_;
};


}


#endif
