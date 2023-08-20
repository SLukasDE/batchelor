#ifndef BATCHELOR_CONTROL_COMMAND_H_
#define BATCHELOR_CONTROL_COMMAND_H_

namespace batchelor {
namespace control {

enum class Command {
	help,
	sendEvent,
	waitTask,
	cancelTask,
	signalTask,
	showTask,
	showTasks
};

} /* namespace control */
} /* namespace batchelor */

#endif /* BATCHELOR_CONTROL_COMMAND_H_ */
