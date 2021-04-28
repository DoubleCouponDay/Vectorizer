#pragma once

#include <iostream>

namespace vectorizer
{
	class ISerializable
	{
		virtual void Serialize(std::ostream& write_stream) const = 0;
		virtual void Deserialize(std::istream& read_stream) = 0;
	};
}