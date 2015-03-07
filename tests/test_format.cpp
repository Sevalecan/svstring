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

	string form{"This is a \"{}\", this is only a \"{}\". {:0.5g}"};

        cout << svstring::format(form, 1, std::string("Spam spam spam spam SPAMMITY SPAM!"), 3.05);
	return 0;
}

