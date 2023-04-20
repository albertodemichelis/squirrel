/*  see copyright notice in squirrel.h */
#ifndef _SQUTILS_H_
#define _SQUTILS_H_

#include <memory>

#if defined(_WIN32)  || defined(_WIN64) 
#include <windows.h>
//#include <memoryapi.h>
#else // __unix__ 
#include <sys/mman.h>
#ifndef MAP_ANONYMOUS
#  define MAP_ANONYMOUS	0x20
#endif
#endif // defined(_WIN32)  || defined(_WIN64) 

#ifdef Yield
#undef Yield
#endif

#ifdef max
#undef max
#endif

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

#ifndef SQ_LIKELY
  #if (defined(__GNUC__) && (__GNUC__ >= 3)) || defined(__clang__)
    #if defined(__cplusplus)
      #define SQ_LIKELY(x)   __builtin_expect(!!(x), true)
      #define SQ_UNLIKELY(x) __builtin_expect(!!(x), false)
    #else
      #define SQ_LIKELY(x)   __builtin_expect(!!(x), 1)
      #define SQ_UNLIKELY(x) __builtin_expect(!!(x), 0)
    #endif
  #else
    #define SQ_LIKELY(x)   (x)
    #define SQ_UNLIKELY(x) (x)
  #endif
#endif

#define HAVE_STATIC_ASSERT() ((_MSC_VER >= 1600 && !defined(__INTEL_COMPILER)) || (__cplusplus > 199711L))

#if !defined(SQ_STATIC_ASSERT) && HAVE_STATIC_ASSERT()
#define SQ_STATIC_ASSERT(x) static_assert((x), "assertion failed: " #x)
#endif

#ifndef SQ_STATIC_ASSERT
#define SQ_STATIC_ASSERT(x) if (sizeof(char[2*((x)?1:0)-1])) ; else
#endif

#undef HAVE_STATIC_ASSERT

//sqvector mini vector class, supports objects by value
template<typename T> class sqvector
{
public:
    sqvector(SQAllocContext ctx)
        : _vals(NULL)
        ,  _size(0)
        , _allocated(0)
        , _alloc_ctx(ctx)
    {
    }
    sqvector(const sqvector<T>& v)
        : _vals(NULL)
        , _size(0)
        , _allocated(0)
        , _alloc_ctx(v._alloc_ctx)
    {
        copy(v);
    }
    void copy(const sqvector<T>& v)
    {
        if (_alloc_ctx != v._alloc_ctx) {
            _releasedata();
            _vals = NULL;
            _allocated = 0;
            _size = 0;
            _alloc_ctx = v._alloc_ctx;
        }
        else if(_size) {
            resize(0); //destroys all previous stuff
        }
        //resize(v._size);
        if(v._size > _allocated) {
            _realloc(v._size);
        }
        for(SQUnsignedInteger i = 0; i < v._size; i++) {
            new ((void *)&_vals[i]) T(v._vals[i]); //-V522
        }
        _size = v._size;
    }
    ~sqvector()
    {
        _releasedata();
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

    typedef T* iterator;
    typedef const T* const_iterator;

    iterator begin() { return &_vals[0]; }
    const_iterator begin() const { return &_vals[0]; }
    iterator end() { return &_vals[_size]; }
    const_iterator end() const { return &_vals[_size]; }
private:
    void _realloc(SQUnsignedInteger newsize)
    {
        newsize = (newsize > 0)?newsize:4;
        _vals = (T*)SQ_REALLOC(_alloc_ctx, _vals, _allocated * sizeof(T), newsize * sizeof(T));
        _allocated = newsize;
    }
    void _releasedata()
    {
        if(_allocated) {
            for(SQUnsignedInteger i = 0; i < _size; i++)
                _vals[i].~T();
            SQ_FREE(_alloc_ctx, _vals, (_allocated * sizeof(T)));
        }
    }
    SQUnsignedInteger _size;
    SQUnsignedInteger _allocated;
};

#define PAGE_SIZE 4096
#define ALIGN_SIZE(len, align) (((len)+(align - 1)) & ~((align)-1))
#define ALIGN_PTR(ptr, align) (((~((uintptr_t)(ptr))) + 1) & ((align) - 1))
#define ALIGN_SIZE_TO_PAGE(len) ALIGN_SIZE(len, PAGE_SIZE)
#define ALIGN_SIZE_TO_WORD(len) ALIGN_SIZE(len, 0x8)

class Arena {
public:

    Arena(SQAllocContext alloc_ctx, const SQChar *name, size_t chunkSize = 4 * PAGE_SIZE) : _alloc_ctx(alloc_ctx), _name(name), _chunks(NULL), _bigChunks(NULL) {
        _chunkSize = ALIGN_SIZE_TO_PAGE(chunkSize);
    }

    ~Arena() {
        release();
    }

    uint8_t *allocate(size_t size) {
        size = ALIGN_SIZE_TO_WORD(size);

        if (size > maxChunkSize) {
            return allocateBigChunk(size);
        }
        else {
            return allocatePull(size);
        }
    }

    void release() {
        struct BigChunk *bch = _bigChunks;
        while (bch) {
            struct BigChunk *cur = bch;
            bch = cur->_next;
            cur->~BigChunk();
            SQ_FREE(_alloc_ctx, cur, sizeof(BigChunk));
        }
        _bigChunks = NULL;

        struct Chunk *ch = _chunks;
        while (ch) {
            struct Chunk *cur = ch;
            ch = cur->_next;
            cur->~Chunk();
            SQ_FREE(_alloc_ctx, cur, sizeof(Chunk));
        }
        _chunks = NULL;
    }

private:
    struct Chunk {
        struct Chunk *_next;
        uint8_t *_start;
        uint8_t *_ptr;
        size_t _size;

        size_t allocated() const { return _ptr - _start; }
        size_t left() const { return _size - allocated(); }

        Chunk(struct Chunk *n, size_t s) : _next(n), _size(s) {
            _start = _ptr = allocatePage(_size);
        }

        ~Chunk() {
            releasePage();
        }

        uint8_t *allocatePage(size_t size) {
#if defined(_WIN32)  || defined(_WIN64) 
            void *result = VirtualAlloc(NULL, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
#else // __unix__ 
            void *result = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
            if (result == MAP_FAILED) {
                fprintf(stderr, "Cannot allocate %zu bytes via mmap, %s\n", size, strerror(errno));
                exit(ERR_MMAP);
    }
#endif // defined(_WIN32)  || defined(_WIN64) 
            memset(result, 0, size);
            return (uint8_t *)result;
        }

        void releasePage() {
#if defined(_WIN32)  || defined(_WIN64) 
            VirtualFree(_start, 0, MEM_RELEASE);
#else // __unix__ 
            munmap(start, size);
#endif // defined(_WIN32)  || defined(_WIN64) 
        }


        uint8_t *allocate(size_t size) {
            assert(size <= left());
            uint8_t *res = _ptr;
            _ptr += size;
            return res;
        }

    };

    uint8_t *allocateBigChunk(size_t size) {
        void *mem = SQ_MALLOC(_alloc_ctx, sizeof(BigChunk));
        struct BigChunk *ch = new(mem) BigChunk(_alloc_ctx, _bigChunks, size);
        _bigChunks = ch;
        return ch->_ptr;
    }


    struct Chunk *findChunk(size_t size) {
        struct Chunk *ch = _chunks;
        while (ch) {
            if (size <= ch->left()) return ch;
            ch = ch->_next;
        }

        void *mem = SQ_MALLOC(_alloc_ctx, sizeof(Chunk));
        ch = new(mem) Chunk(_chunks, _chunkSize);
        _chunks = ch;
        return ch;
    }

    uint8_t *allocatePull(size_t size) {
        struct Chunk *chunk = findChunk(size);
        assert(size <= chunk->left());
        return chunk->allocate(size);
    }

    const static size_t maxChunkSize = 0x1000;

    struct Chunk *_chunks;

    struct BigChunk {

        BigChunk(SQAllocContext alloc_ctx, struct BigChunk *next, size_t size) : _alloc_ctx(alloc_ctx), _next(next), _size(size) {
            _ptr = (uint8_t *)SQ_MALLOC(_alloc_ctx, size);
            memset(_ptr, 0, size);
        }

        ~BigChunk() {
            SQ_FREE(_alloc_ctx, _ptr, _size);
        }

        struct BigChunk *_next;
        SQAllocContext _alloc_ctx;
        uint8_t *_ptr;
        size_t _size;
    };

    SQAllocContext _alloc_ctx;

    struct BigChunk *_bigChunks;

    const SQChar *_name;

    size_t _chunkSize;

};


class ArenaObj {
protected:
    ArenaObj() {}
    virtual ~ArenaObj() {}

private:
    void *operator new(size_t size) { assert(0); return NULL; }

public:
    void *operator new(size_t size, Arena *arena) {
        return arena->allocate(size);
    }

    void operator delete(void *p, Arena *arena) {
        (void)p;
        (void)arena;
    }

    void operator delete(void *p) {
        (void)p;
    }
};

#endif //_SQUTILS_H_
