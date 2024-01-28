#include <batchelor/condition/CObject.h>

namespace batchelor {
namespace condition {

CObject::CObject()
: objectList(new std::vector<CObject>)
{ }

void CObject::add(std::vector<CObject>& aObjectList) {
	for(auto & object : aObjectList) {
		objectList->emplace_back(std::move(object));
	}
}

} /* namespace condition */
} /* namespace batchelor */
