#ifndef BATCHELOR_CONTROL_LOGGER_H_
#define BATCHELOR_CONTROL_LOGGER_H_

#include <esl/logging/Level.h>
#include <esl/logging/Logger.h>

namespace batchelor {
namespace control {

using Logger = esl::logging::Logger<esl::logging::Level::TRACE>;

} /* namespace control */
} /* namespace batchelor */

#endif /* BATCHELOR_CONTROL_LOGGER_H_ */
