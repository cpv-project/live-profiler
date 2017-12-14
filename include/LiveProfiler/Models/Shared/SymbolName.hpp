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
		std::size_t getFileOffsetStart() const { return fileOffsetStart_; }
		std::size_t getFileOffsetEnd() const { return fileOffsetEnd_; }
		void setOriginalName(const std::string& name) { originalName_ = name; }
		void setDemangleName(const std::string& name) { demangleName_ = name; }
		void setPath(const std::shared_ptr<std::string>& path) { path_ = path; }
		void setFileOffsetStart(std::size_t offset) { fileOffsetStart_ = offset; }
		void setFileOffsetEnd(std::size_t offset) { fileOffsetEnd_ = offset; }

		/** Return either the demangle name or the orignal name */
		const std::string& getName() const& {
			return demangleName_.empty() ? originalName_ : demangleName_;
		}

		/** Constructor */
		SymbolName() : SymbolName("", nullptr) { }

		/** Constructor */
		SymbolName(
			const std::string& name,
			const std::shared_ptr<std::string>& path) :
			originalName_(name),
			demangleName_(),
			path_(path),
			fileOffsetStart_(0),
			fileOffsetEnd_(0) { }

	protected:
		std::string originalName_;
		std::string demangleName_;
		std::shared_ptr<std::string> path_;
		std::size_t fileOffsetStart_;
		std::size_t fileOffsetEnd_;
	};
}

