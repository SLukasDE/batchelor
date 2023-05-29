#ifndef BATCHELOR_WORKER_CLI_LOGGER_H_
#define BATCHELOR_WORKER_CLI_LOGGER_H_

#include <esl/logging/Level.h>
#include <esl/logging/Logger.h>

namespace batchelor {
namespace worker {
namespace cli {

using Logger = esl::logging::Logger<esl::logging::Level::TRACE>;

} /* namespace cli */
} /* namespace worker */
} /* namespace batchelor */

#endif /* BATCHELOR_WORKER_CLI_LOGGER_H_ */
