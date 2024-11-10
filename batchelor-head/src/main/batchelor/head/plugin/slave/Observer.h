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

#ifndef BATCHELOR_HEAD_PLUGIN_SLAVE_OBSERVER_H_
#define BATCHELOR_HEAD_PLUGIN_SLAVE_OBSERVER_H_

#include <esl/object/Context.h>
#include <esl/object/InitializeContext.h>
#include <esl/object/Object.h>

#include <batchelor/head/Dao.h>
#include <batchelor/head/plugin/Observer.h>

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace batchelor {
namespace head {
namespace plugin {
namespace slave {

class Observer : public plugin::Observer, public esl::object::InitializeContext {
public:
	struct Settings {
		Settings(const std::vector<std::pair<std::string, std::string>>& settings);

		std::string dbConnectionFactoryId;
	};

	static std::unique_ptr<esl::object::Object> create(const std::vector<std::pair<std::string, std::string>>& settings);

	Observer(const Settings& settings);

	void onUpdateTask(const Dao::Task& task) override;
	void timerEvent() override;

	void initializeContext(esl::object::Context& context) override;

private:
	struct InitializedSettings {
		InitializedSettings(esl::object::Context& context, const Settings& settings);
	};

	const Settings settings;
	std::unique_ptr<InitializedSettings> initializedSettings;
};

} /* namespace slave */
} /* namespace plugin */
} /* namespace head */
} /* namespace batchelor */

#endif /* BATCHELOR_HEAD_PLUGIN_SLAVE_OBSERVER_H_ */
