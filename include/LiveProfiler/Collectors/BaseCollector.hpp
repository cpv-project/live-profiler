#pragma once
#include <memory>
#include <chrono>

namespace LiveProfiler {
	/**
	 * The base class for the collector
	 * 
	 * Terms:
	 * - The collector should be able to reset the state to it's initial state
	 * - The collector should support enabling and disabling collection
	 * - The collector should support collecting performance data for the specified timeout period
	 * - The collector should return different data each time `collect` is called
	 * - The collector should not know there is a class called Profiler
	 */
	template <class Model>
	class BaseCollector :
		public std::enable_shared_from_this<BaseCollector<Model>> {
	public:
		/** Reset the state to it's initial state */
		virtual void reset() = 0;

		/** Enable performance data collection */
		virtual void enable() = 0;

		/** Collect performance data for the specified timeout period */
		virtual const std::vector<Model>& collect(
			std::chrono::high_resolution_clock::duration timeout) = 0;

		/** Disable performance data collection */
		virtual void disable() = 0;
	};
}

