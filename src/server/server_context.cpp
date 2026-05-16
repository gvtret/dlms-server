#include "dlms/server/server_context.hpp"

namespace dlms {
namespace server {

ServerContext::ServerContext()
  : association_(EmptyServerAssociationContext())
  , accessContext_(dlms::cosem::PublicAccessContext())
  , logicalDevice_(0)
{
}

void ServerContext::SetAssociated(bool associated)
{
  association_.associated = associated;
}

bool ServerContext::IsAssociated() const
{
  return association_.associated;
}

void ServerContext::SetAssociationContext(
  const ServerAssociationContext& context)
{
  association_ = context;
  accessContext_ = context.authenticated
    ? dlms::cosem::AuthenticatedAccessContext()
    : dlms::cosem::PublicAccessContext();
}

ServerAssociationContext ServerContext::AssociationContext() const
{
  return association_;
}

void ServerContext::ClearAssociationContext()
{
  association_ = EmptyServerAssociationContext();
  accessContext_ = dlms::cosem::PublicAccessContext();
}

void ServerContext::SetAccessContext(
  const dlms::cosem::CosemAccessContext& context)
{
  accessContext_ = context;
}

dlms::cosem::CosemAccessContext ServerContext::AccessContext() const
{
  return accessContext_;
}

void ServerContext::AttachLogicalDevice(
  dlms::cosem::LogicalDevice* logicalDevice)
{
  logicalDevice_ = logicalDevice;
}

dlms::cosem::LogicalDevice* ServerContext::LogicalDevice()
{
  return logicalDevice_;
}

const dlms::cosem::LogicalDevice* ServerContext::LogicalDevice() const
{
  return logicalDevice_;
}

} // namespace server
} // namespace dlms
