#ifndef SVFORMAT_H
#define SVFORMAT_H
#include <cstdio>
#include <iostream>
#include <string>
#include <algorithm>
#include <array>
#include <bitset>
#include <map>

namespace svstring
{
	// TODO: Possibly combine some of the format data and the format() and all
	// functions it calls into a class object so that they can share
	// a single state. Make sure this doesn't conflict with the idea that we
	// can overload templates with custom version as a user.
	
	struct Format
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

		Format()
		{
			states = 0;
		}
	};
	
	// So we can associate an output string with a single form when passing to the converter.
	struct FormPack
	{
		Format form;
		std::string *ostr;
		FormPack(std::string &p_ostr, const Format &p_form) : form(p_form), ostr(&p_ostr) {}
		FormPack(const FormPack &ipair) : form(ipair.form), ostr(ipair.ostr) {}
		
	};
	
	typedef std::vector<std::pair<bool, std::string>> FormList;
	typedef std::multimap<std::string, FormPack> _Forms;
	
	struct _FormatState
	{
		int32_t numeric_count;
		_Forms form_list;
		
		_FormatState()
		{
			numeric_count = 0;
		}
	};
	
	Format parse_form(std::string &form)
	{
		using namespace std;
		Format format;
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
				format.states[Format::CONVERSION] = 1;
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
					format.states[Format::FILL] = 1;
					
					++zero_offset;
				}
				format.align = spec_str[achar_loc];
				format.states[Format::ALIGN] = 1;
				
				second_half = spec_str.substr(achar_loc+1);
				// increase search offset for 0
				++zero_offset;
			}
			
			// Process the second half, [sign][#][0][width][,][.precision][type]
			size_t sign_loc = second_half.find_first_of(" +-");
			if (sign_loc != string::npos)
			{
				format.sign = second_half[sign_loc];
				format.states[Format::SIGN] = 1;
				// increase search offset for 0
				++zero_offset;
			}
			
			size_t sharp_loc = second_half.find_first_of("#");
			if (sharp_loc != string::npos)
			{
				format.alternate = true;
				format.states[Format::ALTERNATE] = 1;
				// increase search offset for 0
				++zero_offset;
			}
			
			size_t end_offset = zero_offset;
			if (zero_offset < second_half.size() && second_half.at(zero_offset) == '0')
			{
				format.zero_fill = true;
				format.states[Format::ZERO_FILL] = 1;
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
					format.states[Format::TYPE] = 1;
				}
				
				if (period_loc != string::npos)
				{
					// found precision specifier
					size_t prec_end = second_half.find_first_not_of("0123456789", period_loc+1);
					string precision = second_half.substr(period_loc+1, prec_end);
					format.precision = stoi(precision);
					format.states[Format::PRECISION] = 1;
				}
				
				if (comma_loc != string::npos)
				{
					format.thousands_comma = true;
					format.states[Format::THOUSANDS_COMMA] = 1;
				}
				
				size_t width_end = period_loc > comma_loc ? comma_loc : period_loc;
				
				string width = second_half.substr(0, width_end);
				if (width.size() > 0)
				{
					format.width = stoi(width);
					format.states[Format::WIDTH] = 1;
				}
			}
			
		}
		
		return format;
	}
	
	FormList get_forms(std::string &in)
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
	
	template<typename T> void convert(FormPack &formp, const T &a1);
	// template<typename T> std::string convert(FormList::iterator &form, const T *a1);
	
	template<> void convert<std::string>(FormPack &formp, const std::string &a1)
	{
		*(formp.ostr) = a1;
	}
	
	template<> void convert<double>(FormPack &formp, const double &a1)
	{
		*(formp.ostr) = std::to_string(a1);
	}
	
	template<> void convert<int>(FormPack &formp, const int &a1)
	{
		*(formp.ostr) = std::to_string(a1);
	}
	/*
	template<> std::string convert<const char>(FormList::iterator &form, const char *a1)
	{
		return std::string(a1);
	}
	*/
	
	// TODO: Make a complementary function that specializes for NamedArgument as type T
	// Also try to make it not have a cow if we're out of numeric forms, and perhaps throw a warning.. Check
	// Python documentation for what it does in this case.
	template<typename T> void _form_disambiguator(_FormatState &format_state, T &a1)
	{
		// Unnamed numeric case.
		std::string name = std::to_string(format_state.numeric_count++);
		auto range = format_state.form_list.equal_range(name);
		for (auto i = range.first; i != range.second; ++i)
		{
			svstring::convert(i->second, a1);
		}
	}
	
	template<typename T> void _conv_wrapper(_FormatState &format_state, T &a1) // final version.
	{
		svstring::_form_disambiguator(format_state, a1);
	}
	
	template<typename T, typename... Args> void _conv_wrapper(_FormatState &format_state, T &a1, Args... args)
	{
		svstring::_form_disambiguator(format_state, a1);
		svstring::_conv_wrapper(format_state, args...);
	}

	template<typename T, typename... Args> std::string format(T a1, Args... args)
	{
		using namespace std;
		string str{a1}; // Convert it to a string if it isn't already

		size_t total_size = 0;
		FormList forms = get_forms(str);

		string output;
		output.reserve(total_size);
		int numbered_form = 0;
		_FormatState out_state;
		_Forms &out_forms = out_state.form_list;
		
		for (auto &s : forms)
		{
			if (s.first == true)
			{
				Format x = parse_form(s.second);
				s.second.resize(0);
			
				if (x.name.size() == 0)
				{
					out_forms.emplace(std::to_string(numbered_form++), FormPack(s.second, x));
				}
				else
				{
					out_forms.emplace(x.name, FormPack(s.second, x));
				}
			}
		}
		
		_conv_wrapper(out_state, args...);
		
		for (auto &s : forms)
			output.append(s.second);

#ifdef DEBUG
		for (auto &s : forms)
		{
			Format x;
			cout << "New form: " << s.first << ", '" << s.second << "'" << std::endl;
			if (s.first)
			{
				x = parse_form(s.second);
				cout << "Format bitset: " << x.states.to_string() << "\n";
				if (x.name.size())
					cout << " name: " << x.name << endl;
				if (x.states[Format::CONVERSION])
					cout << " conversion: '" << x.conversion << "'" << endl;
				if (x.states[Format::FILL])
				cout << " fill: '" << x.fill << "'" << endl;
				if (x.states[Format::ALIGN])
				cout << " align: '" << x.align << "'" << endl;
				if (x.states[Format::SIGN])
				cout << " sign: '" << x.sign << "'" << endl;
				if (x.states[Format::ALTERNATE])
				cout << " alternate? " << x.alternate << endl;
				if (x.states[Format::ZERO_FILL])
				cout << " zero_fill? " << x.zero_fill << endl;
				if (x.states[Format::THOUSANDS_COMMA])
				cout << " thousands_comma? " << x.thousands_comma << endl;
				if (x.states[Format::WIDTH])
				cout << " width: " << x.width << endl;
				if (x.states[Format::PRECISION])
				cout << " precision: " << x.precision << endl;
				if (x.states[Format::TYPE])
				cout << " type: '" << x.type << "'" << endl;
			}
		}
#endif
		return output;
	}

}
#endif // PYPRINT_H
