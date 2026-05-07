#include "dlms/server/dlms_server.hpp"

namespace dlms {
namespace server {

DlmsServer::DlmsServer(ServerContext& context)
  : dispatcher_(context)
{
}

ServerGetResponse DlmsServer::HandleGet(
  const ServerGetRequest& request)
{
  return dispatcher_.HandleGet(request);
}

ServerSetResponse DlmsServer::HandleSet(
  const ServerSetRequest& request)
{
  return dispatcher_.HandleSet(request);
}

ServerActionResponse DlmsServer::HandleAction(
  const ServerActionRequest& request)
{
  return dispatcher_.HandleAction(request);
}

} // namespace server
} // namespace dlms
