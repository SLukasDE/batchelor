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

#include <batchelor/common/Logger.h>
#include <batchelor/common/Procedure.h>

#include <esl/plugin/Registry.h>
#include <esl/system/Signal.h>
#include <esl/system/SignalManager.h>

#include <set>
#include <string>
#include <vector>

namespace batchelor {
namespace common {
namespace {
Logger logger("batchelor::common::Procedure");
}

void Procedure::procedureRun(esl::object::Context& context) {
	std::vector<esl::system::SignalManager::Handler> signalHandles;
	esl::system::SignalManager* signalManager = esl::plugin::Registry::get().findObject<esl::system::SignalManager>();

	if(signalManager) {
		/* ********************** *
		 * use own signal handler *
		 * ********************** */

		std::set<std::string> stopSignals { {"interrupt"}, {"terminate"}, {"pipe"}};
		for(auto stopSignal : stopSignals) {
			signalHandles.push_back(signalManager->createHandler(esl::system::Signal(stopSignal), [&]() {
				procedureCancel();
			}));
		}
	}
	else {
		logger.warn << "Signal manager NOT found. Cannot add handler for signals\n";
	}

	internalProcedureRun(context);
}

} /* namespace common */
} /* namespace batchelor */
