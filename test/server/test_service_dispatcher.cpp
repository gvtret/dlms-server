#include "dlms/server/server.hpp"

#include <gtest/gtest.h>

#include <memory>

namespace {

class GetTestObject : public dlms::cosem::ICosemObject
{
public:
  GetTestObject()
    : readCount(0u)
    , readStatus(dlms::cosem::CosemStatus::Ok)
  {
    descriptor_.key = MakeKey(1u, 1u);
    rights_.SetAttributeAccess(
      2u, dlms::cosem::AttributeAccessMode::ReadOnly);
    rights_.SetAttributeAccess(
      3u, dlms::cosem::AttributeAccessMode::ReadOnly);
    rights_.SetAttributeAccess(
      4u, dlms::cosem::AttributeAccessMode::AuthenticatedReadOnly);
    data.push_back(0x7Bu);
  }

  static dlms::cosem::CosemObjectKey MakeKey(
    std::uint16_t classId,
    std::uint8_t logicalNamePart)
  {
    dlms::cosem::CosemObjectKey key;
    key.classId = classId;
    key.version = 0u;
    key.logicalName =
      dlms::cosem::CosemLogicalName(1, 0, logicalNamePart, 8, 0, 255);
    return key;
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
    std::uint8_t attributeId,
    dlms::cosem::CosemByteBuffer& output) const
  {
    if (attributeId == 3u) {
      return dlms::cosem::CosemStatus::AttributeNotFound;
    }

    ++readCount;
    if (readStatus != dlms::cosem::CosemStatus::Ok) {
      return readStatus;
    }

    output = data;
    return dlms::cosem::CosemStatus::Ok;
  }

  dlms::cosem::CosemStatus WriteAttribute(
    std::uint8_t,
    const dlms::cosem::CosemByteBuffer&)
  {
    return dlms::cosem::CosemStatus::UnsupportedFeature;
  }

  dlms::cosem::CosemStatus InvokeMethod(
    std::uint8_t,
    const dlms::cosem::CosemByteBuffer&,
    dlms::cosem::CosemByteBuffer&)
  {
    return dlms::cosem::CosemStatus::UnsupportedFeature;
  }

  dlms::cosem::CosemObjectDescriptor descriptor_;
  dlms::cosem::CosemAccessRights rights_;
  mutable std::size_t readCount;
  dlms::cosem::CosemStatus readStatus;
  dlms::cosem::CosemByteBuffer data;
};

dlms::cosem::CosemAttributeDescriptor MakeAttribute(
  const dlms::cosem::CosemObjectKey& key,
  std::uint8_t attributeId)
{
  dlms::cosem::CosemAttributeDescriptor descriptor;
  descriptor.object = key;
  descriptor.attributeId = attributeId;
  return descriptor;
}

dlms::server::ServerGetRequest MakeGetRequest(
  const dlms::cosem::CosemObjectKey& key,
  std::uint8_t attributeId)
{
  dlms::server::ServerGetRequest request;
  request.invokeId = 9u;
  request.descriptor = MakeAttribute(key, attributeId);
  return request;
}

TEST(CosemServiceDispatcher, GetRejectsInvalidDescriptor)
{
  dlms::server::ServerContext context;
  dlms::server::CosemServiceDispatcher dispatcher(context);

  dlms::server::ServerGetRequest request =
    dlms::server::EmptyServerGetRequest();
  request.invokeId = 3u;

  const dlms::server::ServerGetResponse response =
    dispatcher.HandleGet(request);

  EXPECT_EQ(3u, response.invokeId);
  EXPECT_EQ(dlms::server::ServerStatus::InvalidArgument, response.status);
  EXPECT_FALSE(response.hasData);
}

TEST(CosemServiceDispatcher, GetRequiresAssociation)
{
  dlms::server::ServerContext context;
  dlms::cosem::LogicalDevice logicalDevice(1u, "ld-1");
  dlms::server::CosemServiceDispatcher dispatcher(context);

  context.AttachLogicalDevice(&logicalDevice);

  const dlms::server::ServerGetResponse response =
    dispatcher.HandleGet(MakeGetRequest(GetTestObject::MakeKey(1u, 1u), 2u));

  EXPECT_EQ(dlms::server::ServerStatus::NotAssociated, response.status);
  EXPECT_FALSE(response.hasData);
}

TEST(CosemServiceDispatcher, GetRequiresLogicalDevice)
{
  dlms::server::ServerContext context;
  dlms::server::CosemServiceDispatcher dispatcher(context);

  context.SetAssociated(true);

  const dlms::server::ServerGetResponse response =
    dispatcher.HandleGet(MakeGetRequest(GetTestObject::MakeKey(1u, 1u), 2u));

  EXPECT_EQ(dlms::server::ServerStatus::NoLogicalDevice, response.status);
  EXPECT_FALSE(response.hasData);
}

TEST(CosemServiceDispatcher, GetMapsMissingDeniedAndObjectErrors)
{
  dlms::server::ServerContext context;
  dlms::cosem::LogicalDevice logicalDevice(1u, "ld-1");
  const std::shared_ptr<GetTestObject> object(new GetTestObject());
  dlms::server::CosemServiceDispatcher dispatcher(context);

  ASSERT_EQ(dlms::cosem::CosemStatus::Ok,
            logicalDevice.RegisterObject(object));
  context.SetAssociated(true);
  context.AttachLogicalDevice(&logicalDevice);

  EXPECT_EQ(
    dlms::server::ServerStatus::ObjectNotFound,
    dispatcher.HandleGet(
      MakeGetRequest(GetTestObject::MakeKey(1u, 2u), 2u)).status);
  EXPECT_EQ(
    dlms::server::ServerStatus::AccessDenied,
    dispatcher.HandleGet(
      MakeGetRequest(GetTestObject::MakeKey(1u, 1u), 4u)).status);
  EXPECT_EQ(
    dlms::server::ServerStatus::AttributeNotFound,
    dispatcher.HandleGet(
      MakeGetRequest(GetTestObject::MakeKey(1u, 1u), 3u)).status);

  object->readStatus = dlms::cosem::CosemStatus::ObjectError;
  EXPECT_EQ(
    dlms::server::ServerStatus::ObjectError,
    dispatcher.HandleGet(
      MakeGetRequest(GetTestObject::MakeKey(1u, 1u), 2u)).status);
}

TEST(CosemServiceDispatcher, GetReturnsDataOnSuccess)
{
  dlms::server::ServerContext context;
  dlms::cosem::LogicalDevice logicalDevice(1u, "ld-1");
  const std::shared_ptr<GetTestObject> object(new GetTestObject());
  dlms::server::CosemServiceDispatcher dispatcher(context);

  ASSERT_EQ(dlms::cosem::CosemStatus::Ok,
            logicalDevice.RegisterObject(object));
  context.SetAssociated(true);
  context.AttachLogicalDevice(&logicalDevice);

  const dlms::server::ServerGetResponse response =
    dispatcher.HandleGet(MakeGetRequest(GetTestObject::MakeKey(1u, 1u), 2u));

  EXPECT_EQ(9u, response.invokeId);
  EXPECT_EQ(dlms::server::ServerStatus::Ok, response.status);
  ASSERT_TRUE(response.hasData);
  ASSERT_EQ(1u, response.data.size());
  EXPECT_EQ(0x7Bu, response.data[0]);
  EXPECT_EQ(1u, object->readCount);
}

TEST(CosemServiceDispatcher, GetUsesAuthenticatedAccessContext)
{
  dlms::server::ServerContext context;
  dlms::cosem::LogicalDevice logicalDevice(1u, "ld-1");
  const std::shared_ptr<GetTestObject> object(new GetTestObject());
  dlms::server::CosemServiceDispatcher dispatcher(context);

  ASSERT_EQ(dlms::cosem::CosemStatus::Ok,
            logicalDevice.RegisterObject(object));
  context.SetAssociated(true);
  context.SetAccessContext(dlms::cosem::AuthenticatedAccessContext());
  context.AttachLogicalDevice(&logicalDevice);

  const dlms::server::ServerGetResponse response =
    dispatcher.HandleGet(MakeGetRequest(GetTestObject::MakeKey(1u, 1u), 4u));

  EXPECT_EQ(dlms::server::ServerStatus::Ok, response.status);
  EXPECT_TRUE(response.hasData);
}

} // namespace
