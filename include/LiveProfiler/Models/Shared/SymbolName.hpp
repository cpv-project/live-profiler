#pragma once
#include <string>
#include <memory>

namespace LiveProfiler {
	/**
	 * Represent a symbol name in the executable file.
	 * The originalName and path should not be empty, the demangleName can be empty.
	 */
	class SymbolName {
	public:
		/** Getters and Setters **/
		const std::string& getOriginalName() const& { return originalName_; }
		const std::string& getDemangleName() const& { return demangleName_; }
		const std::shared_ptr<std::string>& getPath() const& { return path_; }
		std::size_t getFileOffset() const { return fileOffset_; }
		std::size_t getSymbolSize() const { return symbolSize_; }
		void setOriginalName(const std::string& name) { originalName_ = name; }
		void setDemangleName(const std::string& name) { demangleName_ = name; }
		void setPath(const std::shared_ptr<std::string>& path) { path_ = path; }
		void setFileOffset(std::size_t offset) { fileOffset_ = offset; }
		void setSymbolSize(std::size_t size) { symbolSize_ = size; }

		/** Return either the demangle name or the orignal name */
		const std::string& getName() const& {
			return demangleName_.empty() ? originalName_ : demangleName_;
		}

		/** Constructor */
		SymbolName() :
			originalName_(),
			demangleName_(),
			path_(),
			fileOffset_(0),
			symbolSize_(0) { }

	protected:
		std::string originalName_;
		std::string demangleName_;
		std::shared_ptr<std::string> path_;
		std::size_t fileOffset_;
		std::size_t symbolSize_;
	};
}

