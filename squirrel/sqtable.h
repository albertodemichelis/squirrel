/*  see copyright notice in squirrel.h */
#ifndef _SQTABLE_H_
#define _SQTABLE_H_
/*
* The following code is based on Lua 4.0 (Copyright 1994-2002 Tecgraf, PUC-Rio.)
* http://www.lua.org/copyright.html#4
* http://www.lua.org/source/4.0.1/src_ltable.c.html
*/

#include "sqstring.h"
#include "vartrace.h"

#define hashptr(p)  (SQHash((SQInteger(p) >> 4)))

inline SQHash HashObj(const SQObject &key)
{
    switch(sq_type(key)) {
        case OT_STRING:     return _string(key)->_hash;
        case OT_FLOAT:      return (SQHash)((SQInteger)_float(key));
        case OT_BOOL: case OT_INTEGER:  return (SQHash)((SQInteger)_integer(key));
        default:            return hashptr(key._unVal.pRefCounted);
    }
}

struct SQTable : public SQDelegable
{
private:
    friend struct SQVM;
    struct _HashNode
    {
        _HashNode() { next = NULL; }
        SQObjectPtr val;
        SQObjectPtr key;
        _HashNode *next;
        VT_DECL_SINGLE;
    };
    _HashNode *_firstfree;
    _HashNode *_nodes;
    uint32_t _numofnodes_minus_one;
    uint32_t _usednodes;
    SQAllocContext _alloc_ctx;

///////////////////////////
    void AllocNodes(SQInteger nSize);
    void Rehash(bool force);
    SQTable(SQSharedState *ss, SQInteger nInitialSize);
    void _ClearNodes();
public:
    static SQTable* Create(SQSharedState *ss,SQInteger nInitialSize)
    {
        SQTable *newtable = (SQTable*)SQ_MALLOC(ss->_alloc_ctx, sizeof(SQTable));
        new (newtable) SQTable(ss, nInitialSize);
        newtable->_delegate = NULL;
        return newtable;
    }
    void Finalize();
    SQTable *Clone();
    ~SQTable()
    {
        SetDelegate(NULL);
        REMOVE_FROM_CHAIN(&_sharedstate->_gc_chain, this);
        for (uint32_t i = 0; i <= _numofnodes_minus_one; i++)
          _nodes[i].~_HashNode();
        SQ_FREE(_alloc_ctx, _nodes, (_numofnodes_minus_one + 1) * sizeof(_HashNode));
    }
#ifndef NO_GARBAGE_COLLECTOR
    void Mark(SQCollectable **chain);
    SQObjectType GetType() {return OT_TABLE;}
#endif
    inline _HashNode *_GetStr(const SQRawObjectVal key, SQHash hash) const
    {
        _HashNode *n = &_nodes[hash];
        do{
            if(_rawval(n->key) == key && sq_type(n->key) == OT_STRING){
                return n;
            }
        }while((n = n->next));
        return NULL;
    }
    inline _HashNode *_Get(const SQObjectPtr &key, SQHash hash) const
    {
        _HashNode *n = &_nodes[hash];
        do{
            if(_rawval(n->key) == _rawval(key) && sq_type(n->key) == sq_type(key)){
                return n;
            }
        }while((n = n->next));
        return NULL;
    }
    //for compiler use
    inline bool GetStr(const SQChar* key,SQInteger keylen,SQObjectPtr &val) const
    {
        SQHash hash = _hashstr(key,keylen);
        _HashNode *n = &_nodes[hash & _numofnodes_minus_one];
        _HashNode *res = NULL;
        do{
            if (sq_type(n->key) == OT_STRING &&
               (keylen == _string(n->key)->_len && scstrncmp(_stringval(n->key), key, keylen) == 0))
            {
                res = n;
                break;
            }
        }while((n = n->next));
        if (res) {
            val = _realval(res->val);
            return true;
        }
        return false;
    }
    _HashNode *_Get(const SQObjectPtr &key) const;
    bool Get(const SQObjectPtr &key,SQObjectPtr &val) const;

    VT_CODE(VarTrace * GetVarTracePtr(const SQObjectPtr &key));

    void Remove(const SQObjectPtr &key);
    bool Set(const SQObjectPtr &key, const SQObjectPtr &val);
    //returns true if a new slot has been created false if it was already present
    bool NewSlot(const SQObjectPtr &key,const SQObjectPtr &val  VT_DECL_ARG_DEF);
    SQInteger Next(bool getweakrefs,const SQObjectPtr &refpos, SQObjectPtr &outkey, SQObjectPtr &outval);

    SQInteger CountUsed(){ return _usednodes;}
    void Clear();
    void Release()
    {
        sq_delete(_alloc_ctx, this, SQTable);
    }

};

#endif //_SQTABLE_H_
