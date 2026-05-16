#pragma once

#include "dlms/cosem/cosem.hpp"
#include "dlms/server/server_types.hpp"

namespace dlms {
namespace server {

class ServerContext
{
public:
  ServerContext();

  void SetAssociated(bool associated);
  bool IsAssociated() const;

  void SetAssociationContext(
    const ServerAssociationContext& context);
  ServerAssociationContext AssociationContext() const;
  void ClearAssociationContext();

  void SetAccessContext(
    const dlms::cosem::CosemAccessContext& context);
  dlms::cosem::CosemAccessContext AccessContext() const;

  void AttachLogicalDevice(
    dlms::cosem::LogicalDevice* logicalDevice);
  dlms::cosem::LogicalDevice* LogicalDevice();
  const dlms::cosem::LogicalDevice* LogicalDevice() const;

private:
  ServerAssociationContext association_;
  dlms::cosem::CosemAccessContext accessContext_;
  dlms::cosem::LogicalDevice* logicalDevice_;
};

} // namespace server
} // namespace dlms
