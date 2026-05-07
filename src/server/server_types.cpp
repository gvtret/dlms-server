#include "dlms/server/server_types.hpp"

namespace dlms {
namespace server {

ServerGetRequest EmptyServerGetRequest()
{
  ServerGetRequest request;
  request.invokeId = 0u;
  request.descriptor = dlms::cosem::EmptyCosemAttributeDescriptor();
  return request;
}

ServerSetRequest EmptyServerSetRequest()
{
  ServerSetRequest request;
  request.invokeId = 0u;
  request.descriptor = dlms::cosem::EmptyCosemAttributeDescriptor();
  request.data.clear();
  return request;
}

ServerActionRequest EmptyServerActionRequest()
{
  ServerActionRequest request;
  request.invokeId = 0u;
  request.descriptor = dlms::cosem::EmptyCosemMethodDescriptor();
  request.parameter.clear();
  return request;
}

ServerGetResponse MakeServerGetResponse(
  std::uint8_t invokeId,
  ServerStatus status)
{
  ServerGetResponse response;
  response.invokeId = invokeId;
  response.status = status;
  response.hasData = false;
  response.data.clear();
  return response;
}

ServerGetResponse MakeServerGetDataResponse(
  std::uint8_t invokeId,
  const dlms::cosem::CosemByteBuffer& data)
{
  ServerGetResponse response =
    MakeServerGetResponse(invokeId, ServerStatus::Ok);
  response.hasData = true;
  response.data = data;
  return response;
}

ServerSetResponse MakeServerSetResponse(
  std::uint8_t invokeId,
  ServerStatus status)
{
  ServerSetResponse response;
  response.invokeId = invokeId;
  response.status = status;
  return response;
}

ServerActionResponse MakeServerActionResponse(
  std::uint8_t invokeId,
  ServerStatus status)
{
  ServerActionResponse response;
  response.invokeId = invokeId;
  response.status = status;
  response.hasData = false;
  response.data.clear();
  return response;
}

ServerActionResponse MakeServerActionDataResponse(
  std::uint8_t invokeId,
  const dlms::cosem::CosemByteBuffer& data)
{
  ServerActionResponse response =
    MakeServerActionResponse(invokeId, ServerStatus::Ok);
  response.hasData = true;
  response.data = data;
  return response;
}

} // namespace server
} // namespace dlms
