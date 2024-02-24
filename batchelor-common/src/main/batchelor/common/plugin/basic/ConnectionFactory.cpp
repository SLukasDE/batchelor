/*
 * This file is part of Batchelor.
 * Copyright (C) 2023-2024 Sven Lukas
 *
 * Batchelor is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * Batchelor is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public
 * License along with Batchelor.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <batchelor/common/plugin/basic/ConnectionFactory.h>

#include <esl/io/Input.h>
#include <esl/io/Output.h>
#include <esl/com/http/client/Connection.h>
#include <esl/com/http/client/CURLConnectionFactory.h>
#include <esl/com/http/client/Request.h>
#include <esl/com/http/client/Response.h>

namespace batchelor {
namespace common {
namespace plugin {
namespace basic {
namespace {

std::string extractApiKey(const std::vector<std::pair<std::string, std::string>>& settings) {
	for(const auto& setting : settings) {
		if(setting.first == "api-key") {
			return setting.second;
		}
	}

	return "";
}

std::vector<std::pair<std::string, std::string>> removeApiKey(const std::vector<std::pair<std::string, std::string>>& settings) {
	std::vector<std::pair<std::string, std::string>> rv;

	for(const auto& setting : settings) {
		if(setting.first != "api-key") {
			rv.push_back(setting);
		}
	}

	return rv;
}

class EslConnectionFactory : public esl::com::http::client::ConnectionFactory {
public:
	EslConnectionFactory(const std::vector<std::pair<std::string, std::string>>& settings);

	std::unique_ptr<esl::com::http::client::Connection> createConnection() const override;

private:
	std::unique_ptr<esl::com::http::client::ConnectionFactory> connectionFactory;
	std::string apiKey;
};

class EslConnection : public esl::com::http::client::Connection {
public:
	EslConnection(std::unique_ptr<esl::com::http::client::Connection> connection, const std::string& apiKey);

	esl::com::http::client::Response send(const esl::com::http::client::Request& request, esl::io::Output output, std::function<esl::io::Input (const esl::com::http::client::Response&)> createInput) const override;
	esl::com::http::client::Response send(const esl::com::http::client::Request& request, esl::io::Output output, esl::io::Input input) const override;

private:
	std::unique_ptr<esl::com::http::client::Connection> connection;
	std::string apiKey;
};

EslConnectionFactory::EslConnectionFactory(const std::vector<std::pair<std::string, std::string>>& settings)
: connectionFactory(esl::com::http::client::CURLConnectionFactory::createNative(esl::com::http::client::CURLConnectionFactory::Settings(removeApiKey(settings)))),
  apiKey(extractApiKey(settings))
{ }

std::unique_ptr<esl::com::http::client::Connection> EslConnectionFactory::createConnection() const {
	if(!connectionFactory) {
		return nullptr;
	}

	auto tmp = connectionFactory->createConnection();
	if(!tmp) {
		return nullptr;
	}

	return std::unique_ptr<esl::com::http::client::Connection>(new EslConnection(std::move(tmp), apiKey));
}

EslConnection::EslConnection(std::unique_ptr<esl::com::http::client::Connection> aConnection, const std::string& aApiKey)
: connection(std::move(aConnection)),
  apiKey(aApiKey)
{
	if(!connection) {
		throw std::runtime_error("empty connection");
	}
}

esl::com::http::client::Response EslConnection::send(const esl::com::http::client::Request& aRequest, esl::io::Output output, std::function<esl::io::Input (const esl::com::http::client::Response&)> createInput) const {
	esl::com::http::client::Request request(aRequest);
	if(!apiKey.empty()) {
		request.addHeader("Authorization", "Bearer " + apiKey);
	}
	return connection->send(request, std::move(output), createInput);
}

esl::com::http::client::Response EslConnection::send(const esl::com::http::client::Request& aRequest, esl::io::Output output, esl::io::Input input) const {
	esl::com::http::client::Request request(aRequest);
	if(!apiKey.empty()) {
		request.addHeader("Authorization", "Bearer " + apiKey);
	}
	return connection->send(request, std::move(output), std::move(input));
}
} /* anonymous namespace */

std::unique_ptr<plugin::ConnectionFactory> ConnectionFactory::create(const std::vector<std::pair<std::string, std::string>>& settings) {
	return std::unique_ptr<plugin::ConnectionFactory>(new ConnectionFactory(settings));
}

ConnectionFactory::ConnectionFactory(const std::vector<std::pair<std::string, std::string>>& settings)
: connectionFactory(new EslConnectionFactory(settings))
{ }

esl::com::http::client::ConnectionFactory& ConnectionFactory::get() {
	return *connectionFactory;
}

} /* namespace basic */
} /* namespace plugin */
} /* namespace common */
} /* namespace batchelor */
