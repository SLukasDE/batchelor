#include <batchelor/head/RequestHandler.h>
#include <batchelor/head/Service.h>
#include <batchelor/head/Logger.h>

#include <esl/system/Stacktrace.h>

#include <stdexcept>

namespace batchelor {
namespace head {
namespace {
Logger logger("batchelor::head::RequestHandler");
} /* namespace */

RequestHandler::InitializedSettings::InitializedSettings(esl::object::Context& context, const Settings& settings)
: dbConnectionFactory(context.getObject<esl::database::ConnectionFactory>(settings.dbConnectionFactoryId))
{ }

RequestHandler::InitializedSettings::InitializedSettings(esl::database::ConnectionFactory& aDbConnectionFactory)
: dbConnectionFactory(aDbConnectionFactory)
{ }

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
        else {
            throw esl::system::Stacktrace::add(std::runtime_error("unknown attribute '\"" + setting.first + "\"'."));
        }
    }

	return std::unique_ptr<esl::com::http::server::RequestHandler>(new RequestHandler(settings));
}

RequestHandler::RequestHandler(Settings aSettings)
: service::server::RequestHandler([this](const esl::object::Context& context)
		{
			return std::unique_ptr<service::Service>(new Service(context, initializedSettings->dbConnectionFactory));
		}),
  settings(aSettings)
{ }

RequestHandler::RequestHandler(esl::database::ConnectionFactory& dbConnectionFactory)
: service::server::RequestHandler([this](const esl::object::Context& context)
		{
			return std::unique_ptr<service::Service>(new Service(context, initializedSettings->dbConnectionFactory));
		}),
  initializedSettings(new InitializedSettings(dbConnectionFactory))
{ }

void RequestHandler::initializeContext(esl::object::Context& context) {
	initializedSettings.reset(new InitializedSettings(context, settings));
}

} /* namespace head */
} /* namespace batchelor */
