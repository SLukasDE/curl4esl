/*
MIT License
Copyright (c) 2019-2023 Sven Lukas

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

#include <esl/com/http/client/Connection.h>
#include <esl/com/http/client/Request.h>
#include <esl/com/http/client/Response.h>
#include <esl/io/Input.h>
#include <esl/io/Output.h>

#include <curl/curl.h>

#include <functional>
#include <string>

namespace curl4esl {
inline namespace v1_6 {
namespace com {
namespace http {
namespace client {

class Connection : public esl::com::http::client::Connection {
friend class Send;
public:
	Connection(CURL* curl, std::string hostUrl);
	~Connection();

	esl::com::http::client::Response send(const esl::com::http::client::Request& request, esl::io::Output output, std::function<esl::io::Input (const esl::com::http::client::Response&)> createInput) const override;
	esl::com::http::client::Response send(const esl::com::http::client::Request& request, esl::io::Output output, esl::io::Input input) const override;

private:
	CURL* curl;
	std::string hostUrl;
};

} /* namespace client */
} /* namespace http */
} /* namespace com */
} /* inline namespace v1_6 */
} /* namespace curl4esl */

#endif /* CURL4ESL_COM_HTTP_CLIENT_CONNECTION_H_ */
