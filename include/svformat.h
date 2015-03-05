#ifndef SVFORMAT_H
#define SVFORMAT_H
#include <cstdio>
#include <iostream>
#include <string>
#include <algorithm>
#include <array>
#include <bitset>

namespace svstring
{

    struct _Format
    {
        enum
        {
			CONVERSION = 0,
			FILL,
			ALIGN,
			SIGN,
			ALTERNATE,
			ZERO_FILL,
			THOUSANDS_COMMA,
			WIDTH,
			PRECISION,
			TYPE
        };
        std::bitset<9> states;
		std::string name;
		char conversion;
        char fill;
        int align;
        int sign;
        bool alternate;
        bool zero_fill;
        bool thousands_comma;
        int width;
        int precision;
        char type;

        _Format()
        {
            states = 0;
        }
    };

    template<typename T> std::string _dbl_to_string(T dbl)
    {

    }

    class _VarArg
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
            long double ld_a;
        } data;
        std::string text_data;
        int type;
    public:
        _VarArg(const int in) { data.i_a = in; type = A_INT; }
        _VarArg(const double in) { data.d_a = in; type = A_DOUBLE; }
        _VarArg(const char *in) { text_data = std::string(in); type = A_STRING; }
        _VarArg(const std::string in) { text_data = in; type = A_STRING; }

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

    template<typename T, typename... Args> std::string format(T a1, Args... args)
    {
        using namespace std;
        string str{a1};
        vector<_VarArg> args_list = {args...};

        vector<std::string> pieces{""};
        const char *start = str.data();
        const char *end = start + str.size();
		size_t total_size = 0;

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
                state = INNER_PART1;
				if (pieces.size() > 0 && pieces.back().size() > 0)
				{
					total_size += pieces.back().size();
	                pieces.push_back(std::string(""));
				}
				else if (pieces.size() == 0)
					pieces.push_back(std::string(""));

            }
            else if (txt[0] == '}' && state != TOP_LEVEL && !is_escaped)
            {
                state = TOP_LEVEL;
                // TODO: Update this to accept formatting commands after
                // we start to parse them.
                pieces.back().append(args_list[argument_pos++].getData());

				if (pieces.size() > 0 && pieces.back().size() > 0)
				{
					total_size += pieces.back().size();
	                pieces.push_back(std::string(""));
				}
				else if (pieces.size() == 0)
					pieces.push_back(std::string(""));
            }
            else if (state != TOP_LEVEL)
            {

                // Do inner processing here.
				if (state == INNER_PART1) // field name
				{
					if (txt[0] != '!')
						form_specs.back().name += txt[0]; // add to the name, since that's the state we're in
					else
						state = INNER_PART2;
				}
				if (state == INNER_PART2) // conversion
				{
					// TODO: Throw an exception on more than one character on a conversion.
					if (txt[0] != ':')
						form_specs.back().name += txt[0];
					else
						state = INNER_PART3;
				}
            }
            else if (state == TOP_LEVEL)
            {
                pieces.back() += txt[0];
            }

            if (is_escaped && (txt[0] == '{' || txt[0] == '}'))
                txt++;
        }

		std::string output;
		output.reserve(total_size);
        for(auto &s : pieces)
            output += s;
		return output;
    }

}
#endif // PYPRINT_H
