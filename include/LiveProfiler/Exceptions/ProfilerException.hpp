#pragma once
#include <exception>

namespace LiveProfiler {
	class ProfilerException : public std::runtime_error {
	public:
		using runtime_error::runtime_error;

		/** Constructor like std::system_error */
		ProfilerException(int errorCode, const std::string& message) :
			ProfilerException(errorCode, std::generic_category(), message) { }

		/** Constructor like std::system_error */
		ProfilerException(int errorCode, const std::error_category& category, const std::string& message) :
			ProfilerException(message + ": " + std::error_code(errorCode, category).message()) { }
	};
}

