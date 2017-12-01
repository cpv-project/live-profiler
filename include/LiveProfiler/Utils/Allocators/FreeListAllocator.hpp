#pragma once
#include <memory>
#include <vector>

namespace LiveProfiler {
	/**
	 * Class used to allocate and reuse instances.
	 * T should provide two functions:
	 * - freeResources: called in deallocate
	 * - reset: called in allocate
	 * This is not a standard allocator, it's type safe but not thread safe.
	 */
	template <class T>
	class FreeListAllocator {
	public:
		/** Allocate T instance */
		std::unique_ptr<T> allocate() {
			if (free_.empty()) {
				auto instance = std::make_unique<T>();
				return instance;
			} else {
				auto instance = std::move(free_.back());
				free_.pop_back();
				instance->reset();
				return instance;
			}
		}

		/** Deallocate T instance */
		void deallocate(std::unique_ptr<T>&& instance) {
			instance->freeResources();
			if (free_.size() < maxFree_) {
				free_.emplace_back(std::move(instance));
			}
		}

		/** Constructor */
		FreeListAllocator(std::size_t maxFree) :
			free_(),
			maxFree_(maxFree) { }

	protected:
		std::vector<std::unique_ptr<T>> free_;
		std::size_t maxFree_;
	};
}

