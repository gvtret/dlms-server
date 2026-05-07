#include "dlms/server/xdlms_server_adapter.hpp"

namespace {

dlms::cosem::CosemLogicalName ToCosemLogicalName(
  const dlms::xdlms::CosemLogicalName& logicalName)
{
  return dlms::cosem::CosemLogicalName(
    logicalName[0],
    logicalName[1],
    logicalName[2],
    logicalName[3],
    logicalName[4],
    logicalName[5]);
}

dlms::cosem::CosemAttributeDescriptor ToServerDescriptor(
  const dlms::xdlms::CosemAttributeDescriptor& descriptor)
{
  dlms::cosem::CosemAttributeDescriptor serverDescriptor =
    dlms::cosem::EmptyCosemAttributeDescriptor();
  serverDescriptor.object.classId = descriptor.classId;
  serverDescriptor.object.version = 0u;
  serverDescriptor.object.logicalName =
    ToCosemLogicalName(descriptor.instanceId);
  serverDescriptor.attributeId = descriptor.attributeId;
  return serverDescriptor;
}

bool IsDataAccessResultStatus(dlms::server::ServerStatus status)
{
  return status == dlms::server::ServerStatus::ObjectNotFound
    || status == dlms::server::ServerStatus::AccessDenied
    || status == dlms::server::ServerStatus::AttributeNotFound
    || status == dlms::server::ServerStatus::ObjectError;
}

} // namespace

namespace dlms {
namespace server {

XdlmsServerAdapter::XdlmsServerAdapter(DlmsServer& server)
  : server_(server)
{
}

dlms::xdlms::XdlmsStatus XdlmsServerAdapter::HandleGet(
  const dlms::xdlms::GetIndication& indication,
  dlms::xdlms::GetResult& result)
{
  ServerGetRequest request = EmptyServerGetRequest();
  request.invokeId = indication.invokeId;
  request.descriptor = ToServerDescriptor(indication.descriptor);

  const ServerGetResponse response = server_.HandleGet(request);
  if (response.status == ServerStatus::Ok) {
    if (!response.hasData) {
      return dlms::xdlms::XdlmsStatus::InternalError;
    }

    result = dlms::xdlms::EmptyGetResult();
    result.invokeId = response.invokeId;
    result.hasData = true;
    result.data = response.data;
    return dlms::xdlms::XdlmsStatus::Ok;
  }

  if (IsDataAccessResultStatus(response.status)) {
    result = dlms::xdlms::EmptyGetResult();
    result.invokeId = response.invokeId;
    result.hasAccessResult = true;
    result.accessResult = MapServerStatusToDataAccessResult(response.status);
    return dlms::xdlms::XdlmsStatus::Ok;
  }

  return MapServerStatusToXdlmsStatus(response.status);
}

std::uint8_t MapServerStatusToDataAccessResult(ServerStatus status)
{
  switch (status) {
    case ServerStatus::AccessDenied:
      return 3u;
    case ServerStatus::ObjectNotFound:
    case ServerStatus::AttributeNotFound:
      return 4u;
    case ServerStatus::ObjectError:
      return 250u;
    default:
      return 250u;
  }
}

dlms::xdlms::XdlmsStatus MapServerStatusToXdlmsStatus(ServerStatus status)
{
  switch (status) {
    case ServerStatus::Ok:
      return dlms::xdlms::XdlmsStatus::Ok;
    case ServerStatus::InvalidArgument:
      return dlms::xdlms::XdlmsStatus::InvalidArgument;
    case ServerStatus::NotAssociated:
      return dlms::xdlms::XdlmsStatus::NotAssociated;
    case ServerStatus::NoLogicalDevice:
      return dlms::xdlms::XdlmsStatus::InvalidState;
    case ServerStatus::UnsupportedFeature:
      return dlms::xdlms::XdlmsStatus::UnsupportedFeature;
    case ServerStatus::EncodeRequired:
    case ServerStatus::ObjectNotFound:
    case ServerStatus::AccessDenied:
    case ServerStatus::AttributeNotFound:
    case ServerStatus::MethodNotFound:
    case ServerStatus::ObjectError:
    case ServerStatus::InternalError:
    default:
      return dlms::xdlms::XdlmsStatus::InternalError;
  }
}

} // namespace server
} // namespace dlms
