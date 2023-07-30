#ifndef BATCHELOR_SERVICE_SERVER_REQUESTHANDLER_H_
#define BATCHELOR_SERVICE_SERVER_REQUESTHANDLER_H_

//#include <batchelor/service/server/ServiceFactory.h>
#include <batchelor/service/Service.h>

#include <esl/com/http/server/RequestContext.h>
#include <esl/com/http/server/RequestHandler.h>
#include <esl/io/Input.h>
#include <esl/object/Context.h>

#include <functional>
#include <memory>

namespace batchelor {
namespace service {
namespace server {

class RequestHandler : public esl::com::http::server::RequestHandler {
public:
	esl::io::Input accept(esl::com::http::server::RequestContext& requestContext) const override;

protected:
	RequestHandler(std::function<std::unique_ptr<Service>(esl::object::Context&)> createService);

private:
    //const ServiceFactory& serviceFactory;
    std::function<std::unique_ptr<Service>(esl::object::Context&)> createService;

    std::unique_ptr<Service> makeService(esl::object::Context& context) const;
};

} /* namespace server */
} /* namespace service */
} /* namespace batchelor */

#endif /* BATCHELOR_SERVICE_SERVER_REQUESTHANDLER_H_ */
