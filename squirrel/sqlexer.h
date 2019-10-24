/*  see copyright notice in squirrel.h */
#ifndef _SQLEXER_H_
#define _SQLEXER_H_

#ifdef SQUNICODE
typedef SQChar LexChar;
#else
typedef unsigned char LexChar;
#endif

struct SQLexerMacroState
{
    bool insideStringInterpolation;
    SQLEXREADFUNC prevReadF;
    SQUserPointer prevUserPointer;
    LexChar prevCurrdata;
    sqvector<SQChar> macroStr;
    sqvector<SQChar> macroParams;
    int macroStrPos;

    SQLexerMacroState(SQAllocContext ctx)
        : macroStr(ctx), macroParams(ctx)
    {
        reset();
    }

    void reset()
    {
        insideStringInterpolation = false;
        prevReadF = NULL;
        prevUserPointer = NULL;
        macroStrPos = 0;
        prevCurrdata = 0;
        macroStr.resize(0);
        macroParams.resize(0);
    }

    static SQInteger macroReadF(SQUserPointer self)
    {
        SQLexerMacroState * s = (SQLexerMacroState *) self;
        return s->macroStr[s->macroStrPos++];
    }
};


struct SQLexer
{
    SQLexer(SQSharedState *ss);
    ~SQLexer();
    void Init(SQSharedState *ss,SQLEXREADFUNC rg,SQUserPointer up,CompilerErrorFunc efunc,void *ed);
    void Error(const SQChar *err);
    SQInteger Lex();
    const SQChar *Tok2Str(SQInteger tok);
private:
    SQInteger LexSingleToken();
    SQInteger GetIDType(const SQChar *s,SQInteger len);
    SQInteger ReadString(SQInteger ndelim,bool verbatim);
    SQInteger ReadNumber();
    void LexBlockComment();
    void LexLineComment();
    SQInteger ReadID();
    SQInteger ReadDirective();
    void Next();
    void AppendPosDirective(sqvector<SQChar> & vec);
    bool ProcessReaderMacro();
    void ExitReaderMacro();
#ifdef SQUNICODE
#if WCHAR_SIZE == 2
    SQInteger AddUTF16(SQUnsignedInteger ch);
#endif
#else
    SQInteger AddUTF8(SQUnsignedInteger ch);
#endif
    SQInteger ProcessStringHexEscape(SQChar *dest, SQInteger maxdigits);
    SQInteger _curtoken;
    SQTable *_keywords;
    SQBool _reached_eof;
    SQLexerMacroState macroState;
public:
    SQInteger _prevtoken;
    SQInteger _currentline;
    SQInteger _lasttokenline;
    SQInteger _currentcolumn;
    const SQChar *_svalue;
    SQInteger _nvalue;
    SQFloat _fvalue;
    SQLEXREADFUNC _readf;
    SQUserPointer _up;
    LexChar _currdata;
    SQSharedState *_sharedstate;
    sqvector<SQChar> _longstr;
    CompilerErrorFunc _errfunc;
    void *_errtarget;
};

#endif
