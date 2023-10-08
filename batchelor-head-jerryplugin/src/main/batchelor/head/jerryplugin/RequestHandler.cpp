#include <batchelor/head/common/Plugin.h>

#include <batchelor/head/jerryplugin/RequestHandler.h>

namespace batchelor {
namespace head {
namespace jerryplugin {

namespace {
std::vector<std::reference_wrapper<common::Plugin>> getPlugins(esl::object::Context& context, const std::set<std::string>& pluginIds) {
	std::vector<std::reference_wrapper<common::Plugin>> plugins;

	for(const auto& pluginId : pluginIds) {
		plugins.emplace_back(std::ref(context.getObject<common::Plugin>(pluginId)));
	}

	return plugins;
}
}

std::unique_ptr<esl::com::http::server::RequestHandler> RequestHandler::create(const std::vector<std::pair<std::string, std::string>>& aSettings) {
	RequestHandler::Settings settings;
    for(const auto& setting : aSettings) {
        if(setting.first == "db-connection-factory") {
            if(!settings.dbConnectionFactoryId.empty()) {
                throw esl::system::Stacktrace::add(std::runtime_error("multiple definition of attribute 'db-connection-factory'."));
            }
            settings.dbConnectionFactoryId = setting.second;
            if(settings.dbConnectionFactoryId.empty()) {
                throw esl::system::Stacktrace::add(std::runtime_error("Invalid value \"\" for attribute 'db-connection-factory'."));
            }
        }
        else if(setting.first == "plugin-id") {
            if(setting.second.empty()) {
                throw esl::system::Stacktrace::add(std::runtime_error("Invalid value \"\" for attribute '" + setting.first + "'."));
            }
            if(!settings.pluginIds.insert(setting.second).second) {
                throw esl::system::Stacktrace::add(std::runtime_error("Multiple definition of value \"" + setting.second + "\" for attribute '" + setting.first + "'."));
            }
        }
        else {
            throw esl::system::Stacktrace::add(std::runtime_error("unknown attribute '" + setting.first + "'."));
        }
    }

	return std::unique_ptr<esl::com::http::server::RequestHandler>(new RequestHandler(std::move(settings)));
}

void RequestHandler::initializeContext(esl::object::Context& context) {
	initializedSettings.reset(new InitializedSettings(context, settings));
}

RequestHandler::InitializedSettings::InitializedSettings(esl::object::Context& context, const Settings& settings)
: common::RequestHandler::InitializedSettings(context.getObject<esl::database::ConnectionFactory>(settings.dbConnectionFactoryId), getPlugins(context, settings.pluginIds))
{ }

RequestHandler::RequestHandler(Settings&& aSettings)
: common::RequestHandler(),
  settings(std::move(aSettings))
{ }


} /* namespace jerryplugin */
} /* namespace head */
} /* namespace batchelor */
