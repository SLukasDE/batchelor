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

#ifndef BATCHELOR_COMMON_PLUGIN_CONNECTIONFACTORY_H_
#define BATCHELOR_COMMON_PLUGIN_CONNECTIONFACTORY_H_

#include <esl/com/http/client/ConnectionFactory.h>
#include <esl/object/Object.h>

namespace batchelor {
namespace common {
namespace plugin {

class ConnectionFactory : public esl::object::Object {
public:
	virtual esl::com::http::client::ConnectionFactory& get() = 0;
};

} /* namespace plugin */
} /* namespace common */
} /* namespace batchelor */

#endif /* BATCHELOR_COMMON_PLUGIN_CONNECTIONFACTORY_H_ */
