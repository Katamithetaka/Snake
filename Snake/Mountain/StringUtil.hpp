#pragma once
#ifndef CDDA9379_034F_4B0E_BB9D_90879A47F848
#define CDDA9379_034F_4B0E_BB9D_90879A47F848

#include <string>
#include <functional>

namespace Mountain 
{

	// Trim code from SO and adapted to take any char
	// https://stackoverflow.com/questions/216823/whats-the-best-way-to-trim-stdstring

	// trim from start (in place)
	inline void ltrim(std::string &s, std::function<bool(unsigned char)> f = std::isspace) {
		s.erase(s.begin(), std::find_if(s.begin(), s.end(), [&](unsigned char ch) {
			return !f(ch);
		}));
	}

	// trim from end (in place)
	inline void rtrim(std::string &s, std::function<bool(unsigned char)> f = std::isspace) {
		s.erase(std::find_if(s.rbegin(), s.rend(), [&](unsigned char ch) {
			return !f(ch);
		}).base(), s.end());
	}

	// trim from both ends (in place)
	inline void trim(std::string &s, std::function<bool(unsigned char)> f = std::isspace) {
		ltrim(s, f);
		rtrim(s, f);
	}

	// trim from start (copying)
	inline std::string ltrim_copy(std::string s, std::function<bool(unsigned char)> f = std::isspace) {
		ltrim(s, f);
		return s;
	}

	// trim from end (copying)
	inline std::string rtrim_copy(std::string s, std::function<bool(unsigned char)> f = std::isspace) {
		rtrim(s, f);
		return s;
	}

	// trim from both ends (copying)
	inline std::string trim_copy(std::string s, std::function<bool(unsigned char)> f = std::isspace) {
		trim(s, f);
		return s;
	}

	// Added: trim with a specific char

	inline void trim(std::string& s, const unsigned char& ch)
	{
		return trim(s, [&](unsigned char c) { return c == ch; });
	}

	inline void ltrim(std::string& s, const unsigned char& ch)
	{
		return ltrim(s, [&](unsigned char c) { return c == ch; });
	}

	inline void rtrim(std::string& s, const unsigned char& ch)
	{
		return rtrim(s, [&](unsigned char c) { return c == ch; });
	}

	inline void trim_copy(std::string s, const unsigned char& ch)
	{
		return trim(s, [&](unsigned char c) { return c == ch; });
	}

	inline void ltrim_copy(std::string s, const unsigned char& ch)
	{
		return ltrim(s, [&](unsigned char c) { return c == ch; });
	}

	inline void rtrim_copy(std::string s, const unsigned char& ch)
	{
		return rtrim(s, [&](unsigned char c) { return c == ch; });
	}
}
#endif /* CDDA9379_034F_4B0E_BB9D_90879A47F848 */
