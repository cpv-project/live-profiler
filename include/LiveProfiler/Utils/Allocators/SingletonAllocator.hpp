#pragma once
#include <memory>
#include <unordered_map>
#include <type_traits>

namespace LiveProfiler {
	/**
	 * Class used to allocate the same instance from the same key.
	 * T should be constructible with the arguments from allocate function.
	 * This class is not thread safe.
	 */
	template <class TKey, class T>
	class SingletonAllocator {
	public:
		/** Allocate T instance */
		template <class Head, class... Rest,
			std::enable_if_t<std::is_same<std::decay_t<Head>, std::decay_t<TKey>>::value, int> = 0>
		std::shared_ptr<T> allocate(Head&& key, Rest&&... rest) {
			auto it = mapping_.find(key);
			if (it == mapping_.end()) {
				auto pair = mapping_.emplace(key,
					std::make_shared<T>(std::forward<Head>(key), std::forward<Rest>(rest)...));
				it = pair.first;
			}
			return it->second;
		}

		/**
		 * Allocate T instance.
		 * Use a temporary key to avoid memory allocation.
		 * T should be std::string or any types that have `assign` function.
		 */
		template <class Head, class... Rest,
			std::enable_if_t<!std::is_same<std::decay_t<Head>, std::decay_t<TKey>>::value, int> = 0>
		std::shared_ptr<T> allocate(Head&& head, Rest&&... rest) {
			tempKey_.assign(std::forward<Head>(head), std::forward<Rest>(rest)...);
			return allocate(static_cast<const TKey&>(tempKey_));
		}

		/** Constructor */
		SingletonAllocator() : tempKey_(), mapping_() { }

	protected:
		TKey tempKey_;
		std::unordered_map<TKey, std::shared_ptr<T>> mapping_;
	};
}

