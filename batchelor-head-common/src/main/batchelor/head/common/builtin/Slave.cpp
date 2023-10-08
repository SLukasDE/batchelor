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

#include <batchelor/head/common/builtin/Slave.h>

namespace batchelor {
namespace head {
namespace common {
namespace builtin {

std::unique_ptr<esl::object::Object> Slave::create(const std::vector<std::pair<std::string, std::string>>& aSettings) {
	Settings settings;
	return std::unique_ptr<esl::object::Object>(new Slave(std::move(settings)));
}

Slave::Slave(Settings&& aSettings)
: settings(std::move(aSettings))
{ }

void Slave::onUpdateTask(const Dao::Task& task) {

}

void Slave::timerEvent() {

}

void Slave::initializeContext(esl::object::Context& context) {
	initializedSettings = std::unique_ptr<InitializedSettings>(new  InitializedSettings(context, settings));

}

Slave::InitializedSettings::InitializedSettings(esl::object::Context& context, const Settings& settings) {

};

} /* namespace builtin */
} /* namespace common */
} /* namespace head */
} /* namespace batchelor */
