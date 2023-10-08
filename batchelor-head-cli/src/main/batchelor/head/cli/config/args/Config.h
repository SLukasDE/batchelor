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

#ifndef BATCHELOR_HEAD_CLI_CONFIG_ARGS_CONFIG_H_
#define BATCHELOR_HEAD_CLI_CONFIG_ARGS_CONFIG_H_

#include <batchelor/head/cli/Main.h>

#include <string>
#include <utility>
#include <vector>

namespace batchelor {
namespace head {
namespace cli {
namespace config {
namespace args {

class Config {
public:
	Config(Main::Settings& settings, int argc, const char* argv[]);

	static void printUsage();

	const std::vector<std::string>& getConfigFiles() const noexcept;

private:
	Main::Settings& settings;

	std::vector<std::string> configFiles;

	void setPort(const char* port);
	void setMaximumThreads(const char* maximumThreads);
	void addConfigFile(const char* value);
	void setUsername(const char* value);
	void setPassword(const char* value);
};

} /* namespace args */
} /* namespace config */
} /* namespace cli */
} /* namespace head */
} /* namespace batchelor */

#endif /* BATCHELOR_HEAD_CLI_CONFIG_ARGS_CONFIG_H_ */
