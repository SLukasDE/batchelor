#include "batchelor/head/cli/Main.h"
#include "batchelor/head/cli/Logger.h"

#include "batchelor/head/RequestHandler.h"

#include <esl/com/http/server/Socket.h>
#include <esl/database/ConnectionFactory.h>
#include <esl/plugin/Registry.h>
#include <esl/system/Signal.h>
#include <esl/utility/Signal.h>

#include <memory>
#include <vector>

namespace batchelor {
namespace head {
namespace cli {
namespace {
Logger logger("batchelor::head::cli::Main");
}

Main::Main() {
	std::unique_ptr<esl::database::ConnectionFactory> dbConnectionFactory = esl::plugin::Registry::get().create<esl::database::ConnectionFactory>(
			"eslx/database/SQLiteConnectionFactory", {
//					{"URI", ":memory:"}
					{"URI", "file:test?mode=memory"}
//					{"URI", "file::memory:?mode=rw"}
			});

	RequestHandler requestHandler(*dbConnectionFactory);

	myMutex.lock();

	std::unique_ptr<esl::system::Signal> signal;
	signal = esl::plugin::Registry::get().create<esl::system::Signal>("eslx/system/Signal", {});

	std::vector<esl::system::Signal::Handler> signalHandles;
	signalHandles.push_back(signal->createHandler(esl::utility::Signal("terminate"), [this]() {
		myMutex.unlock();
	}));
	signalHandles.push_back(signal->createHandler(esl::utility::Signal("interrupt"), [this]() {
		myMutex.unlock();
	}));
	signalHandles.push_back(signal->createHandler(esl::utility::Signal("pipe"), [this]() {
		myMutex.unlock();
	}));

	std::unique_ptr<esl::com::http::server::Socket> socket = esl::plugin::Registry::get().create<esl::com::http::server::Socket>(
			"eslx/com/http/server/Socket", {
					{"port", "8080"}
			});
	logger.info << "Listen...\n";
	socket->listen(requestHandler, []{
			logger.info << "Socket released.\n";
	});
	logger.info << "Listen done\n";

	myMutex.lock();
	//std::this_thread::sleep_for(std::chrono::milliseconds(200));
	//std::this_thread::sleep_for(std::chrono::seconds(10));

	logger.info << "Release...\n";
	socket->release();
	logger.info << "Release done\n";

	myMutex.unlock();
}

} /* namespace cli */
} /* namespace head */
} /* namespace batchelor */
