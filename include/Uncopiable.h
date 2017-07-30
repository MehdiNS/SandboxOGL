#pragma once

namespace OGL
{
	class Uncopiable
	{
	public:
		Uncopiable(const Uncopiable&) = delete;
		Uncopiable& operator=(const Uncopiable&) = delete;
		Uncopiable(Uncopiable&&) = default;
		Uncopiable& operator=(Uncopiable&&) = default;
	protected:
		constexpr Uncopiable() = default;
		~Uncopiable() = default;
	};
}