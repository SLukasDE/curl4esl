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

#include <curl4esl/http/client/Send.h>
#include <curl4esl/http/client/Connection.h>
#include <curl4esl/Logger.h>

#include <esl/http/client/NetworkException.h>
#include <esl/utility/String.h>
#include <esl/Stacktrace.h>

#include <sstream>

namespace curl4esl {
namespace http {
namespace client {
namespace {
Logger logger("curl4esl::http::client::Send");
}  // anonymer namespace

Send::Send(CURL* aCurl, esl::http::client::Request aRequest, std::string aRequestUrl)
: curl(aCurl),
  request(std::move(aRequest)),
  requestUrl(std::move(aRequestUrl)),
  responseHandler(request.getResponseHandler())
{
	esl::http::client::RequestHandler* requestHandler = request.getRequestHandler();

	if(requestHandler && requestHandler->isEmpty() == false) {

		/** set read callback function */
		curl_easy_setopt(curl, CURLOPT_READFUNCTION, readDataCallback);

		/** set data object to pass to callback function */
		curl_easy_setopt(curl, CURLOPT_READDATA, this);
	}

	/* ******* *
	* set URL *
	* ******* */

	curl_easy_setopt(curl, CURLOPT_URL, requestUrl.c_str());



	/* ******************* *
	* create POST-Options *
	* ******************* */

	if(requestHandler) {
		curl_easy_setopt(curl, CURLOPT_POST, 1);

		if(requestHandler->hasSize()) {
			long dataSize = static_cast<long>(requestHandler->getSize());
			curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, dataSize);
			/** set data size */
			//curl_easy_setopt(curl, CURLOPT_INFILESIZE, dataSize);
		}
	}
	else {
		curl_easy_setopt(curl, CURLOPT_POST, 0L);
		/** set data size */
		//curl_easy_setopt(curl, CURLOPT_INFILESIZE, 0L);
	}

	/* ******************* *
	 * create HTTP headers *
	 * ******************* */

	/* add content-type header */

	if(requestHandler) {
		addRequestHeader("Content-Type", requestHandler->getContentType().toString());

		if(requestHandler->hasSize() == false) {
			addRequestHeader("Transfer-Encoding", "chunked");
		}
	}

	/* add other headers */
	for(const auto& v : request.getHeaders()) {
		addRequestHeader(v.first, v.second);
	}

	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, requestHeaders);

	curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, request.getMethod().toString().c_str());

	curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, writeHeaderCallback);
	curl_easy_setopt(curl, CURLOPT_HEADERDATA, this);

	/* check if this handler accepts data. If not, then we don't need to install writeDataCallback */

	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeDataCallback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);
}

Send::~Send() {
	if(requestHeaders) {
		curl_slist_free_all(requestHeaders);
	}
}

esl::http::client::Response Send::send() {
	CURLcode res = curl_easy_perform(curl);

	if(res != CURLE_OK) {
		std::ostringstream strStream;
		strStream << "Fehlercode=" << res << " (" << curl_easy_strerror(res) << ") bei curl-Anfrage";

		if(requestHeaders) {
			curl_slist_free_all(requestHeaders);
			requestHeaders = nullptr;
		}

		std::string str = strStream.str();
		throw esl::addStacktrace(esl::http::client::NetworkException(str));
	}

	if(requestHeaders) {
		curl_slist_free_all(requestHeaders);
		requestHeaders = nullptr;
	}

	if(responseHandler) {
		Connection::responseHandler__consumer(*responseHandler, nullptr, 0);
	}

	return getResponse();
}

void Send::addRequestHeader(const std::string& key, const std::string& value) {
	std::string header;

	if(value.empty()) {
		header = key + ";";
	}
	else {
		header = key + ": " + value;
	}

	requestHeaders = curl_slist_append(requestHeaders, header.c_str());
}

size_t Send::readDataCallback(void* data, size_t size, size_t nmemb, void* sendPtr) {
	/* get upload data */
	Send& send = *reinterpret_cast<Send*>(sendPtr);
	return send.readData(static_cast<char*>(data), size * nmemb);
}

std::size_t Send::readData(char* data, std::size_t size) {
	/* get upload data */
	if(request.getRequestHandler()) {
		return request.getRequestHandler()->producer(data, size);
	}
	return 0;
}

size_t Send::writeHeaderCallback(void* data, size_t size, size_t nmemb, void* sendPtr) {
	Send& send = *reinterpret_cast<Send*>(sendPtr);
	return send.writeHeader(static_cast<char*>(data), size * nmemb);
}

std::size_t Send::writeHeader(const char* data, std::size_t size) {
	std::string header(data, size);
	std::size_t seperator = header.find_first_of(":");

	std::string key;
	std::string value;

	if(seperator == std::string::npos) {
		key = esl::utility::String::trim(esl::utility::String::trim(esl::utility::String::trim(header), '\n'), '\r');
	}
	else {
		key = esl::utility::String::trim(esl::utility::String::trim(esl::utility::String::trim(header.substr(0, seperator)), '\n'), '\r');
		value = esl::utility::String::trim(esl::utility::String::trim(esl::utility::String::trim(header.substr(seperator + 1)), '\n'), '\r');
	}

	if(!key.empty()) {
		responseHeaders[key] = value;
	}

	return size;
}

size_t Send::writeDataCallback(void* data, size_t size, size_t nmemb, void* sendPtr) {
	Send& send = *reinterpret_cast<Send*>(sendPtr);
	return send.writeData(static_cast<char*>(data), size * nmemb);
}

std::size_t Send::writeData(const char* data, std::size_t size) {
	if(!response) {
		// switch to receive content
		Connection::request__setResponse(request, getResponse());
	}

	if(responseHandler) {
		bool callAgain = Connection::responseHandler__consumer(*responseHandler, data, size);
		if(callAgain == false) {
			responseHandler = nullptr;
		}
	}

	return size;
}

esl::http::client::Response& Send::getResponse() {
	if(!response) {
		long httpCode = 0;
		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
		responseStatusCode = static_cast<unsigned short>(httpCode);

		response.reset(new esl::http::client::Response(responseStatusCode, std::move(responseHeaders)));
	}

	return *response;
}

} /* namespace client */
} /* namespace http */
} /* namespace curl4esl */
