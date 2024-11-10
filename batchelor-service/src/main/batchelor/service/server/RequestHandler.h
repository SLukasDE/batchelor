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

#ifndef BATCHELOR_SERVICE_SERVER_REQUESTHANDLER_H_
#define BATCHELOR_SERVICE_SERVER_REQUESTHANDLER_H_

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
	RequestHandler(std::function<std::unique_ptr<Service>(const esl::object::Context&)> createService);

private:
    std::function<std::unique_ptr<Service>(const esl::object::Context&)> createService;

    std::unique_ptr<Service> makeService(esl::object::Context& context) const;
};

} /* namespace server */
} /* namespace service */
} /* namespace batchelor */

#endif /* BATCHELOR_SERVICE_SERVER_REQUESTHANDLER_H_ */
