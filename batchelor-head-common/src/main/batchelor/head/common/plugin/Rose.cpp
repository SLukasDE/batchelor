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

#include <batchelor/head/common/plugin/Rose.h>

namespace batchelor {
namespace head {
namespace common {
namespace plugin {

std::unique_ptr<esl::object::Object> Rose::create(const std::vector<std::pair<std::string, std::string>>& aSettings) {
	Settings settings;
	return std::unique_ptr<esl::object::Object>(new Rose(std::move(settings)));
}

Rose::Rose(Settings&& aSettings)
: settings(std::move(aSettings))
{ }

void Rose::onUpdateTask(const Dao::Task& task) {

}

void Rose::timerEvent() {

}

void Rose::initializeContext(esl::object::Context& context) {
	initializedSettings = std::unique_ptr<InitializedSettings>(new  InitializedSettings(context, settings));

}

Rose::InitializedSettings::InitializedSettings(esl::object::Context& context, const Settings& settings) {

};

} /* namespace plugin */
} /* namespace common */
} /* namespace head */
} /* namespace batchelor */
