#ifndef BATCHELOR_HEAD_CRC32_H_
#define BATCHELOR_HEAD_CRC32_H_

#include <cstdint>
#include <string>


namespace batchelor {
namespace head {

class CRC32 {
public:
	CRC32(std::uint32_t initialValue = 0);

	CRC32& pushData(const void* data, std::size_t length);

	std::uint32_t get() const noexcept;

private:
	std::uint32_t value;
};

} /* namespace head */
} /* namespace batchelor */

#endif /* BATCHELOR_HEAD_CRC32_H_ */
