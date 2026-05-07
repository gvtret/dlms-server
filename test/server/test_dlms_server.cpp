#include "dlms/server/server.hpp"

#include <gtest/gtest.h>

#include <memory>

namespace {

class FacadeTestObject : public dlms::cosem::ICosemObject
{
public:
  FacadeTestObject()
  {
    descriptor_.key.classId = 1u;
    descriptor_.key.version = 0u;
    descriptor_.key.logicalName =
      dlms::cosem::CosemLogicalName(1, 0, 1, 8, 0, 255);
    rights_.SetAttributeAccess(
      2u, dlms::cosem::AttributeAccessMode::ReadAndWrite);
    rights_.SetMethodAccess(1u, dlms::cosem::MethodAccessMode::Access);
    value_.push_back(0x10u);
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
    if (attributeId != 2u) {
      return dlms::cosem::CosemStatus::AttributeNotFound;
    }
    output = value_;
    return dlms::cosem::CosemStatus::Ok;
  }

  dlms::cosem::CosemStatus WriteAttribute(
    std::uint8_t attributeId,
    const dlms::cosem::CosemByteBuffer& input)
  {
    if (attributeId != 2u) {
      return dlms::cosem::CosemStatus::AttributeNotFound;
    }
    value_ = input;
    return dlms::cosem::CosemStatus::Ok;
  }

  dlms::cosem::CosemStatus InvokeMethod(
    std::uint8_t methodId,
    const dlms::cosem::CosemByteBuffer& input,
    dlms::cosem::CosemByteBuffer& output)
  {
    if (methodId != 1u) {
      return dlms::cosem::CosemStatus::MethodNotFound;
    }
    output = input;
    return dlms::cosem::CosemStatus::Ok;
  }

  dlms::cosem::CosemObjectKey Key() const
  {
    return descriptor_.key;
  }

private:
  dlms::cosem::CosemObjectDescriptor descriptor_;
  dlms::cosem::CosemAccessRights rights_;
  dlms::cosem::CosemByteBuffer value_;
};

dlms::cosem::CosemAttributeDescriptor MakeAttribute(
  const dlms::cosem::CosemObjectKey& key)
{
  dlms::cosem::CosemAttributeDescriptor descriptor;
  descriptor.object = key;
  descriptor.attributeId = 2u;
  return descriptor;
}

dlms::cosem::CosemMethodDescriptor MakeMethod(
  const dlms::cosem::CosemObjectKey& key)
{
  dlms::cosem::CosemMethodDescriptor descriptor;
  descriptor.object = key;
  descriptor.methodId = 1u;
  return descriptor;
}

TEST(DlmsServer, ForwardsGetSetAndActionThroughDispatcher)
{
  dlms::server::ServerContext context;
  dlms::cosem::LogicalDevice logicalDevice(1u, "ld-1");
  const std::shared_ptr<FacadeTestObject> object(new FacadeTestObject());

  ASSERT_EQ(dlms::cosem::CosemStatus::Ok,
            logicalDevice.RegisterObject(object));
  context.SetAssociated(true);
  context.AttachLogicalDevice(&logicalDevice);

  dlms::server::DlmsServer server(context);

  dlms::server::ServerSetRequest setRequest =
    dlms::server::EmptyServerSetRequest();
  setRequest.invokeId = 1u;
  setRequest.descriptor = MakeAttribute(object->Key());
  setRequest.data.push_back(0x22u);

  const dlms::server::ServerSetResponse setResponse =
    server.HandleSet(setRequest);
  EXPECT_EQ(dlms::server::ServerStatus::Ok, setResponse.status);

  dlms::server::ServerGetRequest getRequest =
    dlms::server::EmptyServerGetRequest();
  getRequest.invokeId = 2u;
  getRequest.descriptor = MakeAttribute(object->Key());

  const dlms::server::ServerGetResponse getResponse =
    server.HandleGet(getRequest);
  EXPECT_EQ(dlms::server::ServerStatus::Ok, getResponse.status);
  ASSERT_TRUE(getResponse.hasData);
  ASSERT_EQ(1u, getResponse.data.size());
  EXPECT_EQ(0x22u, getResponse.data[0]);

  dlms::server::ServerActionRequest actionRequest =
    dlms::server::EmptyServerActionRequest();
  actionRequest.invokeId = 3u;
  actionRequest.descriptor = MakeMethod(object->Key());
  actionRequest.parameter.push_back(0x33u);

  const dlms::server::ServerActionResponse actionResponse =
    server.HandleAction(actionRequest);
  EXPECT_EQ(dlms::server::ServerStatus::Ok, actionResponse.status);
  ASSERT_TRUE(actionResponse.hasData);
  ASSERT_EQ(1u, actionResponse.data.size());
  EXPECT_EQ(0x33u, actionResponse.data[0]);
}

TEST(DlmsServer, ObservesContextChanges)
{
  dlms::server::ServerContext context;
  dlms::cosem::LogicalDevice logicalDevice(1u, "ld-1");
  const std::shared_ptr<FacadeTestObject> object(new FacadeTestObject());

  ASSERT_EQ(dlms::cosem::CosemStatus::Ok,
            logicalDevice.RegisterObject(object));
  context.SetAssociated(true);
  context.AttachLogicalDevice(&logicalDevice);

  dlms::server::DlmsServer server(context);
  dlms::server::ServerGetRequest request =
    dlms::server::EmptyServerGetRequest();
  request.invokeId = 4u;
  request.descriptor = MakeAttribute(object->Key());

  EXPECT_EQ(dlms::server::ServerStatus::Ok,
            server.HandleGet(request).status);

  context.SetAssociated(false);

  EXPECT_EQ(dlms::server::ServerStatus::NotAssociated,
            server.HandleGet(request).status);
}

} // namespace
