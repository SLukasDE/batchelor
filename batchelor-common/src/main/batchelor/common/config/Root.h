#if 0
#ifndef BATCHELOR_COMMON_CONFIG_ROOT_H_
#define BATCHELOR_COMMON_CONFIG_ROOT_H_

#include <batchelor/common/config/Config.h>

#include <tinyxml2/tinyxml2.h>

#include <boost/filesystem/path.hpp>

namespace batchelor {
namespace common {
namespace config {

class Root : public Config {
public:
	Root(const boost::filesystem::path& filename);
};

} /* namespace config */
} /* namespace common */
} /* namespace batchelor */

#endif /* BATCHELOR_COMMON_CONFIG_ROOT_H_ */
#endif
