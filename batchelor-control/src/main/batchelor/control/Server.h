#ifndef BATCHELOR_CONTROL_SERVER_H_
#define BATCHELOR_CONTROL_SERVER_H_

#include <string>

namespace batchelor {
namespace control {

struct Server {
	std::string url;
	std::string username;
	std::string password;
};

} /* namespace control */
} /* namespace batchelor */

#endif /* BATCHELOR_CONTROL_SERVER_H_ */
