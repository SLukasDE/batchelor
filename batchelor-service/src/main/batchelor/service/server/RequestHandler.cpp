#include <batchelor/service/Logger.h>
#include <batchelor/service/server/RequestHandler.h>

#include <esl/com/http/server/exception/StatusCode.h>
#include <esl/io/input/String.h>
#include <esl/io/output/Memory.h>
#include <esl/io/output/String.h>
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
		(this->*processHandler)();
	}

	// POST: "/fetchJob"
	void process_1() {
		schemas::FetchRequest fetchRequest = sergut::JsonDeserializer(getString()).deserializeData<schemas::FetchRequest>();
		schemas::FetchResponse fetchResponse = service->fetchJob(fetchRequest);

		std::string responseContent;
		esl::utility::MIME responseMIME = getResponseMIME();

		if(responseMIME == esl::utility::MIME::Type::applicationXml) {
			sergut::XmlSerializer ser;
			//ser.serializeNestedData("nodes", "node", sergut::XmlValueType::Child, fetchResponse);
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

	// GET: "/jobs[?[state={state}][&][nafter={eventNotAfter}][&][nbefore={eventNotBefore}]]"
	void process_2() {
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

		std::vector<schemas::JobStatusHead> jobIds = service->getJobs(state, eventNotAfter, eventNotBefore);

		std::string responseContent;
		esl::utility::MIME responseMIME = getResponseMIME();

		if(responseMIME == esl::utility::MIME::Type::applicationXml) {
			sergut::XmlSerializer ser;
			ser.serializeNestedData("jobs", "job", sergut::XmlValueType::Child, jobIds);
		    responseContent = ser.str();
		}
		else if(responseMIME == esl::utility::MIME::Type::applicationJson) {
			sergut::JsonSerializer ser;
			ser.serializeData(jobIds);
		    responseContent = ser.str();
		}
		else {
			throw esl::com::http::server::exception::StatusCode(415, "accept header requires \"application/xml\" or \"application/json\"");
		}

		esl::com::http::server::Response response(200, responseMIME);
		esl::io::Output output = esl::io::output::String::create(std::move(responseContent));
		requestContext.getConnection().send(response, std::move(output));
	}

	// GET: "/job/{jobId}"
	void process_3() {
		const std::string& jobId = pathList[1];
		std::unique_ptr<schemas::JobStatusHead> status = service->getJob(jobId);

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

	// POST: "/job"
	void process_4() {
		schemas::RunRequest runRequest = sergut::JsonDeserializer(getString()).deserializeData<schemas::RunRequest>();
		schemas::RunResponse runResponse = service->runJob(runRequest);

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

	// POST: "/signal/{jobId}/{signal}"
	void process_5() {
		const std::string& jobId = pathList[1];
		const std::string& signal = pathList[2];
		service->sendSignal(jobId, signal);

		//throw esl::com::http::server::exception::StatusCode(200, "{}");
		esl::utility::MIME responseMIME = esl::utility::MIME::Type::applicationJson;
		esl::com::http::server::Response response(200, responseMIME);
		esl::io::Output output = esl::io::output::Memory::create(emptyResponse.data(), emptyResponse.size());
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

RequestHandler::RequestHandler(std::function<std::unique_ptr<Service>(esl::object::Context&)> aCreateService)
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

	// POST: "/fetchJob"
	if(pathList.size() == 1 && pathList[0] == "fetchJob"
	&& requestContext.getRequest().getMethod() == esl::utility::HttpMethod::toString(esl::utility::HttpMethod::Type::httpPost)) {
		writer.reset(new InputHandler(requestContext, &InputHandler::process_1, makeService(objectContext), std::move(pathList)));
	}
	// GET: "/jobs[?state={state}]"
	else if(pathList.size() == 1 && pathList[0] == "jobs"
	&& requestContext.getRequest().getMethod() == esl::utility::HttpMethod::toString(esl::utility::HttpMethod::Type::httpGet)) {
		writer.reset(new InputHandler(requestContext, &InputHandler::process_2, makeService(objectContext), std::move(pathList)));
	}
	// GET: "/job/{jobId}"
	else if(pathList.size() == 2 && pathList[0] == "job"
	&& requestContext.getRequest().getMethod() == esl::utility::HttpMethod::toString(esl::utility::HttpMethod::Type::httpGet)) {
		writer.reset(new InputHandler(requestContext, &InputHandler::process_3, makeService(objectContext), std::move(pathList)));
	}
	// POST: "/job"
	else if(pathList.size() == 1 && pathList[0] == "job"
	&& requestContext.getRequest().getMethod() == esl::utility::HttpMethod::toString(esl::utility::HttpMethod::Type::httpPost)) {
		writer.reset(new InputHandler(requestContext, &InputHandler::process_4, makeService(objectContext), std::move(pathList)));
	}
	// POST: "/signal/{jobId}/{signal}"
	else if(pathList.size() == 3 && pathList[0] == "signal"
	&& requestContext.getRequest().getMethod() == esl::utility::HttpMethod::toString(esl::utility::HttpMethod::Type::httpPut)) {
		writer.reset(new InputHandler(requestContext, &InputHandler::process_5, makeService(objectContext), std::move(pathList)));
	}

	if(writer == nullptr) {
		// Invalid request
		logger.error << "Invalid request:\n";
		logger.error << "- getPath():        \"" << requestContext.getPath() << "\"\n";
		logger.error << "- getMethod():      \"" << requestContext.getRequest().getMethod().toString() << "\"\n";
		logger.error << "- getContentType(): \"" << requestContext.getRequest().getContentType().toString() << "\"\n";
        throw esl::com::http::server::exception::StatusCode(405, esl::utility::MIME::Type::applicationJson, "{}");
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
