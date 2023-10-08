/*
 * This file is part of Batchelor.
 * Copyright (C) 2023 Sven Lukas
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

#include <batchelor/head/cli/Main.h>
#include <batchelor/head/cli/Logger.h>

#include "batchelor/head/common/RequestHandler.h"

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

Main::Main(const Settings& aSettings)
: settings(aSettings)
{
	std::unique_ptr<esl::database::ConnectionFactory> dbConnectionFactory = esl::plugin::Registry::get().create<esl::database::ConnectionFactory>(
			"eslx/database/SQLiteConnectionFactory", {
//					{"URI", ":memory:"}
					{"URI", "file:test?mode=memory"}
//					{"URI", "file::memory:?mode=rw"}
			});

	common::RequestHandler requestHandler(*dbConnectionFactory, {});

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
					{"threads", std::to_string(settings.threads == 0 ? 4 : settings.threads)},
					{"port", std::to_string(settings.port == 0 ? 8080 : settings.port)}
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

int Main::getReturnCode() const noexcept {
	return rc;
}

} /* namespace cli */
} /* namespace head */
} /* namespace batchelor */
