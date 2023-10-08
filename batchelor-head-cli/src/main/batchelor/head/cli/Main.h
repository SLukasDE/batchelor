/*
 * This file is part of Batchelor.
 * Copyright (C) 2023 Sven Lukas
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

#ifndef BATCHELOR_HEAD_CLI_MAIN_H_
#define BATCHELOR_HEAD_CLI_MAIN_H_

#include <mutex>
#include <string>

namespace batchelor {
namespace head {
namespace cli {

class Main {
public:
	struct Settings {
		bool isHttps = false;
		unsigned short port = 0;
		unsigned short threads = 0;

		std::string username;
		std::string password;
	};

	Main(const Settings& settings);

	int getReturnCode() const noexcept;

private:
	const Settings& settings;
	int rc = 0;
	std::mutex myMutex;
};

} /* namespace cli */
} /* namespace head */
} /* namespace batchelor */

#endif /* BATCHELOR_HEAD_CLI_MAIN_H_ */
