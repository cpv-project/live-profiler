#pragma once
#include <memory>
#include <vector>
#include <utility>
#include <chrono>
#include <algorithm>
#include <type_traits>
#include "../Exceptions/ProfilerException.hpp"
#include "../Collectors/BaseCollector.hpp"
#include "../Analyzers/BaseAnalyzer.hpp"
#include "../Interceptors/BaseInterceptor.hpp"

namespace LiveProfiler {
	/**
	 * Profiler entry point class
	 *
	 * Terms:
	 * - The profiler should have a model class
	 * - The profiler should have exactly one collector
	 * - The profiler may have one or more analyzers
	 * - The profiler should not be copied, or used in multiple threads
	 * - The collector should not know there is a class called Profiler
	 * - The analyzers should not know there is a class called Profiler
	 * - The interceptors should not know there is a class called Profiler
	 *
	 * How does it work:
	 * When function `collectFor` is called,
	 * profiler will get the performance data from collector in real time and feed analyzers.
	 * When `collectFor` finished, you can get result from analyzers.
	 * Performance data will retained between `collectFor`, you can clear it with function `reset`.
	 *
	 * Example:
	 * TODO
	 */
	template <class Model>
	class Profiler {
	public:
		using ProfilerType = std::shared_ptr<Profiler<Model>>;
		using CollectorType = std::shared_ptr<BaseCollector<Model>>;
		using AnalyzerType = std::shared_ptr<BaseAnalyzer<Model>>;
		using InterceptorType = std::shared_ptr<BaseInterceptor<Model>>;

		/** Use specified collector, replaces the old collector if this function is called twice */
		template <class Collector, class... Args>
		std::shared_ptr<Collector> useCollector(Args&&... args) {
			auto collector = std::make_shared<Collector>(std::forward<Args>(args)...);
			collector_ = collector;
			return collector;
		}

		/** Add analyzer to analyzer list */
		template <class Analyzer, class... Args>
		std::shared_ptr<Analyzer> addAnalyzer(Args&&... args) {
			auto analyzer = std::make_shared<Analyzer>(std::forward<Args>(args)...);
			analyzers_.emplace_back(analyzer);
			return analyzer;
		}

		/* Remove analyzer from analyzer list, return whether the analyzer is in the list */
		bool removeAnalyzer(const AnalyzerType& analyzer) {
			auto it = std::remove(analyzers_.begin(), analyzers_.end(), analyzer);
			auto removed = it != analyzers_.end();
			analyzers_.erase(it, analyzers_.end());
			return removed;
		}

		/** Add interceptor to interceptor list */
		template <class Interceptor, class... Args>
		std::shared_ptr<Interceptor> addInterceptor(Args&&... args) {
			auto interceptor = std::make_shared<Interceptor>(std::forward<Args>(args)...);
			interceptors_.emplace_back(interceptor);
			return interceptor;
		}

		/** Remove interceptor from interceptor list, return whether the interceptor is in the list */
		bool removeInterceptor(const InterceptorType& interceptor) {
			auto it = std::remove(interceptors_.begin(), interceptors_.end(), interceptor);
			auto removed = it != interceptors_.end();
			interceptors_.erase(it, interceptors_.end());
			return removed;
		}

		/** Reset state of collectors and analyzers */
		void reset() {
			collector_->reset();
			for (auto& analyzer : analyzers_) {
				analyzer->reset();
			}
			for (auto& interceptor : interceptors_) {
				interceptor->reset();
			}
		}

		/** collect data within specified time */
		template <class Rep, class Period>
		void collectFor(const std::chrono::duration<Rep, Period>& time) {
			auto collector = collector_;
			if (collector == nullptr) {
				throw ProfilerException("[collectFor] please call `useCollector` first");
			}
			auto start = std::chrono::high_resolution_clock::now();
			CollectorGuard guard(collector);
			while (true) {
				auto now = std::chrono::high_resolution_clock::now();
				auto elapsed = now - start;
				if (time <= elapsed) {
					break;
				}
				auto timeout = time - elapsed;
				auto& models = collector->collect(timeout);
				for (auto& interceptor : interceptors_) {
					interceptor->alter(models);
				}
				for (auto& analyzer : analyzers_) {
					analyzer->feed(models);
				}
			}
		}

		/** Constructor */
		Profiler() :
			collector_(),
			analyzers_(),
			interceptors_() { }
	
	protected:
		/** Disable copy */
		Profiler(const Profiler&) = delete;
		Profiler& operator=(const Profiler&) = delete;

		/** RAII-style class for enable and disable collector */
		class CollectorGuard {
		public:
			CollectorGuard(CollectorType& collector) : collector_(collector) {
				collector_->enable();
			}
			~CollectorGuard() {
				collector_->disable();
			}
		protected:
			CollectorType& collector_;
		};

	protected:
		CollectorType collector_;
		std::vector<AnalyzerType> analyzers_;
		std::vector<InterceptorType> interceptors_;
	};
}

