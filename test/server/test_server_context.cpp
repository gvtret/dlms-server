#include "dlms/server/server.hpp"

#include <gtest/gtest.h>

namespace {

TEST(ServerContext, DefaultsToPublicDetachedState)
{
  dlms::server::ServerContext context;

  EXPECT_FALSE(context.IsAssociated());
  EXPECT_FALSE(context.AssociationContext().associated);
  EXPECT_EQ(0u, context.AssociationContext().clientSap);
  EXPECT_EQ(0u, context.AssociationContext().serverSap);
  EXPECT_FALSE(context.AssociationContext().authenticated);
  EXPECT_FALSE(context.AssociationContext().ciphered);
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

TEST(ServerContext, StoresAndClearsAssociationContext)
{
  dlms::server::ServerContext context;
  dlms::server::ServerAssociationContext association =
    dlms::server::EmptyServerAssociationContext();
  association.associated = true;
  association.clientSap = 32u;
  association.serverSap = 1u;
  association.authenticated = true;
  association.ciphered = true;

  context.SetAssociationContext(association);

  EXPECT_TRUE(context.IsAssociated());
  EXPECT_TRUE(context.AssociationContext().associated);
  EXPECT_EQ(32u, context.AssociationContext().clientSap);
  EXPECT_EQ(1u, context.AssociationContext().serverSap);
  EXPECT_TRUE(context.AssociationContext().authenticated);
  EXPECT_TRUE(context.AssociationContext().ciphered);
  EXPECT_TRUE(context.AccessContext().authenticated);

  context.ClearAssociationContext();

  EXPECT_FALSE(context.IsAssociated());
  EXPECT_EQ(0u, context.AssociationContext().clientSap);
  EXPECT_EQ(0u, context.AssociationContext().serverSap);
  EXPECT_FALSE(context.AssociationContext().authenticated);
  EXPECT_FALSE(context.AssociationContext().ciphered);
  EXPECT_FALSE(context.AccessContext().authenticated);
}

} // namespace
