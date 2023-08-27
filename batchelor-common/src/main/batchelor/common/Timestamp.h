#ifndef BATCHELOR_COMMON_TIMESTAMP_H_
#define BATCHELOR_COMMON_TIMESTAMP_H_

#include <chrono>
#include <string>

namespace batchelor {
namespace common {

class Timestamp final {
public:
	Timestamp() = delete;

	static std::string toJSON(const std::chrono::time_point<std::chrono::system_clock>& time_point);
	static std::chrono::time_point<std::chrono::system_clock> fromJSON(const std::string& str);

	static std::string toString(const std::chrono::time_point<std::chrono::system_clock>& time_point);
	static std::chrono::time_point<std::chrono::system_clock> fromString(const std::string& str);
};

} /* namespace common */
} /* namespace batchelor */

#endif /* BATCHELOR_COMMON_TIMESTAMP_H_ */
