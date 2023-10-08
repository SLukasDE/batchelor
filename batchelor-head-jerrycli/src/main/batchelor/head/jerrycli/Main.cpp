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

#include <batchelor/head/jerrycli/Logger.h>
#include <batchelor/head/jerrycli/Main.h>

#include <batchelor/head/jerryplugin/RequestHandler.h>

#include <jerry4esl/engine/http/Server.h>

#include <eslx/object/JerryProcessingContext.h>

#include <memory>

namespace batchelor {
namespace head {
namespace jerrycli {
namespace {
Logger logger("batchelor::head::jerrycli::Main");
}

Main::Main(const Settings& aSettings)
: settings(aSettings)
  //signal(new zsystem4esl::system::signal::Signal({}))
{

	/* ********************** *
	 * use own signal handler *
	 * ********************** */
/*
	std::set<std::string> stopSignals { {"interrupt"}, {"terminate"}, {"pipe"}};
	for(auto stopSignal : stopSignals) {
		signalHandles.push_back(signal->createHandler(esl::utility::Signal(stopSignal), [this]() {
			stopRunning();
		}));
	}
*/
	eslx::object::JerryProcessingContext::Settings jpcSettings;

	//jpcSettings.setSignalHandler(true);
	jpcSettings.addStopSignal(esl::utility::Signal("interrupt"));
	jpcSettings.addStopSignal(esl::utility::Signal("terminate"));
	jpcSettings.addStopSignal(esl::utility::Signal("pipe"));
	jpcSettings.setTerminateCounter(2);
	jpcSettings.setVerbose(true);

	eslx::object::JerryProcessingContext context(std::move(jpcSettings));

	context.addData(
R"BATCHELOR-DATA(
<context>
<database id="my-db" implementation="eslx/database/SQLiteConnectionFactory">
<!-- parameter key="URI" value="file:my.db?mode=rw"/ -->
<parameter key="URI" value="file:test?mode=memory"/>
</database>
</context>
)BATCHELOR-DATA");

	{
		std::unique_ptr<jerry4esl::engine::http::Server> server(new jerry4esl::engine::http::Server("", {
				{{"threads"}, {std::to_string(settings.threads == 0 ? 4 : settings.threads)}},
				{{"port"}, {std::to_string(settings.port == 0 ? 8080 : settings.port)}}
		}, settings.isHttps));
		server->add(std::move(batchelor::head::jerryplugin::RequestHandler::create({{{"db-connection-factory"},{"my-db"}}})));
		server->setParentObjectContext(&context);
		context.add(std::move(server));
	}

	/* ************************* *
	 * initialize global objects *
	 * ************************* */
	logger.info << "Initialize objects ...\n";
	context.initialize();
	logger.info << "Initialization done.\n";

	logger.debug << "Start all processes...\n";
	rc = context.main();
}

int Main::getReturnCode() const noexcept {
	return rc;
}

} /* namespace jerrycli */
} /* namespace head */
} /* namespace batchelor */
