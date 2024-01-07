#ifndef BATCHELOR_COMMON_PROCEDURE_H_
#define BATCHELOR_COMMON_PROCEDURE_H_

#include <esl/object/Context.h>
#include <esl/object/InitializeContext.h>
#include <esl/object/Procedure.h>

namespace batchelor {
namespace common {

class Procedure : public esl::object::Procedure, esl::object::InitializeContext {
public:
	void procedureRun(esl::object::Context& context) override final;

protected:
	virtual void internalProcedureRun(esl::object::Context& context) = 0;

};

} /* namespace common */
} /* namespace batchelor */

#endif /* BATCHELOR_COMMON_PROCEDURE_H_ */
