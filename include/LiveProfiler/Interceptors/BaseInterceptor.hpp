#pragma once
#include <memory>

namespace LiveProfiler {
	/**
	 * The base class for the interceptor
	 *
	 * Terms:
	 * - The interceptor should be able to reset the state to it's initial state
	 * - The interceptor should be able to alter performance data at any time
	 * - The interceptor should leave performance data in a valid state after alter
	 * - The interceptor should not know there is a class called Profiler
	 */
	template <class Model>
	class BaseInterceptor :
		public std::enable_shared_from_this<BaseInterceptor<Model>> {
	public:
		/** Reset the state to it's initial state */
		virtual void reset() = 0;

		/** Alter performance data */
		virtual void alter(std::vector<std::unique_ptr<Model>>& models) = 0;

		/** Base destructor should be virtual */
		virtual ~BaseInterceptor() = default;
	};
}

