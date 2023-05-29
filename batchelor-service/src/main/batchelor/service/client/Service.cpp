#include <batchelor/service/client/Service.h>
#include <batchelor/service/Logger.h>

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

schemas::FetchResponse Service::fetchJob(const schemas::FetchRequest& fetchRequest) {
	schemas::FetchResponse fetchResponse;

	static const std::string serviceUrl = "fetchJob";
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
    else if(response.getStatusCode() == 204) {
    }
    else {
    	throw esl::system::Stacktrace::add(std::runtime_error("Received not supported status code \"" + std::to_string(response.getStatusCode()) + "\""));
    }

    return fetchResponse;
}

std::vector<schemas::JobStatusHead> Service::getJobs(const std::string& state) {
	std::vector<schemas::JobStatusHead> jobs;

	std::string serviceUrl = "jobs";
	if(!state.empty()) {
		serviceUrl += "?state=" + state;
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
                jobs = deSerializer.deserializeData<std::vector<schemas::JobStatusHead>>();
        	}
        }
        else if(response.getContentType() == esl::utility::MIME::Type::applicationXml) {
        	if(!inputWriterString.getString().empty()) {
                sergut::XmlDeserializer deSerializer(inputWriterString.getString());
                jobs = deSerializer.deserializeNestedData<std::vector<schemas::JobStatusHead>>("jobs", "job");
        	}
        }
        else {
        	throw esl::system::Stacktrace::add(std::runtime_error("Received not supported response content type \"" + response.getContentType().toString() + "\""));
        }
    }
    else if(response.getStatusCode() == 204) {
    }
    else {
    	throw esl::system::Stacktrace::add(std::runtime_error("Received not supported status code \"" + std::to_string(response.getStatusCode()) + "\""));
    }

    return jobs;
}

std::unique_ptr<schemas::JobStatusHead> Service::getJob(const std::string& jobId) {
	std::unique_ptr<schemas::JobStatusHead> status;

	std::string serviceUrl = "job/" + jobId;
    esl::com::http::client::Request request(serviceUrl, esl::utility::HttpMethod::Type::httpGet, esl::utility::MIME::Type::applicationJson);
    request.addHeader("Accept", esl::utility::MIME::toString(esl::utility::MIME::Type::applicationJson) + "," + esl::utility::MIME::toString(esl::utility::MIME::Type::applicationXml));

	esl::io::input::String consumerString;
	esl::io::Input input(static_cast<esl::io::Writer&>(consumerString));

	esl::com::http::client::Response response = connection.send(std::move(request), esl::io::Output(), std::move(input));

    if(response.getStatusCode() == 200) {
        if(response.getContentType() == esl::utility::MIME::Type::applicationJson) {
        	if(!consumerString.getString().empty()) {
        		status.reset(new schemas::JobStatusHead);
                sergut::JsonDeserializer deSerializer(consumerString.getString());
                *status = deSerializer.deserializeData<schemas::JobStatusHead>();
        	}
        }
        else if(response.getContentType() == esl::utility::MIME::Type::applicationXml) {
        	if(!consumerString.getString().empty()) {
        		status.reset(new schemas::JobStatusHead);
                sergut::XmlDeserializer deSerializer(consumerString.getString());
                *status = deSerializer.deserializeData<schemas::JobStatusHead>("status");
        	}
        }
        else {
        	throw esl::system::Stacktrace::add(std::runtime_error("Received not supported response content type \"" + response.getContentType().toString() + "\""));
        }
    }
    else if(response.getStatusCode() == 204) {
    }
    else {
    	throw esl::system::Stacktrace::add(std::runtime_error("Received not supported status code \"" + std::to_string(response.getStatusCode()) + "\""));
    }

    return status;
}

void Service::sendSignal(const schemas::Signal& signal) {
    static const std::string serviceUrl = "signal";

    std::map<std::string, std::string> requestHeaders;
    requestHeaders["Accept"] =
    		esl::utility::MIME::toString(esl::utility::MIME::Type::applicationJson) + "," +
			esl::utility::MIME::toString(esl::utility::MIME::Type::applicationXml);

    sergut::JsonSerializer serializer;
    serializer.serializeData(signal);
    esl::io::Output output(esl::io::output::String::create(serializer.str()));

    esl::com::http::client::Request request(serviceUrl, esl::utility::HttpMethod::Type::httpPost, esl::utility::MIME::Type::applicationJson, std::move(requestHeaders));

	esl::com::http::client::Response response = connection.send(std::move(request), std::move(output), esl::io::Input());

    if(response.getStatusCode() < 200 || response.getStatusCode() > 299) {
        std::string message = "Received wrong status code  \"" + std::to_string(response.getStatusCode()) + "\" from service \"" + serviceUrl + "\"";
		throw esl::system::Stacktrace::add(std::runtime_error(message));
    }
}

schemas::RunResponse Service::runBatch(const schemas::RunRequest& runRequest) {
	schemas::RunResponse runResponse;

	static const std::string serviceUrl = "runBatch";

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
    	throw esl::system::Stacktrace::add(std::runtime_error("Received not supported status code \"" + std::to_string(response.getStatusCode()) + "\""));
    }

    return runResponse;
}

} /* namespace client */
} /* namespace service */
} /* namespace batchelor */
