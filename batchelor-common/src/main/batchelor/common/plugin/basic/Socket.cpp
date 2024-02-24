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

#include <batchelor/common/plugin/basic/Socket.h>

#include <esl/com/http/server/MHDSocket.h>

namespace batchelor {
namespace common {
namespace plugin {
namespace basic {

std::unique_ptr<plugin::Socket> Socket::create(const std::vector<std::pair<std::string, std::string>>& settings) {
	return std::unique_ptr<plugin::Socket>(new Socket(settings));
}

Socket::Socket(const std::vector<std::pair<std::string, std::string>>& settings)
: socket(esl::com::http::server::MHDSocket::createNative(esl::com::http::server::MHDSocket::Settings(settings)))
{ }

esl::com::http::server::Socket& Socket::get() {
	return *socket;
}

} /* namespace basic */
} /* namespace plugin */
} /* namespace common */
} /* namespace batchelor */
