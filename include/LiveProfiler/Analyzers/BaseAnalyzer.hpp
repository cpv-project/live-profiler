#pragma once
#include <memory>
#include <chrono>

namespace LiveProfiler {
	/**
	 * The base class for the analyzer
	 * 
	 * Terms:
	 * - The analyzer should be able to reset the state to it's initial state
	 * - The analyzer should be able to receive performance data at any time
	 * - The analyzer should provide a function named `getResult`, this function can return any type
	 * - The analysis of performance data can be done in real time, or at the time of `getResult`
	 * - The analyzer should not know there is a class called Profiler
	 */
	template <class Model>
	class BaseAnalyzer :
		public std::enable_shared_from_this<BaseAnalyzer<Model>> {
	public:
		/** Reset the state to it's initial state */
		virtual void reset() = 0;

		/** Receive performance data */
		virtual void feed(const std::vector<Model>& models) = 0;
	};
}

