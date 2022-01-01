/*
MIT License
Copyright (c) 2019-2022 Sven Lukas

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

#ifndef CURL4ESL_COM_HTTP_CLIENT_CONNECTIONFACTORY_H_
#define CURL4ESL_COM_HTTP_CLIENT_CONNECTIONFACTORY_H_

#include <esl/com/http/client/Interface.h>
#include <esl/utility/URL.h>

#include <curl/curl.h>

#include <string>
#include <memory>

namespace curl4esl {
namespace com {
namespace http {
namespace client {

class ConnectionFactory : public esl::com::http::client::Interface::ConnectionFactory {
public:
	static std::unique_ptr<esl::com::http::client::Interface::ConnectionFactory> create(const esl::utility::URL& url, const esl::com::http::client::Interface::Settings& settings);

	static inline const char* getImplementation() {
		return "curl4esl";
	}

	ConnectionFactory(std::string url, const esl::com::http::client::Interface::Settings& settings);

	std::unique_ptr<esl::com::http::client::Interface::Connection> createConnection() const override;

private:
	esl::com::http::client::Interface::Settings settings;
	std::string url;
};

} /* namespace client */
} /* namespace http */
} /* namespace com */
} /* namespace curl4esl */

#endif /* CURL4ESL_COM_HTTP_CLIENT_CONNECTIONFACTORY_H_ */