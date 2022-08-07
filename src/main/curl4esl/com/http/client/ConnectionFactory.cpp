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

#include <curl4esl/com/http/client/ConnectionFactory.h>
#include <curl4esl/com/http/client/Connection.h>
#include <curl4esl/Logger.h>

#include <esl/utility/String.h>
#include <esl/utility/URL.h>
#include <esl/utility/String.h>
#include <esl/system/Stacktrace.h>

#include <stdexcept>

namespace curl4esl {
namespace com {
namespace http {
namespace client {

namespace {
Logger logger("curl4esl::com::http::client::ConnectionFactory");

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

std::unique_ptr<esl::com::http::client::ConnectionFactory> ConnectionFactory::create(const std::vector<std::pair<std::string, std::string>>& settings) {
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

	return std::unique_ptr<esl::com::http::client::ConnectionFactory>(new ConnectionFactory(settings));
}

ConnectionFactory::ConnectionFactory(const std::vector<std::pair<std::string, std::string>>& aSettings) {
	bool hasLowSpeedLimit = false;
	bool hasLowSpeedTime = false;
	bool hasUserAgent = false;
	bool hasTimeout = false;
	bool hasSkipSSLVerification = false;

    for(const auto& setting : aSettings) {
		if(setting.first == "url") {
			if(!url.empty()) {
	            throw esl::system::Stacktrace::add(std::runtime_error("curl4esl: multiple definition of attribute 'url'."));
			}
			url = esl::utility::String::rtrim(setting.second, '/');
			if(url.empty()) {
	            throw esl::system::Stacktrace::add(std::runtime_error("curl4esl: Invalid value \"\" for attribute 'url'."));
			}

			esl::utility::URL aURL(url);
			if(aURL.getScheme() != esl::utility::Protocol::Type::http && aURL.getScheme() != esl::utility::Protocol::Type::https) {
	            throw esl::system::Stacktrace::add(std::runtime_error("curl4esl: Invalid scheme in value \"" + url + "\" of attribute 'url'."));
			}
		}

		else if(setting.first == "timeout") {
			if(hasTimeout) {
	            throw esl::system::Stacktrace::add(std::runtime_error("curl4esl: multiple definition of attribute 'timeout'."));
			}
			hasTimeout = true;
			timeout = esl::utility::String::toLong(setting.second);
			if(timeout < 0) {
	            throw esl::system::Stacktrace::add(std::runtime_error("curl4esl: Invalid value \"" + std::to_string(timeout) + "\" for attribute 'timeout'."));
			}
		}

		else if(setting.first == "low-speed-limit") {
			if(hasLowSpeedLimit) {
	            throw esl::system::Stacktrace::add(std::runtime_error("curl4esl: multiple definition of attribute 'low-speed-limit'."));
			}
			hasLowSpeedLimit = true;
			lowSpeedLimit = esl::utility::String::toLong(setting.second);
			if(lowSpeedLimit < 0) {
	            throw esl::system::Stacktrace::add(std::runtime_error("curl4esl: Invalid value \"" + std::to_string(lowSpeedLimit) + "\" for attribute 'low-speed-limit'."));
			}
		}

		else if(setting.first == "low-speed-time") {
			if(hasLowSpeedTime) {
	            throw esl::system::Stacktrace::add(std::runtime_error("curl4esl: multiple definition of attribute 'low-speed-time'."));
			}
			hasLowSpeedTime = true;
			lowSpeedTime = esl::utility::String::toLong(setting.second);
			if(lowSpeedTime < 0) {
	            throw esl::system::Stacktrace::add(std::runtime_error("curl4esl: Invalid value \"" + std::to_string(lowSpeedTime) + "\" for attribute 'low-speed-time'."));
			}
		}

		else if(setting.first == "username") {
			if(!username.empty()) {
	            throw esl::system::Stacktrace::add(std::runtime_error("curl4esl: multiple definition of attribute 'username'."));
			}
			username = setting.second;
			if(username.empty()) {
	            throw esl::system::Stacktrace::add(std::runtime_error("curl4esl: Invalid value \"\" for attribute 'username'."));
			}
		}

		else if(setting.first == "password") {
			if(!password.empty()) {
	            throw esl::system::Stacktrace::add(std::runtime_error("curl4esl: multiple definition of attribute 'password'."));
			}
			password = setting.second;
			if(password.empty()) {
	            throw esl::system::Stacktrace::add(std::runtime_error("curl4esl: Invalid value \"\" for attribute 'password'."));
			}
		}

		else if(setting.first == "proxy-server") {
			if(!proxyServer.empty()) {
	            throw esl::system::Stacktrace::add(std::runtime_error("curl4esl: multiple definition of attribute 'proxy-server'."));
			}
			proxyServer = setting.second;
			if(proxyServer.empty()) {
	            throw esl::system::Stacktrace::add(std::runtime_error("curl4esl: invalid value \"\" for attribute 'proxy-server'."));
			}
		}

		else if(setting.first == "proxy-username") {
			if(!proxyUsername.empty()) {
	            throw esl::system::Stacktrace::add(std::runtime_error("curl4esl: multiple definition of attribute 'proxy-username'."));
			}
			proxyUsername = setting.second;
			if(proxyUsername.empty()) {
	            throw esl::system::Stacktrace::add(std::runtime_error("curl4esl: invalid value \"\" for attribute 'proxy-username'."));
			}
		}

		else if(setting.first == "proxy-password") {
			if(!proxyPassword.empty()) {
	            throw esl::system::Stacktrace::add(std::runtime_error("curl4esl: multiple definition of attribute 'proxy-password'."));
			}
			proxyPassword = setting.second;
			if(proxyPassword.empty()) {
	            throw esl::system::Stacktrace::add(std::runtime_error("curl4esl: invalid value \"\" for attribute 'proxy-password'."));
			}
		}

		else if(setting.first == "user-ugent") {
			if(hasUserAgent) {
	            throw esl::system::Stacktrace::add(std::runtime_error("curl4esl: multiple definition of attribute 'user-ugent'."));
			}
			userAgent = setting.second;
			hasUserAgent = true;
		}

		else if(setting.first == "skip-ssl-verification") {
			if(hasSkipSSLVerification) {
	            throw esl::system::Stacktrace::add(std::runtime_error("curl4esl: multiple definition of attribute 'skip-ssl-verification'."));
			}
			std::string value = esl::utility::String::toLower(setting.second);
			if(value == "true") {
				skipSSLVerification = true;
			}
			else if(value == "false") {
				skipSSLVerification = false;
			}
			else {
		    	throw esl::system::Stacktrace::add(std::runtime_error("curl4esl: Invalid value \"" + setting.second + "\" for attribute 'skip-ssl-verification'"));
			}
		}

		else {
            throw esl::system::Stacktrace::add(std::runtime_error("curl4esl: unknown attribute '\"" + setting.first + "\"'."));
		}
    }

	if(url.empty()) {
        throw esl::system::Stacktrace::add(std::runtime_error("curl4esl: Attribute 'url' is missing."));
	}

	if(hasLowSpeedLimit || hasLowSpeedTime) {
		if(!hasLowSpeedLimit) {
            throw esl::system::Stacktrace::add(std::runtime_error("curl4esl: attribute 'low-speed-time' specified but attribute 'low-speed-limit' is missing."));
		}
		if(!hasLowSpeedTime) {
            throw esl::system::Stacktrace::add(std::runtime_error("curl4esl: attribute 'lowSpeedLimit' specified but attribute 'low-speed-time' is missing."));
		}
		hasLowSpeedDefinition = true;
	}

	if(proxyServer.empty()) {
		if(!proxyUsername.empty()) {
            throw esl::system::Stacktrace::add(std::runtime_error("curl4esl: attribute 'proxy-username' specified but attribute 'proxy-server' is missing."));
		}
		if(!proxyPassword.empty()) {
            throw esl::system::Stacktrace::add(std::runtime_error("curl4esl: attribute 'proxy-password' specified but attribute 'proxy-server' is missing."));
		}
	}
}

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

	if(timeout > 0) {
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);
	}

	if(hasLowSpeedDefinition) {
        curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, lowSpeedLimit);
        curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME, lowSpeedTime);
	}

    /** set basic authentication if present*/
	if(!username.empty() || !password.empty()) {
		std::string basicAuthentication = createAuthenticationStr(username, password);
        curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
        curl_easy_setopt(curl, CURLOPT_USERPWD, basicAuthentication.c_str());
	}

	if(!proxyServer.empty()) {
		curl_easy_setopt(curl, CURLOPT_PROXY, proxyServer.c_str());
	}

	if(!proxyUsername.empty() || !proxyPassword.empty()) {
		std::string proxyAuthentication = createAuthenticationStr(proxyUsername, proxyPassword);
    	curl_easy_setopt(curl, CURLOPT_PROXYUSERPWD, proxyAuthentication.c_str());
	}

    /** set user agent */
	if(!userAgent.empty()) {
	    curl_easy_setopt(curl, CURLOPT_USERAGENT, userAgent.c_str());
	}

	/* ignore SSL certificate */
	if(skipSSLVerification) {
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
	}

	return std::unique_ptr<esl::com::http::client::Connection>(new Connection(curl, url));
}

} /* namespace client */
} /* namespace http */
} /* namespace com */
} /* namespace curl4esl */
