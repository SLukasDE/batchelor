#include <batchelor/condition/Scanner.h>

namespace batchelor {
namespace condition {

Scanner::Scanner(std::istream& in)
: yyFlexLexer(in, std::cout)
{ };

} /* namespace condition */
} /* namespace batchelor */
