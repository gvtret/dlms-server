#include "dlms/server/server_status.hpp"

namespace dlms {
namespace server {

const char* ServerStatusName(ServerStatus status)
{
  switch (status) {
  case ServerStatus::Ok:
    return "Ok";
  case ServerStatus::InvalidArgument:
    return "InvalidArgument";
  case ServerStatus::NotAssociated:
    return "NotAssociated";
  case ServerStatus::NoLogicalDevice:
    return "NoLogicalDevice";
  case ServerStatus::ObjectNotFound:
    return "ObjectNotFound";
  case ServerStatus::AccessDenied:
    return "AccessDenied";
  case ServerStatus::AttributeNotFound:
    return "AttributeNotFound";
  case ServerStatus::MethodNotFound:
    return "MethodNotFound";
  case ServerStatus::ObjectError:
    return "ObjectError";
  case ServerStatus::UnsupportedFeature:
    return "UnsupportedFeature";
  case ServerStatus::EncodeRequired:
    return "EncodeRequired";
  case ServerStatus::InternalError:
    return "InternalError";
  }

  return "Unknown";
}

ServerStatus MapCosemStatus(dlms::cosem::CosemStatus status)
{
  switch (status) {
  case dlms::cosem::CosemStatus::Ok:
    return ServerStatus::Ok;
  case dlms::cosem::CosemStatus::InvalidArgument:
    return ServerStatus::InvalidArgument;
  case dlms::cosem::CosemStatus::ObjectNotFound:
    return ServerStatus::ObjectNotFound;
  case dlms::cosem::CosemStatus::AccessDenied:
    return ServerStatus::AccessDenied;
  case dlms::cosem::CosemStatus::AttributeNotFound:
    return ServerStatus::AttributeNotFound;
  case dlms::cosem::CosemStatus::MethodNotFound:
    return ServerStatus::MethodNotFound;
  case dlms::cosem::CosemStatus::ObjectError:
    return ServerStatus::ObjectError;
  case dlms::cosem::CosemStatus::UnsupportedFeature:
    return ServerStatus::UnsupportedFeature;
  case dlms::cosem::CosemStatus::DuplicateObject:
  case dlms::cosem::CosemStatus::OutputBufferTooSmall:
  case dlms::cosem::CosemStatus::InternalError:
    return ServerStatus::InternalError;
  }

  return ServerStatus::InternalError;
}

} // namespace server
} // namespace dlms
