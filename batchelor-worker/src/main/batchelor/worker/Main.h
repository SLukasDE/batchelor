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

#ifndef BATCHELOR_WORKER_MAIN_H_
#define BATCHELOR_WORKER_MAIN_H_

#include <batchelor/common/Main.h>

namespace batchelor {
namespace worker {

class Main : public common::Main {
public:
	Main(int argc, const char* argv[]);

private:
	static int run(int argc, const char* argv[]);
};

} /* namespace worker */
} /* namespace batchelor */

#endif /* BATCHELOR_WORKER_MAIN_H_ */
