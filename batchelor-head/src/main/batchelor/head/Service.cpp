#include <batchelor/head/Service.h>
#include <batchelor/head/Logger.h>

#include <batchelor/common/types/State.h>

#include <esl/system/Stacktrace.h>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <chrono>
#include <ctime>
#include <stdexcept>
#include <time.h>

namespace batchelor {
namespace head {
namespace {
Logger logger("batchelor::head::Service");

std::string getJSONTimestamp(const std::chrono::time_point<std::chrono::system_clock>& time_point) {
	auto millisecs = std::chrono::duration_cast<std::chrono::milliseconds>(time_point.time_since_epoch());
	std::time_t timestamp = millisecs.count() / 1000;
	int millisec = static_cast<int>(millisecs.count() % 1000);

	std::tm tm;
	localtime_r(&timestamp, &tm);

	char buffer[100];
	char* bufferPtr = &buffer[0];
	std::size_t length;

	length = strftime(bufferPtr, 100, "%FT%T", &tm);
	if(length == 0) {
		return "";
	}
	bufferPtr += length;

	sprintf(bufferPtr, ".%03d", millisec);
	bufferPtr += 4;

	length = strftime(bufferPtr, 100-length-4, "%z", &tm);
	if(length == 0) {
		return "";
	}

	bufferPtr[length+1] = 0;
	bufferPtr[length-0] = bufferPtr[length-1];
	bufferPtr[length-1] = bufferPtr[length-2];
	bufferPtr[length-2] = ':';

	return buffer;
}
}

Service::Service(const esl::object::Context& aContext, esl::database::ConnectionFactory& aDbConnectionFactory)
: context(aContext),
  dbConnectionFactory(aDbConnectionFactory)
{ }

service::schemas::FetchResponse Service::fetchJob(const service::schemas::FetchRequest& fetchRequest) {
	service::schemas::FetchResponse rv;

	return rv;
}

std::vector<service::schemas::JobStatusHead> Service::getJobs(const std::string& state) {
	std::vector<service::schemas::JobStatusHead> rv;

	return rv;
}

std::unique_ptr<service::schemas::JobStatusHead> Service::getJob(const std::string& jobId) {
	std::unique_ptr<service::schemas::JobStatusHead> rv;

	return rv;
}

void Service::sendSignal(const service::schemas::Signal& signal) {
}

service::schemas::RunResponse Service::runBatch(const service::schemas::RunRequest& runRequest) {
	boost::uuids::uuid jobIdUUID; // initialize uuid

	Dao::Job job;
	job.jobId = boost::uuids::to_string(jobIdUUID);
	job.batchId = runRequest.batchId;
	job.priority = runRequest.priority;
	job.arguments = runRequest.arguments;
	job.envVars = runRequest.envVars;
	job.settings = runRequest.settings;
#if 1
	job.createdTS = std::chrono::system_clock::now();
#else
	job.createdTS = getJSONTimestamp(std::chrono::system_clock::now());
#endif
	job.state = common::types::State::Type::queued;
	getDao().saveJob(job);

	//logger.info << "job.createdTS: " << getJSONTimestamp(job.createdTS) << "\n";

	service::schemas::RunResponse rv;

	//rv.jobId = "1234-ab3c-1234-ab3c-1234-ab3c-1234-ab3c";
	rv.jobId = boost::uuids::to_string(jobIdUUID);
	rv.message = "queued";

	return rv;
}

esl::database::Connection& Service::getDBConnection() const {
	if(!dbConnection) {
		dbConnection = dbConnectionFactory.createConnection();
	}

	if(!dbConnection) {
		throw esl::system::Stacktrace::add(std::runtime_error("no db connection available."));
	}

	return *dbConnection;
}

Dao& Service::getDao() const {
	if(!dao) {
		dao.reset(new Dao(getDBConnection()));
	}

	return *dao;
}


} /* namespace head */
} /* namespace batchelor */
