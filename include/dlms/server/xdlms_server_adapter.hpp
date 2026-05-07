#pragma once

#include "dlms/server/dlms_server.hpp"
#include "dlms/server/server_status.hpp"
#include "dlms/xdlms/xdlms_server.hpp"

#include <cstdint>

namespace dlms {
namespace server {

class XdlmsServerAdapter : public dlms::xdlms::IXdlmsServerHandler
{
public:
  explicit XdlmsServerAdapter(DlmsServer& server);

  dlms::xdlms::XdlmsStatus HandleGet(
    const dlms::xdlms::GetIndication& indication,
    dlms::xdlms::GetResult& result);

  dlms::xdlms::XdlmsStatus HandleSet(
    const dlms::xdlms::SetIndication& indication,
    dlms::xdlms::SetResult& result);

  dlms::xdlms::XdlmsStatus HandleAction(
    const dlms::xdlms::ActionIndication& indication,
    dlms::xdlms::ActionResult& result);

private:
  DlmsServer& server_;
};

std::uint8_t MapServerStatusToDataAccessResult(ServerStatus status);
std::uint8_t MapServerStatusToActionResult(ServerStatus status);
dlms::xdlms::XdlmsStatus MapServerStatusToXdlmsStatus(ServerStatus status);

} // namespace server
} // namespace dlms
