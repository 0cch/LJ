#ifndef __LJ_DRIVER_H__
#define __LJ_DRIVER_H__
#include <string>
#include <map>
#include "lj_parser.hpp"

// Tell Flex the lexer's prototype ...
# define YY_DECL LJ::LJ_Parser::symbol_type yylex(LJ_Driver& driver)
// ... and declare it for the parser's sake.
YY_DECL;

class LJ_Driver
{
public:
	LJ_Driver();
	virtual ~LJ_Driver();

	std::map<std::string, int> variables;

	int result_;

	// Handling the scanner.
	void ScanBegin();
	void ScanEnd();
	bool trace_scanning_;

	// Run the parser on file F.
	// Return 0 on success.
	int Parse(const std::string& f);
	// The name of the file being parsed.
	// Used later to pass the file name to the location tracker.
	std::string file_;
	// Whether parser traces should be generated.
	bool trace_parsing_;

	// Error handling.
	void Error(const LJ::location& l, const std::string& m);
	void Error(const std::string& m);
};
#endif // ! LJ_Driver_HH
