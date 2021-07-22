#include "Result.hpp"

namespace Mountain
{
	std::string to_string(const Result& result) 
	{
		switch(result)
		{
			case Mountain::Result::eSuccess:
				return "Success";
			case Mountain::Result::eNoSuitableDevices:
				return "No suitable devices";
			case Mountain::Result::eUnknown:
			default:
				return "Unknown error";
		}

	}
}


std::ostream& operator<<(std::ostream& os, const Mountain::Result& result)
{
	return os << Mountain::to_string(result);
}
