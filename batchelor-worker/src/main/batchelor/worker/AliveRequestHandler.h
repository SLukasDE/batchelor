#ifndef BATCHELOR_WORKER_ALIVEREQUESTHANDLER_H_
#define BATCHELOR_WORKER_ALIVEREQUESTHANDLER_H_

#include <esl/com/http/server/RequestContext.h>
#include <esl/com/http/server/RequestHandler.h>
#include <esl/io/Input.h>
//#include <esl/object/Context.h>

//#include <functional>
//#include <memory>

namespace batchelor {
namespace worker {

class AliveRequestHandler : public esl::com::http::server::RequestHandler {
public:
	esl::io::Input accept(esl::com::http::server::RequestContext& requestContext) const override;
};

} /* namespace worker */
} /* namespace batchelor */

#endif /* BATCHELOR_WORKER_ALIVEREQUESTHANDLER_H_ */
