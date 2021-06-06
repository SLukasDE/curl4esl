/*
MIT License
Copyright (c) 2019-2021 Sven Lukas

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

#ifndef CURL4ESL_COM_HTTP_CLIENT_CONNECTION_H_
#define CURL4ESL_COM_HTTP_CLIENT_CONNECTION_H_

#include <esl/com/http/client/Interface.h>
#include <esl/com/http/client/Request.h>
#include <esl/utility/URL.h>
#include <esl/object/Values.h>

#include <curl/curl.h>

#include <string>

namespace curl4esl {
namespace com {
namespace http {
namespace client {

class Connection : public esl::com::http::client::Interface::Connection {
friend class Send;
public:
	static std::unique_ptr<esl::com::http::client::Interface::Connection> create(const esl::utility::URL& hostUrl, const esl::object::Values<std::string>& settings);

	static inline const char* getImplementation() {
		return "curl4esl";
	}

	Connection(std::string hostUrl, const esl::object::Values<std::string>& settings);
	~Connection();

	esl::com::http::client::Response send(esl::com::http::client::Request request, esl::io::Output output, esl::com::http::client::Interface::CreateInput createInput) const override;
	esl::com::http::client::Response send(esl::com::http::client::Request request, esl::io::Output output, esl::io::Input input) const override;

private:
	CURL* curl;
	std::string hostUrl;
};

} /* namespace client */
} /* namespace http */
} /* namespace com */
} /* namespace curl4esl */

#endif /* CURL4ESL_COM_HTTP_CLIENT_CONNECTION_H_ */
