#include <esl/module/Library.h>
#include <curl4esl/Module.h>

extern "C" esl::module::Module* esl__module__library__getModule(const std::string& moduleName) {
	return curl4esl::getModulePointer(moduleName);
}
