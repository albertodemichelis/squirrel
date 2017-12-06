/*
Copyright (c) 2017 Daniel Atanasov

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include <new>
#include <squirrel.h>
#include <sqstdaux.h>
#include <sqstdio.h>
#include <sqstdstreamreader.h>
#include <sqstdtextio.h>

#define SQTC_TOOFEW	(-2)
#define SQTC_ILSEQ	(1)
#define SQTC_ILUNI	(2)

typedef SQInteger (*SQTCReadFct)( SQUserPointer user, uint8_t *p, SQInteger n);
typedef SQInteger (*SQTCWriteFct)( SQUserPointer user, const uint8_t *p, SQInteger n);

static HSQMEMBERHANDLE rdr__stream_handle;
static HSQMEMBERHANDLE wrt__stream_handle;

static const uint8_t utf8_lengths[16] =
{
	1,1,1,1,1,1,1,1,        /* 0000 to 0111 : 1 byte (plain ASCII) */
	0,0,0,0,                /* 1000 to 1011 : not valid */
	2,2,                    /* 1100 to 1101 : 2 bytes */
	3,                      /* 1110 : 3 bytes */
	4                       /* 1111 :4 bytes */
};

static uint8_t utf8_byte_masks[5] = {0,0,0x1f,0x0f,0x07};
static uint32_t utf8_min_codept[5] = {0,0,0x80,0x800,0x10000};

struct TextConverter;

typedef struct encodings_list_tag
{
	const SQChar **names;
	SQInteger (TextConverter::*Read)( uint32_t *pwc);
	SQInteger (TextConverter::*Write)( uint32_t wc);
	SQBool isBig;

} encodings_list_t;

static const encodings_list_tag *find_encoding( const SQChar *name);

struct TextConverter
{
	TextConverter( SQTCReadFct read, SQTCWriteFct write, SQUserPointer user) {
		BytesRead = read;
		BytesWrite = write;
		_BytesUser = user;
		_BytesReadChar = &TextConverter::Read_UTF8;
		_BytesWriteChar = &TextConverter::Write_UTF8;
        _bad_char = '?';
        isRdBigEndian = SQTrue;	// Default is Big Endian
        isWrBigEndian = SQTrue;	// Default is Big Endian
	}

	SQTCReadFct BytesRead;
	SQTCWriteFct BytesWrite;
	SQUserPointer _BytesUser;

	uint32_t _bad_char;

	SQInteger (TextConverter::*_BytesReadChar)( uint32_t *pwc);
	SQInteger (TextConverter::*_BytesWriteChar)( uint32_t wc);

	SQInteger ReadChar( uint32_t *pwc) { return (this->*_BytesReadChar)( pwc); }
	SQInteger WriteChar( uint32_t wc) { return (this->*_BytesWriteChar)( wc); }

	/* ================
		Read
	================ */

	SQInteger BytesReadU8( uint8_t *pc)
	{
		return (BytesRead( _BytesUser, pc, 1) == 1) ? SQ_OK : SQ_ERROR;
	}

	SQInteger BytesReadU16LE( uint16_t *pc)
	{
		uint8_t buf[2];
		if( BytesRead( _BytesUser, buf, 2) == 2) {
			*pc = (buf[1] << 8) | buf[0];
			return SQ_OK;
		}
		return SQ_ERROR;
	}

	SQInteger BytesReadU16BE( uint16_t *pc)
	{
		uint8_t buf[2];
		if( BytesRead( _BytesUser, buf, 2) == 2) {
			*pc = (buf[0] << 8) | buf[1];
			return SQ_OK;
		}
		return SQ_ERROR;
	}

	SQBool isRdBigEndian;

	SQInteger BytesReadU16( uint16_t *pc) { if( !isRdBigEndian) return BytesReadU16LE(pc); else return BytesReadU16BE(pc); };

	/* ================
		Write
	================ */

	SQInteger BytesWriteU8( uint8_t c)
	{
		return (BytesWrite( _BytesUser, &c, 1) == 1) ? SQ_OK : SQ_ERROR;;
	}

	SQInteger BytesWriteU16LE( uint16_t c)
	{
		uint8_t buf[2] = { (uint8_t)c, (uint8_t)(c>>8) };
		if( BytesWrite( _BytesUser, buf, 2) == 2) {
			return SQ_OK;
		}
		return SQ_ERROR;
	}

	SQInteger BytesWriteU16BE( uint16_t c)
	{
		uint8_t buf[2] = { (uint8_t)(c>>8), (uint8_t)c };
		if( BytesWrite( _BytesUser, buf, 2) == 2) {
			return SQ_OK;
		}
		return SQ_ERROR;
	}

	SQBool isWrBigEndian;

	SQInteger BytesWriteU16( uint16_t c) { if( !isWrBigEndian) return BytesWriteU16LE(c); else return BytesWriteU16BE(c); };

	/* ================
		"" - native char
	================ */

	SQInteger Read_CHAR( uint32_t *pwc)
	{
		SQChar c;
		if( BytesRead( _BytesUser, (uint8_t*)&c, sizeof(SQChar)) != sizeof(SQChar)) return SQ_ERROR;
		*pwc = c;
		return SQ_OK;
	}

	SQInteger Write_CHAR( uint32_t wc)
	{
		SQChar c = (SQChar)wc;
		return (BytesWrite( _BytesUser, (uint8_t*)&c, sizeof(SQChar)) == sizeof(SQChar)) ? SQ_OK : SQ_ERROR;
	}

	/* ================
		ASCII
	================ */

	SQInteger Read_ASCII( uint32_t *pwc)
	{
		uint8_t c;
		if( SQ_FAILED(BytesReadU8( &c))) return SQ_ERROR;		// EOF (SQCCV_TOOFEW)
		*pwc = (c <= 0x7F) ? c : _bad_char;
		return SQ_OK;
	}

	SQInteger Write_ASCII( uint32_t wc)
	{
		uint8_t c = (wc <= 0x7F) ? (uint8_t)wc : (uint8_t)_bad_char;
		return BytesWriteU8( c);
	}

	/* ================
		UTF-8
	================ */

	SQInteger Read_UTF8( uint32_t *pwc)
	{
        while(1) {
            uint8_t c;
            if( SQ_FAILED(BytesReadU8( &c))) return SQ_ERROR;		// EOF (SQCCV_TOOFEW)
            if( c < 0x80) { *pwc = c; return SQ_OK; }
            //else if( c < 0xC2) { *pwc = _bad_char; return SQTC_ILSEQ; }	// SQCCV_ILSEQ
            else if( c < 0xC0) { *pwc = _bad_char; return SQTC_ILSEQ; }	// SQCCV_ILSEQ
            else {
                SQInteger codelen = utf8_lengths[ c >> 4];
                SQInteger n = codelen;
                uint32_t wc = c & utf8_byte_masks[codelen];
                while( --n) {
                    if( SQ_FAILED(BytesReadU8( &c))) { return SQTC_TOOFEW; }		// EOF (SQCCV_TOOFEW)
                    if( (c & 0xC0) != 0x80) { *pwc = _bad_char; return SQTC_ILSEQ; }	// SQCCV_ILSEQ
                    wc <<= 6; wc |= c & 0x3F;
                }
                if( wc < utf8_min_codept[codelen]) { *pwc = _bad_char; return SQTC_ILSEQ; }	// SQCCV_ILSEQ
                if( wc != 0xFEFF) {
                    *pwc = wc;
                    return SQ_OK;
                }
            }
        }
	}

	SQInteger Write_UTF8( uint32_t wc)
	{
		if( wc < 0xD800 || (wc >= 0xE000 && wc != 0xFFFE && wc < 0x110000)) {
			SQInteger codelen;

			if( wc <= 0x007F)			{ uint8_t c = (uint8_t)wc; return BytesWrite( _BytesUser, &c, 1); }
			else if( wc <= 0x07FF)		{ codelen = 2; }
			else if( wc <= 0xFFFFL)		{ codelen = 3; }
			else						{ codelen = 4; }

			uint8_t r[4];
			switch( codelen)
			{ /* note: code falls through cases! */
				case 4: r[3] = 0x80 | (wc & 0x3f); wc = wc >> 6; wc |= 0x10000;
				case 3: r[2] = 0x80 | (wc & 0x3f); wc = wc >> 6; wc |= 0x800;
				case 2: r[1] = 0x80 | (wc & 0x3f); wc = wc >> 6; wc |= 0xc0;
				case 1: r[0] = wc;
			}
			return BytesWrite( _BytesUser, r, codelen);
		}
		Write_UTF8(_bad_char);	// SQCCV_ILUNI
		return SQTC_ILUNI;
	}

	/* ================
		UTF-16
	================ */

	SQInteger Read_UTF16( uint32_t *pwc)
	{
		while(1) {
			uint16_t c;
			if( SQ_FAILED(BytesReadU16( &c))) return SQTC_TOOFEW;	// EOF (SQCCV_TOOFEW)
			if( c == 0xFEFF)		{ /* ok */ }
			else if( c == 0xFFFE)  { isRdBigEndian = !isRdBigEndian; }
			else if( c >= 0xD800 && c < 0xDC00) {
				uint32_t wc = c;
				if( SQ_FAILED(BytesReadU16( &c))) return SQTC_TOOFEW;	// EOF (SQCCV_TOOFEW)
				if( c >= 0xDC00 && c < 0xE000) {
					wc = 0x10000 + ((wc - 0xD800) << 10) + (c - 0xDC00);
					*pwc = wc;
					return SQ_OK;
				}
				else { *pwc = _bad_char; return SQTC_ILSEQ;	} // SQCCV_ILSEQ
			}
			else if( c >= 0xDC00 && c < 0xE000) { *pwc = _bad_char; return SQTC_ILSEQ; }		// SQCCV_ILSEQ
			else { *pwc = c; return SQ_OK; }
		}
	}

	SQInteger Write_UTF16( uint32_t wc)
	{
		if( wc < 0xD800 || (wc >= 0xE000 && wc != 0xFFFE && wc < 0x110000)) {
			// check and write BOM
			if( wc >= 0x10000) {
				uint16_t w1, w2;

				wc -= 0x10000;
				w2 = 0xDC00 | (wc & 0x03FF);
				w1 = 0xD800 | ((wc >> 10) & 0x03FF);

				if( SQ_SUCCEEDED(BytesWriteU16( w1)))
					if( SQ_SUCCEEDED(BytesWriteU16( w2)))
						return SQ_OK;
				return SQ_ERROR;
			}
			else if( wc < 0x10000) {
				return BytesWriteU16( (uint16_t)wc);
			}
		}
		Write_UTF16(_bad_char); // SQCCV_ILUNI;
		return SQTC_ILUNI;
	}

	/* ================
		Set encoding
	================ */

	SQInteger SetReadEncoding( const SQChar *name)
	{
		const encodings_list_tag *enc = find_encoding( name);
		if( enc != NULL) {
			_BytesReadChar = enc->Read;
			isRdBigEndian = enc->isBig;
			return SQ_OK;
		}
		//_BytesReadChar = &TextConverter::Read_NULL;
		return SQ_ERROR;
	}

	SQInteger SetWriteEncoding( const SQChar *name)
	{
		const encodings_list_tag *enc = find_encoding( name);
		if( enc != NULL) {
			_BytesWriteChar = enc->Write;
			isWrBigEndian = enc->isBig;
			return SQ_OK;
		}
		//_BytesWriteChar = &TextConverter::Write_NULL;
		return SQ_ERROR;
	}
};

/* ====================================
		conversion names
==================================== */

static const SQChar *__NAMES_CHAR[] = { _SC(""), NULL };
static const SQChar *__NAMES_ASCII[] = { _SC("ASCII"), NULL };
static const SQChar *__NAMES_UTF_8[] = { _SC("UTF-8"), NULL };
static const SQChar *__NAMES_UTF_16BE[] = { _SC("UTF-16"), _SC("UTF-16BE"), _SC("UCS-2BE"), NULL };
static const SQChar *__NAMES_UTF_16LE[] = { _SC("UTF-16LE"), _SC("UCS-2"), _SC("UCS-2LE"), NULL };

static encodings_list_t encodings_list[] = {
	{ __NAMES_CHAR,		&TextConverter::Read_CHAR, &TextConverter::Write_CHAR, SQFalse },
	{ __NAMES_ASCII,	&TextConverter::Read_ASCII, &TextConverter::Write_ASCII, SQFalse },
	{ __NAMES_UTF_8,	&TextConverter::Read_UTF8,  &TextConverter::Write_UTF8, SQFalse },
	{ __NAMES_UTF_16BE,	&TextConverter::Read_UTF16, &TextConverter::Write_UTF16, SQTrue },
	{ __NAMES_UTF_16LE,	&TextConverter::Read_UTF16, &TextConverter::Write_UTF16, SQFalse },
	{ NULL, NULL, NULL, SQFalse }
};

static int compare_encoding_name( const SQChar *iname, const SQChar *oname)
{
	SQChar i, o;
	while( ((i = *iname)!=_SC('\0')) & ((o = *oname)!=_SC('\0'))) {		// & - both i and o must be read
		if( !(i==o) && !(i>=_SC('A') && (i^o)==0x20)
		  && !( i==_SC('-') && o==_SC('_')) ) {
			if( i==_SC('-'))
				iname++;
			else
				break;
		}
		else { iname++; oname++; }
	}

	return i-o;
}

static const encodings_list_tag *find_encoding( const SQChar *name)
{
	const encodings_list_t *enc = encodings_list;

	while( enc->names) {
		const SQChar **pname = enc->names;
		while( *pname) {
			if( compare_encoding_name( *pname, name) == 0) {
				return enc;
			}
			pname++;
		}
		enc++;
	}
	return NULL;
}

/* ====================================
		Guess encoding
==================================== */

const SQChar *sqstd_guessencoding_srdr(SQSRDR srdr)
{
	uint8_t buf[3];

	sqstd_srdrmark(srdr,sizeof(buf));
	if( sqstd_sread( buf, 2, (SQSTREAM)srdr) == 2) {
		if( (buf[0] == 0xFE) && (buf[1] == 0xFF))		return _SC("UTF-16BE");
		else if( (buf[0] == 0xFF) && (buf[1] == 0xFE))	return _SC("UTF-16LE");
		else if( (buf[0] == 0xEF) && (buf[1] == 0xBB)) {
			if( sqstd_sread( buf+2, 1, (SQSTREAM)srdr) == 1) {
				if( buf[2] == 0xBF)						return _SC("UTF-8");
			}
		}
	}
	sqstd_srdrreset(srdr);
	return NULL;
}

/* ====================================
		Text Reader
==================================== */

#define CBUFF_SIZE	16

static SQInteger SQTextReaderRead( SQUserPointer user, uint8_t *p, SQInteger n);
static SQInteger SQTextReaderWrite( SQUserPointer user, const uint8_t *p, SQInteger n);

struct SQTextReader : public SQStream
{
	SQTextReader( SQStream *stream, SQBool owns_close, SQBool owns_release) : _converter(SQTextReaderRead,SQTextReaderWrite,this) {
		_stream = stream;
		_owns_close = owns_close;
		_owns_release = owns_release;
		_buf_len = 0;
		_buf_pos = 0;
#ifdef SQUNICODE
		_converter.SetWriteEncoding(_SC("UTF-16"));
#else // SQUNICODE
		_converter.SetWriteEncoding(_SC("UTF-8"));
#endif // SQUNICODE
	}

	SQInteger Read( void *buffer, SQInteger size) {
		SQInteger preread = 0;
		if( _buf_pos > 0) {								// have something left in _buf
			SQInteger left = _buf_len - _buf_pos;
			SQInteger toread = left;
			if( toread > size)
				toread = size;
			memcpy( buffer, _buf + _buf_pos, toread);
			if( left == toread)							// buffer is empty now
				_buf_pos = 0;
			else
				_buf_pos += toread;
			if( toread == size)							// no more to read
				return size;
			preread = toread;
			size -= toread;
			buffer = (uint8_t*)buffer + toread;
		}												// nothing in _buf
		while( size > 0) {
			uint32_t wc;
			_buf_len = 0;
			if( SQ_FAILED(_converter.ReadChar( &wc))) return preread;
			if( SQ_FAILED(_converter.WriteChar( wc))) return preread;
			SQInteger toread = _buf_len;
			if( toread > size) {
				toread = size;
				_buf_pos = size;
			}
			memcpy( buffer, _buf, toread);
			preread += toread;
			size -= toread;
			buffer = (uint8_t*)buffer + toread;
		}
		return preread;
	}

	SQInteger cnvWrite( const uint8_t *p, SQInteger n)
	{
		if( (_buf_len + n) > CBUFF_SIZE) n = CBUFF_SIZE - _buf_len;
		memcpy( _buf + _buf_len, p, n);
		_buf_len += n;
		return n;
	}

	SQInteger cnvRead( uint8_t *p, SQInteger n)
	{
		return _stream->Read( p, n);
	}

	SQInteger SetEncoding( const SQChar *encoding)
	{
		return _converter.SetReadEncoding( encoding);
	}

    SQInteger Write( SQ_UNUSED_ARG(const void *buffer), SQ_UNUSED_ARG(SQInteger size)) { return -1; }
    SQInteger Flush() { return 0; }
    SQInteger Tell() { return -1; }
    SQInteger Len() { return -1; }
    SQInteger Seek( SQ_UNUSED_ARG(SQInteger offset), SQ_UNUSED_ARG(SQInteger origin)) { return -1; }
    bool IsValid() { return (_stream != NULL) && _stream->IsValid(); }
    bool EOS() { return (_buf_pos == 0) && ((_stream == NULL) || _stream->EOS()); }
    SQInteger Close()
	{
		SQInteger r = 0;
		if( (_stream != NULL) && _owns_close) {
			r = _stream->Close();
			_owns_close = SQFalse;
		}
		return r;
	}

    void _Release()
	{
		if( (_stream != NULL) && _owns_release) _stream->_Release();
		this->~SQTextReader();
		sq_free(this,sizeof(SQTextReader));
	}

protected:
	TextConverter _converter;
	SQStream *_stream;
	SQBool _owns_close;
	SQBool _owns_release;
	SQInteger _buf_len;
	SQInteger _buf_pos;
	uint8_t _buf[CBUFF_SIZE];
};

SQInteger SQTextReaderRead( SQUserPointer user, uint8_t *p, SQInteger n)
{
	SQTextReader *rdr = (SQTextReader*)user;
	return rdr->cnvRead( p, n);
}

SQInteger SQTextReaderWrite( SQUserPointer user, const uint8_t *p, SQInteger n)
{
	SQTextReader *rdr = (SQTextReader*)user;
	return rdr->cnvWrite( p, n);
}

// ------------------------------------
// TextReader API

SQSTREAM sqstd_textreader_srdr(SQSRDR srdr,SQBool owns_close,SQBool owns_release,const SQChar *encoding,SQBool guess)
{
	if( guess) {
		const SQChar *genc = sqstd_guessencoding_srdr(srdr);
		if( genc != NULL)
			encoding = genc;
	}
	SQTextReader *rdr = new (sq_malloc(sizeof(SQTextReader)))SQTextReader( (SQStream*)srdr, owns_close, owns_release);
	if( encoding != NULL) {
		if( SQ_FAILED(rdr->SetEncoding( encoding))) {
			rdr->_Release();
			return NULL;
		}
	}
	return (SQSTREAM)rdr;
}

SQSTREAM sqstd_textreader(SQSTREAM stream,SQBool owns,const SQChar *encoding,SQBool guess)
{
	SQBool owns_release = SQFalse;
	if( guess) {
		SQSRDR srdr = sqstd_streamreader(stream,owns,0);
		if( srdr == NULL) return NULL;
		stream = (SQSTREAM)srdr;
		owns = SQTrue;
		owns_release = SQTrue;
	}
	return sqstd_textreader_srdr((SQSRDR)stream,owns,owns_release,encoding,guess);
}

// ------------------------------------
// TextReader Bindings

static SQInteger _textreader__typeof(HSQUIRRELVM v)
{
    sq_pushstring(v,_sqstd_textreader_decl.name,-1);
    return 1;
}

static SQInteger _textreader_constructor(HSQUIRRELVM v)
{
	SQInteger top = sq_gettop(v);
	SQSTREAM stream;
	SQBool owns = SQFalse;
	const SQChar *encoding = NULL;
	SQBool guess = SQFalse;
	SQBool stream_is_reader = SQTrue;

    if( SQ_FAILED( sq_getinstanceup( v,2,(SQUserPointer*)&stream,(SQUserPointer)SQSTD_STREAMREADER_TYPE_TAG))) {
		stream_is_reader = SQFalse;
	    if( SQ_FAILED( sq_getinstanceup( v,2,(SQUserPointer*)&stream,(SQUserPointer)SQSTD_STREAM_TYPE_TAG)))
	        return sq_throwerror(v,_SC("invalid argument type"));
	}
	if( top > 2) {
		sq_getbool(v, 3, &owns);
		if( top > 3) {
			sq_getstring(v, 4, &encoding);
			if( top > 4) {
				sq_getbool(v, 5, &guess);
			}
		}
	}

	SQSTREAM rdr;
	if( stream_is_reader) {
		rdr = sqstd_textreader_srdr((SQSRDR)stream,owns,SQFalse,encoding,guess);
	}
	else {
		rdr = sqstd_textreader(stream,owns,encoding,guess);
	}
	if( rdr == NULL)
		return sq_throwerror(v, _SC("cannot create textreader"));

    if(SQ_FAILED(sq_setinstanceup(v,1,rdr))) {
		sqstd_srelease(rdr);
        return sq_throwerror(v, _SC("cannot create textreader instance"));
    }

	// save stream in _stream member
	sq_push(v,2);
	sq_setbyhandle(v,1,&rdr__stream_handle);

    sq_setreleasehook(v,1,__sqstd_stream_releasehook);
    return 0;
}

//bindings
#define _DECL_TEXTREADER_FUNC(name,nparams,typecheck) {_SC(#name),_textreader_##name,nparams,typecheck}
static const SQRegFunction _textreader_methods[] = {
    _DECL_TEXTREADER_FUNC(constructor,-2,_SC("xxbsb")),
    _DECL_TEXTREADER_FUNC(_typeof,1,_SC("x")),
    {NULL,(SQFUNCTION)0,0,NULL}
};

static const SQRegMember _textreader_members[] = {
	{_SC("_stream"), &rdr__stream_handle },
	{NULL,NULL}
};

const SQRegClass _sqstd_textreader_decl = {
	&_sqstd_stream_decl,	// base_class
    _SC("std_textreader"),	// reg_name
    _SC("textreader"),		// name
	_textreader_members,	// members
	_textreader_methods,	// methods
	NULL,				// globals
};

/* ====================================
		Text Writer
==================================== */

static SQInteger SQTextWriterRead( SQUserPointer user, uint8_t *p, SQInteger n);
static SQInteger SQTextWriterWrite( SQUserPointer user, const uint8_t *p, SQInteger n);

struct SQTextWriter : public SQStream
{
	SQTextWriter( SQStream *stream, SQBool owns) : _converter(SQTextWriterRead,SQTextWriterWrite,this) {
		_stream = stream;
		_owns = owns;
		_tmp_size = 0;
		_tmp_pos = 0;
#ifdef SQUNICODE
		_converter.SetReadEncoding(_SC("UTF-16"));
#else // SQUNICODE
		_converter.SetReadEncoding(_SC("UTF-8"));
#endif // SQUNICODE
	}

    SQInteger Write(const void *buffer, SQInteger size) {
		SQInteger total = 0;
		_in_buf = (const uint8_t*)buffer;
		_in_size = size;
		_in_pos = 0;
		if( _tmp_size > 0) {
			while( _in_pos < _in_size) {
				SQInteger r;
				uint32_t wc;
				_tmp_buf[_tmp_size] = _in_buf[_in_pos];
				_in_pos++;
				_tmp_size++;
				_tmp_pos = 0;
				r = _converter.ReadChar( &wc);
				if( SQ_SUCCEEDED(r)) {
					_tmp_size = 0;
					if( SQ_FAILED(_converter.WriteChar( wc))) return _in_pos;
					break;
				}
				else if( r != SQTC_TOOFEW) {
					_tmp_size = 0;
					return _in_pos;
				}
			}
			total = _in_pos;
		}
		while( _in_pos < _in_size) {
			SQInteger r;
			SQInteger saved_pos = _in_pos;
			uint32_t wc;
			r = _converter.ReadChar( &wc);
			if( SQ_FAILED(r)) {
				if( r == SQTC_TOOFEW) {
					_tmp_size = _in_size - saved_pos;
					memcpy( _tmp_buf, _in_buf + saved_pos, _tmp_size);
					total += _tmp_size;
				}
				return total;
			}
			if( SQ_FAILED(_converter.WriteChar( wc))) return total;
			total += _in_pos - saved_pos;
		}
		return total;
	}

	SQInteger cnvRead( uint8_t *p, SQInteger n)
	{
		if( _tmp_size > 0) {
			SQInteger bleft = _tmp_size - _tmp_pos;
			if( bleft > n) {
				bleft = n;
			}
			memcpy( p, _tmp_buf + _tmp_pos, bleft);
			_tmp_pos += bleft;
			return bleft;
		}
		else {
			SQInteger bleft = _in_size - _in_pos;
			if( bleft > n) {
				bleft = n;
			}
			memcpy( p, _in_buf + _in_pos, bleft);
			_in_pos += bleft;
			return bleft;
		}
	}

	SQInteger cnvWrite( const uint8_t *p, SQInteger n)
	{
		return _stream->Write( (const void*)p, n);
	}

	SQInteger SetEncoding( const SQChar *encoding)
	{
		return _converter.SetWriteEncoding( encoding);
	}

	SQInteger Read( SQ_UNUSED_ARG(void *buffer), SQ_UNUSED_ARG(SQInteger size)) { return -1; }
    SQInteger Flush()
	{
		if( _stream != NULL) _stream->Flush();
		return 0;
	}
    SQInteger Tell() { return -1; }
    SQInteger Len() { return -1; }
    SQInteger Seek( SQ_UNUSED_ARG(SQInteger offset), SQ_UNUSED_ARG(SQInteger origin)) { return -1; }
    bool IsValid() { return (_stream != NULL) && _stream->IsValid(); }
    bool EOS() { return SQFalse; }
    SQInteger Close()
	{
		SQInteger r = 0;
		if( (_stream != NULL) && (_tmp_size > 0)) {
			uint32_t wc = _converter._bad_char;
			_converter.WriteChar( wc);
			_tmp_size = 0;
		}
		Flush();
		if( (_stream != NULL) && _owns) {
			r = _stream->Close();
			_stream = NULL;
			_owns = SQFalse;
		}
		return r;
	}

    void _Release()
	{
		this->~SQTextWriter();
		sq_free(this,sizeof(SQTextWriter));
	}

protected:
	TextConverter _converter;
	SQStream *_stream;
	SQBool _owns;
	const uint8_t* _in_buf;
	SQInteger _in_size;
	SQInteger _in_pos;
	SQInteger _tmp_size;
	SQInteger _tmp_pos;
	uint8_t _tmp_buf[CBUFF_SIZE];
};

SQInteger SQTextWriterRead( SQUserPointer user, uint8_t *p, SQInteger n)
{
	SQTextWriter *wrt = (SQTextWriter*)user;
	return wrt->cnvRead( p, n);
}

SQInteger SQTextWriterWrite( SQUserPointer user, const uint8_t *p, SQInteger n)
{
	SQTextWriter *wrt = (SQTextWriter*)user;
	return wrt->cnvWrite( p, n);
}

// ------------------------------------
// TextWriter API

SQSTREAM sqstd_textwriter(SQSTREAM stream,SQBool owns,const SQChar *encoding)
{
	SQTextWriter *wrt = new (sq_malloc(sizeof(SQTextWriter)))SQTextWriter( (SQStream*)stream, owns);
	if( encoding) {
		if( SQ_FAILED(wrt->SetEncoding( encoding))) {
			wrt->_Release();
			return NULL;
		}
	}
	return (SQSTREAM)wrt;
}

// ------------------------------------
// TextWriter Bindings

static SQInteger _textwriter__typeof(HSQUIRRELVM v)
{
    sq_pushstring(v,_sqstd_textwriter_decl.name,-1);
    return 1;
}

static SQInteger _textwriter_constructor(HSQUIRRELVM v)
{
	SQInteger top = sq_gettop(v);
	SQSTREAM stream;
	SQBool owns = SQFalse;
	const SQChar *encoding = NULL;

    if( SQ_FAILED( sq_getinstanceup( v,2,(SQUserPointer*)&stream,(SQUserPointer)SQSTD_STREAM_TYPE_TAG))) {
        return sq_throwerror(v,_SC("invalid argument type"));
	}

	if( top > 2) {
		sq_getbool(v, 3, &owns);
		if( top > 3) {
			sq_getstring(v, 4, &encoding);
		}
	}

	SQSTREAM wrt = sqstd_textwriter(stream,owns,encoding);
	if( wrt == NULL)
		return sq_throwerror(v, _SC("cannot create textwriter"));

    if(SQ_FAILED(sq_setinstanceup(v,1,wrt))) {
		sqstd_srelease(wrt);
        return sq_throwerror(v, _SC("cannot create textwriter instance"));
    }

	// save stream in _stream member
	sq_push(v,2);
	sq_setbyhandle(v,1,&wrt__stream_handle);

    sq_setreleasehook(v,1,__sqstd_stream_releasehook);
    return 0;
}

//bindings
#define _DECL_TEXTWRITER_FUNC(name,nparams,typecheck) {_SC(#name),_textwriter_##name,nparams,typecheck}
static const SQRegFunction _textwriter_methods[] = {
    _DECL_TEXTWRITER_FUNC(constructor,-2,_SC("xxbs")),
    _DECL_TEXTWRITER_FUNC(_typeof,1,_SC("x")),
    {NULL,(SQFUNCTION)0,0,NULL}
};

static const SQRegMember _textwriter_members[] = {
	{_SC("_stream"), &wrt__stream_handle },
	{NULL,NULL}
};

const SQRegClass _sqstd_textwriter_decl = {
	&_sqstd_stream_decl,	// base_class
    _SC("std_textwriter"),	// reg_name
    _SC("textwriter"),		// name
	_textwriter_members,	// members
	_textwriter_methods,	// methods
	NULL,				// globals
};

/* ====================================
		Register
==================================== */

SQUIRREL_API SQRESULT sqstd_register_textiolib(HSQUIRRELVM v)
{
	if(SQ_FAILED(sqstd_registerclass(v,&_sqstd_textreader_decl)))
	{
		return SQ_ERROR;
	}
 	sq_poptop(v);

	if(SQ_FAILED(sqstd_registerclass(v,&_sqstd_textwriter_decl)))
	{
		return SQ_ERROR;
	}
 	sq_poptop(v);

	return SQ_OK;
}
