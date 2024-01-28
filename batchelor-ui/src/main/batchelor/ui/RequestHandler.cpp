#include <batchelor/service/schemas/TaskStatusHead.h>

#include <batchelor/ui/RequestHandler.h>
#include <batchelor/ui/Service.h>
#include <batchelor/ui/Logger.h>

#include <esl/com/http/server/Response.h>
#include <esl/com/http/server/Request.h>
#include <esl/io/input/Closed.h>
#include <esl/io/Output.h>
#include <esl/io/output/String.h>
#include <esl/system/Stacktrace.h>
#include <esl/utility/HttpMethod.h>
#include <esl/utility/MIME.h>
#include <esl/utility/String.h>

#include <iostream>
#include <stdexcept>

namespace batchelor {
namespace ui {
namespace {
Logger logger("batchelor::ui::RequestHandler");
}

RequestHandler::Settings::Settings(const std::vector<std::pair<std::string, std::string>>& settings) {
    for(const auto& setting : settings) {
        if(setting.first == "http-connection-factory") {
            if(setting.second.empty()) {
                throw esl::system::Stacktrace::add(std::runtime_error("Invalid value \"\" for attribute '" + setting.first + "'."));
            }
            if(connectionFactoryIds.insert(setting.second).second == false) {
                throw esl::system::Stacktrace::add(std::runtime_error("Multiple definition of attribute '" + setting.first + "'."));
            }
        }
        else {
            throw esl::system::Stacktrace::add(std::runtime_error("unknown attribute '" + setting.first + "'."));
        }
    }

    if(connectionFactoryIds.empty()) {
        throw esl::system::Stacktrace::add(std::runtime_error("Definition of attribute 'http-conneciton-factory' is missing."));
    }
}

RequestHandler::Settings::Settings(const Procedure::Settings& settings)
: namespaceId(settings.namespaceId),
  connectionFactoryIds(settings.connectionFactoryIds)
{ }

RequestHandler::InitializedSettings::InitializedSettings(esl::object::Context& context, const Settings& settings)
{
	for(const auto& connectionId : settings.connectionFactoryIds) {
		common::plugin::ConnectionFactory& connectionFactory = context.getObject<common::plugin::ConnectionFactory>(connectionId);
		connectionFactories.emplace_back(connectionId, std::ref(connectionFactory));
	}
}

RequestHandler::RequestHandler(const Settings& aSettings)
: settings(aSettings)
{ }

std::unique_ptr<esl::com::http::server::RequestHandler> RequestHandler::create(const std::vector<std::pair<std::string, std::string>>& settings) {
	return std::unique_ptr<esl::com::http::server::RequestHandler>(new RequestHandler(Settings(settings)));
}


void RequestHandler::initializeContext(esl::object::Context& context) {
	initializedSettings.reset(new InitializedSettings(context, settings));
}

esl::io::Input RequestHandler::accept(esl::com::http::server::RequestContext& requestContext) const {
	Service client(*this);

	std::vector<std::string> pathList =  esl::utility::String::split(esl::utility::String::trim(requestContext.getPath(), '/'), '/', false);

	// GET: "/show-task/{taskId}"
	if(pathList.size() == 2 && pathList[0] == "show-task"
	&& requestContext.getRequest().getMethod() == esl::utility::HttpMethod::toString(esl::utility::HttpMethod::Type::httpGet)) {

		std::unique_ptr<service::schemas::TaskStatusHead> tasksStatus = client.getTask(settings.namespaceId, pathList[1]);

		std::string str =
				"<!DOCTYPE html>\n"
				"<html lang=\"de\">\n"
				"  <head>\n"
				"    <meta charset=\"utf-8\">\n"
				"    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
				"    <title>Batchelor UI</title>\n"
				"  </head>\n"
				"  <body>\n"
				"  Task \"" + pathList[1] + "\"\n";
		if(tasksStatus) {
			str +=
					"    <table border=8>\n"
					"      <tr>\n"
					"        <th align=left>Task ID</th>\n"
					"        <td align=left>" + tasksStatus->runConfiguration.taskId + "</td>\n"
					"      </tr>\n"
					"      <tr>\n"
					"        <th align=left>Event</th>\n"
					"        <td align=left>" + tasksStatus->runConfiguration.eventType + "</td>\n"
					"      </tr>\n"
					"      <tr>\n"
					"        <th align=left>Status</th>\n"
					"        <td align=left>" + tasksStatus->state + "</td>\n"
					"      </tr>\n"
					"      <tr>\n"
					"        <th align=left>Return Code</th>\n"
					"        <td align=left>" + std::to_string(tasksStatus->returnCode) + "</td>\n"
					"      </tr>\n"
					"      <tr>\n"
					"        <th align=left>Message</th>\n"
					"        <td align=left>" + tasksStatus->message + "</td>\n"
					"      </tr>\n"
					"      <tr>\n"
					"        <th align=left>Created TS</th>\n"
					"        <td align=left>" + tasksStatus->tsCreated + "</td>\n"
					"      </tr>\n"
					"      <tr>\n"
					"        <th align=left>Running TS</th>\n"
					"        <td align=left>" + tasksStatus->tsRunning + "</td>\n"
					"      </tr>\n"
					"      <tr>\n"
					"        <th align=left>Finished TS</th>\n"
					"        <td align=left>" + tasksStatus->tsFinished + "</td>\n"
					"      </tr>\n"
					"      <tr>\n"
					"        <th align=left>Last heartbeat TS</th>\n"
					"        <td align=left>" + tasksStatus->tsLastHeartBeat + "</td>\n"
					"      </tr>\n"
					"    </table>\n"
					"    <h1>Metrics</h1>\n"
					"    <table border=8>\n"
					"      <tr>\n"
					"        <th align=left>Key</th>\n"
					"        <th align=left>Value</th>\n"
					"      </tr>\n";
			for(const auto& entry : tasksStatus->runConfiguration.metrics) {
				str +=
						"      <tr>\n"
						"        <td align=left>" + entry.key + "</td>\n"
						"        <td align=left>" + entry.value + "</td>\n"
						"      </tr>\n";
			}
			str +=
					"    </table>\n"
					"    <h1>Settings</h1>\n"
					"    <table border=8>\n"
					"      <tr>\n"
					"        <th align=left>Key</th>\n"
					"        <th align=left>Value</th>\n"
					"      </tr>\n";
			for(const auto& entry : tasksStatus->runConfiguration.settings) {
				str +=
						"      <tr>\n"
						"        <td align=left>" + entry.key + "</td>\n"
						"        <td align=left>" + entry.value + "</td>\n"
						"      </tr>\n";
			}
			str +=
					"    </table>\n";
		}
		str +=
				"  </body>\n"
				"</html>\n";
		esl::io::Output output = esl::io::output::String::create(str);
		esl::com::http::server::Response response(200, esl::utility::MIME::Type::textHtml);
		requestContext.getConnection().send(response, std::move(output));
	}
	// GET: "/show-tasks[?state={state}][&eventNotAfter={eventNotAfter}][&eventNotBefore={eventNotBefore}]"
	else if(pathList.size() == 1 && pathList[0] == "show-tasks"
	&& requestContext.getRequest().getMethod() == esl::utility::HttpMethod::toString(esl::utility::HttpMethod::Type::httpGet)) {
	//if(requestContext.getPath() == "/show-tasks") {
		std::vector<service::schemas::TaskStatusHead> tasksStatus = client.getTasks(settings.namespaceId, ""/*state*/, ""/*eventNotAfter*/, ""/*eventNotBefore*/);
		std::string str =
				"<!DOCTYPE html>\n"
				"<html lang=\"de\">\n"
				"  <head>\n"
				"    <meta charset=\"utf-8\">\n"
				"    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
				"    <title>Batchelor UI</title>\n"
				"  </head>\n"
				"  <body>\n"
				"    <table border=8>\n"
				"      <tr>\n"
				"        <th align=left>Task ID</th>\n"
				"        <th align=left>Event</th>\n"
				"        <th align=left>State</th>\n"
				"        <th align=left>Return code</th>\n"
				"        <th align=left>Message</th>\n"
				"        <th align=left>Created TS</th>\n"
				"        <th align=left>Running TS</th>\n"
				"        <th align=left>Finished TS</th>\n"
				"        <th align=left>Last heartbeat TS</th>\n"
				"        <th align=left>Details</th>\n"
				"      </tr>\n";
		for(const auto& taskStatus : tasksStatus) {
			str +=
					"      <tr>\n"
					"        <td align=right>" + taskStatus.runConfiguration.taskId + "</td>\n"
					"        <td align=right>" + taskStatus.runConfiguration.eventType + "</td>\n"
					"        <td align=right>" + taskStatus.state + "</td>\n"
					"        <td align=right>" + std::to_string(taskStatus.returnCode) + "</td>\n"
					"        <td align=right>" + taskStatus.message + "</td>\n"
					"        <td align=right>" + taskStatus.tsCreated + "</td>\n"
					"        <td align=right>" + taskStatus.tsRunning + "</td>\n"
					"        <td align=right>" + taskStatus.tsFinished + "</td>\n"
					"        <td align=right>" + taskStatus.tsLastHeartBeat + "</td>\n"
					"        <td align=right><a href=\"./show-task/" + taskStatus.runConfiguration.taskId + "\">link</a></td>\n"
					"      </tr>\n";
		}
		str +=
				"    </table>\n"
				"  </body>\n"
				"</html>\n";
		esl::io::Output output = esl::io::output::String::create(str);
		esl::com::http::server::Response response(200, esl::utility::MIME::Type::textHtml);
		requestContext.getConnection().send(response, std::move(output));
	}
	else {
		esl::com::http::server::Response response(404, esl::utility::MIME::Type::textHtml);
		std::string str =
				"<!DOCTYPE html>\n"
				"<html lang=\"de\">\n"
				"  <head>\n"
				"    <meta charset=\"utf-8\">\n"
				"    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
				"    <title>Batchelor UI - unknown URL</title>\n"
				"  </head>\n"
				"  <body>\n"
				"    <table border=8>\n"
				"      <tr>\n"
				"        <th align=left>URL</th>\n"
				"        <th align=left>Description</th>\n"
				"      </tr>\n"
				"      <tr>\n"
				"        <td align=left>show-tasks</td>\n"
				"        <td align=left>shows all tasks in the queue of the head server</td>\n"
				"      </tr>\n"
				"      <tr>\n"
				"        <td align=left>show-task/{task-id}</td>\n"
				"        <td align=left>shows all information of the specified tasks</td>\n"
				"      </tr>\n"
				"  </body>\n"
				"</html>\n";
		esl::io::Output output = esl::io::output::String::create(str);
		requestContext.getConnection().send(response, std::move(output));

		std::cout << std::endl;
		for(std::size_t i = 0; i<pathList.size(); ++i) {
			std::cout << "pathList[" << i << "] = \"" << pathList[i] << "\"\n";
		}
		std::cout << std::endl;
	}

	return esl::io::input::Closed::create();
}

std::unique_ptr<esl::com::http::client::Connection> RequestHandler::createHTTPConnection() const {
	if(!httpConnectionFactory && initializedSettings && nextConnectionFactory < initializedSettings->connectionFactories.size()) {
		httpConnectionFactory = &initializedSettings->connectionFactories[nextConnectionFactory].second.get();
		nextConnectionFactory = (nextConnectionFactory + 1) % initializedSettings->connectionFactories.size();
	}
	if(!httpConnectionFactory) {
		if(!initializedSettings) {
			logger.warn << "InizializeContext has not been called.\n";
		}
		throw esl::system::Stacktrace::add(std::runtime_error("cannot create http connection factory."));
	}

	auto httpConnection = httpConnectionFactory->get().createConnection();
	if(!httpConnection) {
		throw esl::system::Stacktrace::add(std::runtime_error("cannot create http connection."));
	}
	return httpConnection;
}

} /* namespace ui */
} /* namespace batchelor */
