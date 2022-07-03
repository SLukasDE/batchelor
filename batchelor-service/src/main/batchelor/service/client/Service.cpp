#include <batchelor/service/client/Service.h>
#include <batchelor/service/Logger.h>

#include <esl/com/http/client/Request.h>
#include <esl/com/http/client/Response.h>
#include <esl/io/Input.h>
#include <esl/io/input/String.h>
#include <esl/io/Output.h>
#include <esl/io/output/String.h>
#include <esl/utility/HttpMethod.h>
#include <esl/utility/MIME.h>
#include <esl/system/stacktrace/IStacktrace.h>

#include <map>
#include <stdexcept>
#include <string>
#if 0
#include "sergut/JsonDeserializer.h"
#include "sergut/XmlDeserializer.h"
#include "sergut/JsonSerializer.h"
#include "sergut/XmlSerializer.h"
#endif

namespace batchelor {
namespace service {
namespace client {

namespace {
Logger logger("batchelor::service::client::Service");
}

Service::Service(const esl::com::http::client::Connection& aConnection)
: connection(aConnection)
{ }

void Service::setStatusCancelByWorkerId(const std::string& workerId) {
}

std::unique_ptr<schemas::Job> Service::fetchJob(const std::string& workerId, const schemas::Procedures& ids) {
	return nullptr;
}

std::unique_ptr<schemas::Status> Service::getStatus(const std::string& jobId) {
	return nullptr;
}

void Service::setStatus(const std::string& jobId, const std::vector<schemas::Status>& jobs) {
}

} /* namespace client */
} /* namespace service */
} /* namespace batchelor */
