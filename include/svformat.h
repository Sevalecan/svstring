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

		enum // a value of 0 for any of these bits implies default conditions.
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
		std::bitset<10> states;
		std::string name;
		char conversion;
		char fill;
		char align;
		char sign;
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

		_VarArg(const int in)
		{
			data.i_a = in;
			type = A_INT;
		}

		_VarArg(const double in)
		{
			data.d_a = in;
			type = A_DOUBLE;
		}

		_VarArg(const char *in)
		{
			text_data = std::string(in);
			type = A_STRING;
		}

		_VarArg(const std::string in)
		{
			text_data = in;
			type = A_STRING;
		}

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
	
	_Format parse_form(std::string &form)
	{
		using namespace std;
		_Format format;
		const char *start = form.data();
		const char *end = start + form.size();
		int state = 0;
		
		string spec_str;
		
		enum { // formspec scan piece
			
		};
		
		for (const char *txt = start; txt < end; txt++)
		{
			const char &chr = *txt;
			
			if (state == 0 && chr == '!')
				state = 1;
			else if (chr == ':')
			{
				spec_str = string(txt+1, end - (txt+1));
				break;
			}
			else if (state == 0)
			{
				format.name += chr;
			}
			else if (state == 1)
			{
				format.states[_Format::CONVERSION] = 1;
				format.conversion = chr; // set the conversion character.
				// FIXME: throw exception if we have more than one
				// or an invalid character
			}
		}
		
		if (spec_str.size())
		{
			size_t zero_offset = 0; // search offset for that "0" prefix.
			size_t achar_loc = spec_str.find_last_of("<>=^"); // alignment char
			std::string second_half(spec_str); // half after fill and align.
			if (achar_loc != string::npos)
			{
				// FIXME: Throw if this char is farther than location 1.
				if (achar_loc > 0)
				{
					format.fill = spec_str[0];
					format.states[_Format::FILL] = 1;
					
					++zero_offset;
				}
				format.align = spec_str[achar_loc];
				format.states[_Format::ALIGN] = 1;
				
				second_half = spec_str.data()+achar_loc+1;
				// increase search offset for 0
				++zero_offset;
			}
			
			// Process the second half, [sign][#][0][width][,][.precision][type]
			size_t sign_loc = second_half.find_first_of(" +-");
			if (sign_loc != string::npos)
			{
				format.sign = second_half[sign_loc];
				format.states[_Format::SIGN] = 1;
				// increase search offset for 0
				++zero_offset;
			}
			
			size_t sharp_loc = second_half.find_first_of("#");
			if (sharp_loc != string::npos)
			{
				format.alternate = true;
				format.states[_Format::ALTERNATE] = 1;
				// increase search offset for 0
				++zero_offset;
			}
			
			size_t end_offset = zero_offset;
			if (zero_offset < second_half.size() && second_half.at(zero_offset) == '0')
			{
				format.zero_fill = true;
				format.states[_Format::ZERO_FILL] = 1;
				++end_offset;
			}
			
			if (end_offset < second_half.size())
			{ // Reuse zero offset
				second_half = second_half.substr(end_offset);
				size_t comma_loc = second_half.find(',');
				size_t period_loc = second_half.find('.');
				const string slist("0123456789.,");
				
				if (slist.find(second_half.back()) == string::npos)
				{
					// We have a type char.
					format.type = second_half.back();
					format.states[_Format::TYPE] = 1;
				}
				
				if (period_loc != string::npos)
				{
					// found precision specifier
					size_t prec_end = second_half.find_first_not_of("0123456789", period_loc+1);
					string precision = second_half.substr(period_loc+1, prec_end);
					format.precision = stoi(precision);
					format.states[_Format::PRECISION] = 1;
				}
				
				if (comma_loc != string::npos)
				{
					format.thousands_comma = true;
					format.states[_Format::THOUSANDS_COMMA] = 1;
				}
				
				size_t width_end = period_loc > comma_loc ? comma_loc : period_loc;
				
				string width = second_half.substr(0, width_end);
				if (width.size() > 0)
				{
					format.width = stoi(width);
					format.states[_Format::WIDTH] = 1;
				}
			}
			
		}
		
		return format;
	}
	
	std::vector<std::pair<bool, std::string>> get_forms(std::string &in)
	{
		using namespace std;
		const char *start = in.data();
		const char *end = start+in.size();
		typedef pair<bool, string> fpair;
		vector<fpair> forms;
		
		const char *forma;
		const char *formb = start;
		int state = 0;
		
		for(const char *txt = start; txt < end; ++txt)
		{
			const char &current = *txt;
			const char &next = *(txt+1);
			const bool is_end = txt + 1 >= end;
			const bool is_escaped = !is_end ? current == next : 0;
			// Will only matter for brackets.
			if (current == '{' && state == 0 && !is_escaped && !is_end)
			{
				if (txt > formb)
					forms.push_back(fpair(false, string(formb, txt-formb)));
				state = 1;
				forma = txt+1;
			}
			else if (current == '}' && state == 1 && !is_escaped)
			{
				formb = txt+1;
				forms.push_back(fpair(true, string(forma, txt-forma)));
				state = 0;
			}
		}
		
		return forms;
	}

	template<typename T, typename... Args> std::string format(T a1, Args... args)
	{
		using namespace std;
		string str{a1};
		vector<_VarArg> args_list = {args...};

		const char *start = str.data();
		const char *end = start + str.size();
		size_t total_size = 0;
		size_t argument_pos = 0;
		auto forms = get_forms(a1);

		string output;
		output.reserve(total_size);
		
#ifdef DEBUG
		for (auto &s : forms)
		{
			_Format x;
			cout << "New form: " << s.first << ", '" << s.second << "'" << std::endl;
			if (s.first)
			{
				x = parse_form(s.second);
				cout << "Format bitset: " << x.states.to_string() << "\n";
				if (x.name.size())
					cout << " name: " << x.name << endl;
				if (x.states[_Format::CONVERSION])
					cout << " conversion: '" << x.conversion << "'" << endl;
				if (x.states[_Format::FILL])
				cout << " fill: '" << x.fill << "'" << endl;
				if (x.states[_Format::ALIGN])
				cout << " align: '" << x.align << "'" << endl;
				if (x.states[_Format::SIGN])
				cout << " sign: '" << x.sign << "'" << endl;
				if (x.states[_Format::ALTERNATE])
				cout << " alternate? " << x.alternate << endl;
				if (x.states[_Format::ZERO_FILL])
				cout << " zero_fill? " << x.zero_fill << endl;
				if (x.states[_Format::THOUSANDS_COMMA])
				cout << " thousands_comma? " << x.thousands_comma << endl;
				if (x.states[_Format::WIDTH])
				cout << " width: " << x.width << endl;
				if (x.states[_Format::PRECISION])
				cout << " precision: " << x.precision << endl;
				if (x.states[_Format::TYPE])
				cout << " type: '" << x.type << "'" << endl;
			}
		}
#endif
		return output;
	}

}
#endif // PYPRINT_H
