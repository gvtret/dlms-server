#pragma once

#include "dlms/server/server_context.hpp"
#include "dlms/server/server_types.hpp"

namespace dlms {
namespace server {

class CosemServiceDispatcher
{
public:
  explicit CosemServiceDispatcher(ServerContext& context);

  ServerGetResponse HandleGet(const ServerGetRequest& request) const;

private:
  ServerContext& context_;
};

} // namespace server
} // namespace dlms
