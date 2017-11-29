#pragma once
#include <exception>

namespace LiveProfiler {
	class ProfilerException : std::runtime_error {
	public:
		using runtime_error::runtime_error;
	};
}

