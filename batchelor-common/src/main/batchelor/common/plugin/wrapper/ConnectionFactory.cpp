#include <batchelor/common/plugin/wrapper/ConnectionFactory.h>

#include <stdexcept>

namespace batchelor {
namespace common {
namespace plugin {
namespace wrapper {

ConnectionFactory::Settings::Settings(const std::vector<std::pair<std::string, std::string>>& settings) {
	for(const auto& setting : settings) {
		if(setting.first == "connection-factory-id") {
			if(!connectionFactoryId.empty()) {
				throw std::runtime_error("Multiple definition of parameter \"" + setting.first + "\"");
			}

			connectionFactoryId = setting.second;

			if(connectionFactoryId.empty()) {
				throw std::runtime_error("Invalid value \"" + setting.second + "\" for parameter \"" + setting.first + "\"");
			}
		}
		else {
			throw std::runtime_error("Unknown parameter key=\"" + setting.first + "\" with value=\"" + setting.second + "\"");
		}
	}

	if(connectionFactoryId.empty()) {
		throw std::runtime_error("Definition of parameter \"connection-factory-id\" is missing");
	}
}

ConnectionFactory::InitializedSettings::InitializedSettings(esl::object::Context& context, const Settings& settings)
: connectionFactory(context.getObject<esl::com::http::client::ConnectionFactory>(settings.connectionFactoryId))
{ }

std::unique_ptr<plugin::ConnectionFactory> ConnectionFactory::create(const std::vector<std::pair<std::string, std::string>>& settings) {
	return std::unique_ptr<plugin::ConnectionFactory>(new ConnectionFactory(Settings(settings)));
}

ConnectionFactory::ConnectionFactory(const Settings& aSettings)
: settings(aSettings)
{ }

esl::com::http::client::ConnectionFactory& ConnectionFactory::get() {
	if(!initializedSettings) {
		throw std::runtime_error("ConnectionFactory wrapper is not initialized");
	}
	return initializedSettings->connectionFactory;
}

void ConnectionFactory::initializeContext(esl::object::Context& context) {
	initializedSettings.reset(new InitializedSettings(context, settings));
}

} /* namespace wrapper */
} /* namespace plugin */
} /* namespace common */
} /* namespace batchelor */
