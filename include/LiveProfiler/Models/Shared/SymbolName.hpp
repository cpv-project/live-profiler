#pragma once
#include <string>
#include <memory>

namespace LiveProfiler {
	/** Represent a symbol name in the executable file */
	class SymbolName {
	public:
		/** Getters and Setters **/
		const std::string& getOriginalName() const& { return originalName_; }
		const std::string& getDemangleName() const& { return demangleName_; }
		const std::shared_ptr<std::string>& getPath() const& { return path_; }
		void setOriginalName(const std::string& name) { originalName_ = name; }
		void setDemangleName(const std::string& name) { demangleName_ = name; }
		void setPath(const std::shared_ptr<std::string>& path) { path_ = path; }

		/** Constructor */
		SymbolName() :
			originalName_(),
			demangleName_(),
			path_() { }

	protected:
		std::string originalName_;
		std::string demangleName_;
		std::shared_ptr<std::string> path_;
	};
}

