/*	see copyright notice in squirrel.h */
#ifndef _SQOPCODES_H_
#define _SQOPCODES_H_

#define MAX_FUNC_STACKSIZE 0xFF
#define MAX_LITERALS ((SQInteger)0x7FFFFFFF)

enum BitWiseOP {
	BW_AND = 0,
	BW_OR = 2,	
	BW_XOR = 3,
	BW_SHIFTL = 4,
	BW_SHIFTR = 5,
	BW_USHIFTR = 6
};

enum CmpOP {
	CMP_G = 0,
	CMP_GE = 2,	
	CMP_L = 3,
	CMP_LE = 4,
	CMP_3W = 5
};

enum NewObjectType {
	NOT_TABLE = 0,
	NOT_ARRAY = 1,
	NOT_CLASS = 2
};

enum AppendArrayType {
	AAT_STACK = 0,
	AAT_LITERAL = 1,
	AAT_INT = 2,
	AAT_FLOAT = 3,
	AAT_BOOL = 4
};

#define SQ_OP_CODE_LIST() \
	ENUM_OP(_OP_LINE, 0x00)\
	ENUM_OP(_OP_LOAD, 0x01)\
	ENUM_OP(_OP_LOADINT, 0x02)\
	ENUM_OP(_OP_LOADFLOAT, 0x03)\
	ENUM_OP(_OP_DLOAD, 0x04)\
	ENUM_OP(_OP_TAILCALL, 0x05)\
	ENUM_OP(_OP_CALL, 0x06)\
	ENUM_OP(_OP_PREPCALL, 0x07)\
	ENUM_OP(_OP_PREPCALLK, 0x08)\
	ENUM_OP(_OP_GETK, 0x09)\
	ENUM_OP(_OP_MOVE, 0x0A)\
	ENUM_OP(_OP_NEWSLOT, 0x0B)\
	ENUM_OP(_OP_DELETE, 0x0C)\
	ENUM_OP(_OP_SET, 0x0D)\
	ENUM_OP(_OP_GET, 0x0E)\
	ENUM_OP(_OP_EQ, 0x0F)\
	ENUM_OP(_OP_NE, 0x10)\
	ENUM_OP(_OP_ADD, 0x11)\
	ENUM_OP(_OP_SUB, 0x12)\
	ENUM_OP(_OP_MUL, 0x13)\
	ENUM_OP(_OP_DIV, 0x14)\
	ENUM_OP(_OP_MOD, 0x15)\
	ENUM_OP(_OP_BITW, 0x16)\
	ENUM_OP(_OP_RETURN, 0x17)\
	ENUM_OP(_OP_LOADNULLS, 0x18)\
	ENUM_OP(_OP_LOADROOT, 0x19)\
	ENUM_OP(_OP_LOADBOOL, 0x1A)\
	ENUM_OP(_OP_DMOVE, 0x1B)\
	ENUM_OP(_OP_JMP, 0x1C)\
	ENUM_OP(_OP_JCMP, 0x1D)\
	ENUM_OP(_OP_JZ, 0x1E)\
	ENUM_OP(_OP_SETOUTER, 0x1F)\
	ENUM_OP(_OP_GETOUTER, 0x20)\
	ENUM_OP(_OP_NEWOBJ, 0x21)\
	ENUM_OP(_OP_APPENDARRAY, 0x22)\
	ENUM_OP(_OP_COMPARITH, 0x23)\
	ENUM_OP(_OP_INC, 0x24)\
	ENUM_OP(_OP_INCL, 0x25)\
	ENUM_OP(_OP_PINC, 0x26)\
	ENUM_OP(_OP_PINCL, 0x27)\
	ENUM_OP(_OP_CMP, 0x28)\
	ENUM_OP(_OP_EXISTS, 0x29)\
	ENUM_OP(_OP_INSTANCEOF, 0x2A)\
	ENUM_OP(_OP_AND, 0x2B)\
	ENUM_OP(_OP_OR, 0x2C)\
	ENUM_OP(_OP_NEG, 0x2D)\
	ENUM_OP(_OP_NOT, 0x2E)\
	ENUM_OP(_OP_BWNOT, 0x2F)\
	ENUM_OP(_OP_CLOSURE, 0x30)\
	ENUM_OP(_OP_YIELD, 0x31)\
	ENUM_OP(_OP_RESUME, 0x32)\
	ENUM_OP(_OP_FOREACH, 0x33)\
	ENUM_OP(_OP_POSTFOREACH, 0x34)\
	ENUM_OP(_OP_CLONE, 0x35)\
	ENUM_OP(_OP_TYPEOF, 0x36)\
	ENUM_OP(_OP_PUSHTRAP, 0x37)\
	ENUM_OP(_OP_POPTRAP, 0x38)\
	ENUM_OP(_OP_THROW, 0x39)\
	ENUM_OP(_OP_NEWSLOTA, 0x3A)\
	ENUM_OP(_OP_GETBASE, 0x3B)\
	ENUM_OP(_OP_CLOSE, 0x3C)\
	ENUM_OP(_OP__LAST__, 0x3d)

//#define ENUM_OP(a,b) a = b,
//there is no point right now to manually number the opcodes
#define ENUM_OP(a,b) a,
enum SQOpcode
{
    SQ_OP_CODE_LIST()
};
#undef ENUM_OP

struct SQInstructionDesc {	  
	const SQChar *name;		  
};							  

struct SQInstruction 
{
	SQInstruction(){};
	SQInstruction(SQOpcode _op,SQInteger a0=0,SQInteger a1=0,SQInteger a2=0,SQInteger a3=0)
	{	op = _op;
		_arg0 = (unsigned char)a0;_arg1 = (SQInt32)a1;
		_arg2 = (unsigned char)a2;_arg3 = (unsigned char)a3;
	}
    
	
	SQInt32 _arg1;
	unsigned char op;
	unsigned char _arg0;
	unsigned char _arg2;
	unsigned char _arg3;
};

#include "squtils.h"
typedef sqvector<SQInstruction> SQInstructionVec;

#define NEW_SLOT_ATTRIBUTES_FLAG	0x01
#define NEW_SLOT_STATIC_FLAG		0x02

#endif // _SQOPCODES_H_
