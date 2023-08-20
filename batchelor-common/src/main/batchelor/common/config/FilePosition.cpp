#include <batchelor/common/config/Config.h>
#include <batchelor/common/config/FilePosition.h>

#include <stdexcept>

namespace batchelor {
namespace common {
namespace config {

esl::io::FilePosition::Injector<std::runtime_error> FilePosition::add(const Config& config, const std::string& what) {
	return esl::io::FilePosition::add<std::runtime_error>(config.getFilename(), config.getLineNo(), std::runtime_error(what));
}

esl::io::FilePosition::Injector<std::runtime_error> FilePosition::add(const Config& config, const char* what) {
	return esl::io::FilePosition::add<std::runtime_error>(config.getFilename(), config.getLineNo(), std::runtime_error(what));
}

esl::io::FilePosition::Injector<std::runtime_error> FilePosition::add(const Config& config, tinyxml2::XMLError xmlError) {
	return esl::io::FilePosition::add<std::runtime_error>(config.getFilename(), config.getLineNo(), std::runtime_error(Config::toString(xmlError)));
}

} /* namespace config */
} /* namespace common */
} /* namespace batchelor */
