#ifndef BATCHELOR_SERVICE_LOGGER_H_
#define BATCHELOR_SERVICE_LOGGER_H_

#include <esl/logging/Logger.h>
#include <esl/logging/Level.h>

namespace batchelor {
namespace service {

using Logger = esl::logging::Logger<esl::logging::Level::TRACE>;

} /* namespace service */
} /* namespace batchelor */

#endif /* BATCHELOR_SERVICE_LOGGER_H_ */
