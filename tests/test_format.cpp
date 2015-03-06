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

	string form{"This is a \"{}\", this is only a \"{fail}\". {:0.5g}"};

        try {
	cout << "Format: '" << form << "'" << endl;
	cout << svstring::format(form, std::string("test"), "test") << endl;
        }
        catch (std::exception &e)
        {
            std::cout << "What? " << e.what() << "\n Exiting.\n";
            return 0;
        }
	return 0;
}

