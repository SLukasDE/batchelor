#ifndef BATCHELOR_CONDITION_DRIVER_H_
#define BATCHELOR_CONDITION_DRIVER_H_

#include <batchelor/condition/Scanner.h>
#include <batchelor/condition/Parser.h>

#include <memory>
#include <string>
#include <cstddef>
#include <istream>

namespace batchelor {
namespace condition {

class Driver {
public:
	enum IdentifierType {
		itUnknown, itString
	};

	Driver() = default;

   /**
    * parse - parse from a c++ input stream
    * @param is - std::istream&, valid input stream
    */
   void parse(std::istream &iss);

   IdentifierType getIdentifierType(const std::string& identifier) const;
   void addSomething();

   std::ostream& print(std::ostream &stream);

private:
   std::unique_ptr<Scanner> scanner;
   std::unique_ptr<Parser> parser;

   const std::string red   = "\033[1;31m";
   const std::string blue  = "\033[1;36m";
   const std::string norm  = "\033[0m";
};

} /* namespace condition */
} /* namespace batchelor */

#endif /* BATCHELOR_CONDITION_DRIVER_H_ */
