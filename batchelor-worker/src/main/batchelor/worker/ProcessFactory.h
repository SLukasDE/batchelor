#ifndef BATCHELOR_WORKER_PROCESSFACTORY_H_
#define BATCHELOR_WORKER_PROCESSFACTORY_H_

#include <batchelor/worker/Process.h>

#include <esl/object/Object.h>

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace batchelor {
namespace worker {

class Procedure;

class ProcessFactory : public esl::object::Object {
public:
	virtual bool isBusy(Procedure& procedure) = 0;
	virtual std::unique_ptr<Process> createProcess(Procedure& procedure, const std::vector<std::pair<std::string, std::string>>& settings) = 0;
};

} /* namespace worker */
} /* namespace batchelor */

#endif /* BATCHELOR_WORKER_PROCESSFACTORY_H_ */
