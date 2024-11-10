/*
 * This file is part of Batchelor.
 * Copyright (C) 2023-2024 Sven Lukas
 *
 * Batchelor is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * Batchelor is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public
 * License along with Batchelor.  If not, see <https://www.gnu.org/licenses/>.
 */

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
