#include <batchelor/condition/Foo.h>

namespace batchelor {
namespace condition {

void CObject::InsertObjectList(std::vector<CObject>& aObjectList) {
	for(auto & object : aObjectList) {
		objectList->emplace_back(std::move(object));
	}
}

} /* namespace condition */
} /* namespace batchelor */
