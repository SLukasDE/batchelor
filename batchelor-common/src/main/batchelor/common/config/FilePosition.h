#ifndef BATCHELOR_COMMON_CONFIG_FILEPOSITION_H_
#define BATCHELOR_COMMON_CONFIG_FILEPOSITION_H_

#include <batchelor/common/config/Config.h>

#include <tinyxml2/tinyxml2.h>

#include <esl/io/FilePosition.h>

#include <string>

namespace batchelor {
namespace common {
namespace config {

class FilePosition final {
public:
	FilePosition() = delete;

	template <class E>
	static esl::io::FilePosition::Injector<E> add(const Config& config, const E& e);

	static esl::io::FilePosition::Injector<std::runtime_error> add(const Config& config, const std::string& what);
	static esl::io::FilePosition::Injector<std::runtime_error> add(const Config& config, const char* what);
	static esl::io::FilePosition::Injector<std::runtime_error> add(const Config& config, tinyxml2::XMLError xmlError);
};

template <class E>
esl::io::FilePosition::Injector<E> FilePosition::add(const Config& config, const E& e) {
	return esl::io::FilePosition::add<E>(config.getFilename(), config.getLineNo(), e);
}

} /* namespace config */
} /* namespace common */
} /* namespace batchelor */

#endif /* BATCHELOR_COMMON_CONFIG_FILEPOSITION_H_ */
