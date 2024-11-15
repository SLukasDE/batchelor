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

#include <batchelor/service/Logger.h>
#include <batchelor/service/server/RequestHandler.h>

#include <esl/com/http/server/exception/StatusCode.h>
#include <esl/com/http/server/Response.h>
#include <esl/io/input/String.h>
#include <esl/io/output/Memory.h>
#include <esl/io/output/String.h>
#include <esl/utility/HttpMethod.h>
#include <esl/utility/MIME.h>
#include <esl/utility/String.h>

#include "sergut/JsonDeserializer.h"
#include "sergut/XmlDeserializer.h"
#include "sergut/JsonSerializer.h"
#include "sergut/XmlSerializer.h"

#include <map>
#include <string>
#include <vector>

namespace batchelor {
namespace service {
namespace server {
namespace {
Logger logger("batchelor::service::server::RequestHandler");

const std::string emptyResponse = "{}";

std::vector<std::string> exctractAccepts(const std::map<std::string, std::string>& headers) {
	std::vector<std::string> accepts;

    for(const auto& entry : headers) {
    	if(entry.first != "Accept") {
    		continue;
    	}

		std::vector<std::string> tmpAccepts = esl::utility::String::split(entry.second, ',');
		for(const std::string& tmpAccept : tmpAccepts) {
    		std::vector<std::string> acceptSplit = esl::utility::String::split(tmpAccept, ';');
			if(!acceptSplit.empty()) {
				accepts.push_back(esl::utility::String::trim(acceptSplit[0]));
			}
		}

		break;
    }

	return accepts;
}

std::vector<esl::utility::MIME> exctractAcceptMIMEs(const std::map<std::string, std::string>& headers) {
	std::vector<esl::utility::MIME> acceptMIMEs;

	std::vector<std::string> accepts = exctractAccepts(headers);
	for(auto& accept : accepts) {
		acceptMIMEs.push_back(esl::utility::MIME(std::move(accept)));
    }

	return acceptMIMEs;
}


class InputHandler : public esl::io::input::String {
public:
	using ProcessHandler = void (InputHandler::*)();

	InputHandler(esl::com::http::server::RequestContext& aRequestContext, ProcessHandler aProcessHandler, std::unique_ptr<Service> aService, std::vector<std::string>&& aPathList)
	: requestContext(aRequestContext),
	  processHandler(aProcessHandler),
	  service(std::move(aService)),
	  pathList(std::move(aPathList)),
	  acceptMIMEs(exctractAcceptMIMEs(requestContext.getRequest().getHeaders()))
	  //acceptEncodings(exctractAccepts(requestContext.getRequest().getHeaders())),
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

	// GET: "/alive"
	void process_1() {
		service->alive();

		esl::utility::MIME responseMIME = esl::utility::MIME::Type::applicationJson;
		esl::com::http::server::Response response(200, responseMIME);
		esl::io::Output output = esl::io::output::Memory::create(emptyResponse.data(), emptyResponse.size());
		requestContext.getConnection().send(response, std::move(output));
	}

	// POST: "/fetch-task/{namespaceId}"
	void process_2() {
		const std::string& namespaceId = pathList[1];
		schemas::FetchRequest fetchRequest = sergut::JsonDeserializer(getString()).deserializeData<schemas::FetchRequest>();
		schemas::FetchResponse fetchResponse = service->fetchTask(namespaceId, fetchRequest);

		std::string responseContent;
		esl::utility::MIME responseMIME = getResponseMIME();

		if(responseMIME == esl::utility::MIME::Type::applicationXml) {
			sergut::XmlSerializer ser;
			ser.serializeData("fetchResponse", fetchResponse);
		    responseContent = ser.str();
		}
		else if(responseMIME == esl::utility::MIME::Type::applicationJson) {
		    sergut::JsonSerializer ser;
		    ser.serializeData(fetchResponse);
		    responseContent = ser.str();
		}
		else {
			throw esl::com::http::server::exception::StatusCode(415, "accept header requires \"application/xml\" or \"application/json\"");
		}

		esl::com::http::server::Response response(200, responseMIME);
		esl::io::Output output = esl::io::output::String::create(std::move(responseContent));
		requestContext.getConnection().send(response, std::move(output));
	}

	// GET: "/tasks/{namespaceId}[?[state={state}][&][nafter={eventNotAfter}][&][nbefore={eventNotBefore}]]"
	void process_3() {
		const std::string& namespaceId = pathList[1];
		std::string state;
		if(requestContext.getRequest().hasArgument("state")) {
			state = requestContext.getRequest().getArgument("state");
		}

		std::string eventNotAfter;
		if(requestContext.getRequest().hasArgument("nafter")) {
			eventNotAfter = requestContext.getRequest().getArgument("nafter");
		}

		std::string eventNotBefore;
		if(requestContext.getRequest().hasArgument("nbefore")) {
			eventNotBefore = requestContext.getRequest().getArgument("nbefore");
		}

		std::vector<schemas::TaskStatusHead> taskStatus = service->getTasks(namespaceId, state, eventNotAfter, eventNotBefore);

		std::string responseContent;
		esl::utility::MIME responseMIME = getResponseMIME();

		if(responseMIME == esl::utility::MIME::Type::applicationXml) {
			sergut::XmlSerializer ser;
			ser.serializeNestedData("tasks", "task", sergut::XmlValueType::Child, taskStatus);
		    responseContent = ser.str();
		}
		else if(responseMIME == esl::utility::MIME::Type::applicationJson) {
			sergut::JsonSerializer ser;
			ser.serializeData(taskStatus);
		    responseContent = ser.str();
		}
		else {
			throw esl::com::http::server::exception::StatusCode(415, "accept header requires \"application/xml\" or \"application/json\"");
		}

		esl::com::http::server::Response response(200, responseMIME);
		esl::io::Output output = esl::io::output::String::create(std::move(responseContent));
		requestContext.getConnection().send(response, std::move(output));
	}

	// GET: "/task/{namespaceId}/{taskId}"
	void process_4() {
		const std::string& namespaceId = pathList[1];
		const std::string& taskId = pathList[2];
		std::unique_ptr<schemas::TaskStatusHead> status = service->getTask(namespaceId, taskId);

		if(!status) {
			throw esl::com::http::server::exception::StatusCode(404, "{}");
		}

		std::string responseContent;
		esl::utility::MIME responseMIME = getResponseMIME();

		if(responseMIME == esl::utility::MIME::Type::applicationXml) {
			sergut::XmlSerializer ser;
			ser.serializeData("status", *status);
		    responseContent = ser.str();
		}
		else if(responseMIME == esl::utility::MIME::Type::applicationJson) {
			sergut::JsonSerializer ser;
			ser.serializeData(*status);
		    responseContent = ser.str();
		}
		else {
			throw esl::com::http::server::exception::StatusCode(415, "accept header requires \"application/xml\" or \"application/json\"");
		}

		esl::com::http::server::Response response(200, responseMIME);
		esl::io::Output output = esl::io::output::String::create(std::move(responseContent));
		requestContext.getConnection().send(response, std::move(output));
	}

	// POST: "/task/{namespaceId}"
	void process_5() {
		const std::string& namespaceId = pathList[1];
		schemas::RunRequest runRequest = sergut::JsonDeserializer(getString()).deserializeData<schemas::RunRequest>();
		schemas::RunResponse runResponse = service->runTask(namespaceId, runRequest);

		std::string responseContent;
		esl::utility::MIME responseMIME = getResponseMIME();

		if(responseMIME == esl::utility::MIME::Type::applicationXml) {
			sergut::XmlSerializer ser;
			//ser.serializeNestedData("nodes", "node", sergut::XmlValueType::Child, fetchResponse);
			ser.serializeData("runResponse", runResponse);
		    responseContent = ser.str();
		}
		else if(responseMIME == esl::utility::MIME::Type::applicationJson) {
		    sergut::JsonSerializer ser;
		    ser.serializeData(runResponse);
		    responseContent = ser.str();
		}
		else {
			throw esl::com::http::server::exception::StatusCode(415, "accept header requires \"application/xml\" or \"application/json\"");
		}

		esl::com::http::server::Response response(200, responseMIME);
		esl::io::Output output = esl::io::output::String::create(std::move(responseContent));
		requestContext.getConnection().send(response, std::move(output));
	}

	// POST: "/signal/{namespaceId}/{taskId}/{signal}"
	void process_6() {
		const std::string& namespaceId = pathList[1];
		const std::string& taskId = pathList[2];
		const std::string& signal = pathList[3];
		service->sendSignal(namespaceId, taskId, signal);

		//throw esl::com::http::server::exception::StatusCode(200, "{}");
		esl::utility::MIME responseMIME = esl::utility::MIME::Type::applicationJson;
		esl::com::http::server::Response response(200, responseMIME);
		esl::io::Output output = esl::io::output::Memory::create(emptyResponse.data(), emptyResponse.size());
		requestContext.getConnection().send(response, std::move(output));
	}

	// GET: "/event-types/{namespaceId}"
	void process_7() {
		const std::string& namespaceId = pathList[1];

		std::vector<std::string> eventTypes = service->getEventTypes(namespaceId);

		std::string responseContent;
		esl::utility::MIME responseMIME = getResponseMIME();

		if(responseMIME == esl::utility::MIME::Type::applicationXml) {
			sergut::XmlSerializer ser;
			ser.serializeNestedData("event-types", "event-type", sergut::XmlValueType::Child, eventTypes);
		    responseContent = ser.str();
		}
		else if(responseMIME == esl::utility::MIME::Type::applicationJson) {
			sergut::JsonSerializer ser;
			ser.serializeData(eventTypes);
		    responseContent = ser.str();
		}
		else {
			throw esl::com::http::server::exception::StatusCode(415, "accept header requires \"application/xml\" or \"application/json\"");
		}

		esl::com::http::server::Response response(200, responseMIME);
		esl::io::Output output = esl::io::output::String::create(std::move(responseContent));
		requestContext.getConnection().send(response, std::move(output));
	}

private:
    esl::com::http::server::RequestContext& requestContext;
    ProcessHandler processHandler;
    std::unique_ptr<Service> service;
	const std::vector<std::string> pathList;
	const std::vector<esl::utility::MIME> acceptMIMEs;

	esl::utility::MIME getResponseMIME() const {
		for(const auto& acceptMIME : acceptMIMEs) {
			if(acceptMIME == esl::utility::MIME::Type::applicationXml) {
				return esl::utility::MIME::Type::applicationXml;
			}
			else if(acceptMIME == esl::utility::MIME::Type::applicationJson) {
				return esl::utility::MIME::Type::applicationJson;
			}
		}

		return esl::utility::MIME();
	}
};
}

RequestHandler::RequestHandler(std::function<std::unique_ptr<Service>(const esl::object::Context&)> aCreateService)
: createService(aCreateService)
{ }

esl::io::Input RequestHandler::accept(esl::com::http::server::RequestContext& requestContext) const {
	std::unique_ptr<esl::io::Writer> writer;

	logger.trace << "Request:\n";
	logger.trace << "- getPath():        \"" << requestContext.getPath() << "\"\n";
	logger.trace << "- getMethod():      \"" << requestContext.getRequest().getMethod().toString() << "\"\n";
	logger.trace << "- getContentType(): \"" << requestContext.getRequest().getContentType().toString() << "\"\n";

	std::vector<std::string> pathList =  esl::utility::String::split(esl::utility::String::trim(requestContext.getPath(), '/'), '/');
	if(pathList.size() == 1 && pathList[0].empty()) {
		pathList.clear();
	}

	if(pathList.size() == 0) {
		return esl::io::Input();
	}

	esl::object::Context& objectContext = requestContext.getObjectContext();

	// GET: "/alive"
	if(pathList.size() == 1 && pathList[0] == "alive"
	&& requestContext.getRequest().getMethod() == esl::utility::HttpMethod::toString(esl::utility::HttpMethod::Type::httpGet)) {
		writer.reset(new InputHandler(requestContext, &InputHandler::process_1, makeService(objectContext), std::move(pathList)));
	}
	// POST: "/fetch-task/{namespaceId}"
	else if(pathList.size() == 2 && pathList[0] == "fetch-task"
	&& requestContext.getRequest().getMethod() == esl::utility::HttpMethod::toString(esl::utility::HttpMethod::Type::httpPost)) {
		writer.reset(new InputHandler(requestContext, &InputHandler::process_2, makeService(objectContext), std::move(pathList)));
	}
	// GET: "/tasks/{namespaceId}[?state={state}]"
	else if(pathList.size() == 2 && pathList[0] == "tasks"
	&& requestContext.getRequest().getMethod() == esl::utility::HttpMethod::toString(esl::utility::HttpMethod::Type::httpGet)) {
		writer.reset(new InputHandler(requestContext, &InputHandler::process_3, makeService(objectContext), std::move(pathList)));
	}
	// GET: "/task/{namespaceId}/{taskId}"
	else if(pathList.size() == 3 && pathList[0] == "task"
	&& requestContext.getRequest().getMethod() == esl::utility::HttpMethod::toString(esl::utility::HttpMethod::Type::httpGet)) {
		writer.reset(new InputHandler(requestContext, &InputHandler::process_4, makeService(objectContext), std::move(pathList)));
	}
	// POST: "/task/{namespaceId}"
	else if(pathList.size() == 2 && pathList[0] == "task"
	&& requestContext.getRequest().getMethod() == esl::utility::HttpMethod::toString(esl::utility::HttpMethod::Type::httpPost)) {
		writer.reset(new InputHandler(requestContext, &InputHandler::process_5, makeService(objectContext), std::move(pathList)));
	}
	// POST: "/signal/{namespaceId}/{taskId}/{signal}"
	else if(pathList.size() == 4 && pathList[0] == "signal"
	&& requestContext.getRequest().getMethod() == esl::utility::HttpMethod::toString(esl::utility::HttpMethod::Type::httpPost)) {
		writer.reset(new InputHandler(requestContext, &InputHandler::process_6, makeService(objectContext), std::move(pathList)));
	}
	// GET: "/event-types/{namespaceId}"
	else if(pathList.size() == 2 && pathList[0] == "event-types"
	&& requestContext.getRequest().getMethod() == esl::utility::HttpMethod::toString(esl::utility::HttpMethod::Type::httpGet)) {
		writer.reset(new InputHandler(requestContext, &InputHandler::process_7, makeService(objectContext), std::move(pathList)));
	}

	if(writer == nullptr) {
#if 1
		return esl::io::Input();
#else
		// Invalid request
		logger.error << "Invalid request:\n";
		logger.error << "- getPath():        \"" << requestContext.getPath() << "\"\n";
		logger.error << "- getMethod():      \"" << requestContext.getRequest().getMethod().toString() << "\"\n";
		logger.error << "- getContentType(): \"" << requestContext.getRequest().getContentType().toString() << "\"\n";
        throw esl::com::http::server::exception::StatusCode(405, esl::utility::MIME::Type::applicationJson, "{}");
#endif
	}

	return esl::io::Input(std::move(writer));

}

std::unique_ptr<Service> RequestHandler::makeService(esl::object::Context& context) const {
	std::unique_ptr<Service> service = createService(context);
	if(!service) {
        throw esl::com::http::server::exception::StatusCode(405, esl::utility::MIME::Type::applicationJson, "{}");
	}
	return service;
}

} /* namespace server */
} /* namespace service */
} /* namespace batchelor */
