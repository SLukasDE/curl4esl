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

#include <curl4esl/com/http/client/Connection.h>
#include <curl4esl/com/http/client/Send.h>
#include <curl4esl/Logger.h>

#include <esl/com/http/client/Response.h>
#include <esl/utility/String.h>
#include <esl/Stacktrace.h>

#include <cstdio>
#include <sstream>
#include <fstream>
#include <memory>

namespace curl4esl {
namespace com {
namespace http {
namespace client {

namespace {
Logger logger("curl4esl::com::http::client::Connection");
}  // anonymer namespace

Connection::Connection(CURL* aCurl, std::string aHostUrl)
: curl(aCurl),
  hostUrl(aHostUrl)
{ }

Connection::~Connection() {
    curl_easy_cleanup(curl);
}

esl::com::http::client::Response Connection::send(const esl::com::http::client::Request& request, esl::io::Output output, esl::com::http::client::Interface::CreateInput createInput) const {
	std::string requestUrl = hostUrl;
	if(request.getPath().empty() == false && request.getPath().at(0) != '/') {
		requestUrl += "/";
	}
	requestUrl += request.getPath();

	Send send(curl, request, requestUrl, output, createInput);
	return send.execute();
}

esl::com::http::client::Response Connection::send(const esl::com::http::client::Request& request, esl::io::Output output, esl::io::Input input) const {
	std::string requestUrl = hostUrl;
	if(request.getPath().empty() == false && request.getPath().at(0) != '/') {
		requestUrl += "/";
	}
	requestUrl += request.getPath();

	Send send(curl, request, requestUrl, output, std::move(input));
	return send.execute();
}

} /* namespace client */
} /* namespace http */
} /* namespace com */
} /* namespace curl4esl */
