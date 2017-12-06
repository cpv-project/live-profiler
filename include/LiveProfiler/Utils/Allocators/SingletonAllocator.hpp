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

		/** Allocate T instance, avoid universal reference overload take TKey&& */
		std::shared_ptr<T> allocate(TKey&& key) {
			return allocate(static_cast<const TKey&>(key));
		}

		/**
		 * Allocate T instance.
		 * Use a temporary key to avoid memory allocation.
		 * T should be std::string or any types that have `assign` function.
		 */
		template <class... Args>
		std::shared_ptr<T> allocate(Args&&... args) {
			tempKey_.assign(std::forward<Args>(args)...);
			return allocate(static_cast<const TKey&>(tempKey_));
		}

		/** Constructor */
		SingletonAllocator() : tempKey_(), mapping_() { }

	protected:
		TKey tempKey_;
		std::unordered_map<TKey, std::shared_ptr<T>> mapping_;
	};
}

