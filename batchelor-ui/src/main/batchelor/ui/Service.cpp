#include <batchelor/ui/Service.h>

#include <batchelor/service/client/Service.h>

namespace batchelor {
namespace ui {

Service::Service(const RequestHandler& aRequestHandler)
: requestHandler(aRequestHandler)
{ }

void Service::alive() {
	auto httpConnection = requestHandler.createHTTPConnection();
	return service::client::Service(*httpConnection).alive();
}

service::schemas::FetchResponse Service::fetchTask(const std::string& namespaceId, const service::schemas::FetchRequest& fetchRequest) {
	auto httpConnection = requestHandler.createHTTPConnection();
	return service::client::Service(*httpConnection).fetchTask(namespaceId, fetchRequest);
}

std::vector<service::schemas::TaskStatusHead> Service::getTasks(const std::string& namespaceId, const std::string& state, const std::string& eventNotAfter, const std::string& eventNotBefore) {
	auto httpConnection = requestHandler.createHTTPConnection();
	return service::client::Service(*httpConnection).getTasks(namespaceId, state, eventNotAfter, eventNotBefore);
}

std::unique_ptr<service::schemas::TaskStatusHead> Service::getTask(const std::string& namespaceId, const std::string& taskId) {
	auto httpConnection = requestHandler.createHTTPConnection();
	return service::client::Service(*httpConnection).getTask(namespaceId, taskId);
}

service::schemas::RunResponse Service::runTask(const std::string& namespaceId, const service::schemas::RunRequest& runRequest) {
	auto httpConnection = requestHandler.createHTTPConnection();
	return service::client::Service(*httpConnection).runTask(namespaceId, runRequest);
}

void Service::sendSignal(const std::string& namespaceId, const std::string& taskId, const std::string& signal) {
	auto httpConnection = requestHandler.createHTTPConnection();
	return service::client::Service(*httpConnection).sendSignal(namespaceId, taskId, signal);
}

std::vector<std::string> Service::getEventTypes(const std::string& namespaceId) {
	auto httpConnection = requestHandler.createHTTPConnection();
	return service::client::Service(*httpConnection).getEventTypes(namespaceId);
}

} /* namespace ui */
} /* namespace batchelor */
