#include <batchelor/common/plugin/oidc/ConnectionFactory.h>
#include <batchelor/common/plugin/oidc/ProxyConnectionFactory.h>
#include <batchelor/common/plugin/oidc/TokenFactoryClientCredentials.h>

#include <curl4esl/com/http/client/ConnectionFactory.h>

#include <esl/com/http/client/CURLConnectionFactory.h>

#include <stdexcept>

namespace batchelor {
namespace common {
namespace plugin {
namespace oidc {

std::unique_ptr<plugin::ConnectionFactory> ConnectionFactory::create(const std::vector<std::pair<std::string, std::string>>& settings) {
	return std::unique_ptr<plugin::ConnectionFactory>(new ConnectionFactory(settings));
}

ConnectionFactory::ConnectionFactory(const std::vector<std::pair<std::string, std::string>>& settings)
{
	std::vector<std::pair<std::string, std::string>> curlUserSettings;
	std::vector<std::pair<std::string, std::string>> curlIDPSettings;

	std::string url;
	std::string identityProvider;
	std::string clientId;
	std::string clientSecret;

	for(const auto& setting : settings) {
		if(setting.first.size() > 5 && setting.first.substr(0, 10) == "user-curl.") {
			std::string key = setting.first.substr(10);

			if(key == "url") {
				if(!url.empty()) {
					throw std::runtime_error("Multiple definition of parameter \"" + setting.first + "\"");
				}

				url = setting.second;

				if(url.empty()) {
					throw std::runtime_error("Invalid value \"" + setting.second + "\" for parameter \"" + setting.first + "\"");
				}
			}
			else {
				curlUserSettings.emplace_back(key, setting.second);
			}
		}
		else if(setting.first.size() > 5 && setting.first.substr(0, 9) == "idp-curl.") {
			std::string key = setting.first.substr(9);

			if(key == "url") {
				if(!identityProvider.empty()) {
					throw std::runtime_error("Multiple definition of parameter \"" + setting.first + "\"");
				}

				identityProvider = setting.second;

				if(identityProvider.empty()) {
					throw std::runtime_error("Invalid value \"" + setting.second + "\" for parameter \"" + setting.first + "\"");
				}
			}
			else {
				curlIDPSettings.emplace_back(key, setting.second);
			}
		}
		else if(setting.first == "url") {
			if(!url.empty()) {
				throw std::runtime_error("Multiple definition of parameter \"" + setting.first + "\"");
			}

			url = setting.second;

			if(url.empty()) {
				throw std::runtime_error("Invalid value \"" + setting.second + "\" for parameter \"" + setting.first + "\"");
			}
		}
		else if(setting.first == "identity-provider") {
			if(!identityProvider.empty()) {
				throw std::runtime_error("Multiple definition of parameter \"" + setting.first + "\"");
			}

			identityProvider = setting.second;

			if(identityProvider.empty()) {
				throw std::runtime_error("Invalid value \"" + setting.second + "\" for parameter \"" + setting.first + "\"");
			}
		}
		else if(setting.first == "client-id") {
			if(!clientId.empty()) {
				throw std::runtime_error("Multiple definition of parameter \"" + setting.first + "\"");
			}

			clientId = setting.second;

			if(clientId.empty()) {
				throw std::runtime_error("Invalid value \"" + setting.second + "\" for parameter \"" + setting.first + "\"");
			}
		}
		else if(setting.first == "client-secret") {
			if(!clientSecret.empty()) {
				throw std::runtime_error("Multiple definition of parameter \"" + setting.first + "\"");
			}

			clientSecret = setting.second;

			if(clientSecret.empty()) {
				throw std::runtime_error("Invalid value \"" + setting.second + "\" for parameter \"" + setting.first + "\"");
			}
		}
		else {
			throw std::runtime_error("Unknown parameter key=\"" + setting.first + "\" with value=\"" + setting.second + "\"");
		}
	}

	if(url.empty()) {
		throw std::runtime_error("Definition of parameter \"url\" is missing");
	}
	if(identityProvider.empty()) {
		throw std::runtime_error("Definition of parameter \"identity-provider\" is missing");
	}
	if(clientId.empty()) {
		throw std::runtime_error("Definition of parameter \"client-id\" is missing");
	}
	if(clientSecret.empty()) {
		throw std::runtime_error("Definition of parameter \"client-secret\" is missing");
	}

	curlUserSettings.emplace_back("url", url);
	curlIDPSettings.emplace_back("url", identityProvider);

	tokenFactory.reset(new TokenFactoryClientCredentials(std::unique_ptr<esl::com::http::client::ConnectionFactory>(new curl4esl::com::http::client::ConnectionFactory(esl::com::http::client::CURLConnectionFactory::Settings(curlIDPSettings))), clientId, clientSecret));
	connectionFactoryOriginal.reset(new curl4esl::com::http::client::ConnectionFactory(esl::com::http::client::CURLConnectionFactory::Settings(curlUserSettings)));
	connectionFactoryProxy.reset(new ProxyConnectionFactory(*connectionFactoryOriginal, *tokenFactory));
}

esl::com::http::client::ConnectionFactory& ConnectionFactory::get() {
	return *connectionFactoryProxy;
}

} /* namespace oidc */
} /* namespace plugin */
} /* namespace common */
} /* namespace batchelor */
