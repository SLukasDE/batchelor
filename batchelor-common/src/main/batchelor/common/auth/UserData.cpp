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

#include <batchelor/common/auth/UserData.h>
#include <batchelor/common/config/args/ArgumentsException.h>

#include <esl/object/Value.h>
#include <esl/utility/String.h>

#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace batchelor {
namespace common {
namespace auth {

namespace {
const std::string strRoleExecute("execute");
const std::string strRoleReadOnly("read-only");
const std::string strRoleWorker("worker");
}

std::set<UserData::Role> UserData::getRoles(const esl::object::Context& context, const std::string& namespaceId) {
	std::set<Role> roles;

	auto authDataPtr = context.findObject<esl::object::Value<std::vector<std::pair<std::string, std::string>>>>("auth-data");
	if(authDataPtr) {
		const std::vector<std::pair<std::string, std::string>>& authData = authDataPtr->get();
		for(const auto& keyValue : authData) {
			if(keyValue.first != "batchelor") {
				continue;
			}
			auto namespaceRole = esl::utility::String::split(keyValue.second, ':', false);
			if(namespaceRole.size() != 2) {
				throw std::runtime_error("Wrong auth-data format for key 'batchelor': \"" + keyValue.second + "\"");
			}

			if(namespaceRole[0].empty() == false && namespaceRole[0] != "*" && namespaceRole[0] != namespaceId) {
				continue;
			}

			if(namespaceRole[1] == "execute") {
				roles.insert(Role::execute);
			}
			else if(namespaceRole[1] == "read-only") {
				roles.insert(Role::readOnly);
			}
			else if(namespaceRole[1] == "worker") {
				roles.insert(Role::worker);
			}
			else {
				throw std::runtime_error("Wrong auth-data role for key 'batchelor': \"" + keyValue.second + "\"");
			}
		}
	}

	return roles;
}

UserData::Role UserData::toRole(const std::string& roleStr) {
	if(roleStr == strRoleExecute) {
		return Role::execute;
	}

	if(roleStr == strRoleReadOnly) {
		return Role::readOnly;
	}

	if(roleStr == strRoleWorker) {
		return Role::worker;
	}

	throw config::args::ArgumentsException("Invalid value \"" + roleStr + "\" for role.");
}

const std::string& UserData::fromRole(UserData::Role role) {
	switch(role) {
	case UserData::Role::execute:
		return strRoleExecute;
	case UserData::Role::readOnly:
		return strRoleReadOnly;
	case UserData::Role::worker:
		return strRoleWorker;
	}
	return strRoleWorker;
}

} /* namespace auth */
} /* namespace common */
} /* namespace batchelor */
