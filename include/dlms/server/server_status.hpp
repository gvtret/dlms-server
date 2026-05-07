#pragma once

#include "dlms/cosem/cosem_status.hpp"

namespace dlms {
namespace server {

enum class ServerStatus
{
  Ok,
  InvalidArgument,
  NotAssociated,
  NoLogicalDevice,
  ObjectNotFound,
  AccessDenied,
  AttributeNotFound,
  MethodNotFound,
  ObjectError,
  UnsupportedFeature,
  EncodeRequired,
  InternalError
};

const char* ServerStatusName(ServerStatus status);
ServerStatus MapCosemStatus(dlms::cosem::CosemStatus status);

} // namespace server
} // namespace dlms
