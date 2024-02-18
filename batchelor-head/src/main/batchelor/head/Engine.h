#ifndef BATCHELOR_HEAD_ENGINE_H_
#define BATCHELOR_HEAD_ENGINE_H_

#include <batchelor/head/Dao.h>

#include <esl/database/ConnectionFactory.h>

namespace batchelor {
namespace head {

class Engine {
public:
	virtual ~Engine() = default;

	virtual esl::database::ConnectionFactory& getDbConnectionFactory() const noexcept = 0;
	virtual void onUpdateTask(const Dao::Task& task) = 0;

};

} /* namespace head */
} /* namespace batchelor */

#endif /* BATCHELOR_HEAD_ENGINE_H_ */
