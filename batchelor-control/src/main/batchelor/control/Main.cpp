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

#include <batchelor/control/config/args/Config.h>
#include <batchelor/control/config/xml/Config.h>
#include <batchelor/control/Main.h>
#include <batchelor/control/Plugin.h>
#include <batchelor/control/Procedure.h>

#include <esl/object/SimpleContext.h>

namespace batchelor {
namespace control {

Main::Main(int argc, const char* argv[])
: common::Main(run, config::args::Config::printUsage, argc, argv)
{ }

int Main::run(int argc, const char* argv[]) {
	esl::plugin::Registry& registry(esl::plugin::Registry::get());
	Plugin::install(registry, nullptr);

	Procedure::Settings settings;
	esl::object::SimpleContext context;
	config::args::Config argsConfig(context, settings, argc, argv);

	/* load additional data from config files */
	for(const auto& configFile : argsConfig.getConfigFiles()) {
		config::xml::Config(settings, configFile);
	}

	if(settings.namespaceId.empty()) {
		settings.namespaceId = "default";
	}

	Procedure procedure{ settings };
	procedure.initializeContext(context);
	procedure.procedureRun(context);

	return procedure.getReturnCode();
}

} /* namespace control */
} /* namespace batchelor */
