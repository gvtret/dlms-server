#include "dlms/server/server_context.hpp"

namespace dlms {
namespace server {

ServerContext::ServerContext()
  : associated_(false)
  , accessContext_(dlms::cosem::PublicAccessContext())
  , logicalDevice_(0)
{
}

void ServerContext::SetAssociated(bool associated)
{
  associated_ = associated;
}

bool ServerContext::IsAssociated() const
{
  return associated_;
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
