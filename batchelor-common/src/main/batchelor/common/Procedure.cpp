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
