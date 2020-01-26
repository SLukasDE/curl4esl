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

#include <curl4esl/Connection.h>
#include <esl/http/client/RequestStatic.h>
#include <esl/http/client/RequestDynamic.h>
#include <esl/http/client/RequestFile.h>
#include <esl/http/client/Response.h>
#include <esl/http/client/HttpException.h>
#include <esl/http/client/NetworkException.h>
#include <esl/Stacktrace.h>
#include <esl/logging/Logger.h>
#include <cstdio>
#include <sstream>

namespace curl4esl {

namespace {

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

std::size_t readStaticCallback(void* data, std::size_t size, std::size_t nmemb, void* requestPtr) {
	/* get upload data */
	esl::http::client::RequestStatic* request = reinterpret_cast<esl::http::client::RequestStatic*>(requestPtr);
	return request->write(data, size * nmemb);
}

size_t readDynamicCallback(void *data, std::size_t size, std::size_t nmemb, void *requestPtr) {
	/* get upload data */
	esl::http::client::RequestDynamic* request;
	request = reinterpret_cast<esl::http::client::RequestDynamic*>(requestPtr);
	return request->write(data, size * nmemb);
}


}  // anonymer namespace


Connection::Connection(const std::string& aHostUrl, const long timeout,
        const std::string& username,
        const std::string& password,
        const std::string& proxy,
        const std::string& proxyUser,
        const std::string& proxyPassword,
        const std::string& userAgent)
: esl::http::client::Interface::Connection(),
  curlPtr(curlSingleton.easyInit()),
  hostUrl(aHostUrl),
  basicAuthentication(createAuthenticationStr(username, password)),
  proxyAuthentication(createAuthenticationStr(proxyUser, proxyPassword))
{
    curl_easy_setopt(curlPtr, CURLOPT_NOPROGRESS, 1L);
    curl_easy_setopt(curlPtr, CURLOPT_TCP_KEEPALIVE, 1L);
    curl_easy_setopt(curlPtr, CURLOPT_AUTOREFERER, 1L);
    curl_easy_setopt(curlPtr, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curlPtr, CURLOPT_MAXREDIRS, 10L);
    curl_easy_setopt(curlPtr, CURLOPT_COOKIEFILE, "");

    // dont want to get a sig alarm on timeoutInSec
    // (nur wen Timeout gesetzt wird ?)
    curl_easy_setopt(curlPtr, CURLOPT_NOSIGNAL, 1L);

    /** set user agent */
    curl_easy_setopt(curlPtr, CURLOPT_USERAGENT, userAgent.c_str());

    /** set basic authentication if present*/
    if(basicAuthentication.size() > 0) {
        curl_easy_setopt(curlPtr, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
        curl_easy_setopt(curlPtr, CURLOPT_USERPWD, basicAuthentication.c_str());
    }

    if(timeout) {
        curl_easy_setopt(curlPtr, CURLOPT_TIMEOUT, timeout);
    }

    if(!proxy.empty()) {
    	curl_easy_setopt(curlPtr, CURLOPT_PROXY, proxy.c_str());
        if(!proxyAuthentication.empty()) {
        	curl_easy_setopt(curlPtr, CURLOPT_PROXYUSERPWD, proxyAuthentication.c_str());
        }
    }
}

Connection::~Connection() {
    //	curl_slist_free_all(hlist);
    curl_easy_cleanup(curlPtr);
}

void Connection::send(esl::http::client::Response &response, const esl::http::client::RequestDynamic& request, const std::string& method) const {
	curl_easy_setopt(curlPtr, CURLOPT_POST, 1);

	/** set read callback function */
	curl_easy_setopt(curlPtr, CURLOPT_READFUNCTION, readDynamicCallback);

	/** set data object to pass to callback function */
	curl_easy_setopt(curlPtr, CURLOPT_READDATA, this);

	struct curl_slist *chunk = NULL;
	chunk = curl_slist_append(chunk, "Transfer-Encoding: chunked");
	curl_easy_setopt(curlPtr, CURLOPT_HTTPHEADER, chunk);

	prepareRequest(response, request, method);
}

void Connection::send(esl::http::client::Response &response, const esl::http::client::RequestStatic& request, const std::string& method) const {
    // request.addToConnection(*this);
	long dataSize = static_cast<long>(request.getDataSize());

	if(dataSize > 0) {
		curl_easy_setopt(curlPtr, CURLOPT_POST, 1);
		curl_easy_setopt(curlPtr, CURLOPT_POSTFIELDSIZE, dataSize);
	}
	else  {
		curl_easy_setopt(curlPtr, CURLOPT_POST, 0);
	}

	/** set read callback function */
	curl_easy_setopt(curlPtr, CURLOPT_READFUNCTION, readStaticCallback);

	/** set data object to pass to callback function */
	curl_easy_setopt(curlPtr, CURLOPT_READDATA, &request);

	/** set data size */
	curl_easy_setopt(curlPtr, CURLOPT_INFILESIZE, &dataSize);

	prepareRequest(response, request, method);
}

void Connection::send(esl::http::client::Response &response, const esl::http::client::RequestFile& request, const std::string& method) const {
	FILE* file = fopen (request.getFilename().c_str(), "r");
	if(file == nullptr) {
        throw esl::addStacktrace(std::runtime_error("curl: cannot open file \"" + request.getFilename() + "\""));
	}

	/** set read callback function */
	curl_easy_setopt(curlPtr, CURLOPT_READFUNCTION, nullptr);

	/** set data object to pass to callback function */
	curl_easy_setopt(curlPtr, CURLOPT_READDATA, file);

	prepareRequest(response, request, method);

	fclose(file);
}

void Connection::prepareRequest(esl::http::client::Response& response, const esl::http::client::Request& request, const std::string& method) const {
    const std::string requestUrl = (hostUrl + "/" + request.getServicePath());
    esl::logger.debug << "Anfrage: '" << requestUrl << "'" << std::endl;

    /** set query URL */
    //	curl_easy_setopt(curl, CURLOPT_URL, (hostUrl + request.getServicePath()).c_str());
    curl_easy_setopt(curlPtr, CURLOPT_URL, requestUrl.c_str());

    /** create Header */
    // Content Type header hinzu
    curl_slist* hlist = nullptr;
    std::string header;
    if(request.getContentType().empty() == false) {
        header = "Content-Type: " + request.getContentType();
        hlist = curl_slist_append(hlist, header.c_str());
    }
    /** set http headers */
    for(auto v : request.getHeaders()) {
        header = v.first;
        header += ": ";
        header += v.second;
        hlist = curl_slist_append(hlist, header.c_str());
    }
    curl_easy_setopt(curlPtr, CURLOPT_HTTPHEADER, hlist);



    // request.addToConnection(*this);



    curl_easy_setopt(curlPtr, CURLOPT_CUSTOMREQUEST, method.c_str());

    curl_easy_setopt(curlPtr, CURLOPT_WRITEFUNCTION, esl::http::client::Response::writeCallback);
    curl_easy_setopt(curlPtr, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curlPtr, CURLOPT_HEADERFUNCTION, esl::http::client::Response::headerCallback);
    curl_easy_setopt(curlPtr, CURLOPT_HEADERDATA, &response);

    CURLcode res = curl_easy_perform(curlPtr);

    if(res == CURLE_OK) {
        long httpCode = 0;
        curl_easy_getinfo(curlPtr, CURLINFO_RESPONSE_CODE, &httpCode);
        if(httpCode != httpOk) {
            curl_slist_free_all(hlist);
            throw esl::addStacktrace(esl::http::client::HttpException(httpCode, "HTTP-Fehler erhalten"));
        }
    }
    else {
        std::ostringstream strStream;
        strStream << "Fehlercode=" << res << " (" << curl_easy_strerror(res) << ") bei curl-Anfrage";
        curl_slist_free_all(hlist);
        std::string str = strStream.str();
        throw esl::addStacktrace(esl::http::client::NetworkException(str));
    }
    curl_slist_free_all(hlist);
}

} /* namespace curl4esl */
