#ifndef __MI_INTERPRETER_LEX_H__
#define __MI_INTERPRETER_LEX_H__

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <exception>
#include <string>
#include <list>
#include <vector>
#include <stack>
#include <map>
#include <algorithm>
#include <set>

namespace MI
{
class CToken;

class ILexer
{
public:
    enum{ RET_NORMAL,RET_EOL,RET_EOF,};
    virtual const CToken* top()const=0;
	virtual CToken* pop()=0;
    virtual int makeToken()=0;
    virtual bool isEOL()const=0;
    virtual bool isEOF()const=0;
	virtual int skipLine()=0;
	virtual void skipToken()=0;
	virtual ~ILexer(){}

	class Exception : public std::exception
	{
	public:
		virtual const char* what() const throw();

		Exception(int lineNum,const std::string& word,const std::string& reason);
		virtual ~Exception()throw(){}
	protected:
		std::string word_,reason_;
		int lineNumber_;
		std::string what_;
	protected:
		void makeReason();
	};
};


class CLexer : public ILexer
{
    typedef std::map<std::string,unsigned int> StringMap;
public:
	virtual CToken* pop();
    virtual const CToken* top()const;
    
    /// トークンを作成する
    virtual int makeToken();
    virtual bool isEOF()const;
    virtual bool isEOL()const;

    CToken* getIdentifier();   ///< 識別子を切り出す
    CToken* getConstant();      ///< 数値定数を切り出す
	CToken* getHex();		   ///< 16進数定数を切り出す
    CToken* getLiteral(char sep,bool bEscape);       ///< 文字列リテラルを切り出す
    CToken* getKeyword();       ///< キーワードを切り出す
    CToken* getOperator();      ///< 演算子を切り出す
    CToken* getSeparator();     ///< 区切り文字を切り出す
    CToken* getEOF();           ///< EOFトークンを作成する
    CToken* getEOL();           ///< 改行トークンを作成する
    int skipBlanks();
    virtual int skipLine();
	virtual void skipToken();
    bool isComment()const;
   
    CLexer(){}
    explicit CLexer(const char* text);
    ~CLexer();
protected:
    const char* text_;     // 解析対象の文字列
    const char* pt_;       // 現在、解析している位置のポインタ
    int   nowPos_;         // 現在、何文字目を指しているか
    int   nowLine_;        // 現在、何行目か

    int   makeTokenRet_;    
    CToken*     topToken_; // 現在、保持しているトークン
    static StringMap keywordMap_;
    static StringMap operatorMap_;

protected:
    int endTokenAnalyze(const char* a,const char* b)
    {
        int len = b - a;
        pt_ = b;
        nowPos_ += len;
        return len;
    }

	void onError(const char* top,int n,const char* reason);
	void onLiteralError();
	void onAnalyzeError();
};


}


#endif //__MI_INTERPRETER_LEX_H__
