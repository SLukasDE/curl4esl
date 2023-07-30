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

#ifndef CURL4ESL_COM_HTTP_CLIENT_CONNECTIONFACTORY_H_
#define CURL4ESL_COM_HTTP_CLIENT_CONNECTIONFACTORY_H_

#include <esl/com/http/client/ConnectionFactory.h>
#include <esl/com/http/client/Connection.h>

#include <curl/curl.h>

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace curl4esl {
inline namespace v1_6 {
namespace com {
namespace http {
namespace client {

class ConnectionFactory : public esl::com::http::client::ConnectionFactory {
public:
	static std::unique_ptr<esl::com::http::client::ConnectionFactory> create(const std::vector<std::pair<std::string, std::string>>& settings);

	ConnectionFactory(const std::vector<std::pair<std::string, std::string>>& settings);

	std::unique_ptr<esl::com::http::client::Connection> createConnection() const override;

private:
	std::string url;

	long timeout = 0;

	bool hasLowSpeedDefinition = false;
	long lowSpeedLimit = 0;
	long lowSpeedTime = 0;

	std::string username;
	std::string password;

	std::string proxyServer;

	std::string proxyUsername;
	std::string proxyPassword;

	std::string userAgent = "esl-http-client";

	bool skipSSLVerification = false;
};

} /* namespace client */
} /* namespace http */
} /* namespace com */
} /* inline namespace v1_6 */
} /* namespace curl4esl */

#endif /* CURL4ESL_COM_HTTP_CLIENT_CONNECTIONFACTORY_H_ */
