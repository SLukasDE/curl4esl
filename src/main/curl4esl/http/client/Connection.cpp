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

#include <curl4esl/http/client/Connection.h>
#include <curl4esl/http/client/RequestHandlerStatic.h>
#include <curl4esl/http/client/RequestHandlerDynamic.h>
#include <curl4esl/http/client/RequestHandlerFile.h>
#include <curl4esl/http/client/ResponseHandler.h>
#include <curl4esl/Logger.h>

#include <esl/http/client/RequestStatic.h>
#include <esl/http/client/RequestDynamic.h>
#include <esl/http/client/RequestFile.h>
#include <esl/http/client/Response.h>
#include <esl/http/client/NetworkException.h>
#include <esl/utility/String.h>
#include <esl/Stacktrace.h>
#include <esl/logging/Logger.h>

#include <cstdio>
#include <sstream>
#include <fstream>

namespace curl4esl {
namespace http {
namespace client {

namespace {

Logger logger("curl4esl::http::client::Connection");

enum {
    httpOk = 200
};

struct CurlSingleton {
	CurlSingleton() {
        curl_global_init( CURL_GLOBAL_ALL );
    }

    ~CurlSingleton() {
        curl_global_cleanup();
    }

    CURL* easyInit() {
    	CURL* curlPtr = curl_easy_init();
    	if(curlPtr == nullptr) {
            throw esl::addStacktrace(std::runtime_error("curl init error"));
    	}
    	return curlPtr;
    }
};
CurlSingleton curlSingleton;

std::string createAuthenticationStr(const std::string& username, const std::string& password) {
    if(!username.empty() && !password.empty()) {
        return username+":"+password;
    }
    return username;
}


/**
* @brief header callback for libcurl
*
* @param data returned (header line)
* @param size of data
* @param nmemb memblock
* @param headersPtr pointer to user data object to save headr data
* @return size * nmemb;
*/
size_t writeHeaderCallback(void* data, size_t size, size_t nmemb, void* headersPtr) {
	std::map<std::string, std::string>& headers = *reinterpret_cast<std::map<std::string, std::string>*>(headersPtr);

	std::string header(reinterpret_cast<char*>(data), size*nmemb);
	std::size_t seperator = header.find_first_of(":");

	std::string key;
	std::string value;

	if(seperator == std::string::npos) {
		key = esl::utility::String::trim(header);
	}
	else {
		key = esl::utility::String::trim(header.substr(0, seperator));
		value = esl::utility::String::trim(header.substr(seperator + 1));
	}

	if(!key.empty()) {
		headers[key] = value;
	}

	return size*nmemb;
}

curl_slist* addHeader(curl_slist* hlist, const std::string& key, const std::string& value) {
	std::string header;

	if(value.empty()) {
		header = key + ";";
	}
	else {
		header = key + ": " + value;
	}

    return curl_slist_append(hlist, header.c_str());
}

}  // anonymer namespace


std::unique_ptr<esl::http::client::Interface::Connection> Connection::create(const esl::utility::URL& hostUrl, const esl::object::Values<std::string>& values) {
	if(hostUrl.getScheme() != esl::utility::Protocol::protocolHttp && hostUrl.getScheme() != esl::utility::Protocol::protocolHttps) {
        throw esl::addStacktrace(std::runtime_error("Unknown scheme in URL: \"" + hostUrl.getScheme().toString() + "\""));
	}

	std::string url = hostUrl.getScheme().toString() + "://" + hostUrl.getHostname();

	if(! hostUrl.getPort().empty()) {
		url += ":" + hostUrl.getPort() + "/";
	}

	std::string path = esl::utility::String::trim(hostUrl.getPath(), '/');
	if(! path.empty()) {
		url += path + "/";
	}

	return std::unique_ptr<esl::http::client::Interface::Connection>(new Connection(url, values));
}

Connection::Connection(std::string aHostUrl, const esl::object::Values<std::string>& values)
: esl::http::client::Interface::Connection(),
  curl(curlSingleton.easyInit()),
  hostUrl(std::move(aHostUrl))
{
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
    curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);
    curl_easy_setopt(curl, CURLOPT_AUTOREFERER, 1L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 10L);
    curl_easy_setopt(curl, CURLOPT_COOKIEFILE, "");

    // dont want to get a sig alarm on timeoutInSec
    // (nur wen Timeout gesetzt wird ?)
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);

	if(values.hasValue("timeout")) {
		long timeout = std::stol(values.getValue("timeout"));
	    if(timeout) {
	        curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);
	    }
	}


    /** set basic authentication if present*/
	{
		bool hasUsernamePassword = false;
		std::string username;
		std::string password;

		if(values.hasValue("username")) {
			hasUsernamePassword = true;
			username = values.getValue("username");
		}

		if(values.hasValue("password")) {
			hasUsernamePassword = true;
			password = values.getValue("password");
		}

		if(hasUsernamePassword) {
			std::string basicAuthentication = createAuthenticationStr(username, password);
	        curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
	        curl_easy_setopt(curl, CURLOPT_USERPWD, basicAuthentication.c_str());
		}
	}

	if(values.hasValue("proxy")) {
		std::string proxy = values.getValue("proxy");
    	curl_easy_setopt(curl, CURLOPT_PROXY, proxy.c_str());

		bool hasUsernamePassword = false;
		std::string proxyUser;
		std::string proxyPassword;

		if(values.hasValue("proxyUser")) {
			hasUsernamePassword = true;
			proxyUser = values.getValue("proxyUser");
		}

		if(values.hasValue("proxyPassword")) {
			hasUsernamePassword = true;
			proxyPassword = values.getValue("proxyPassword");
		}

        if(hasUsernamePassword) {
			std::string proxyAuthentication = createAuthenticationStr(proxyUser, proxyPassword);
        	curl_easy_setopt(curl, CURLOPT_PROXYUSERPWD, proxyAuthentication.c_str());
        }
	}

    /** set user agent */
	if(values.hasValue("userAgent")) {
		std::string userAgent = values.getValue("userAgent");
	    curl_easy_setopt(curl, CURLOPT_USERAGENT, userAgent.c_str());
	}
	else {
	    curl_easy_setopt(curl, CURLOPT_USERAGENT, "esl-http-client");
	}
}

Connection::~Connection() {
    curl_easy_cleanup(curl);
}

esl::http::client::Response Connection::send(esl::http::client::RequestDynamic& request, esl::http::client::ResponseHandler* responseHandler) const {
	bool isEmpty = request.isEmpty();
	bool hasSize = isEmpty ? false : request.hasSize();
	std::size_t size = hasSize ? 0 : request.getSize();

	if(isEmpty == false) {
		RequestHandlerDynamic requestHandlerDynamic(request);

		/** set read callback function */
		curl_easy_setopt(curl, CURLOPT_READFUNCTION, RequestHandlerDynamic::readDataCallback);

		/** set data object to pass to callback function */
		curl_easy_setopt(curl, CURLOPT_READDATA, &requestHandlerDynamic);
	}

	return prepareRequest(request, responseHandler, isEmpty, hasSize, size);
}

esl::http::client::Response Connection::send(const esl::http::client::RequestStatic& request, esl::http::client::ResponseHandler* responseHandler) const {
	if(request.getSize() > 0) {
		RequestHandlerStatic requestHandlerStatic(request);

		/** set read callback function */
		curl_easy_setopt(curl, CURLOPT_READFUNCTION, RequestHandlerStatic::readDataCallback);

		/** set data object to pass to callback function */
		curl_easy_setopt(curl, CURLOPT_READDATA, &requestHandlerStatic);
	}

	return prepareRequest(request, responseHandler, request.getSize() == 0, true, request.getSize());
}

esl::http::client::Response Connection::send(const esl::http::client::RequestFile& request, esl::http::client::ResponseHandler* responseHandler) const {
	RequestHandlerFile requestHandlerFile(request);

	if(requestHandlerFile.getSize() > 0) {
		/** set read callback function */
		curl_easy_setopt(curl, CURLOPT_READFUNCTION, RequestHandlerFile::readDataCallback);

		/** set data object to pass to callback function */
		curl_easy_setopt(curl, CURLOPT_READDATA, &requestHandlerFile);
	}

	return prepareRequest(request, responseHandler, requestHandlerFile.getSize() == 0, true, requestHandlerFile.getSize());
}

esl::http::client::Response Connection::prepareRequest(const esl::http::client::Request& request, esl::http::client::ResponseHandler* responseHandler, bool isEmpty, bool hasSize, std::size_t size) const {
	const std::string requestUrl = hostUrl + "/" + esl::utility::String::ltrim(request.getPath(), '/');
	logger.debug << "Request: '" << requestUrl << "'" << std::endl;

	/* ******* *
	 * set URL *
	 * ******* */

	curl_easy_setopt(curl, CURLOPT_URL, requestUrl.c_str());



	/* ******************* *
	 * create POST-Options *
	 * ******************* */

	if(isEmpty) {
		curl_easy_setopt(curl, CURLOPT_POST, 0L);
		/** set data size */
		//curl_easy_setopt(curl, CURLOPT_INFILESIZE, 0L);
	}
	else {
		curl_easy_setopt(curl, CURLOPT_POST, 1);

		if(hasSize) {
			long dataSize = static_cast<long>(size);
			curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, dataSize);
			/** set data size */
			//curl_easy_setopt(curl, CURLOPT_INFILESIZE, dataSize);
		}
	}

	/* ******************* *
	 * create HTTP headers *
	 * ******************* */

	/* add content-type header */
	curl_slist* headerList = nullptr;

	if(isEmpty == false) {
		headerList = addHeader(headerList, "Content-Type", request.getContentType().toString());

		if(hasSize == false) {
			headerList = addHeader(headerList, "Transfer-Encoding", "chunked");
		}
	}

	/* add other headers */
	for(const auto& v : request.getHeaders()) {
		headerList = addHeader(headerList, v.first, v.second);
	}

	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerList);




    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, request.getMethod().toString().c_str());

	std::map<std::string, std::string> headers;
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, writeHeaderCallback);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, &headers);

    /* check if this handler accepts data. If not, then we don't need to install writeDataCallback */
    ResponseHandler tmpResponseHandler(responseHandler);

    if(responseHandler) {
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, ResponseHandler::writeDataCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &tmpResponseHandler);
    }

    CURLcode res = curl_easy_perform(curl);

    if(res != CURLE_OK) {
        std::ostringstream strStream;
        strStream << "Fehlercode=" << res << " (" << curl_easy_strerror(res) << ") bei curl-Anfrage";

    	if(headerList) {
    	    curl_slist_free_all(headerList);
    	}

        std::string str = strStream.str();
        throw esl::addStacktrace(esl::http::client::NetworkException(str));
    }

    long httpCode = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);

	if(headerList) {
	    curl_slist_free_all(headerList);
	}

	return esl::http::client::Response(static_cast<unsigned short>(httpCode), std::move(headers));
}

} /* namespace client */
} /* namespace http */
} /* namespace curl4esl */
