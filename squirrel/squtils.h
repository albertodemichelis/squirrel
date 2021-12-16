/*  see copyright notice in squirrel.h */
#ifndef _SQUTILS_H_
#define _SQUTILS_H_

#include <memory>

typedef struct SQAllocContextT * SQAllocContext;

void sq_vm_init_alloc_context(SQAllocContext * ctx);
void sq_vm_destroy_alloc_context(SQAllocContext * ctx);
void sq_vm_assign_to_alloc_context(SQAllocContext ctx, HSQUIRRELVM vm);
void *sq_vm_malloc(SQAllocContext ctx, SQUnsignedInteger size);
void *sq_vm_realloc(SQAllocContext ctx, void *p,SQUnsignedInteger oldsize,SQUnsignedInteger size);
void sq_vm_free(SQAllocContext ctx, void *p,SQUnsignedInteger size);

#define sq_new(__ctx,__ptr,__type, ...) {__ptr=(__type *)sq_vm_malloc((__ctx),sizeof(__type));new (__ptr) __type(__VA_ARGS__);}
#define sq_delete(__ctx,__ptr,__type) {__ptr->~__type();sq_vm_free((__ctx),__ptr,sizeof(__type));}
#define SQ_MALLOC(__ctx,__size) sq_vm_malloc((__ctx),(__size));
#define SQ_FREE(__ctx,__ptr,__size) sq_vm_free((__ctx),(__ptr),(__size));
#define SQ_REALLOC(__ctx,__ptr,__oldsize,__size) sq_vm_realloc((__ctx),(__ptr),(__oldsize),(__size));

#define sq_aligning(v) (((size_t)(v) + (SQ_ALIGNMENT-1)) & (~(SQ_ALIGNMENT-1)))

//sqvector mini vector class, supports objects by value
template<typename T> class sqvector
{
public:
    sqvector(SQAllocContext ctx)
    {
        _vals = NULL;
        _size = 0;
        _allocated = 0;
        _alloc_ctx = ctx;
    }
    sqvector(const sqvector<T>& v)
    {
        copy(v);
    }
    void copy(const sqvector<T>& v)
    {
        if(_size) {
            resize(0); //destroys all previous stuff
        }
        //resize(v._size);
        if(v._size > _allocated) {
            _realloc(v._size);
        }
        for(SQUnsignedInteger i = 0; i < v._size; i++) {
            new ((void *)&_vals[i]) T(v._vals[i]);
        }
        _size = v._size;
        _alloc_ctx = v._alloc_ctx;
    }
    ~sqvector()
    {
        if(_allocated) {
            for(SQUnsignedInteger i = 0; i < _size; i++)
                _vals[i].~T();
            SQ_FREE(_alloc_ctx, _vals, (_allocated * sizeof(T)));
        }
    }
    void reserve(SQUnsignedInteger newsize) { _realloc(newsize); }
    void resize(SQUnsignedInteger newsize, const T& fill = T())
    {
        if(newsize > _allocated)
            _realloc(newsize);
        if(newsize > _size) {
            while(_size < newsize) {
                new ((void *)&_vals[_size]) T(fill);
                _size++;
            }
        }
        else{
            for(SQUnsignedInteger i = newsize; i < _size; i++) {
                _vals[i].~T();
            }
            _size = newsize;
        }
    }
    void shrinktofit() { if(_size > 4) { _realloc(_size); } }
    T& top() const { return _vals[_size - 1]; }
    inline SQUnsignedInteger size() const { return _size; }
    bool empty() const { return (_size <= 0); }
    inline T &push_back(const T& val = T())
    {
        if(_allocated <= _size)
            _realloc(_size * 2);
        return *(new ((void *)&_vals[_size++]) T(val));
    }
    inline void pop_back()
    {
        _size--; _vals[_size].~T();
    }
    void insert(SQUnsignedInteger idx, const T& val)
    {
        resize(_size + 1);
        for(SQUnsignedInteger i = _size - 1; i > idx; i--) {
            _vals[i] = _vals[i - 1];
        }
        _vals[idx] = val;
    }
    void remove(SQUnsignedInteger idx)
    {
        _vals[idx].~T();
        if(idx < (_size - 1)) {
            memmove(&_vals[idx], &_vals[idx+1], sizeof(T) * (_size - idx - 1));
        }
        _size--;
    }
    SQUnsignedInteger capacity() { return _allocated; }
    inline T &back() const { return _vals[_size - 1]; }
    inline T& operator[](SQUnsignedInteger pos) const{ return _vals[pos]; }
    T* _vals;
    SQAllocContext _alloc_ctx;
private:
    void _realloc(SQUnsignedInteger newsize)
    {
        newsize = (newsize > 0)?newsize:4;
        _vals = (T*)SQ_REALLOC(_alloc_ctx, _vals, _allocated * sizeof(T), newsize * sizeof(T));
        _allocated = newsize;
    }
    SQUnsignedInteger _size;
    SQUnsignedInteger _allocated;
};



class SQConstStringsCollection
{
  sqvector<const SQChar *> v;

public:
  SQConstStringsCollection(SQAllocContext ctx) : v(ctx) {}

  ~SQConstStringsCollection()
  {
    while (!v.empty())
    {
      delete[] v.top();
      v.pop_back();
    }
  }

  const SQChar * perpetuate(const SQChar * s)
  {
    if (!s)
      return perpetuate(_SC(""));

    for (int i = int(v.size()) - 1; i >= 0; i--)
    {
      if (scstrcmp(v[i], s) == 0)
        return v[i];
    }

    int count = int(scstrlen(s)) + 1;
    SQChar * res = new SQChar[count];
    memcpy(res, s, sizeof(SQChar) * count);

    v.push_back(res);

    return res;
  }
};

#endif //_SQUTILS_H_
