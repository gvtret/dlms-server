#include "dlms/server/server.hpp"

#include <gtest/gtest.h>

#include <memory>

namespace {

class GetTestObject : public dlms::cosem::ICosemObject
{
public:
  GetTestObject()
    : readCount(0u)
    , writeCount(0u)
    , invokeCount(0u)
    , readStatus(dlms::cosem::CosemStatus::Ok)
    , writeStatus(dlms::cosem::CosemStatus::Ok)
    , invokeStatus(dlms::cosem::CosemStatus::Ok)
  {
    descriptor_.key = MakeKey(1u, 1u);
    rights_.SetAttributeAccess(
      2u, dlms::cosem::AttributeAccessMode::ReadOnly);
    rights_.SetAttributeAccess(
      3u, dlms::cosem::AttributeAccessMode::ReadOnly);
    rights_.SetAttributeAccess(
      4u, dlms::cosem::AttributeAccessMode::AuthenticatedReadOnly);
    rights_.SetAttributeAccess(
      5u, dlms::cosem::AttributeAccessMode::WriteOnly);
    rights_.SetAttributeAccess(
      6u, dlms::cosem::AttributeAccessMode::WriteOnly);
    rights_.SetAttributeAccess(
      7u, dlms::cosem::AttributeAccessMode::AuthenticatedWriteOnly);
    rights_.SetMethodAccess(
      1u, dlms::cosem::MethodAccessMode::Access);
    rights_.SetMethodAccess(
      2u, dlms::cosem::MethodAccessMode::Access);
    rights_.SetMethodAccess(
      3u, dlms::cosem::MethodAccessMode::AuthenticatedAccess);
    data.push_back(0x7Bu);
    actionData.push_back(0x41u);
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
    std::uint8_t attributeId,
    const dlms::cosem::CosemByteBuffer& input)
  {
    if (attributeId == 6u) {
      return dlms::cosem::CosemStatus::AttributeNotFound;
    }

    ++writeCount;
    if (writeStatus != dlms::cosem::CosemStatus::Ok) {
      return writeStatus;
    }

    writtenData = input;
    return dlms::cosem::CosemStatus::Ok;
  }

  dlms::cosem::CosemStatus InvokeMethod(
    std::uint8_t methodId,
    const dlms::cosem::CosemByteBuffer& input,
    dlms::cosem::CosemByteBuffer& output)
  {
    if (methodId == 2u) {
      return dlms::cosem::CosemStatus::MethodNotFound;
    }

    ++invokeCount;
    lastActionInput = input;
    if (invokeStatus != dlms::cosem::CosemStatus::Ok) {
      return invokeStatus;
    }

    output = actionData;
    return dlms::cosem::CosemStatus::Ok;
  }

  dlms::cosem::CosemObjectDescriptor descriptor_;
  dlms::cosem::CosemAccessRights rights_;
  mutable std::size_t readCount;
  std::size_t writeCount;
  std::size_t invokeCount;
  dlms::cosem::CosemStatus readStatus;
  dlms::cosem::CosemStatus writeStatus;
  dlms::cosem::CosemStatus invokeStatus;
  dlms::cosem::CosemByteBuffer data;
  dlms::cosem::CosemByteBuffer actionData;
  dlms::cosem::CosemByteBuffer writtenData;
  dlms::cosem::CosemByteBuffer lastActionInput;
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

dlms::server::ServerSetRequest MakeSetRequest(
  const dlms::cosem::CosemObjectKey& key,
  std::uint8_t attributeId)
{
  dlms::server::ServerSetRequest request;
  request.invokeId = 10u;
  request.descriptor = MakeAttribute(key, attributeId);
  request.data.push_back(0x55u);
  return request;
}

dlms::cosem::CosemMethodDescriptor MakeMethod(
  const dlms::cosem::CosemObjectKey& key,
  std::uint8_t methodId)
{
  dlms::cosem::CosemMethodDescriptor descriptor;
  descriptor.object = key;
  descriptor.methodId = methodId;
  return descriptor;
}

dlms::server::ServerActionRequest MakeActionRequest(
  const dlms::cosem::CosemObjectKey& key,
  std::uint8_t methodId)
{
  dlms::server::ServerActionRequest request;
  request.invokeId = 11u;
  request.descriptor = MakeMethod(key, methodId);
  request.parameter.push_back(0x66u);
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

TEST(CosemServiceDispatcher, SetRejectsInvalidNotAssociatedAndMissingDevice)
{
  dlms::server::ServerContext context;
  dlms::cosem::LogicalDevice logicalDevice(1u, "ld-1");
  dlms::server::CosemServiceDispatcher dispatcher(context);

  dlms::server::ServerSetRequest invalid =
    dlms::server::EmptyServerSetRequest();
  invalid.invokeId = 12u;

  EXPECT_EQ(dlms::server::ServerStatus::InvalidArgument,
            dispatcher.HandleSet(invalid).status);

  context.AttachLogicalDevice(&logicalDevice);
  EXPECT_EQ(
    dlms::server::ServerStatus::NotAssociated,
    dispatcher.HandleSet(
      MakeSetRequest(GetTestObject::MakeKey(1u, 1u), 5u)).status);

  context.SetAssociated(true);
  context.AttachLogicalDevice(0);
  EXPECT_EQ(
    dlms::server::ServerStatus::NoLogicalDevice,
    dispatcher.HandleSet(
      MakeSetRequest(GetTestObject::MakeKey(1u, 1u), 5u)).status);
}

TEST(CosemServiceDispatcher, SetMapsMissingDeniedAndObjectErrors)
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
    dispatcher.HandleSet(
      MakeSetRequest(GetTestObject::MakeKey(1u, 2u), 5u)).status);
  EXPECT_EQ(
    dlms::server::ServerStatus::AccessDenied,
    dispatcher.HandleSet(
      MakeSetRequest(GetTestObject::MakeKey(1u, 1u), 7u)).status);
  EXPECT_EQ(
    dlms::server::ServerStatus::AttributeNotFound,
    dispatcher.HandleSet(
      MakeSetRequest(GetTestObject::MakeKey(1u, 1u), 6u)).status);

  object->writeStatus = dlms::cosem::CosemStatus::ObjectError;
  EXPECT_EQ(
    dlms::server::ServerStatus::ObjectError,
    dispatcher.HandleSet(
      MakeSetRequest(GetTestObject::MakeKey(1u, 1u), 5u)).status);
}

TEST(CosemServiceDispatcher, SetWritesDataOnSuccess)
{
  dlms::server::ServerContext context;
  dlms::cosem::LogicalDevice logicalDevice(1u, "ld-1");
  const std::shared_ptr<GetTestObject> object(new GetTestObject());
  dlms::server::CosemServiceDispatcher dispatcher(context);

  ASSERT_EQ(dlms::cosem::CosemStatus::Ok,
            logicalDevice.RegisterObject(object));
  context.SetAssociated(true);
  context.AttachLogicalDevice(&logicalDevice);

  const dlms::server::ServerSetRequest request =
    MakeSetRequest(GetTestObject::MakeKey(1u, 1u), 5u);
  const dlms::server::ServerSetResponse response =
    dispatcher.HandleSet(request);

  EXPECT_EQ(10u, response.invokeId);
  EXPECT_EQ(dlms::server::ServerStatus::Ok, response.status);
  EXPECT_EQ(request.data, object->writtenData);
  EXPECT_EQ(1u, object->writeCount);
}

TEST(CosemServiceDispatcher, ActionRejectsInvalidNotAssociatedAndMissingDevice)
{
  dlms::server::ServerContext context;
  dlms::cosem::LogicalDevice logicalDevice(1u, "ld-1");
  dlms::server::CosemServiceDispatcher dispatcher(context);

  dlms::server::ServerActionRequest invalid =
    dlms::server::EmptyServerActionRequest();
  invalid.invokeId = 13u;

  EXPECT_EQ(dlms::server::ServerStatus::InvalidArgument,
            dispatcher.HandleAction(invalid).status);

  context.AttachLogicalDevice(&logicalDevice);
  EXPECT_EQ(
    dlms::server::ServerStatus::NotAssociated,
    dispatcher.HandleAction(
      MakeActionRequest(GetTestObject::MakeKey(1u, 1u), 1u)).status);

  context.SetAssociated(true);
  context.AttachLogicalDevice(0);
  EXPECT_EQ(
    dlms::server::ServerStatus::NoLogicalDevice,
    dispatcher.HandleAction(
      MakeActionRequest(GetTestObject::MakeKey(1u, 1u), 1u)).status);
}

TEST(CosemServiceDispatcher, ActionMapsMissingDeniedAndObjectErrors)
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
    dispatcher.HandleAction(
      MakeActionRequest(GetTestObject::MakeKey(1u, 2u), 1u)).status);
  EXPECT_EQ(
    dlms::server::ServerStatus::AccessDenied,
    dispatcher.HandleAction(
      MakeActionRequest(GetTestObject::MakeKey(1u, 1u), 3u)).status);
  EXPECT_EQ(
    dlms::server::ServerStatus::MethodNotFound,
    dispatcher.HandleAction(
      MakeActionRequest(GetTestObject::MakeKey(1u, 1u), 2u)).status);

  object->invokeStatus = dlms::cosem::CosemStatus::ObjectError;
  EXPECT_EQ(
    dlms::server::ServerStatus::ObjectError,
    dispatcher.HandleAction(
      MakeActionRequest(GetTestObject::MakeKey(1u, 1u), 1u)).status);
}

TEST(CosemServiceDispatcher, ActionReturnsDataOnSuccess)
{
  dlms::server::ServerContext context;
  dlms::cosem::LogicalDevice logicalDevice(1u, "ld-1");
  const std::shared_ptr<GetTestObject> object(new GetTestObject());
  dlms::server::CosemServiceDispatcher dispatcher(context);

  ASSERT_EQ(dlms::cosem::CosemStatus::Ok,
            logicalDevice.RegisterObject(object));
  context.SetAssociated(true);
  context.AttachLogicalDevice(&logicalDevice);

  const dlms::server::ServerActionRequest request =
    MakeActionRequest(GetTestObject::MakeKey(1u, 1u), 1u);
  const dlms::server::ServerActionResponse response =
    dispatcher.HandleAction(request);

  EXPECT_EQ(11u, response.invokeId);
  EXPECT_EQ(dlms::server::ServerStatus::Ok, response.status);
  ASSERT_TRUE(response.hasData);
  ASSERT_EQ(1u, response.data.size());
  EXPECT_EQ(0x41u, response.data[0]);
  EXPECT_EQ(request.parameter, object->lastActionInput);
  EXPECT_EQ(1u, object->invokeCount);
}

} // namespace
