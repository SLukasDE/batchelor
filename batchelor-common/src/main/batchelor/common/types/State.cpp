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

#include <batchelor/common/types/State.h>

#include <esl/system/Stacktrace.h>

#include <stdexcept>

namespace batchelor {
namespace common {
namespace types {

static const std::string stateStrQueued = "queued";
static const std::string stateStrRunning = "running";
static const std::string stateStrZombie = "zombie";
static const std::string stateStrDone = "done";
static const std::string stateStrSignaled = "signaled";

const std::string& State::toString(const Type& state) {
	switch(state) {
	case State::Type::queued:
		return stateStrQueued;

	case State::Type::running:
		return stateStrRunning;

	case State::Type::zombie:
		return stateStrZombie;

	case State::Type::done:
		return stateStrDone;

	case State::Type::signaled:
		return stateStrSignaled;

	default:
		break;
	}
	throw esl::system::Stacktrace::add(std::runtime_error("unknown state."));
}

State::Type State::toState(const std::string& state) {
	if(state == stateStrQueued) {
		return State::Type::queued;
	}
	else if(state == stateStrRunning) {
		return State::Type::running;
	}
	else if(state == stateStrZombie) {
		return State::Type::zombie;
	}
	else if(state == stateStrDone) {
		return State::Type::done;
	}
	else if(state == stateStrSignaled) {
		return State::Type::signaled;
	}

	throw esl::system::Stacktrace::add(std::runtime_error("unknown state string."));
}

} /* namespace types */
} /* namespace common */
} /* namespace batchelor */
