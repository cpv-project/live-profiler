#pragma once
#include "BasePerfLinuxCollector.hpp"
#include "../Models/CpuSampleModel.hpp"

namespace LiveProfiler {
	/**
	 * Collector for collecting cpu samples on linux, based on perf_events
	 *
	 * Q: Why callchain is incomplete for my program?
	 * A: Backtrace is based on frame pointer, please compile with -fno-omit-frame-pointer option.
	 */
	class CpuSampleLinuxCollector : public BasePerfLinuxCollector<CpuSampleModel> {
	public:
		/**
		 * Set whether to include callchain in samples.
		 * Exclude callchain in samples will improve performance.
		 * Default value is true.
		 */
		void setIncludeCallChain(bool includeCallChain) {
			if (includeCallChain) {
				sampleType_ |= PERF_SAMPLE_CALLCHAIN;
			} else {
				sampleType_ &= ~PERF_SAMPLE_CALLCHAIN;
			}
		}

		/** Constructor */
		CpuSampleLinuxCollector() : BasePerfLinuxCollector(
			PERF_TYPE_SOFTWARE,
			PERF_COUNT_SW_CPU_CLOCK,
			PERF_SAMPLE_IP | PERF_SAMPLE_TID | PERF_SAMPLE_CALLCHAIN) { }

	protected:
		/** Take samples for executing instruction (actually is the next instruction) */
		void takeSamples(std::unique_ptr<LinuxPerfEntry>& entry) override {
			assert(entry != nullptr);
			auto& records = entry->getRecords();
			for (auto* record : records) {
				// check if the record is sample
				if (record->type != PERF_RECORD_SAMPLE) {
					continue;
				}
				auto* data = reinterpret_cast<const CpuSampleRawData*>(record);
				// setup model data
				auto result = resultAllocator_.allocate();
				auto ip = data->ip;
				result->setIp(ip);
				result->setPid(data->pid);
				result->setTid(data->tid);
				result->setSymbolName(nullptr);
				auto& callChainIps = result->getCallChainIps();
				auto& callChainSymbolNames = result->getCallChainSymbolNames();
				if (data->header.size >= sizeof(CpuSampleRawDataWithCallChain)) {
					auto* dataWithCallChain = reinterpret_cast<const CpuSampleRawDataWithCallChain*>(data);
					for (std::size_t i = 0; i < dataWithCallChain->nr; ++i) {
						auto callChainIp = dataWithCallChain->ips[i];
						// don't include special instruction pointer
						if ((callChainIp & SpecialInstructionPointerMask) == SpecialInstructionPointerMask) {
							continue;
						}
						// don't include self
						if (callChainIp == ip) {
							continue;
						}
						callChainIps.emplace_back(callChainIp);
						callChainSymbolNames.emplace_back(nullptr);
					}
				}
				// append model data
				results_.emplace_back(std::move(result));
			}
			// all records handled, update read offset
			entry->updateReadOffset();
		}

		/**
		 * There some instruction pointer should be exclude from callchain like 0xfffffffffffffe00.
		 * They looks like a switch between kernel space and user space.
		 * This mask works with both x64 and i386.
		 */
		static const std::uint64_t SpecialInstructionPointerMask = 0xffffffffffff0000;

		/** See man perf_events, section PERF_RECORD_SAMPLE */
		struct CpuSampleRawData {
			::perf_event_header header;
			std::uint64_t ip;
			std::uint32_t pid;
			std::uint32_t tid;
		};

		/** See man perf_events, section PERF_RECORD_SAMPLE */
		struct CpuSampleRawDataWithCallChain {
			CpuSampleRawData data;
			std::uint64_t nr;
			std::uint64_t ips[];
		};
	};
}

