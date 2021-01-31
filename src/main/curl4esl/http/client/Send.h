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

#ifndef CURL4ESL_HTTP_CLIENT_SEND_H_
#define CURL4ESL_HTTP_CLIENT_SEND_H_

#include <esl/http/client/Request.h>
#include <esl/http/client/RequestHandler.h>
#include <esl/http/client/Response.h>
#include <esl/http/client/ResponseHandler.h>

#include <curl/curl.h>

#include <cstddef>
#include <string>
#include <map>
#include <memory>

namespace curl4esl {
namespace http {
namespace client {

class Send {
public:
	Send(CURL* curl, esl::http::client::Request request, std::string requestUrl);
	~Send();

	esl::http::client::Response send();

private:
	void addRequestHeader(const std::string& key, const std::string& value);

	static size_t readDataCallback(void* data, size_t size, size_t nmemb, void* sendPtr);
	std::size_t readData(char* data, std::size_t size);

	/**
	* @brief header callback for libcurl
	*
	* @param data returned (header line)
	* @param size of data
	* @param nmemb memblock
	* @param headersPtr pointer to user data object to save headr data
	* @return size * nmemb;
	*/
	static size_t writeHeaderCallback(void* data, size_t size, size_t nmemb, void* sendPtr);
	std::size_t writeHeader(const char* data, std::size_t size);

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
	static size_t writeDataCallback(void* data, size_t size, size_t nmemb, void* sendPtr);
	std::size_t writeData(const char* data, std::size_t size);

	esl::http::client::Response& getResponse();

	CURL* curl;

	esl::http::client::Request request;
	const std::string requestUrl;
	curl_slist* requestHeaders = nullptr;

	std::unique_ptr<esl::http::client::Response> response;
	std::map<std::string, std::string> responseHeaders;
	unsigned short responseStatusCode = 0;

	/* Initialized with requst.getResponseHandler().
	 * Don't use requst.getResponseHandler() anymore after initialization.
	 * Use 'responseHandler' instead and set 'responseHandler' to nullptr if resounseHandler->consume(...) return false.
	 */
	esl::http::client::ResponseHandler* responseHandler;
};

} /* namespace client */
} /* namespace http */
} /* namespace curl4esl */

#endif /* CURL4ESL_HTTP_CLIENT_SEND_H_ */
