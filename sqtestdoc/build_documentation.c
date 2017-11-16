#include <squirrel.h>
#include <sqstdblob.h>
#include <sqstdsystem.h>
#include <sqstdio.h>
#include <sqstdmath.h>
#include <sqstdstring.h>
#include <sqstdaux.h>

static void reference_embedding_creating_a_c_function_example_1(HSQUIRRELVM v)
{
	#include "reference/embedding/references_from_c_example_1.h"
}

static void reference_embedding_calling_a_function_example_1(HSQUIRRELVM v)
{
	#include "reference/embedding/calling_a_function_example_1.h"
}

int main(void)
{
	return 0;
}