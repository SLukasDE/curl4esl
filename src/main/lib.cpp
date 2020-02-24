#include <esl/module/Library.h>
#include <curl4esl/Module.h>

esl::module::Library::GetModule esl__module__library__getModule = &curl4esl::getModule;
