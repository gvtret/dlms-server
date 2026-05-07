#include "dlms/server/server.hpp"

#include <gtest/gtest.h>

namespace {

TEST(ServerContext, DefaultsToPublicDetachedState)
{
  dlms::server::ServerContext context;

  EXPECT_FALSE(context.IsAssociated());
  EXPECT_FALSE(context.AccessContext().authenticated);
  EXPECT_EQ(0, context.LogicalDevice());
}

TEST(ServerContext, StoresAssociationAccessAndLogicalDevice)
{
  dlms::server::ServerContext context;
  dlms::cosem::LogicalDevice logicalDevice(1u, "ld-1");

  context.SetAssociated(true);
  context.SetAccessContext(dlms::cosem::AuthenticatedAccessContext());
  context.AttachLogicalDevice(&logicalDevice);

  EXPECT_TRUE(context.IsAssociated());
  EXPECT_TRUE(context.AccessContext().authenticated);
  EXPECT_EQ(&logicalDevice, context.LogicalDevice());

  context.SetAssociated(false);
  context.AttachLogicalDevice(0);

  EXPECT_FALSE(context.IsAssociated());
  EXPECT_EQ(0, context.LogicalDevice());
}

} // namespace
