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

#ifndef CURL4ESL_HTTP_CLIENT_RESPONSEHANDLER_H_
#define CURL4ESL_HTTP_CLIENT_RESPONSEHANDLER_H_

#include <esl/http/client/ResponseHandler.h>

#include <cstdlib>

namespace curl4esl {
namespace http {
namespace client {

class ResponseHandler {
public:
	ResponseHandler(esl::http::client::ResponseHandler* handler);

	/**
	* @brief write callback function for libcurl
	*
	* @param data returned data of size (size*nmemb)
	* @param size size parameter
	* @param nmemb memblock parameter
	* @param responseHandlerPtr pointer to user data to save/work with return data
	*
	* @return (size * nmemb)
	*/
	static size_t writeDataCallback(void* data, size_t size, size_t nmemb, void* responseHandlerPtr);

private:
	esl::http::client::ResponseHandler* handler;
};

} /* namespace client */
} /* namespace http */
} /* namespace curl4esl */

#endif /* CURL4ESL_HTTP_CLIENT_RESPONSEHANDLER_H_ */
