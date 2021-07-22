#pragma once

#ifndef C35CACA9_4DFC_4375_B3C7_ECB578BD60EC
#define C35CACA9_4DFC_4375_B3C7_ECB578BD60EC

#include <string>
#include <iostream>

namespace Mountain
{

	enum class Result
	{
		eSuccess,
		eNoSuitableDevices = 1,

		eUnknown
	};




	template<typename Enum, typename T>
	struct ResultType
	{

		using type = std::tuple<Enum, T>;
	};

	template<typename Enum>
	struct ResultType<Enum, void>
	{
		using type = Enum;
	};

}

namespace Mountain
{
	std::string to_string(const Result& result);
}

std::ostream& operator<<(std::ostream& os, const Mountain::Result& result);


#endif /* C35CACA9_4DFC_4375_B3C7_ECB578BD60EC */
