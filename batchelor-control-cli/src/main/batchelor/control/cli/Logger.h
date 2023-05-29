#ifndef BATCHELOR_CONTROL_CLI_LOGGER_H_
#define BATCHELOR_CONTROL_CLI_LOGGER_H_

#include <esl/logging/Level.h>
#include <esl/logging/Logger.h>

namespace batchelor {
namespace control {
namespace cli {

using Logger = esl::logging::Logger<esl::logging::Level::TRACE>;

} /* namespace cli */
} /* namespace control */
} /* namespace batchelor */

#endif /* BATCHELOR_CONTROL_CLI_LOGGER_H_ */
