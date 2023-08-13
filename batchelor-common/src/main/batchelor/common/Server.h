#ifndef BATCHELOR_COMMON_SERVER_H_
#define BATCHELOR_COMMON_SERVER_H_

#include <string>

namespace batchelor {
namespace common {

struct Server {
	std::string url;
	std::string username;
	std::string password;
};

} /* namespace common */
} /* namespace batchelor */

#endif /* BATCHELOR_COMMON_SERVER_H_ */
