#include <iostream>
#include "lj_driver.hpp"

int main(int argc, char *argv[])
{
	int res = 0;
	LJ::LJ_Driver driver;
	for (++argv; argv[0]; ++argv) {
		if (*argv == std::string("-p"))
			driver.trace_parsing_ = true;
		else if (*argv == std::string("-s"))
			driver.trace_scanning_ = true;
		else if (!driver.Parse(*argv)) {
			driver.Dump();
		}
	}
	return res;
}
