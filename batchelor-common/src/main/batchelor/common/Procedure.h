/*
 * This file is part of Batchelor.
 * Copyright (C) 2023-2024 Sven Lukas
 *
 * Batchelor is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * Batchelor is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public
 * License along with Batchelor.  If not, see <https://www.gnu.org/licenses/>.
 */

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
