#include "lj_driver.hpp"
#include "lj_parser.hpp"

LJ_Driver::LJ_Driver()
: trace_scanning_(false), trace_parsing_(false)
{
	variables["one"] = 1;
	variables["two"] = 2;
}

LJ_Driver::~LJ_Driver()
{
}

int LJ_Driver::Parse(const std::string &f)
{
	file_ = f;
	ScanBegin();
	LJ::LJ_Parser parser(*this);
	parser.set_debug_level(trace_parsing_);
	int res = parser.parse();
	ScanEnd();
	return res;
}

void LJ_Driver::Error(const LJ::location& l, const std::string& m)
{
	std::cerr << l << ": " << m << std::endl;
}

void LJ_Driver::Error(const std::string& m)
{
	std::cerr << m << std::endl;
}
