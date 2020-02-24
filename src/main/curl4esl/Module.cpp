/*
MIT License
Copyright (c) 2019, 2020 Sven Lukas

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <curl4esl/Module.h>
#include <curl4esl/Connection.h>

#include <esl/http/client/Interface.h>
#include <esl/module/Interface.h>

#include <memory>
#include <new>         // placement new
#include <type_traits> // aligned_storage

namespace curl4esl {

namespace {

class Module : public esl::module::Module {
public:
	Module();
};

typename std::aligned_storage<sizeof(Module), alignof(Module)>::type moduleBuffer; // memory for the object;
Module& module = reinterpret_cast<Module&> (moduleBuffer);
bool isInitialized = false;

esl::http::client::Interface::Connection* createConnection(
		const std::string& hostUrl,
        const long timeout,
        const std::string& username,
        const std::string& password,
        const std::string& proxy,
        const std::string& proxyUser,
        const std::string& proxyPassword,
        const std::string& userAgent) {
	return new Connection(hostUrl, timeout, username, password, proxy, proxyUser, proxyPassword, userAgent);
}

Module::Module()
: esl::module::Module()
{
	esl::module::Module::initialize(*this);

	addInterface(std::unique_ptr<const esl::module::Interface>(new esl::http::client::Interface(
			getId(), "curl", &createConnection)));
}

} /* anonymous namespace */

esl::module::Module& getModule() {
	if(isInitialized == false) {
		/* ***************** *
		 * initialize module *
		 * ***************** */

		isInitialized = true;
		new (&module) Module(); // placement new
	}
	return module;
}

} /* namespace curl4esl */
