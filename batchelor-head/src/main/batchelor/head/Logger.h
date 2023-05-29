#ifndef BATCHELOR_HEAD_LOGGER_H_
#define BATCHELOR_HEAD_LOGGER_H_

#include <esl/logging/Level.h>
#include <esl/logging/Logger.h>

namespace batchelor {
namespace head {

using Logger = esl::logging::Logger<esl::logging::Level::TRACE>;

} /* namespace head */
} /* namespace batchelor */

#endif /* BATCHELOR_HEAD_LOGGER_H_ */
