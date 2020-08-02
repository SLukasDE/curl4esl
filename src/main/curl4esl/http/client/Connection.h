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

#ifndef CURL4ESL_HTTP_CLIENT_CONNECTION_H_
#define CURL4ESL_HTTP_CLIENT_CONNECTION_H_

#include <esl/http/client/Interface.h>
#include <esl/http/client/Request.h>
#include <esl/http/client/Response.h>
#include <esl/http/client/ResponseHandler.h>
#include <esl/object/Values.h>
#include <esl/utility/URL.h>

#include <curl/curl.h>

#include <string>
#include <memory>

namespace curl4esl {
namespace http {
namespace client {

class Connection : public esl::http::client::Interface::Connection {
public:
	static std::unique_ptr<esl::http::client::Interface::Connection> create(const esl::utility::URL& hostUrl, const esl::object::Values<std::string>& settings);

	static inline const char* getImplementation() {
		return "curl4esl";
	}

	Connection(std::string hostUrl, const esl::object::Values<std::string>& settings);
	~Connection();

	esl::http::client::Response send(esl::http::client::RequestDynamic& request, esl::http::client::ResponseHandler* responseHandler) const override;
	esl::http::client::Response send(const esl::http::client::RequestStatic& request, esl::http::client::ResponseHandler* responseHandler) const override;
	esl::http::client::Response send(const esl::http::client::RequestFile& request, esl::http::client::ResponseHandler* responseHandler) const override;

private:
	esl::http::client::Response prepareRequest(const esl::http::client::Request& request, esl::http::client::ResponseHandler* responseHandler, bool isEmpty, bool hasSize, std::size_t size) const;

	CURL* curl;
	std::string hostUrl;
};

} /* namespace client */
} /* namespace http */
} /* namespace curl4esl */

#endif /* CURL4ESL_HTTP_CLIENT_CONNECTION_H_ */
