#include "dlms/server/server.hpp"

#include <gtest/gtest.h>

#include <memory>

namespace {

class AdapterTestObject : public dlms::cosem::ICosemObject
{
public:
  AdapterTestObject()
    : readStatus(dlms::cosem::CosemStatus::Ok)
    , writeStatus(dlms::cosem::CosemStatus::Ok)
    , readCount(0u)
    , writeCount(0u)
    , lastWriteAttributeId(0u)
  {
    descriptor_.key.classId = 3u;
    descriptor_.key.version = 0u;
    descriptor_.key.logicalName =
      dlms::cosem::CosemLogicalName(1, 0, 1, 8, 0, 255);
    rights_.SetAttributeAccess(
      2u, dlms::cosem::AttributeAccessMode::ReadAndWrite);
    rights_.SetAttributeAccess(
      3u, dlms::cosem::AttributeAccessMode::AuthenticatedReadOnly);
    rights_.SetAttributeAccess(
      4u, dlms::cosem::AttributeAccessMode::WriteOnly);
    data.push_back(0x12u);
    data.push_back(0x34u);
  }

  dlms::cosem::CosemObjectDescriptor Descriptor() const
  {
    return descriptor_;
  }

  dlms::cosem::CosemAccessRights AccessRights() const
  {
    return rights_;
  }

  dlms::cosem::CosemStatus ReadAttribute(
    std::uint8_t,
    dlms::cosem::CosemByteBuffer& output) const
  {
    ++readCount;
    if (readStatus != dlms::cosem::CosemStatus::Ok) {
      return readStatus;
    }

    output = data;
    return dlms::cosem::CosemStatus::Ok;
  }

  dlms::cosem::CosemStatus WriteAttribute(
    std::uint8_t attributeId,
    const dlms::cosem::CosemByteBuffer& input)
  {
    ++writeCount;
    lastWriteAttributeId = attributeId;
    lastWriteData = input;
    return writeStatus;
  }

  dlms::cosem::CosemStatus InvokeMethod(
    std::uint8_t,
    const dlms::cosem::CosemByteBuffer&,
    dlms::cosem::CosemByteBuffer&)
  {
    return dlms::cosem::CosemStatus::MethodNotFound;
  }

  dlms::cosem::CosemObjectDescriptor descriptor_;
  dlms::cosem::CosemAccessRights rights_;
  dlms::cosem::CosemStatus readStatus;
  dlms::cosem::CosemStatus writeStatus;
  mutable std::size_t readCount;
  std::size_t writeCount;
  std::uint8_t lastWriteAttributeId;
  dlms::cosem::CosemByteBuffer data;
  dlms::cosem::CosemByteBuffer lastWriteData;
};

dlms::xdlms::GetIndication MakeIndication(std::uint8_t attributeId)
{
  dlms::xdlms::GetIndication indication =
    dlms::xdlms::EmptyGetIndication();
  indication.invokeId = 6u;
  indication.descriptor.classId = 3u;
  indication.descriptor.instanceId =
    dlms::xdlms::CosemLogicalName(1, 0, 1, 8, 0, 255);
  indication.descriptor.attributeId = attributeId;
  return indication;
}

dlms::xdlms::SetIndication MakeSetIndication(std::uint8_t attributeId)
{
  dlms::xdlms::SetIndication indication =
    dlms::xdlms::EmptySetIndication();
  indication.invokeId = 8u;
  indication.descriptor.classId = 3u;
  indication.descriptor.instanceId =
    dlms::xdlms::CosemLogicalName(1, 0, 1, 8, 0, 255);
  indication.descriptor.attributeId = attributeId;
  indication.data.push_back(0x12u);
  indication.data.push_back(0x00u);
  indication.data.push_back(0x2Au);
  return indication;
}

void AttachObject(
  dlms::server::ServerContext& context,
  dlms::cosem::LogicalDevice& logicalDevice,
  const std::shared_ptr<AdapterTestObject>& object)
{
  ASSERT_EQ(dlms::cosem::CosemStatus::Ok,
            logicalDevice.RegisterObject(object));
  context.AttachLogicalDevice(&logicalDevice);
  context.SetAssociated(true);
}

} // namespace

TEST(XdlmsServerAdapter, HandleGetMapsSuccessData)
{
  dlms::server::ServerContext context;
  dlms::cosem::LogicalDevice logicalDevice(1u, "ld-1");
  const std::shared_ptr<AdapterTestObject> object(new AdapterTestObject());
  AttachObject(context, logicalDevice, object);

  dlms::server::DlmsServer server(context);
  dlms::server::XdlmsServerAdapter adapter(server);
  dlms::xdlms::GetResult result = dlms::xdlms::EmptyGetResult();

  EXPECT_EQ(dlms::xdlms::XdlmsStatus::Ok,
            adapter.HandleGet(MakeIndication(2u), result));

  EXPECT_EQ(6u, result.invokeId);
  EXPECT_TRUE(result.hasData);
  ASSERT_EQ(2u, result.data.size());
  EXPECT_EQ(0x12u, result.data[0]);
  EXPECT_EQ(0x34u, result.data[1]);
  EXPECT_FALSE(result.hasAccessResult);
  EXPECT_EQ(1u, object->readCount);
}

TEST(XdlmsServerAdapter, HandleGetMapsAccessDeniedToDataAccessResult)
{
  dlms::server::ServerContext context;
  dlms::cosem::LogicalDevice logicalDevice(1u, "ld-1");
  const std::shared_ptr<AdapterTestObject> object(new AdapterTestObject());
  AttachObject(context, logicalDevice, object);

  dlms::server::DlmsServer server(context);
  dlms::server::XdlmsServerAdapter adapter(server);
  dlms::xdlms::GetResult result = dlms::xdlms::EmptyGetResult();

  EXPECT_EQ(dlms::xdlms::XdlmsStatus::Ok,
            adapter.HandleGet(MakeIndication(3u), result));

  EXPECT_EQ(6u, result.invokeId);
  EXPECT_FALSE(result.hasData);
  EXPECT_TRUE(result.hasAccessResult);
  EXPECT_EQ(3u, result.accessResult);
}

TEST(XdlmsServerAdapter, HandleGetMapsMissingObjectToDataAccessResult)
{
  dlms::server::ServerContext context;
  dlms::cosem::LogicalDevice logicalDevice(1u, "ld-1");
  context.AttachLogicalDevice(&logicalDevice);
  context.SetAssociated(true);

  dlms::server::DlmsServer server(context);
  dlms::server::XdlmsServerAdapter adapter(server);
  dlms::xdlms::GetResult result = dlms::xdlms::EmptyGetResult();

  EXPECT_EQ(dlms::xdlms::XdlmsStatus::Ok,
            adapter.HandleGet(MakeIndication(2u), result));

  EXPECT_TRUE(result.hasAccessResult);
  EXPECT_EQ(4u, result.accessResult);
}

TEST(XdlmsServerAdapter, HandleGetMapsAssociationAndStateFailures)
{
  dlms::server::ServerContext context;
  dlms::cosem::LogicalDevice logicalDevice(1u, "ld-1");
  dlms::server::DlmsServer server(context);
  dlms::server::XdlmsServerAdapter adapter(server);
  dlms::xdlms::GetResult result = dlms::xdlms::EmptyGetResult();

  context.AttachLogicalDevice(&logicalDevice);
  EXPECT_EQ(dlms::xdlms::XdlmsStatus::NotAssociated,
            adapter.HandleGet(MakeIndication(2u), result));

  context.SetAssociated(true);
  context.AttachLogicalDevice(0);
  EXPECT_EQ(dlms::xdlms::XdlmsStatus::InvalidState,
            adapter.HandleGet(MakeIndication(2u), result));
}

TEST(XdlmsServerAdapter, HandleSetMapsSuccessAndForwardsData)
{
  dlms::server::ServerContext context;
  dlms::cosem::LogicalDevice logicalDevice(1u, "ld-1");
  const std::shared_ptr<AdapterTestObject> object(new AdapterTestObject());
  AttachObject(context, logicalDevice, object);

  dlms::server::DlmsServer server(context);
  dlms::server::XdlmsServerAdapter adapter(server);
  dlms::xdlms::SetResult result = dlms::xdlms::EmptySetResult();

  EXPECT_EQ(dlms::xdlms::XdlmsStatus::Ok,
            adapter.HandleSet(MakeSetIndication(4u), result));

  EXPECT_EQ(8u, result.invokeId);
  EXPECT_EQ(0u, result.accessResult);
  EXPECT_EQ(1u, object->writeCount);
  EXPECT_EQ(4u, object->lastWriteAttributeId);
  ASSERT_EQ(3u, object->lastWriteData.size());
  EXPECT_EQ(0x12u, object->lastWriteData[0]);
  EXPECT_EQ(0x2Au, object->lastWriteData[2]);
}

TEST(XdlmsServerAdapter, HandleSetMapsAccessDeniedToDataAccessResult)
{
  dlms::server::ServerContext context;
  dlms::cosem::LogicalDevice logicalDevice(1u, "ld-1");
  const std::shared_ptr<AdapterTestObject> object(new AdapterTestObject());
  AttachObject(context, logicalDevice, object);

  dlms::server::DlmsServer server(context);
  dlms::server::XdlmsServerAdapter adapter(server);
  dlms::xdlms::SetResult result = dlms::xdlms::EmptySetResult();

  EXPECT_EQ(dlms::xdlms::XdlmsStatus::Ok,
            adapter.HandleSet(MakeSetIndication(3u), result));

  EXPECT_EQ(8u, result.invokeId);
  EXPECT_EQ(3u, result.accessResult);
  EXPECT_EQ(0u, object->writeCount);
}

TEST(XdlmsServerAdapter, HandleSetMapsMissingObjectToDataAccessResult)
{
  dlms::server::ServerContext context;
  dlms::cosem::LogicalDevice logicalDevice(1u, "ld-1");
  context.AttachLogicalDevice(&logicalDevice);
  context.SetAssociated(true);

  dlms::server::DlmsServer server(context);
  dlms::server::XdlmsServerAdapter adapter(server);
  dlms::xdlms::SetResult result = dlms::xdlms::EmptySetResult();

  EXPECT_EQ(dlms::xdlms::XdlmsStatus::Ok,
            adapter.HandleSet(MakeSetIndication(4u), result));

  EXPECT_EQ(8u, result.invokeId);
  EXPECT_EQ(4u, result.accessResult);
}

TEST(XdlmsServerAdapter, HandleSetMapsAssociationAndStateFailures)
{
  dlms::server::ServerContext context;
  dlms::cosem::LogicalDevice logicalDevice(1u, "ld-1");
  dlms::server::DlmsServer server(context);
  dlms::server::XdlmsServerAdapter adapter(server);
  dlms::xdlms::SetResult result = dlms::xdlms::EmptySetResult();

  context.AttachLogicalDevice(&logicalDevice);
  EXPECT_EQ(dlms::xdlms::XdlmsStatus::NotAssociated,
            adapter.HandleSet(MakeSetIndication(4u), result));

  context.SetAssociated(true);
  context.AttachLogicalDevice(0);
  EXPECT_EQ(dlms::xdlms::XdlmsStatus::InvalidState,
            adapter.HandleSet(MakeSetIndication(4u), result));
}

TEST(XdlmsServerAdapter, StatusMappingUsesStableContracts)
{
  EXPECT_EQ(3u,
            dlms::server::MapServerStatusToDataAccessResult(
              dlms::server::ServerStatus::AccessDenied));
  EXPECT_EQ(4u,
            dlms::server::MapServerStatusToDataAccessResult(
              dlms::server::ServerStatus::ObjectNotFound));
  EXPECT_EQ(250u,
            dlms::server::MapServerStatusToDataAccessResult(
              dlms::server::ServerStatus::ObjectError));

  EXPECT_EQ(dlms::xdlms::XdlmsStatus::UnsupportedFeature,
            dlms::server::MapServerStatusToXdlmsStatus(
              dlms::server::ServerStatus::UnsupportedFeature));
  EXPECT_EQ(dlms::xdlms::XdlmsStatus::InternalError,
            dlms::server::MapServerStatusToXdlmsStatus(
              dlms::server::ServerStatus::EncodeRequired));
}
