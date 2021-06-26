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
#include <esl/logging/Logger.h>

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
}  // anonymer namespace


std::unique_ptr<esl::com::http::client::Interface::Connection> Connection::create(const esl::utility::URL& hostUrl, const esl::com::http::client::Interface::Settings& settings) {
	if(hostUrl.getScheme() != esl::utility::Protocol::protocolHttp && hostUrl.getScheme() != esl::utility::Protocol::protocolHttps) {
        throw esl::addStacktrace(std::runtime_error("Unknown scheme in URL: \"" + hostUrl.getScheme().toString() + "\""));
	}
/*
	std::string url = hostUrl.getScheme().toString() + "://" + hostUrl.getHostname();

	if(! hostUrl.getPort().empty()) {
		url += ":" + hostUrl.getPort() + "/";
	}

	std::string path = esl::utility::String::trim(hostUrl.getPath(), '/');
	if(! path.empty()) {
		url += path;
	}
	*/

	return std::unique_ptr<esl::com::http::client::Interface::Connection>(new Connection(hostUrl.toString(), settings));
}

Connection::Connection(std::string aHostUrl, const esl::com::http::client::Interface::Settings& settings)
: esl::com::http::client::Interface::Connection(),
  curl(curlSingleton.easyInit()),
  hostUrl(esl::utility::String::rtrim(aHostUrl, '/'))
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

	long lowSpeedLimit = 0;
	bool hasLowSpeedLimit = false;

	long lowSpeedTime = 0;
	bool hasLowSpeedTime = false;

	std::string username;
	bool hasUsername = false;

	std::string password;
	bool hasPassword = false;

	bool hasProxyServer = false;

	std::string proxyUsername;
	bool hasProxyUsername = false;

	std::string proxyPassword;
	bool hasProxyPassword = false;

	std::string userAgent;
	bool hasUserAgent = false;

    for(const auto& setting : settings) {
		if(setting.first == "timeout") {
			long timeout = std::stol(setting.second);
			if(timeout) {
				curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);
			}
		}

		else if(setting.first == "lowSpeedLimit") {
			hasLowSpeedLimit = true;
			lowSpeedLimit = std::stol(setting.second);
		}

		else if(setting.first == "lowSpeedTime") {
			hasLowSpeedTime = true;
			lowSpeedTime = std::stol(setting.second);
		}

		else if(setting.first == "username") {
			hasUsername = true;
			username = setting.second;
		}

		else if(setting.first == "password") {
			hasPassword = true;
			password = setting.second;
		}

		else if(setting.first == "proxyServer") {
			hasProxyServer = true;
			curl_easy_setopt(curl, CURLOPT_PROXY, setting.second.c_str());
		}

		else if(setting.first == "proxyUsername") {
			hasProxyUsername = true;
			proxyUsername = setting.second;
		}

		else if(setting.first == "proxyPassword") {
			hasProxyPassword = true;
			proxyPassword = setting.second;
		}

		else if(setting.first == "userAgent") {
			hasUserAgent = true;
			userAgent = setting.second;
		}
    }

	if(hasLowSpeedLimit || hasLowSpeedTime) {
		if(!hasLowSpeedLimit) {
            throw esl::addStacktrace(std::runtime_error("curl4esl: 'lowSpeedTime' specified but 'lowSpeedLimit' is missing."));
		}

		if(!hasLowSpeedTime) {
            throw esl::addStacktrace(std::runtime_error("curl4esl: 'lowSpeedLimit' specified but 'lowSpeedTime' is missing."));
		}

        curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, lowSpeedLimit);
        curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME, lowSpeedTime);
	}

    /** set basic authentication if present*/
	if(hasUsername || hasPassword) {
		std::string basicAuthentication = createAuthenticationStr(username, password);
        curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
        curl_easy_setopt(curl, CURLOPT_USERPWD, basicAuthentication.c_str());
	}

	if(hasProxyUsername || hasProxyPassword) {
		if(hasProxyServer) {
			std::string proxyAuthentication = createAuthenticationStr(proxyUsername, proxyPassword);
        	curl_easy_setopt(curl, CURLOPT_PROXYUSERPWD, proxyAuthentication.c_str());
		}
		else {
			logger.warn << "Definition of key \"proxyUser\" or \"proxyPassword\" without definition of proxy server \"proxyServer\".\n";
		}
	}

    /** set user agent */
	if(hasUserAgent) {
	    curl_easy_setopt(curl, CURLOPT_USERAGENT, userAgent.c_str());
	}
	else {
	    curl_easy_setopt(curl, CURLOPT_USERAGENT, "esl-http-client");
	}

	/* ignore SSL certificate */
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
}

Connection::~Connection() {
    curl_easy_cleanup(curl);
}

esl::com::http::client::Response Connection::send(esl::com::http::client::Request request, esl::io::Output output, esl::com::http::client::Interface::CreateInput createInput) const {
	std::string requestUrl = hostUrl;
	if(request.getPath().empty() == false && request.getPath().at(0) != '/') {
		requestUrl += "/";
	}
	requestUrl += request.getPath();

	Send send(curl, request, requestUrl, output, createInput);
	return send.execute();
}

esl::com::http::client::Response Connection::send(esl::com::http::client::Request request, esl::io::Output output, esl::io::Input input) const {
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
