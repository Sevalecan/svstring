#include "svformat.h"
#include <iostream>
#include <string>

// Test file for svstring::format

int main()
{
	using std::cout;
	using std::string;
	using std::endl;
	using svstring::format;

	string form{"This is a \"{}\", this is only a \"{}\"."};

	cout << "Format: '" << form << "'" << endl;
	cout << svstring::format(form, std::string("test"), "test") << endl;
	return 0;
}

