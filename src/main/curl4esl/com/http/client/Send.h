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

#ifndef CURL4ESL_COM_HTTP_CLIENT_SEND_H_
#define CURL4ESL_COM_HTTP_CLIENT_SEND_H_

//#include <esl/com/http/client/IConnectionFactory.h>
#include <esl/com/http/client/Request.h>
#include <esl/com/http/client/Response.h>
#include <esl/io/Input.h>
#include <esl/io/Output.h>

#include <curl/curl.h>

#include <cstddef>
#include <cstdint>
#include <exception>
#include <functional>
#include <list>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace curl4esl {
inline namespace v1_6 {
namespace com {
namespace http {
namespace client {

class Send {
public:
	Send(CURL* curl, const esl::com::http::client::Request& request, const std::string& requestUrl, esl::io::Output& output, std::function<esl::io::Input (const esl::com::http::client::Response&)> createInput);
	Send(CURL* curl, const esl::com::http::client::Request& request, const std::string& requestUrl, esl::io::Output& output, esl::io::Input input);
	~Send();

	esl::com::http::client::Response execute();

private:
	Send(CURL* curl, const esl::com::http::client::Request& request, const std::string& requestUrl, esl::io::Output& output, esl::io::Input input, std::function<esl::io::Input (const esl::com::http::client::Response&)> createInput);

	void addRequestHeader(const std::string& key, const std::string& value);

	static size_t readDataCallback(void* data, size_t size, size_t nmemb, void* sendPtr);
	std::size_t readData(void* data, std::size_t size);

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
	std::size_t writeData(const std::uint8_t* data, const std::size_t size);

	const esl::com::http::client::Response& getResponse();

	CURL* curl;

	curl_slist* requestHeaders = nullptr;

	bool firstWriteData = true;
	esl::io::Input input;
	std::function<esl::io::Input (const esl::com::http::client::Response&)> createInput;
	esl::io::Output& output;

	std::unique_ptr<esl::com::http::client::Response> response;
	std::map<std::string, std::string> responseHeaders;
	unsigned short responseStatusCode = 0;

	using Chunk = std::vector<std::uint8_t>;
	std::size_t currentPos = 0;

	std::list<Chunk> receiveBuffer;

	std::exception_ptr exceptionPtr;
};

} /* namespace client */
} /* namespace http */
} /* namespace com */
} /* inline namespace v1_6 */
} /* namespace curl4esl */

#endif /* CURL4ESL_COM_HTTP_CLIENT_SEND_H_ */
