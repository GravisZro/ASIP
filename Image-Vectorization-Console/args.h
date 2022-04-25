#pragma once
#include "parameters.h"


namespace Args {

	bool parse(int argc, char *argv[], Parameters* params);

	void help();
}