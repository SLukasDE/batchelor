#ifndef BATCHELOR_COMMON_PLUGIN_WRAPPER_CONNECTIONFACTORY_H_
#define BATCHELOR_COMMON_PLUGIN_WRAPPER_CONNECTIONFACTORY_H_

#include <batchelor/common/plugin/ConnectionFactory.h>

#include <esl/com/http/client/ConnectionFactory.h>
#include <esl/object/InitializeContext.h>
#include <esl/object/Context.h>

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace batchelor {
namespace common {
namespace plugin {
namespace wrapper {

class ConnectionFactory : public plugin::ConnectionFactory, esl::object::InitializeContext {
public:
	static std::unique_ptr<plugin::ConnectionFactory> create(const std::vector<std::pair<std::string, std::string>>& settings);

	struct Settings {
		Settings(const std::vector<std::pair<std::string, std::string>>& settings);

		std::string connectionFactoryId;
	};

	ConnectionFactory(const Settings& settings);

	esl::com::http::client::ConnectionFactory& get() override;

	void initializeContext(esl::object::Context& context) override;

private:
	struct InitializedSettings {
		InitializedSettings(esl::object::Context& context, const Settings& settings);

		esl::com::http::client::ConnectionFactory& connectionFactory;
	};

	const Settings& settings;
	std::unique_ptr<InitializedSettings> initializedSettings;
};

} /* namespace wrapper */
} /* namespace plugin */
} /* namespace common */
} /* namespace batchelor */

#endif /* BATCHELOR_COMMON_PLUGIN_WRAPPER_CONNECTIONFACTORY_H_ */
