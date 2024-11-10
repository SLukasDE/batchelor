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

#ifndef BATCHELOR_HEAD_PLUGIN_OBSERVER_H_
#define BATCHELOR_HEAD_PLUGIN_OBSERVER_H_

#include <batchelor/head/Dao.h>

#include <esl/object/Object.h>

namespace batchelor {
namespace head {
namespace plugin {

class Observer : public virtual esl::object::Object {
public:
	virtual void onUpdateTask(const Dao::Task& task) = 0;
	virtual void timerEvent() = 0;
};

} /* namespace plugin */
} /* namespace head */
} /* namespace batchelor */

#endif /* BATCHELOR_HEAD_PLUGIN_OBSERVER_H_ */
