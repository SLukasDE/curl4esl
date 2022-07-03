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

#include <curl4esl/com/http/client/Send.h>
#include <curl4esl/com/http/client/Connection.h>
#include <curl4esl/Logger.h>

#include <esl/com/http/client/exception/NetworkError.h>
#include <esl/utility/String.h>
#include <esl/system/Stacktrace.h>

#include <sstream>
#include <cstring>

namespace curl4esl {
namespace com {
namespace http {
namespace client {

namespace {
Logger logger("curl4esl::com::http::client::Send");
}  // anonymer namespace

Send::Send(CURL* curl, const esl::com::http::client::Request& request, const std::string& requestUrl, esl::io::Output& output, std::function<esl::io::Input (const esl::com::http::client::Response&)> createInput)
: Send(curl, request, requestUrl, output, esl::io::Input(), createInput)
{ }

Send::Send(CURL* curl, const esl::com::http::client::Request& request, const std::string& requestUrl, esl::io::Output& output, esl::io::Input input)
: Send(curl, request, requestUrl, output, std::move(input), nullptr)
{ }

Send::Send(CURL* aCurl, const esl::com::http::client::Request& request, const std::string& requestUrl, esl::io::Output& aOutput, esl::io::Input aInput, std::function<esl::io::Input (const esl::com::http::client::Response&)> aCreateInput)
: curl(aCurl),
  firstWriteData(aCreateInput),
  input(std::move(aInput)),
  createInput(aCreateInput),
  output(aOutput)
{
	/* ******* *
	* set URL *
	* ******* */

	curl_easy_setopt(curl, CURLOPT_URL, requestUrl.c_str());

	/* ******************* *
	* create POST-Options *
	* ******************* */

	if(output) {
		/** set read callback function */
		curl_easy_setopt(curl, CURLOPT_READFUNCTION, readDataCallback);

		/** set data object to pass to callback function */
		curl_easy_setopt(curl, CURLOPT_READDATA, this);

		curl_easy_setopt(curl, CURLOPT_POST, 1);

		if(output.getReader().hasSize()) {
			long dataSize = static_cast<long>(output.getReader().getSize());
			curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, dataSize);
			/** set data size */
			//curl_easy_setopt(curl, CURLOPT_INFILESIZE, dataSize);
		}
		else {
			addRequestHeader("Transfer-Encoding", "chunked");
		}
	}
	else {
		// No data to send
		curl_easy_setopt(curl, CURLOPT_POST, 0L);
		/** set data size */
		//curl_easy_setopt(curl, CURLOPT_INFILESIZE, 0L);
	}

	/* ******************* *
	 * create HTTP headers *
	 * ******************* */

	/* add content-type header */
	if(request.getContentType()) {
		addRequestHeader("Content-Type", request.getContentType().toString());
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

esl::com::http::client::Response Send::execute() {
	CURLcode rc = curl_easy_perform(curl);

	if(rc != CURLE_OK) {
		std::ostringstream strStream;
		strStream << "Fehlercode=" << rc << " (" << curl_easy_strerror(rc) << ") bei curl-Anfrage";

		if(requestHeaders) {
			curl_slist_free_all(requestHeaders);
			requestHeaders = nullptr;
		}

		std::string str = strStream.str();
		throw esl::system::Stacktrace::add(esl::com::http::client::exception::NetworkError(static_cast<int>(rc), str));
	}

	if(requestHeaders) {
		curl_slist_free_all(requestHeaders);
		requestHeaders = nullptr;
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
	size_t rv = 0;
	Send& send = *reinterpret_cast<Send*>(sendPtr);
	try {
		/* get upload data */
		rv = send.readData(data, size * nmemb);
	}
	catch(const std::runtime_error& e) {
		logger.warn << "std::runtime_error" << "\n";
		logger.warn << "what() = " << e.what() << "\n";

		const esl::system::Stacktrace* stacktrace = esl::system::Stacktrace::get(e);
		if(stacktrace) {
			logger.warn << "Stacktrace:\n";
			stacktrace->dump(logger.warn);
		}

		return 0;
	}
	catch(const std::exception& e) {
		logger.warn << "std::exception" << "\n";
		logger.warn << "what() = " << e.what() << "\n";

		const esl::system::Stacktrace* stacktrace = esl::system::Stacktrace::get(e);
		if(stacktrace) {
			logger.warn << "Stacktrace:\n";
			stacktrace->dump(logger.warn);
		}

		return 0;
	}
	catch(...) {
		logger.warn << "(exception)\n";
		return 0;
	}

	return rv;
}

std::size_t Send::readData(void* data, std::size_t size) {
	/* Signal libcurl to abort transmitting if there is no output available */
	if(!output) {
		return 0;
	}

	/* send upload data */
	std::size_t rv = output.getReader().read(data, size);
	if(rv == esl::io::Reader::npos) {
		output = esl::io::Output();
		return 0;
	}

	return rv;
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
	size_t rv = 0;
	Send& send = *reinterpret_cast<Send*>(sendPtr);
	try {
		rv = send.writeData(static_cast<std::uint8_t*>(data), size * nmemb);
	}
	catch(const std::runtime_error& e) {
		logger.warn << "std::runtime_error" << "\n";
		logger.warn << "what() = " << e.what() << "\n";

		const esl::system::Stacktrace* stacktrace = esl::system::Stacktrace::get(e);
		if(stacktrace) {
			logger.warn << "Stacktrace:\n";
			stacktrace->dump(logger.warn);
		}

		return 0;
	}
	catch(const std::exception& e) {
		logger.warn << "std::exception" << "\n";
		logger.warn << "what() = " << e.what() << "\n";

		const esl::system::Stacktrace* stacktrace = esl::system::Stacktrace::get(e);
		if(stacktrace) {
			logger.warn << "Stacktrace:\n";
			stacktrace->dump(logger.warn);
		}

		return 0;
	}
	catch(...) {
		logger.warn << "(exception)\n";
		return 0;
	}

	return rv;
}

std::size_t Send::writeData(const std::uint8_t* data, const std::size_t size) {
	if(firstWriteData) {
		firstWriteData = false;
		input = createInput(getResponse());
	}

	/* Signal libcurl to abort receiving if there is no input available */
	if(!input) {
		return 0;
	}

	/* ************************************ *
	 * flush buffer if something has queued *
	 * ************************************ */
	while(receiveBuffer.empty() == false) {
		Chunk& chunk = receiveBuffer.front();

		std::size_t sizeRemaining = chunk.size() - currentPos;
		std::size_t sizeWritten = input.getWriter().write(&chunk[currentPos], sizeRemaining);

		if(sizeWritten == esl::io::Writer::npos) {
			input = esl::io::Input();
			receiveBuffer.clear();
			return 0;
		}

		/* check if writer is stalled */
		if(sizeWritten == 0) {
			if(size > 0) {
				/* put new data into queue */
				receiveBuffer.push_back(Chunk(size));
				Chunk& chunk = receiveBuffer.back();
				std::memcpy(&chunk[0], data, size);
			}
			return size;
		}

		currentPos += sizeWritten;
		if(currentPos == chunk.size()) {
			currentPos = 0;
			receiveBuffer.pop_front();
		}
	}

	/* ************************************************ *
	 * Signal writer that no more data will be received *
	 * ************************************************ */
	if(size == 0) {
		input.getWriter().write(data, 0);
		input = esl::io::Input();
		receiveBuffer.clear();
		return 0;
	}

	/* ********************* *
	 * Writer data to writer *
	 * ********************* */
	currentPos = 0;
	while(currentPos < size) {
		std::size_t sizeRemaining = size - currentPos;
		std::size_t sizeWritten = input.getWriter().write(&data[currentPos], sizeRemaining);

		if(sizeWritten == esl::io::Writer::npos) {
			input = esl::io::Input();
			receiveBuffer.clear();
			return 0;
		}

		if(sizeWritten == 0) {
			break;
		}

		currentPos += sizeWritten;
	}

	/* put unwritten  data into queue */
	if(currentPos < size) {
		std::size_t sizeRemaining = size - currentPos;
		receiveBuffer.push_back(Chunk(sizeRemaining));
		Chunk& chunk = receiveBuffer.back();
		std::memcpy(&chunk[0], &data[currentPos], sizeRemaining);
		currentPos = 0;
	}

	return size;
}

const esl::com::http::client::Response& Send::getResponse() {
	if(!response) {
		long httpCode = 0;
		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
		responseStatusCode = static_cast<unsigned short>(httpCode);

		response.reset(new esl::com::http::client::Response(responseStatusCode, std::move(responseHeaders)));
	}

	return *response;
}

} /* namespace client */
} /* namespace http */
} /* namespace com */
} /* namespace curl4esl */
