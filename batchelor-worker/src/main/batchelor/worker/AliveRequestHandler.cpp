#include <batchelor/worker/AliveRequestHandler.h>

#include <esl/com/http/server/Response.h>
#include <esl/io/input/Closed.h>
#include <esl/io/Output.h>
#include <esl/io/output/Memory.h>
#include <esl/utility/MIME.h>

namespace batchelor {
namespace worker {

const std::string emptyResponse = "{}";

esl::io::Input AliveRequestHandler::accept(esl::com::http::server::RequestContext& requestContext) const {
	esl::utility::MIME responseMIME = esl::utility::MIME::Type::applicationJson;
	esl::com::http::server::Response response(200, responseMIME);
	esl::io::Output output = esl::io::output::Memory::create(emptyResponse.data(), emptyResponse.size());
	requestContext.getConnection().send(response, std::move(output));

	return esl::io::input::Closed::create();
}

} /* namespace worker */
} /* namespace batchelor */
