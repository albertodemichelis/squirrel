#include <squirrel.h>
#include <sqstdblob.h>
#include <sqstdsystem.h>
#include <sqstdio.h>
#include <sqstdmath.h>
#include <sqstdstring.h>
#include <sqstdaux.h>
#include <stdio.h>

static void calling_a_function_1(HSQUIRRELVM v) {
	#include "reference/embedding/calling_a_function_code_1.h"
}

#include "reference/embedding/compiling_a_script_code_2.h"

static void compiling_a_script_3(HSQUIRRELVM v) {
	#include "reference/embedding/compiling_a_script_code_3.h"
}

static void compiling_a_script_4(HSQUIRRELVM v) {
	#include "reference/embedding/compiling_a_script_code_4.h"
}

#include "reference/embedding/creating_a_c_function_code_1.h"
#include "reference/embedding/creating_a_c_function_code_2.h"

static void references_from_c_1(HSQUIRRELVM v) {
	#include "reference/embedding/references_from_c_code_1.h"
}

#include "reference/embedding/vm_initialization_code_1.h"

// This one was moved since the error it has polutes the global namespace
// Since we are compiling in C we can't put each of these in a namespace of their own
static void compiling_a_script_1(HSQUIRRELVM v) {
	#include "reference/embedding/compiling_a_script_code_1.h"
}
