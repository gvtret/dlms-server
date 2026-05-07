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

ServerSetResponse CosemServiceDispatcher::HandleSet(
  const ServerSetRequest& request)
{
  const dlms::cosem::CosemStatus descriptorStatus =
    dlms::cosem::ValidateAttributeDescriptor(request.descriptor);
  if (descriptorStatus != dlms::cosem::CosemStatus::Ok) {
    return MakeServerSetResponse(
      request.invokeId, MapCosemStatus(descriptorStatus));
  }

  if (!context_.IsAssociated()) {
    return MakeServerSetResponse(
      request.invokeId, ServerStatus::NotAssociated);
  }

  dlms::cosem::LogicalDevice* logicalDevice = context_.LogicalDevice();
  if (!logicalDevice) {
    return MakeServerSetResponse(
      request.invokeId, ServerStatus::NoLogicalDevice);
  }

  const dlms::cosem::CosemStatus writeStatus =
    logicalDevice->WriteAttribute(
      request.descriptor, context_.AccessContext(), request.data);
  return MakeServerSetResponse(
    request.invokeId, MapCosemStatus(writeStatus));
}

ServerActionResponse CosemServiceDispatcher::HandleAction(
  const ServerActionRequest& request)
{
  const dlms::cosem::CosemStatus descriptorStatus =
    dlms::cosem::ValidateMethodDescriptor(request.descriptor);
  if (descriptorStatus != dlms::cosem::CosemStatus::Ok) {
    return MakeServerActionResponse(
      request.invokeId, MapCosemStatus(descriptorStatus));
  }

  if (!context_.IsAssociated()) {
    return MakeServerActionResponse(
      request.invokeId, ServerStatus::NotAssociated);
  }

  dlms::cosem::LogicalDevice* logicalDevice = context_.LogicalDevice();
  if (!logicalDevice) {
    return MakeServerActionResponse(
      request.invokeId, ServerStatus::NoLogicalDevice);
  }

  dlms::cosem::CosemByteBuffer data;
  const dlms::cosem::CosemStatus invokeStatus =
    logicalDevice->InvokeMethod(
      request.descriptor, context_.AccessContext(), request.parameter, data);
  if (invokeStatus != dlms::cosem::CosemStatus::Ok) {
    return MakeServerActionResponse(
      request.invokeId, MapCosemStatus(invokeStatus));
  }

  return MakeServerActionDataResponse(request.invokeId, data);
}

} // namespace server
} // namespace dlms
