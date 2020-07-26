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

#ifndef CURL4ESL_HTTP_CLIENT_REQUESTHANDLERDYNAMIC_H_
#define CURL4ESL_HTTP_CLIENT_REQUESTHANDLERDYNAMIC_H_

#include <esl/http/client/RequestDynamic.h>

#include <cstddef>

namespace curl4esl {
namespace http {
namespace client {

class RequestHandlerDynamic {
public:
	RequestHandlerDynamic(esl::http::client::RequestDynamic& request);

	static size_t readDataCallback(void* data, size_t size, size_t nmemb, void* requestPtr);

private:
	esl::http::client::RequestDynamic& request;
};

} /* namespace client */
} /* namespace http */
} /* namespace curl4esl */

#endif /* CURL4ESL_HTTP_CLIENT_REQUESTHANDLERDYNAMIC_H_ */
