#pragma once

#include "dlms/cosem/cosem.hpp"
#include "dlms/server/server_status.hpp"

#include <cstdint>

namespace dlms {
namespace server {

struct ServerGetRequest
{
  std::uint8_t invokeId;
  dlms::cosem::CosemAttributeDescriptor descriptor;
};

struct ServerSetRequest
{
  std::uint8_t invokeId;
  dlms::cosem::CosemAttributeDescriptor descriptor;
  dlms::cosem::CosemByteBuffer data;
};

struct ServerActionRequest
{
  std::uint8_t invokeId;
  dlms::cosem::CosemMethodDescriptor descriptor;
  dlms::cosem::CosemByteBuffer parameter;
};

struct ServerGetResponse
{
  std::uint8_t invokeId;
  ServerStatus status;
  bool hasData;
  dlms::cosem::CosemByteBuffer data;
};

struct ServerSetResponse
{
  std::uint8_t invokeId;
  ServerStatus status;
};

struct ServerActionResponse
{
  std::uint8_t invokeId;
  ServerStatus status;
  bool hasData;
  dlms::cosem::CosemByteBuffer data;
};

ServerGetRequest EmptyServerGetRequest();
ServerSetRequest EmptyServerSetRequest();
ServerActionRequest EmptyServerActionRequest();

ServerGetResponse MakeServerGetResponse(
  std::uint8_t invokeId,
  ServerStatus status);
ServerGetResponse MakeServerGetDataResponse(
  std::uint8_t invokeId,
  const dlms::cosem::CosemByteBuffer& data);
ServerSetResponse MakeServerSetResponse(
  std::uint8_t invokeId,
  ServerStatus status);
ServerActionResponse MakeServerActionResponse(
  std::uint8_t invokeId,
  ServerStatus status);
ServerActionResponse MakeServerActionDataResponse(
  std::uint8_t invokeId,
  const dlms::cosem::CosemByteBuffer& data);

} // namespace server
} // namespace dlms
