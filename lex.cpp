#include "lex.h"
#include "token.h"
#include "charAnalyzer.h"

namespace MI
{

CLexer::StringMap CLexer::keywordMap_;
CLexer::StringMap CLexer::operatorMap_;



void ILexer::Exception::makeReason()
{
	char str[256];
	snprintf(str,256,"(%d)：%s：%s",this->lineNumber_,word_.c_str(),reason_.c_str());
	what_ = str;
}

const char* ILexer::Exception::what() const throw()
{
	return what_.c_str();
}

ILexer::Exception::Exception(int lineNum,const std::string& word,const std::string& reason) 
:	std::exception(),
	word_(word),reason_(reason),lineNumber_(lineNum)
{
	makeReason();
}



CLexer::CLexer(const char* text) 
    : makeTokenRet_(0),text_(text),pt_(text),nowPos_(0),nowLine_(0),topToken_(NULL)
{
    static bool bInit=true;
    if(bInit)
    {
        bInit = false;
    }
}

CLexer::~CLexer()
{
    delete topToken_;
}

// makeToken {{{
int CLexer::makeToken()
{
    //if(isEOF()) return ILexer::RET_EOF;
    
    CToken* token=NULL;
    int ret = ILexer::RET_NORMAL; 
    // トークンの作成
    while(token == NULL)
    {
        skipBlanks();
        if(isComment())
        {
            skipLine();
            continue;
        }

        if( (token = getEOF()) != NULL){ret = ILexer::RET_EOF; break;}
        if( (token = getEOL()) != NULL){ret = ILexer::RET_EOL; nowLine_++;break;}
        
        // トークン取得開始
        if( (token = getKeyword())    != NULL) break; 
        if( (token = getOperator())   != NULL) break;
        if( (token = getIdentifier()) != NULL) break;
		if( (token = getLiteral('\'',false)) != NULL) break;
        if( (token = getLiteral('"',true))   != NULL) break;
        if( (token = getConstant())   != NULL) break;
        if( (token = getSeparator())  != NULL) break;

        // エラー処理
		onAnalyzeError();
        
    }
    
    topToken_ = token;
    makeTokenRet_ = ret;
    
    return ret;
}
// }}}

const CToken* CLexer::top()const
{
    return topToken_;
}

CToken* CLexer::pop()
{
    CToken* ret = topToken_;
    makeToken();
    return ret;
}

void CLexer::skipToken()
{
	CToken* ret = topToken_;
	makeToken();
	delete ret;
}

bool CLexer::isEOL()const
{
    return makeTokenRet_ == ILexer::RET_EOL;
}

bool CLexer::isEOF()const
{
    return makeTokenRet_ == ILexer::RET_EOF;
}

int CLexer::skipBlanks()
{
    const char* pt = pt_;
    CCharAnalyzer obj;

    while(obj.isSpace(*pt)){
        ++pt;
    }
    
    return endTokenAnalyze(pt_,pt); 
}

int CLexer::skipLine()
{
    const char* pt = pt_;
    CCharAnalyzer obj;

    while(!obj.isEOL(*pt)){
        ++pt;
    }
    if(*pt != '\0')
	{
        pt++;
		nowLine_++;
	}
	return endTokenAnalyze(pt_,pt);
}

CToken* CLexer::getIdentifier()
{
    const char* pt = pt_;
    CCharAnalyzer obj;
    CToken* token=NULL;
    
    if(obj.isAlphabet(*pt) || *pt == '_')
    {
        do
        {
            ++pt;
        }while(obj.isIdentifierChar(*pt));
        token = new CToken(
                    CToken::TK_IDENTIFIER,
                    0,pt_,pt - pt_        );
        endTokenAnalyze(pt_,pt);
    }
    
    return token;
}

CToken* CLexer::getConstant()
{
    const char* pt = pt_;
    CCharAnalyzer obj;
    CToken* token=NULL;
    
    if(obj.isNumber(*pt))
    {
        do
        {
            ++pt;
        }while(obj.isNumber(*pt));
        token = new CToken(
                    CToken::TK_CONSTANT,
                    0,pt_,pt - pt_        );

        endTokenAnalyze(pt_,pt);
    }
    
    return token;
}

CToken* CLexer::getHex()
{
	const char* pt = pt_;
	CToken* token = NULL;
	class IsHex{
	public:
		bool operator()(char c)const{
			return '0' <= c && c <= '9' || 'A' <= c && c <= 'F' || 'a' <= c && c <= 'f';
		}
	};
	IsHex isHex;
	if(*pt == '#')
	{
		do
        {
            ++pt;
        }while(isHex(*pt));
        token = new CToken(
                    CToken::TK_CONSTANT,
					CToken::CT_HEX,pt_,pt - pt_        );
        endTokenAnalyze(pt_,pt);
	}
	return token;
}

CToken* CLexer::getLiteral(char sep,bool bEscape)
{
    const char* pt = pt_;
    CCharAnalyzer obj;
    CToken* token=NULL;
    
    if(*pt == sep)
    {
        do
        {
            ++pt;
            // SJISはスキップ
            /*if(obj.isSJIS1st(*pt))
            {
                ++pt;
            }
            */
            // "を見つけたときはエスケープシーケンスかどうか
            // 確かめる
            /*else*/ if(*pt == sep && *(pt-1) != '\\')
            {
                break;
            }
        }while(!obj.isEOL(*pt));
        token = new CToken(
                    CToken::TK_LITERAL,
                    0,pt_+1,pt -(pt_+1));
        if(*pt == sep) pt++;
		else onLiteralError();
        endTokenAnalyze(pt_,pt);
    }
    
    return token;
}

CToken* CLexer::getKeyword()
{
    StringMap::iterator it;
    CCharAnalyzer obj;
    size_t len;
    CToken* token=NULL;
    
    it = keywordMap_.begin();
    
    while(it != keywordMap_.end())
    {
        len = (*it).first.length();
        // if ... {{{
        if( 
            strncmp((*it).first.c_str(),pt_,len) == 0 &&
            !obj.isIdentifierChar(*(pt_+len)) )
         {
            //keywordの文字列とマッチして
            //その次の文字が識別子の文字列じゃないなら
            //キーワード
            token = new CToken(
                CToken::TK_KEYWORD,
                (*it).second,
                (*it).first.c_str(),len);
            endTokenAnalyze(pt_,pt_ + len );
            break;
        }
        // }}}
        ++it;
    }
    return token;
}

CToken* CLexer::getOperator()
{
    CToken* token = NULL;
    const char* pt = pt_;
    int len=0;
    int subType;
    

    switch(*pt)
    {
    case '+':
        len = 1;
        if(topToken_ == NULL ||
           topToken_->getType() == CToken::TK_OPERATOR ||
           topToken_->getType() == CToken::TK_KEYWORD  ||
           topToken_->getType() == CToken::TK_EOL)
        {
            // 単項のプラス
            subType = CToken::OP_PLUS_SINGLE;
        }
        else
        {
            // 2項演算子
            subType = CToken::OP_PLUS;
        }
        break;
    case '-':
        len = 1;
        if(topToken_ == NULL ||
           topToken_->getType() == CToken::TK_OPERATOR ||
           topToken_->getType() == CToken::TK_KEYWORD  ||
           topToken_->getType() == CToken::TK_EOL)
        {
            // 単項のプラス
            subType = CToken::OP_MINUS_SINGLE;
        }
        else
        {
            // 2項演算子
            subType = CToken::OP_MINUS;
        }
        break;
    case '*':
        len = 1;
        subType = CToken::OP_MUL;
        break;
    case '/':
        len = 1;
        subType = CToken::OP_DIV;
        break;
    case '=':
        if( *(pt+1) == '=')
        {
            //len = 2;
            //subType = CToken::OP_EQUAL;
        }
        else
        {
            len = 1;
            subType = CToken::OP_ASSIGN;
        }
        break;
    case '>':
    case '<':
    case '|':
    case '&':
        break;
    }

    if(len != 0)
    {
        token = new CToken(CToken::TK_OPERATOR,subType,pt,len);
        endTokenAnalyze(pt_,pt+len);
    }
    
    return token;
}

CToken* CLexer::getSeparator()
{
    CToken* token = NULL;
    const char* pt = pt_;
	static const char* gsepChar =",.()[]{}:;";

    int i=0;
    for(i=0;i<sizeof(gsepChar) / sizeof(char);++i)
    {
        if(*pt == gsepChar[i])
        {
            token = new CToken(CToken::TK_SEPARATOR,i,pt_,1);
            endTokenAnalyze(pt_,pt+1);
            break;
        }
    }
    return token;
}

CToken* CLexer::getEOL()
{
    CCharAnalyzer obj;
    CToken* tok=NULL;
    if(obj.isEOL(*pt_))
    {
        tok = new CToken(CToken::TK_EOL,0,pt_,1);
        endTokenAnalyze(pt_,pt_+1);
    }
    return tok;
}

CToken* CLexer::getEOF()
{
    CToken* tok=NULL;
    if(*pt_ == '\0')
    {
        tok = new CToken(CToken::TK_EOF,0,pt_,1);
    }
    return tok;
}

bool CLexer::isComment()const
{
    return *pt_ == ';';
}

void CLexer::onAnalyzeError()
{
	onError(pt_,-1,"解析できない文字列です");
}

void CLexer::onLiteralError()
{
	onError(pt_,-1,"文字列リテラルが閉じられていません");
}

void CLexer::onError(const char* top,int n,const char* reason)
{
	const char* pt = top;
	std::string err;
	CCharAnalyzer obj;
	while( n != 0 && !(obj.isSpace(*pt) || obj.isEOL(*pt)) )
	{
		--n;
		++pt;
	}
	err.assign(pt_,pt-pt_);
	throw ILexer::Exception(this->nowLine_,err,reason);
}

} // end namespace mi
