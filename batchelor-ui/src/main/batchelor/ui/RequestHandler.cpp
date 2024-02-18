#include <batchelor/service/schemas/RunRequest.h>
#include <batchelor/service/schemas/RunResponse.h>
#include <batchelor/service/schemas/TaskStatusHead.h>

#include <batchelor/ui/RequestHandler.h>
#include <batchelor/ui/Service.h>
#include <batchelor/ui/Logger.h>

#include <esl/com/http/server/exception/StatusCode.h>
#include <esl/com/http/server/Request.h>
#include <esl/com/http/server/Response.h>
#include <esl/io/input/Closed.h>
#include <esl/io/input/String.h>
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

const std::string htmlHeader =
		"<!DOCTYPE html>\n"
		"<html>\n"
		"  <head>\n"
		"    <meta charset=\"utf-8\">\n"
		"    <title>Batchelor UI</title>\n"
		"    <!-- Bootstrap CSS einbinden -->\n"
		"    <link rel=\"stylesheet\" href=\"https://cdnjs.cloudflare.com/ajax/libs/bootstrap/5.3.0/css/bootstrap.min.css\">\n"
		"  </head>\n"
		"  <body>\n"
		"    <div class=\"container\">\n";

const std::string htmlFooter =
		"  </div>\n"
		"  \n"
		"  <!-- Bootstrap JS einbinden -->\n"
		"  <script src=\"https://cdnjs.cloudflare.com/ajax/libs/bootstrap/5.3.0/js/bootstrap.bundle.min.js\"></script>\n"
		"  </body>\n"
		"</html>\n";


class InputHandler : public esl::io::input::String {
public:
	using ProcessHandler = void (InputHandler::*)();

	InputHandler(esl::com::http::server::RequestContext& aRequestContext, std::unique_ptr<Service> aService, const std::string& aNamespaceId, ProcessHandler aProcessHandler)
	: requestContext(aRequestContext),
	  processHandler(aProcessHandler),
	  service(std::move(aService)),
	  namespaceId(aNamespaceId)
	{
		if(processHandler == nullptr) {
	        //throw esl::com::http::server::exception::StatusCode(500, "processHandler is nullptr");
			throw esl::system::Stacktrace::add(std::runtime_error("processHandler is nullptr"));
		}
	}

	void process() override {
		try {
			(this->*processHandler)();
		}
		catch(const esl::com::http::server::exception::StatusCode& e) {
			throw;
		}
		catch(const std::exception& e) {
	        throw esl::com::http::server::exception::StatusCode(500, esl::utility::MIME::Type::textPlain, e.what());
		}
		catch(...) {
	        throw esl::com::http::server::exception::StatusCode(500, esl::utility::MIME::Type::textPlain, "unknown error");
		}
	}

	static std::vector<std::pair<std::string, std::string>> parseFormData(const std::string& str) {
		std::vector<std::pair<std::string, std::string>> rv;

		std::vector<std::string> formsData = esl::utility::String::split(str, '&', true);
		for(const auto& formData : formsData) {
			std::vector<std::string> keyValue = esl::utility::String::split(formData, '=', true);
			if(keyValue.size() == 1) {
				rv.emplace_back(keyValue[0], "");
			}
			else if(keyValue.size() == 2) {
				rv.emplace_back(keyValue[0], keyValue[1]);
			}
			else {
				std::cout << "\"" << formData << "\": " << keyValue.size() << "\n";
			}
		}

		return rv;
	}

	// POST: "/send-event"
	void process_1() {
		std::string eventType;

		service::schemas::RunRequest runRequest;

		{
			std::map<int, std::pair<std::string, std::string>> settings;
			std::map<int, std::pair<std::string, std::string>> metrics;

			std::vector<std::pair<std::string, std::string>> keyValues = parseFormData(getString());
			for(const auto& keyValue : keyValues) {
				if(keyValue.first == "eventType") {
					runRequest.eventType = keyValue.second;
				}
				else if(keyValue.first == "condition") {
					runRequest.condition = keyValue.second;
				}
				else if(keyValue.first.substr(0, 12) == "setting_key_") {
					settings[std::stoi(keyValue.first.substr(12))].first = keyValue.second;
				}
				else if(keyValue.first.substr(0, 14) == "setting_value_") {
					settings[std::stoi(keyValue.first.substr(14))].second = keyValue.second;
				}
				else if(keyValue.first.substr(0, 12) == "metrics_key_") {
					metrics[std::stoi(keyValue.first.substr(12))].first = keyValue.second;
				}
				else if(keyValue.first.substr(0, 14) == "metrics_value_") {
					metrics[std::stoi(keyValue.first.substr(14))].second = keyValue.second;
				}
			}

			for(const auto& setting : settings) {
				runRequest.settings.emplace_back(service::schemas::Setting::make(setting.second.first, setting.second.second));
			}
			for(const auto& metric : metrics) {
				runRequest.metrics.emplace_back(service::schemas::Setting::make(metric.second.first, metric.second.second));
			}
		}

		service::schemas::RunResponse runResponse;
		runResponse = service->runTask(namespaceId, runRequest);

		esl::com::http::server::Response response(200, esl::utility::MIME::Type::textHtml);
		//response.addHeader("Location:", "./show-task/" + taskId);

		std::string responseContent;
		responseContent +=  htmlHeader +
				"<a href=\"..\">Home</a>\n"
				"    <h1>Event sent</h1>\n"
				"    <table class=\"table table-striped table-material\">\n"
				"      <tr>\n"
				"        <th align=left>Task ID</th>\n"
				"        <td align=left><a href=\"./show-task/" + runResponse.taskId + "\">" + runResponse.taskId + "</a></td>\n"
				"      </tr>\n"
				"      <tr>\n"
				"        <th align=left>Message</th>\n"
				"        <td align=left>" + runResponse.message + "</td>\n"
				"      </tr>\n"
				"    </table>\n"
		        + htmlFooter;
		esl::io::Output output = esl::io::output::String::create(std::move(responseContent));
		requestContext.getConnection().send(response, std::move(output));
	}

private:
    esl::com::http::server::RequestContext& requestContext;
    ProcessHandler processHandler;
    std::unique_ptr<Service> service;
    std::string namespaceId;
};

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
	std::unique_ptr<Service> client(new Service(*this));
	//Service client(*this);

	std::vector<std::string> pathList =  esl::utility::String::split(esl::utility::String::trim(requestContext.getPath(), '/'), '/', false);

	// GET: "/show-task/{taskId}"
	if(pathList.size() == 2 && pathList[0] == "show-task"
	&& requestContext.getRequest().getMethod() == esl::utility::HttpMethod::toString(esl::utility::HttpMethod::Type::httpGet)) {
		return responseShowTask(requestContext, *client, pathList[1]);
	}
	// GET: "/show-tasks[?state={state}][&eventNotAfter={eventNotAfter}][&eventNotBefore={eventNotBefore}]"
	else if(pathList.size() == 1 && pathList[0] == "show-tasks"
	&& requestContext.getRequest().getMethod() == esl::utility::HttpMethod::toString(esl::utility::HttpMethod::Type::httpGet)) {
		return responseShowTasks(requestContext, *client);
	}
	// GET: "/send-event[?eventType={event-type}]"
	if(pathList.size() == 1 && pathList[0] == "send-event"
	&& requestContext.getRequest().getMethod() == esl::utility::HttpMethod::toString(esl::utility::HttpMethod::Type::httpGet)) {
		return responseSendEvent(requestContext, *client, requestContext.getRequest().hasArgument("eventType") ? requestContext.getRequest().getArgument("eventType") : "");
	}
	// POST: "/send-event"
	if(pathList.size() == 1 && pathList[0] == "send-event"
	&& requestContext.getRequest().getMethod() == esl::utility::HttpMethod::toString(esl::utility::HttpMethod::Type::httpPost)) {
		std::unique_ptr<esl::io::Writer> writer(new InputHandler(requestContext, std::move(client), settings.namespaceId, &InputHandler::process_1));
		return esl::io::Input(std::move(writer));
		//return responseSendEventTypePost(requestContext, client, requestContext.getRequest().hasArgument("eventType") ? requestContext.getRequest().getArgument("eventType") : "");
	}
	// GET: "/show-event-types"
	else if(pathList.size() == 1 && pathList[0] == "show-event-types"
	&& requestContext.getRequest().getMethod() == esl::utility::HttpMethod::toString(esl::utility::HttpMethod::Type::httpGet)) {
		return responseShowEventTypes(requestContext, *client);
	}
	// GET: "/"
	else if((pathList.size() == 0 || (pathList.size() == 1 && pathList[0] == ""))
	&& requestContext.getRequest().getMethod() == esl::utility::HttpMethod::toString(esl::utility::HttpMethod::Type::httpGet)) {
		return responseMainPage(requestContext);
	}
	else {
		std::string str;
		str +=
				"<!DOCTYPE html>\n"
				"<html>\n"
				"  <head>\n"
				"    <meta charset=\"utf-8\">\n"
				"    <title>404</title>\n"
				"  </head>\n"
				"  <body>\n"
				"404\n"
				"  </body>\n"
				"</html>\n";

		esl::io::Output output = esl::io::output::String::create(str);
		esl::com::http::server::Response response(404, esl::utility::MIME::Type::textHtml);
		requestContext.getConnection().send(response, std::move(output));

		std::cout << "path: \"" << requestContext.getPath() << "\"\n";
		std::cout << "size: " << pathList.size() << "\n";
		for(std::size_t i=0; i< pathList.size(); ++i) {
			std::cout << "- \"" << pathList[i] << "\"\n";
		}

		return esl::io::input::Closed::create();
	}

	return esl::io::Input();
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

esl::io::Input RequestHandler::responseShowTask(esl::com::http::server::RequestContext& requestContext, service::Service& service, const std::string& taskId) const {
	std::unique_ptr<service::schemas::TaskStatusHead> tasksStatus;
	try {
		tasksStatus = service.getTask(settings.namespaceId, taskId);
	}
	catch(...) {
	}

	std::string str;
	if(tasksStatus) {
		str +=  htmlHeader +
				"<a href=\"..\">Home</a>\n"
				"    <h1>Task</h1>\n"
				"    <table class=\"table table-striped table-material\">\n"
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
				"    <table class=\"table table-striped table-material\">\n"
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
				"    <table class=\"table table-striped table-material\">\n"
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
				"    </table>\n"
		        + htmlFooter;

		esl::io::Output output = esl::io::output::String::create(str);
		esl::com::http::server::Response response(200, esl::utility::MIME::Type::textHtml);
		requestContext.getConnection().send(response, std::move(output));
	}
	else {
		str +=  htmlHeader +
				"<a href=\"..\">Home</a>\n"
				"Unknown task id \"" + taskId + "\"\n"
				+ htmlFooter;

		esl::io::Output output = esl::io::output::String::create(str);
		esl::com::http::server::Response response(404, esl::utility::MIME::Type::textHtml);
		requestContext.getConnection().send(response, std::move(output));
	}

	return esl::io::input::Closed::create();
}

esl::io::Input RequestHandler::responseShowTasks(esl::com::http::server::RequestContext& requestContext, service::Service& service) const {
	std::vector<service::schemas::TaskStatusHead> tasksStatus = service.getTasks(settings.namespaceId, ""/*state*/, ""/*eventNotAfter*/, ""/*eventNotBefore*/);
	std::string str =
			htmlHeader +
			"<a href=\"..\">Home</a>\n"
			"    <table class=\"table table-striped table-material\">\n"
			"      <thead>\n"
			"      <tr>\n"
			"        <th align=left>Task ID</th>\n"
			"        <th align=left>Event</th>\n"
			"        <th align=left>State</th>\n"
			"        <th align=left>Return code</th>\n"
			"        <th align=left>Message</th>\n"
#if 0
			"        <th align=left>Created TS</th>\n"
			"        <th align=left>Running TS</th>\n"
			"        <th align=left>Finished TS</th>\n"
			"        <th align=left>Last heartbeat TS</th>\n"
#endif
			"      </tr>\n"
			"      </thead>\n"
			"      <tbody>\n";
	for(const auto& taskStatus : tasksStatus) {
		str +=
				"      <tr>\n"
				"        <td align=left><a href=\"./show-task/" + taskStatus.runConfiguration.taskId + "\">" + taskStatus.runConfiguration.taskId + "</a></td>\n"
				"        <td align=left>" + taskStatus.runConfiguration.eventType + "</td>\n"
				"        <td align=left>" + taskStatus.state + "</td>\n"
				"        <td align=left>" + std::to_string(taskStatus.returnCode) + "</td>\n"
				"        <td align=left>" + taskStatus.message + "</td>\n"
#if 0
				"        <td align=left>" + taskStatus.tsCreated + "</td>\n"
				"        <td align=left>" + taskStatus.tsRunning + "</td>\n"
				"        <td align=left>" + taskStatus.tsFinished + "</td>\n"
				"        <td align=left>" + taskStatus.tsLastHeartBeat + "</td>\n"
#endif
				"      </tr>\n";
	}
	str +=
			"    </tbody>\n"
			"    </table>\n"
			+ htmlFooter;
	esl::io::Output output = esl::io::output::String::create(str);
	esl::com::http::server::Response response(200, esl::utility::MIME::Type::textHtml);
	requestContext.getConnection().send(response, std::move(output));

	return esl::io::input::Closed::create();
}

esl::io::Input RequestHandler::responseSendEvent(esl::com::http::server::RequestContext& requestContext, service::Service& service, const std::string& eventType) const {
	std::string str;
	str = R"V0G0N(
<!DOCTYPE html>
<html>
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<!-- Bootstrap CSS einbinden -->
<link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/bootstrap/5.3.0/css/bootstrap.min.css">
<title>Batchelor UI</title>
</head>

<body>
<div class="container">
<a href="..">Home</a>
<h1>Send event</h1>

<form method="post" action="./send-event">
  <h4>Event type</h4>
  <input type="text" name="eventType" class="form-control" placeholder="event type")V0G0N";
	if(!eventType.empty()) {
		str += " value=\"" + eventType + "\"";
	}
	str += R"V0G0N(>
  <p>
  
  <hr>
  <h4>Condition</h4>
  <input type="text" name="condition" class="form-control" placeholder="condition">
  <p>
  
  <hr>
  <h4>Settings</h4>
  <table class="table table-striped table-material" id="settingsTable">
    <tbody id="settingsTbody">
      <tr>
        <th align=left>Key</th>
        <th align=left>Value</th>
        <th align=left></th>
      </tr>
    </tbody>
  </table>
  <button id="addSetting" class="btn btn-primary" type="button">Weitere Zeile hinzufügen</button>
  <p>
  
  <hr>
  <h4>Metrics</h4>
  <table class="table table-striped table-material" id="metricsTable">
    <tbody id="metricsTbody">
      <tr>
        <th align=left>Key</th>
        <th align=left>Value</th>
        <th align=left></th>
      </tr>
    </tbody>
  </table>
  <button id="addMetrics" class="btn btn-primary" type="button">Weitere Zeile hinzufügen</button>
  <p>
  
  <hr>
  <button class="btn btn-success" type="submit">Formular absenden</button>
</form>
</div>

<!-- Bootstrap JS -->
<script src="https://cdn.jsdelivr.net/npm/bootstrap@5.1.0/dist/js/bootstrap.bundle.min.js"></script>

<script>
    document.getElementById('addSetting').addEventListener('click', function() {
      var tbody = document.getElementById('settingsTbody');
      
      var tr = document.createElement('tr');
      tbody.appendChild(tr);
      
      
      var tdInputKey = document.createElement('td');
      tr.appendChild(tdInputKey);
      
      var inputKey = document.createElement('input');
      inputKey.type = 'text';
      inputKey.classList.add('form-control');
      inputKey.placeholder = 'key';
      tdInputKey.appendChild(inputKey);

      
      var tdInputValue = document.createElement('td');
      tr.appendChild(tdInputValue);
      
      var inputValue = document.createElement('input');
      inputValue.type = 'text';
      inputValue.classList.add('form-control');
      inputValue.placeholder = 'value';
      tdInputValue.appendChild(inputValue);
      
      
      var tdButton = document.createElement('td');
      tr.appendChild(tdButton);
      
      var button = document.createElement('button');
      button.type = 'button';
      button.classList.add('btn', 'btn-danger');
      button.innerText = 'Zeile entfernen';
      button.addEventListener('click', function() {
        tr.remove();
      });
      tdButton.appendChild(button);
      
      var settingsTable = document.getElementById('settingsTable');
      var r=0; //start counting rows in table
      while(row=settingsTable.rows[r+1])
      {
        var idInputKey=row.cells[0].children[0];
        idInputKey.name = 'setting_key_'+r;
        
        var idInputValue=row.cells[1].children[0];
        idInputValue.name = 'setting_value_'+r;
        
        r++;
      }
    });
    
    document.getElementById('addMetrics').addEventListener('click', function() {
      var tbody = document.getElementById('metricsTbody');
      
      var tr = document.createElement('tr');
      tbody.appendChild(tr);
      
      var tdInputKey = document.createElement('td');
      tr.appendChild(tdInputKey);
      
      var inputKey = document.createElement('input');
      inputKey.type = 'text';
      inputKey.classList.add('form-control');
      inputKey.placeholder = 'key';
      tdInputKey.appendChild(inputKey);

      
      var tdInputValue = document.createElement('td');
      tr.appendChild(tdInputValue);
      
      var inputValue = document.createElement('input');
      inputValue.type = 'text';
      inputValue.classList.add('form-control');
      inputValue.placeholder = 'value';
      tdInputValue.appendChild(inputValue);
      
      
      var tdButton = document.createElement('td');
      tr.appendChild(tdButton);
      
      var button = document.createElement('button');
      button.type = 'button';
      button.classList.add('btn', 'btn-danger');
      button.innerText = 'Zeile entfernen';
      button.addEventListener('click', function() {
        tr.remove();
      });
      tdButton.appendChild(button);
      
      var settingsTable = document.getElementById('metricsTable');
      var r=0; //start counting rows in table
      while(row=settingsTable.rows[r+1])
      {
        var idInputKey=row.cells[0].children[0];
        idInputKey.name = 'metric_key_'+r;
        
        var idInputValue=row.cells[1].children[0];
        idInputValue.name = 'metric_value_'+r;
        
        r++;
      }
    });
</script>
</body>
</html>
)V0G0N";

	esl::io::Output output = esl::io::output::String::create(str);
	esl::com::http::server::Response response(200, esl::utility::MIME::Type::textHtml);
	requestContext.getConnection().send(response, std::move(output));

	return esl::io::input::Closed::create();
}

esl::io::Input RequestHandler::responseShowEventTypes(esl::com::http::server::RequestContext& requestContext, service::Service& service) const {
	std::vector<std::string> eventTypes = service.getEventTypes(settings.namespaceId);
	std::string str =
			htmlHeader +
			"<a href=\"..\">Home</a>\n"
			"    <table class=\"table table-striped table-material\">\n"
			"      <thead>\n"
			"      <tr>\n"
			"        <th align=left>Event type</th>\n"
			"      </tr>\n"
			"      </thead>\n"
			"      <tbody>\n";
	for(const auto& eventType : eventTypes) {
		str +=
				"      <tr>\n"
				"        <td align=left><a href=\"./send-event?eventType=" + eventType + "\">" + eventType + "</a></td>\n"
				"      </tr>\n";
	}
	str +=
			"    </tbody>\n"
			"    </table>\n"
			+ htmlFooter;
	esl::io::Output output = esl::io::output::String::create(str);
	esl::com::http::server::Response response(200, esl::utility::MIME::Type::textHtml);
	requestContext.getConnection().send(response, std::move(output));

	return esl::io::input::Closed::create();
}

esl::io::Input RequestHandler::responseMainPage(esl::com::http::server::RequestContext& requestContext) const {
	esl::com::http::server::Response response(404, esl::utility::MIME::Type::textHtml);
	std::string str =
			htmlHeader +
			"    <table class=\"table table-striped table-material\">\n"
			"      <thead>\n"
			"      <tr>\n"
			"        <th align=left>URL</th>\n"
			"        <th align=left>Description</th>\n"
			"      </tr>\n"
			"      </thead>\n"
			"      \n"
			"      <tbody>\n"
			"      <tr>\n"
			"        <td align=left><a href=\"./show-tasks\">show-tasks</a></td>\n"
			"        <td align=left>shows all tasks in the queue of the head server</td>\n"
			"      </tr>\n"
			"      <tr>\n"
			"        <td align=left>show-task/{task-id}</td>\n"
			"        <td align=left>shows detailed information of the specified tasks</td>\n"
			"      </tr>\n"
			"      <tr>\n"
			"        <td align=left><a href=\"./show-event-types\">show-event-types</a></td>\n"
			"        <td align=left>shows all available event types of the head server</td>\n"
			"      </tr>\n"
			"      <tr>\n"
			"        <td align=left><a href=\"./send-event\">send-event</a></td>\n"
			"        <td align=left>sends an event to the head server</td>\n"
			"      </tr>\n"
			"      </tbody>\n"
			"    </table>\n"
			+ htmlFooter;
	esl::io::Output output = esl::io::output::String::create(str);
	requestContext.getConnection().send(response, std::move(output));

	return esl::io::input::Closed::create();
}

} /* namespace ui */
} /* namespace batchelor */
