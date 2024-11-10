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

#ifndef BATCHELOR_COMMON_AUTH_USERDATA_H_
#define BATCHELOR_COMMON_AUTH_USERDATA_H_

//#include <batchelor/common/auth/Role.h>

#include <esl/object/Context.h>

#include <map>
#include <set>
#include <string>

namespace batchelor {
namespace common {
namespace auth {

struct UserData {
	enum class Role {
		execute,
		readOnly,
		worker
	};

	std::map<std::string, std::set<Role>> rolesByNamespace;

	static std::set<Role> getRoles(const esl::object::Context& context, const std::string& namespaceId);
	static Role toRole(const std::string& roleStr);
	static const std::string& fromRole(Role role);
};

} /* namespace auth */
} /* namespace common */
} /* namespace batchelor */

#endif /* BATCHELOR_COMMON_AUTH_USERDATA_H_ */
