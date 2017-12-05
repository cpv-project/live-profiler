#pragma once
#include <sys/types.h>
#include <vector>
#include <memory>
#include <chrono>
#include <utility>
#include "../../Allocators/SingletonAllocator.hpp"
#include "LinuxExecutableSymbolResolver.hpp"
#include "LinuxProcessAddressMap.hpp"

namespace LiveProfiler {
	/**
	 * Class used to find out address is mapped to which file for specificed linux process
	 * The maps is read from /proc/$pid/maps,
	 * because maps may change continuously it needs reload under certain conditions.
	 * And because I want to avoid frequent reload so locate a newly mapped address may fail,
	 * unless `forceReload` option is used.
	 */
	class LinuxProcessAddressLocator {
	public:
		/** Default parameters */
		static const std::size_t DefaultMapsUpdateMinInterval = 1;

		/**
		 * Locate file path and offset for the specified address.
		 * Return (nullptr, 0) if locate failed.
		 * When `forceReload` option is true,
		 * maps will be force to reload after first locate is failed,
		 * it can ensure no newly mapped address is missed but may reduce performance.
		 */
		std::pair<std::shared_ptr<std::string>, std::ptrdiff_t> locate(
			std::intptr_t address, bool forceReload) {
			
		}

		/** Reload maps from /proc/$pid/maps */
		void reload() {
			
		}

		/** Constructor */
		LinuxProcessAddressLocator(
			pid_t pid,
			const std::shared_ptr<SingletonAllocator<std::string, std::string>>& pathAllocator,
			const std::shared_ptr<SingletonAllocator<
				std::string, LinuxExecutableSymbolResolver>>& resolverAllocator) :
			pid_(pid),
			pathAllocator_(pathAllocator),
			resolverAllocator_(resolverAllocator),
			maps_(),
			mapsUpdated_(),
			mapsUpdateMinInterval_(std::chrono::milliseconds(DefaultMapsUpdateMinInterval)),
			line_() { }

	protected:
		pid_t pid_;
		std::shared_ptr<SingletonAllocator<std::string, std::string>> pathAllocator_;
		std::shared_ptr<SingletonAllocator<std::string, LinuxExecutableSymbolResolver>> resolverAllocator_;
		std::vector<LinuxProcessAddressMap> maps_;
		std::chrono::high_resolution_clock::time_point mapsUpdated_;
		std::chrono::high_resolution_clock::duration mapsUpdateMinInterval_;
		std::string line_;
	};
}

