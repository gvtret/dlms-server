#pragma once

#include "dlms/server/service_dispatcher.hpp"
#include "dlms/server/server_context.hpp"
#include "dlms/server/server_types.hpp"

namespace dlms {
namespace server {

class IServerService
{
public:
  virtual ~IServerService();

  virtual ServerGetResponse HandleGet(const ServerGetRequest& request) = 0;
  virtual ServerSetResponse HandleSet(const ServerSetRequest& request) = 0;
  virtual ServerActionResponse HandleAction(
    const ServerActionRequest& request) = 0;
};

class DlmsServer : public IServerService
{
public:
  explicit DlmsServer(ServerContext& context);

  ServerGetResponse HandleGet(const ServerGetRequest& request);
  ServerSetResponse HandleSet(const ServerSetRequest& request);
  ServerActionResponse HandleAction(const ServerActionRequest& request);

private:
  CosemServiceDispatcher dispatcher_;
};

} // namespace server
} // namespace dlms
