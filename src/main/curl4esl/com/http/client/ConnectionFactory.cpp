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

#include <curl4esl/com/http/client/ConnectionFactory.h>
#include <curl4esl/com/http/client/Connection.h>

#include <esl/Logger.h>
#include <esl/system/Stacktrace.h>
#include <esl/utility/URL.h>

#include <stdexcept>

namespace curl4esl {
inline namespace v1_6 {
namespace com {
namespace http {
namespace client {

namespace {
esl::Logger logger("curl4esl::com::http::client::ConnectionFactory");

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
            throw esl::system::Stacktrace::add(std::runtime_error("curl init error"));
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

}

ConnectionFactory::ConnectionFactory(const esl::com::http::client::CURLConnectionFactory::Settings& aSettings)
: settings(aSettings)
{ }

std::unique_ptr<esl::com::http::client::Connection> ConnectionFactory::createConnection() const {
	CURL* curl = curlSingleton.easyInit();

	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
	curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
    curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);
    curl_easy_setopt(curl, CURLOPT_AUTOREFERER, 0L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 0L);
    //curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 10L);
    curl_easy_setopt(curl, CURLOPT_COOKIEFILE, "");

    // dont want to get a sig alarm on timeoutInSec
    // (nur wen Timeout gesetzt wird ?)
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);

	if(settings.timeout > 0) {
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, settings.timeout);
	}

	if(settings.hasLowSpeedDefinition) {
        curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, settings.lowSpeedLimit);
        curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME, settings.lowSpeedTime);
	}

    /** set basic authentication if present*/
	if(!settings.username.empty() || !settings.password.empty()) {
		std::string basicAuthentication = createAuthenticationStr(settings.username, settings.password);
        curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
        curl_easy_setopt(curl, CURLOPT_USERPWD, basicAuthentication.c_str());
	}

	if(!settings.proxyServer.empty()) {
		curl_easy_setopt(curl, CURLOPT_PROXY, settings.proxyServer.c_str());
	}

	if(!settings.proxyUsername.empty() || !settings.proxyPassword.empty()) {
		std::string proxyAuthentication = createAuthenticationStr(settings.proxyUsername, settings.proxyPassword);
    	curl_easy_setopt(curl, CURLOPT_PROXYUSERPWD, proxyAuthentication.c_str());
	}

    /** set user agent */
	if(!settings.userAgent.empty()) {
	    curl_easy_setopt(curl, CURLOPT_USERAGENT, settings.userAgent.c_str());
	}

	/* ignore SSL certificate */
	if(settings.skipSSLVerification) {
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
	}

	return std::unique_ptr<esl::com::http::client::Connection>(new Connection(curl, settings.url));
}

} /* namespace client */
} /* namespace http */
} /* namespace com */
} /* inline namespace v1_6 */
} /* namespace curl4esl */
