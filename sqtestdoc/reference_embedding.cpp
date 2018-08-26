#include <squirrel.h>
#include <sqstdblob.h>
#include <sqstdsystem.h>
#include <sqstdio.h>
#include <sqstdmath.h>
#include <sqstdstring.h>
#include <sqstdaux.h>
#include <stdio.h>

namespace reference_embedding_calling_a_function_1 {
	static void f(HSQUIRRELVM v) {
		#include "reference/embedding/calling_a_function_code_1.h"
	}
}

namespace reference_embedding_compiling_a_script_1 {
	static void f(HSQUIRRELVM v) {
		#include "reference/embedding/compiling_a_script_code_1.h"
	}
}

namespace reference_embedding_compiling_a_script_2 {
	#include "reference/embedding/compiling_a_script_code_2.h"
}

namespace reference_embedding_compiling_a_script_3 {
	static void f(HSQUIRRELVM v) {
		#include "reference/embedding/compiling_a_script_code_3.h"
	}
}

namespace reference_embedding_compiling_a_script_4 {
	static void f(HSQUIRRELVM v) {
		#include "reference/embedding/compiling_a_script_code_4.h"
	}
}

namespace reference_embedding_creating_a_c_function_1 {
	#include "reference/embedding/creating_a_c_function_code_1.h"
}

namespace reference_embedding_creating_a_c_function_2 {
	#include "reference/embedding/creating_a_c_function_code_2.h"
}

namespace reference_embedding_references_from_c_1 {
	static void f(HSQUIRRELVM v) {
		#include "reference/embedding/references_from_c_code_1.h"
	}
}

namespace reference_embedding_vm_initialization_1 {
	#include "reference/embedding/vm_initialization_code_1.h"
}
