#include <batchelor/service/client/Service.h>
#include <batchelor/service/Logger.h>
#include <batchelor/service/schemas/Signal.h>

#include <esl/com/http/client/Request.h>
#include <esl/com/http/client/Response.h>
#include <esl/io/Input.h>
#include <esl/io/input/String.h>
#include <esl/io/Output.h>
#include <esl/io/output/String.h>
#include <esl/utility/String.h>
#include <esl/utility/HttpMethod.h>
#include <esl/utility/MIME.h>
#include <esl/system/Stacktrace.h>

#include <map>
#include <stdexcept>

#include "sergut/JsonDeserializer.h"
#include "sergut/XmlDeserializer.h"
#include "sergut/JsonSerializer.h"
#include "sergut/XmlSerializer.h"

namespace batchelor {
namespace service {
namespace client {

namespace {
Logger logger("batchelor::service::client::Service");
}

Service::Service(const esl::com::http::client::Connection& aConnection)
: connection(aConnection)
{ }

schemas::FetchResponse Service::fetchTask(const schemas::FetchRequest& fetchRequest) {
	schemas::FetchResponse fetchResponse;

	static const std::string serviceUrl = "fetchTask";
    esl::com::http::client::Request request(serviceUrl, esl::utility::HttpMethod::Type::httpPost, esl::utility::MIME::Type::applicationJson);
    request.addHeader("Accept", esl::utility::MIME::toString(esl::utility::MIME::Type::applicationJson) + "," + esl::utility::MIME::toString(esl::utility::MIME::Type::applicationXml));

    sergut::JsonSerializer serializer;
    serializer.serializeData(fetchRequest);
    esl::io::Output output(esl::io::output::String::create(serializer.str()));

	esl::io::input::String consumerString;
	esl::io::Input input(static_cast<esl::io::Writer&>(consumerString));

	esl::com::http::client::Response response = connection.send(std::move(request), std::move(output), std::move(input));

    if(response.getStatusCode() == 200) {
        if(response.getContentType() == esl::utility::MIME::Type::applicationJson) {
        	if(!consumerString.getString().empty()) {
                sergut::JsonDeserializer deSerializer(consumerString.getString());
                fetchResponse = deSerializer.deserializeData<schemas::FetchResponse>();
        	}
        }
        else if(response.getContentType() == esl::utility::MIME::Type::applicationXml) {
        	if(!consumerString.getString().empty()) {
                sergut::XmlDeserializer deSerializer(consumerString.getString());
                fetchResponse = deSerializer.deserializeData<schemas::FetchResponse>("response");
        	}
        }
        else {
        	throw esl::system::Stacktrace::add(std::runtime_error("Received not supported response content type \"" + response.getContentType().toString() + "\""));
        }
    }
    else {
    	throw esl::system::Stacktrace::add(std::runtime_error("Received not supported status code \"" + std::to_string(response.getStatusCode()) + "\""));
    }

    return fetchResponse;
}

std::vector<schemas::TaskStatusHead> Service::getTasks(const std::string& state, const std::string& eventNotAfter, const std::string& eventNotBefore) {
	std::vector<schemas::TaskStatusHead> tasks;

	std::string serviceUrl = "tasks";
	{
		std::string args;

		if(!state.empty()) {
			args += args.empty() ? "?" : "&";
			args += "state=" + state;
		}
		if(!eventNotAfter.empty()) {
			args += args.empty() ? "?" : "&";
			args += "nafter=" + eventNotAfter;
		}
		if(eventNotBefore.empty()) {
			args += args.empty() ? "?" : "&";
			args += "nbefore=" + eventNotBefore;
		}

		serviceUrl += args;
	}

    esl::com::http::client::Request request(serviceUrl, esl::utility::HttpMethod::Type::httpGet, esl::utility::MIME::Type::applicationJson);
    request.addHeader("Accept", esl::utility::MIME::toString(esl::utility::MIME::Type::applicationJson) + "," + esl::utility::MIME::toString(esl::utility::MIME::Type::applicationXml));

	esl::io::input::String inputWriterString;
	esl::io::Writer& inputWriter(inputWriterString);
	esl::io::Input input(inputWriter);

	esl::com::http::client::Response response = connection.send(std::move(request), esl::io::Output(), std::move(input));

    if(response.getStatusCode() == 200) {
        if(response.getContentType() == esl::utility::MIME::Type::applicationJson) {
        	if(!inputWriterString.getString().empty()) {
                sergut::JsonDeserializer deSerializer(inputWriterString.getString());
                tasks = deSerializer.deserializeData<std::vector<schemas::TaskStatusHead>>();
        	}
        }
        else if(response.getContentType() == esl::utility::MIME::Type::applicationXml) {
        	if(!inputWriterString.getString().empty()) {
                sergut::XmlDeserializer deSerializer(inputWriterString.getString());
                tasks = deSerializer.deserializeNestedData<std::vector<schemas::TaskStatusHead>>("tasks", "task");
        	}
        }
        else {
        	throw esl::system::Stacktrace::add(std::runtime_error("Received not supported response content type \"" + response.getContentType().toString() + "\""));
        }
    }
    else {
    	throw esl::system::Stacktrace::add(std::runtime_error("Received not supported status code \"" + std::to_string(response.getStatusCode()) + "\""));
    }

    return tasks;
}

std::unique_ptr<schemas::TaskStatusHead> Service::getTask(const std::string& taskId) {
	std::unique_ptr<schemas::TaskStatusHead> status;

	std::string serviceUrl = "task/" + taskId;
    esl::com::http::client::Request request(serviceUrl, esl::utility::HttpMethod::Type::httpGet, esl::utility::MIME::Type::applicationJson);
    request.addHeader("Accept", esl::utility::MIME::toString(esl::utility::MIME::Type::applicationJson) + "," + esl::utility::MIME::toString(esl::utility::MIME::Type::applicationXml));

	esl::io::input::String consumerString;
	esl::io::Input input(static_cast<esl::io::Writer&>(consumerString));

	esl::com::http::client::Response response = connection.send(std::move(request), esl::io::Output(), std::move(input));

    if(response.getStatusCode() == 200) {
        if(response.getContentType() == esl::utility::MIME::Type::applicationJson) {
        	if(!consumerString.getString().empty()) {
        		status.reset(new schemas::TaskStatusHead);
                sergut::JsonDeserializer deSerializer(consumerString.getString());
                *status = deSerializer.deserializeData<schemas::TaskStatusHead>();
        	}
        }
        else if(response.getContentType() == esl::utility::MIME::Type::applicationXml) {
        	if(!consumerString.getString().empty()) {
        		status.reset(new schemas::TaskStatusHead);
                sergut::XmlDeserializer deSerializer(consumerString.getString());
                *status = deSerializer.deserializeData<schemas::TaskStatusHead>("status");
        	}
        }
        else {
        	throw esl::system::Stacktrace::add(std::runtime_error("Received not supported response content type \"" + response.getContentType().toString() + "\""));
        }
    }
    else if(response.getStatusCode() == 404) {
    }
    else {
    	throw esl::system::Stacktrace::add(std::runtime_error("Received not supported status code \"" + std::to_string(response.getStatusCode()) + "\""));
    }

    return status;
}

schemas::RunResponse Service::runTask(const schemas::RunRequest& runRequest) {
	schemas::RunResponse runResponse;

	static const std::string serviceUrl = "task";

#if 0
    std::map<std::string, std::string> requestHeaders;
    requestHeaders["Accept"] =
    		esl::utility::MIME::toString(esl::utility::MIME::Type::applicationJson) + "," +
			esl::utility::MIME::toString(esl::utility::MIME::Type::applicationXml);

    esl::com::http::client::Request request(serviceUrl, esl::utility::HttpMethod::Type::httpPost, esl::utility::MIME::Type::applicationJson, std::move(requestHeaders));
#else
    esl::com::http::client::Request request(serviceUrl, esl::utility::HttpMethod::Type::httpPost, esl::utility::MIME::Type::applicationJson);
    request.addHeader("Accept", esl::utility::MIME::toString(esl::utility::MIME::Type::applicationJson) + "," + esl::utility::MIME::toString(esl::utility::MIME::Type::applicationXml));
#endif

    sergut::JsonSerializer serializer;
    serializer.serializeData(runRequest);
    esl::io::Output output(esl::io::output::String::create(serializer.str()));

	esl::io::input::String consumerString;
	esl::io::Input input(static_cast<esl::io::Writer&>(consumerString));

	esl::com::http::client::Response response = connection.send(std::move(request), std::move(output), std::move(input));

    if(response.getStatusCode() == 200) {
        if(response.getContentType() == esl::utility::MIME::Type::applicationJson) {
        	if(!consumerString.getString().empty()) {
                sergut::JsonDeserializer deSerializer(consumerString.getString());
                runResponse = deSerializer.deserializeData<schemas::RunResponse>();
        	}
        }
        else if(response.getContentType() == esl::utility::MIME::Type::applicationXml) {
        	if(!consumerString.getString().empty()) {
                sergut::XmlDeserializer deSerializer(consumerString.getString());
                runResponse = deSerializer.deserializeData<schemas::RunResponse>("response");
        	}
        }
        else {
        	throw esl::system::Stacktrace::add(std::runtime_error("Received not supported response content type \"" + response.getContentType().toString() + "\""));
        }
    }
    else if(response.getStatusCode() == 204) {
    }
    else {
    	throw esl::system::Stacktrace::add(std::runtime_error("Received not supported status code \"" + std::to_string(response.getStatusCode()) + "\"\ncontent type: \"" + response.getContentType().toString() + "\"\ncontent: \"" + consumerString.getString() + "\""));
    }

    return runResponse;
}

void Service::sendSignal(const std::string& taskId, const std::string& signal) {
    static const std::string serviceUrl = "signal/" + taskId + "/" + signal;

    esl::com::http::client::Request request(serviceUrl, esl::utility::HttpMethod::Type::httpPost, esl::utility::MIME::Type::applicationJson);

	esl::com::http::client::Response response = connection.send(std::move(request), esl::io::Output(), esl::io::Input());

    if(response.getStatusCode() < 200 || response.getStatusCode() > 299) {
        std::string message = "Received wrong status code  \"" + std::to_string(response.getStatusCode()) + "\" from service \"" + serviceUrl + "\"";
		throw esl::system::Stacktrace::add(std::runtime_error(message));
    }
}

} /* namespace client */
} /* namespace service */
} /* namespace batchelor */
