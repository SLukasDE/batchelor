#ifndef BATCHELOR_COMMON_CONFIG_CONFIG_H_
#define BATCHELOR_COMMON_CONFIG_CONFIG_H_

#include <tinyxml2/tinyxml2.h>

#include <string>
#include <utility>

namespace batchelor {
namespace common {
namespace config {

class Config {
public:
	Config() = delete;
	Config(std::string filename);
	Config(std::string filename, std::size_t lineNo);
	virtual ~Config() = default;

	static std::string toString(tinyxml2::XMLError xmlError);
	std::string evaluate(const std::string& expression, const std::string& language) const;

	void setFilename(std::string filename);
	const std::string& getFilename() const noexcept;

	void setLineNo(std::size_t aLineNo);
	void setLineNo(const tinyxml2::XMLElement& aElement);
	std::size_t getLineNo() const noexcept;

protected:
	static bool stringToBool(bool& b, std::string str);

private:
	std::string filename;
	std::size_t lineNo = 0;
};

} /* namespace config */
} /* namespace common */
} /* namespace batchelor */

#endif /* BATCHELOR_COMMON_CONFIG_CONFIG_H_ */
