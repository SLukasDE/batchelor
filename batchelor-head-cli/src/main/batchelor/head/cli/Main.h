#ifndef BATCHELOR_HEAD_CLI_MAIN_H_
#define BATCHELOR_HEAD_CLI_MAIN_H_

#include <mutex>

namespace batchelor {
namespace head {
namespace cli {

class Main {
public:
	Main();

private:
	std::mutex myMutex;
};

} /* namespace cli */
} /* namespace head */
} /* namespace batchelor */

#endif /* BATCHELOR_HEAD_CLI_MAIN_H_ */
