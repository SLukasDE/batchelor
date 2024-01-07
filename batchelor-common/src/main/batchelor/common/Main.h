#ifndef BATCHELOR_COMMON_MAIN_H_
#define BATCHELOR_COMMON_MAIN_H_

namespace batchelor {
namespace common {

struct Main {
	virtual ~Main();

	int getReturnCode() const noexcept;

protected:
	Main(int(*run)(int, const char**), void(*printUsage)(), int argc, const char* argv[]);

private:
	int rc = -1;
};

} /* namespace common */
} /* namespace batchelor */

#endif /* BATCHELOR_COMMON_MAIN_H_ */
