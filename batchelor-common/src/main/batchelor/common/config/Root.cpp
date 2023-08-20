#if 0
#include <batchelor/common/config/FilePosition.h>
#include <batchelor/common/config/Root.h>

namespace batchelor {
namespace common {
namespace config {

Root::Root(const boost::filesystem::path& filename)
: Config(filename.generic_string())
{
	tinyxml2::XMLDocument xmlDocument;
	tinyxml2::XMLError xmlError = xmlDocument.LoadFile(filename.generic_string().c_str());
	if(xmlError != tinyxml2::XML_SUCCESS) {
		throw FilePosition::add(*this, xmlError);
	}

	const tinyxml2::XMLElement* element = xmlDocument.RootElement();
	if(element == nullptr) {
		throw FilePosition::add(*this, "No root element in file \"" + filename.generic_string() + "\"");
	}

//	setXMLFile(filename.generic_string(), *element);
//	loadXML(*element);
}

} /* namespace config */
} /* namespace common */
} /* namespace batchelor */
#endif
