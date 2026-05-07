#include "dlms/server/service_dispatcher.hpp"

namespace dlms {
namespace server {

CosemServiceDispatcher::CosemServiceDispatcher(ServerContext& context)
  : context_(context)
{
}

ServerGetResponse CosemServiceDispatcher::HandleGet(
  const ServerGetRequest& request) const
{
  const dlms::cosem::CosemStatus descriptorStatus =
    dlms::cosem::ValidateAttributeDescriptor(request.descriptor);
  if (descriptorStatus != dlms::cosem::CosemStatus::Ok) {
    return MakeServerGetResponse(
      request.invokeId, MapCosemStatus(descriptorStatus));
  }

  if (!context_.IsAssociated()) {
    return MakeServerGetResponse(
      request.invokeId, ServerStatus::NotAssociated);
  }

  const dlms::cosem::LogicalDevice* logicalDevice =
    context_.LogicalDevice();
  if (!logicalDevice) {
    return MakeServerGetResponse(
      request.invokeId, ServerStatus::NoLogicalDevice);
  }

  dlms::cosem::CosemByteBuffer data;
  const dlms::cosem::CosemStatus readStatus =
    logicalDevice->ReadAttribute(
      request.descriptor, context_.AccessContext(), data);
  if (readStatus != dlms::cosem::CosemStatus::Ok) {
    return MakeServerGetResponse(
      request.invokeId, MapCosemStatus(readStatus));
  }

  return MakeServerGetDataResponse(request.invokeId, data);
}

} // namespace server
} // namespace dlms
