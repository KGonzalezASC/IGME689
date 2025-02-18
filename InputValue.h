#ifndef __INPUT_VALUE_H__
#define __INPUT_VALUE_H__

#include <type_traits>
#include <optional>
#include <any>

struct InputValue
{
private:
	std::any inputValue;
public:
	InputValue(std::any value) : inputValue(value) {}

	template <typename T>
	std::optional<T> GetValue() const
	{
		if (inputValue.type() == typeid(T))
		{
			return std::any_cast<T>(inputValue);
		}
		return std::nullopt;
	}
};

#endif