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

#include <batchelor/head/Main.h>

#include <iostream>
#include <string>

extern const std::string artefactVersionStr;

#define _STRINGIFY(x) #x
#define STRINGIFY(x) _STRINGIFY(x)
const std::string artefactVersionStr = STRINGIFY(TRANSFORMER_ARTEFACT_VERSION);

int main(int argc, const char* argv[]) {
	std::cout << "batchelor-head version " << artefactVersionStr << std::endl;
	//std::cout << "Compiled at " << __DATE__ << " " << __TIME__ << std::endl;

	return batchelor::head::Main(argc, argv).getReturnCode();
}
