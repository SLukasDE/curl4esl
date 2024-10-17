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

#include <esl/com/http/client/CURLConnectionFactory.h>
#include <esl/system/Stacktrace.h>
#include <esl/utility/Protocol.h>
#include <esl/utility/String.h>
#include <esl/utility/URL.h>

#include <curl4esl/com/http/client/ConnectionFactory.h>

#include <stdexcept>

namespace esl {
inline namespace v1_6 {
namespace com {
namespace http {
namespace client {

CURLConnectionFactory::Settings::Settings(const std::vector<std::pair<std::string, std::string>>& settings) {
	bool hasLowSpeedLimit = false;
	bool hasLowSpeedTime = false;
	bool hasUserAgent = false;
	bool hasTimeout = false;
	bool hasSkipSSLVerification = false;

    for(const auto& setting : settings) {
		if(setting.first == "url") {
			if(!url.empty()) {
	            throw system::Stacktrace::add(std::runtime_error("curl4esl: multiple definition of attribute 'url'."));
			}
			url = utility::String::rtrim(setting.second, '/');
			if(url.empty()) {
	            throw system::Stacktrace::add(std::runtime_error("curl4esl: Invalid value \"\" for attribute 'url'."));
			}

			esl::utility::URL aURL(url);
			if(aURL.getScheme() != esl::utility::Protocol::Type::http && aURL.getScheme() != esl::utility::Protocol::Type::https) {
	            throw system::Stacktrace::add(std::runtime_error("curl4esl: Invalid scheme in value \"" + url + "\" of attribute 'url'."));
			}
		}

		else if(setting.first == "timeout") {
			if(hasTimeout) {
	            throw system::Stacktrace::add(std::runtime_error("curl4esl: multiple definition of attribute 'timeout'."));
			}
			hasTimeout = true;
			timeout = utility::String::toNumber<decltype(timeout)>(setting.second);
			if(timeout < 0) {
	            throw system::Stacktrace::add(std::runtime_error("curl4esl: Invalid value \"" + std::to_string(timeout) + "\" for attribute 'timeout'."));
			}
		}

		else if(setting.first == "low-speed-limit") {
			if(hasLowSpeedLimit) {
	            throw system::Stacktrace::add(std::runtime_error("curl4esl: multiple definition of attribute 'low-speed-limit'."));
			}
			hasLowSpeedLimit = true;
			lowSpeedLimit = utility::String::toNumber<decltype(lowSpeedLimit)>(setting.second);
			if(lowSpeedLimit < 0) {
	            throw system::Stacktrace::add(std::runtime_error("curl4esl: Invalid value \"" + std::to_string(lowSpeedLimit) + "\" for attribute 'low-speed-limit'."));
			}
		}

		else if(setting.first == "low-speed-time") {
			if(hasLowSpeedTime) {
	            throw system::Stacktrace::add(std::runtime_error("curl4esl: multiple definition of attribute 'low-speed-time'."));
			}
			hasLowSpeedTime = true;
			lowSpeedTime = utility::String::toNumber<decltype(lowSpeedTime)>(setting.second);
			if(lowSpeedTime < 0) {
	            throw system::Stacktrace::add(std::runtime_error("curl4esl: Invalid value \"" + std::to_string(lowSpeedTime) + "\" for attribute 'low-speed-time'."));
			}
		}

		else if(setting.first == "username") {
			if(!username.empty()) {
	            throw system::Stacktrace::add(std::runtime_error("curl4esl: multiple definition of attribute 'username'."));
			}
			username = setting.second;
			if(username.empty()) {
	            throw system::Stacktrace::add(std::runtime_error("curl4esl: Invalid value \"\" for attribute 'username'."));
			}
		}

		else if(setting.first == "password") {
			if(!password.empty()) {
	            throw system::Stacktrace::add(std::runtime_error("curl4esl: multiple definition of attribute 'password'."));
			}
			password = setting.second;
			if(password.empty()) {
	            throw system::Stacktrace::add(std::runtime_error("curl4esl: Invalid value \"\" for attribute 'password'."));
			}
		}

		else if(setting.first == "proxy-server") {
			if(!proxyServer.empty()) {
	            throw system::Stacktrace::add(std::runtime_error("curl4esl: multiple definition of attribute 'proxy-server'."));
			}
			proxyServer = setting.second;
			if(proxyServer.empty()) {
	            throw system::Stacktrace::add(std::runtime_error("curl4esl: invalid value \"\" for attribute 'proxy-server'."));
			}
		}

		else if(setting.first == "proxy-username") {
			if(!proxyUsername.empty()) {
	            throw system::Stacktrace::add(std::runtime_error("curl4esl: multiple definition of attribute 'proxy-username'."));
			}
			proxyUsername = setting.second;
			if(proxyUsername.empty()) {
	            throw system::Stacktrace::add(std::runtime_error("curl4esl: invalid value \"\" for attribute 'proxy-username'."));
			}
		}

		else if(setting.first == "proxy-password") {
			if(!proxyPassword.empty()) {
	            throw system::Stacktrace::add(std::runtime_error("curl4esl: multiple definition of attribute 'proxy-password'."));
			}
			proxyPassword = setting.second;
			if(proxyPassword.empty()) {
	            throw system::Stacktrace::add(std::runtime_error("curl4esl: invalid value \"\" for attribute 'proxy-password'."));
			}
		}

		else if(setting.first == "user-ugent") {
			if(hasUserAgent) {
	            throw system::Stacktrace::add(std::runtime_error("curl4esl: multiple definition of attribute 'user-ugent'."));
			}
			userAgent = setting.second;
			hasUserAgent = true;
		}

		else if(setting.first == "skip-ssl-verification") {
			if(hasSkipSSLVerification) {
	            throw system::Stacktrace::add(std::runtime_error("curl4esl: multiple definition of attribute 'skip-ssl-verification'."));
			}
			std::string value = utility::String::toLower(setting.second);
			if(value == "true") {
				skipSSLVerification = true;
			}
			else if(value == "false") {
				skipSSLVerification = false;
			}
			else {
		    	throw system::Stacktrace::add(std::runtime_error("curl4esl: Invalid value \"" + setting.second + "\" for attribute 'skip-ssl-verification'"));
			}
		}

		else {
			throw system::Stacktrace::add(std::runtime_error("Key \"" + setting.first + "\" is unknown"));
		}
    }

	if(url.empty()) {
        throw system::Stacktrace::add(std::runtime_error("curl4esl: Attribute 'url' is missing."));
	}

	if(hasLowSpeedLimit || hasLowSpeedTime) {
		if(!hasLowSpeedLimit) {
            throw system::Stacktrace::add(std::runtime_error("curl4esl: attribute 'low-speed-time' specified but attribute 'low-speed-limit' is missing."));
		}
		if(!hasLowSpeedTime) {
            throw system::Stacktrace::add(std::runtime_error("curl4esl: attribute 'lowSpeedLimit' specified but attribute 'low-speed-time' is missing."));
		}
		hasLowSpeedDefinition = true;
	}

	if(proxyServer.empty()) {
		if(!proxyUsername.empty()) {
            throw system::Stacktrace::add(std::runtime_error("curl4esl: attribute 'proxy-username' specified but attribute 'proxy-server' is missing."));
		}
		if(!proxyPassword.empty()) {
            throw system::Stacktrace::add(std::runtime_error("curl4esl: attribute 'proxy-password' specified but attribute 'proxy-server' is missing."));
		}
	}
}

CURLConnectionFactory::CURLConnectionFactory(const Settings& settings)
: connectionFactory(createNative(settings))
{ }

std::unique_ptr<ConnectionFactory> CURLConnectionFactory::create(const std::vector<std::pair<std::string, std::string>>& settings) {
	return std::unique_ptr<ConnectionFactory>(new CURLConnectionFactory(Settings(settings)));
}

std::unique_ptr<ConnectionFactory> CURLConnectionFactory::createNative(const Settings& settings) {
	return std::unique_ptr<ConnectionFactory>(new curl4esl::com::http::client::ConnectionFactory(settings));
}

std::unique_ptr<Connection> CURLConnectionFactory::createConnection() const {
	return connectionFactory->createConnection();
}

} /* namespace client */
} /* namespace http */
} /* namespace com */
} /* inline namespace v1_6 */
} /* namespace esl */
