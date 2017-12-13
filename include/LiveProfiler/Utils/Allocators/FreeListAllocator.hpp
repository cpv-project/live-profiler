#pragma once
#include <memory>
#include <vector>
#include <cassert>

namespace LiveProfiler {
	/**
	 * Class used to allocate and reuse instances.
	 * T should provide two functions:
	 * - freeResources: called in deallocate, with forwarded parameters
	 * - reset: called in allocate, with forwarded parameters
	 * This is not a standard allocator, it's type safe but not thread safe.
	 */
	template <class T>
	class FreeListAllocator {
	public:
		/** Allocate T instance */
		template <class... Args>
		std::unique_ptr<T> allocate(Args&&... args) {
			if (free_.empty()) {
				auto instance = std::make_unique<T>();
				instance->reset(std::forward<Args>(args)...);
				return instance;
			} else {
				auto instance = std::move(free_.back());
				free_.pop_back();
				instance->reset(std::forward<Args>(args)...);
				return instance;
			}
		}

		/** Deallocate T instance */
		template <class... Args>
		void deallocate(std::unique_ptr<T>&& instance, Args&&... args) {
			assert(instance != nullptr);
			instance->freeResources(std::forward<Args>(args)...);
			if (free_.size() < maxFree_) {
				free_.emplace_back(std::move(instance));
			}
		}

		/** Constructor */
		explicit FreeListAllocator(std::size_t maxFree) :
			free_(),
			maxFree_(maxFree) { }

	protected:
		std::vector<std::unique_ptr<T>> free_;
		std::size_t maxFree_;
	};
}

