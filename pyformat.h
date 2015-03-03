#ifndef PYFORMAT_H
#define PYFORMAT_H
#include <cstdio>
#include <iostream>
#include <string>
#include <algorithm>
#include <array>

class VarArg
{
	enum
	{
		A_INT = 0,
		A_DOUBLE,
		A_STRING
	};

	union
	{
		int i_a;
		double d_a;
	} data;
	std::string text_data;
	int type;
public:
	VarArg(const int in) { data.i_a = in; type = A_INT; }
	VarArg(const double in) { data.d_a = in; type = A_DOUBLE; }
	VarArg(const char *in) { text_data = std::string(in); type = A_STRING; }
	VarArg(const std::string in) { text_data = in; type = A_STRING; }

	int getType()
	{
		return type;
	}

	std::string getData()
	{
		if (type == A_STRING)
			return text_data;
		else if (type == A_INT)
			return std::to_string(data.i_a);
		else
			return std::to_string(data.d_a);
	}
};

template<typename T, typename... Args> void pyformat(T a1, Args... args)
{
	using namespace std;
	string str{a1};
	vector<VarArg> args_list = {args...};

	vector<std::string> pieces{""};
	const char *start = str.data();
	const char *end = start + str.size();

	enum {
		TOP_LEVEL = 0,
		INNER_PART1, // field_name
		INNER_PART2, // conversion
		INNER_PART3, // format_spec
	} state = TOP_LEVEL;

	size_t argument_pos = 0;

	for (const char *txt = start; txt < end; txt++)
	{
		const bool is_end = !(txt+1 < end);
		// Will only matter for brackets.
		const bool is_escaped = !is_end ? txt[0] == txt[1] : 0;


		if (txt[0] == '{' && state == TOP_LEVEL && !is_escaped && !is_end)
		{
			// TODO: Remove {start and end} debugging text and make it ""
			state = INNER_PART1;
			pieces.push_back(std::string(""));
		}
		else if (txt[0] == '}' && state != TOP_LEVEL && !is_escaped)
		{
			state = TOP_LEVEL;
			// TODO: Update this to accept formatting commands after
			// we start to parse them.
			pieces.back().append(args_list[argument_pos++].getData());
			pieces.push_back(std::string(""));
		}
		else if (state != TOP_LEVEL)
		{
			// Do inner processing here.
			pieces.back() += "p.";
			pieces.back() += txt[0];
		}
		else if (state == TOP_LEVEL)
		{
			pieces.back() += txt[0];
		}

		if (is_escaped && (txt[0] == '{' || txt[0] == '}'))
			txt++;
	}

	for(auto &s : pieces)
		std::cout << s;
}

#endif // PYPRINT_H
