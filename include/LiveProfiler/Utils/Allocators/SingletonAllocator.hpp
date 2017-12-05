#pragma once
#include <memory>
#include <unordered_map>

namespace LiveProfiler {
	/**
	 * Class used to allocate the same instance from the same key.
	 * T should be constructible from TKey.
	 * This class is not thread safe.
	 */
	template <class TKey, class T>
	class SingletonAllocator {
	public:
		/** Allocate T instance */
		std::shared_ptr<T> allocate(const TKey& key) {
			auto it = mapping_.find(key);
			if (it == mapping_.end()) {
				auto pair = mapping_.emplace(key, std::make_shared<T>(key));
				it = pair.first;
			}
			return it->second;
		}

		/** Constructor */
		SingletonAllocator() : mapping_() { }

	protected:
		std::unordered_map<TKey, std::shared_ptr<T>> mapping_;
	};
}

