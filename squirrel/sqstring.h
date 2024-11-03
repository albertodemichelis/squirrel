/*  see copyright notice in squirrel.h */
#ifndef _SQSTRING_H_
#define _SQSTRING_H_

inline SQHash _hashstr (const SQChar *s, size_t l)
{
	SQHash h = (SQHash)l;  /* seed */
	size_t step = (l >> 5) + 1;  /* if string is too long, don't hash all its chars */
	size_t l1;
	for (l1 = l; l1 >= step; l1 -= step)
		h = h ^ ((h << 5) + (h >> 2) + ((unsigned short)s[l1 - 1]));
	return h;
}

inline SQHash _hashstr2(const SQChar* as, size_t al, const SQChar* bs, size_t bl)
{
    size_t l = al + bl;
    SQHash h = (SQHash)l;  /* seed */
    SQInteger step = (SQInteger)((l >> 5) + 1);  /* if string is too long, don't hash all its chars */
    SQInteger l1 = (SQInteger)l;
    for (; l1 >= step; l1 -= step) {
        SQInteger idx = l1 - 1 - al;
        if (idx < 0) {
            break;
        }
        h = h ^ ((h << 5) + (h >> 2) + ((unsigned short)bs[idx]));
    }
    for (; l1 >= step; l1 -= step) {
        SQInteger idx = l1 - 1;
        h = h ^ ((h << 5) + (h >> 2) + ((unsigned short)as[idx]));
    }
    return h;
}

struct SQString : public SQRefCounted
{
    SQString(){}
    ~SQString(){}
public:
    static SQString *Create(SQSharedState *ss, const SQChar *, SQInteger len = -1 );
    static SQString* Concat(SQSharedState* ss, const SQChar* a, SQInteger alen, const SQChar* b, SQInteger blen);
    SQInteger Next(const SQObjectPtr &refpos, SQObjectPtr &outkey, SQObjectPtr &outval);
    void Release();
    SQSharedState *_sharedstate;
    SQString *_next; //chain for the string table
    SQInteger _len;
    SQHash _hash;
    SQChar _val[1];
};



#endif //_SQSTRING_H_
