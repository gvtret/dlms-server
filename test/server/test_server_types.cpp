#include "dlms/server/server.hpp"

#include <gtest/gtest.h>

namespace {

TEST(ServerTypes, EmptyRequestsClearFields)
{
  const dlms::server::ServerGetRequest getRequest =
    dlms::server::EmptyServerGetRequest();
  const dlms::server::ServerSetRequest setRequest =
    dlms::server::EmptyServerSetRequest();
  const dlms::server::ServerActionRequest actionRequest =
    dlms::server::EmptyServerActionRequest();

  EXPECT_EQ(0u, getRequest.invokeId);
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            dlms::cosem::ValidateAttributeDescriptor(getRequest.descriptor));
  EXPECT_EQ(0u, setRequest.invokeId);
  EXPECT_TRUE(setRequest.data.empty());
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            dlms::cosem::ValidateAttributeDescriptor(setRequest.descriptor));
  EXPECT_EQ(0u, actionRequest.invokeId);
  EXPECT_TRUE(actionRequest.parameter.empty());
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            dlms::cosem::ValidateMethodDescriptor(actionRequest.descriptor));
}

TEST(ServerTypes, ResponseHelpersPreserveInvokeStatusAndData)
{
  dlms::cosem::CosemByteBuffer data;
  data.push_back(0x01u);
  data.push_back(0x02u);

  const dlms::server::ServerGetResponse getError =
    dlms::server::MakeServerGetResponse(
      3u, dlms::server::ServerStatus::AccessDenied);
  const dlms::server::ServerGetResponse getSuccess =
    dlms::server::MakeServerGetDataResponse(4u, data);
  const dlms::server::ServerSetResponse setResponse =
    dlms::server::MakeServerSetResponse(
      5u, dlms::server::ServerStatus::Ok);
  const dlms::server::ServerActionResponse actionError =
    dlms::server::MakeServerActionResponse(
      6u, dlms::server::ServerStatus::MethodNotFound);
  const dlms::server::ServerActionResponse actionSuccess =
    dlms::server::MakeServerActionDataResponse(7u, data);

  EXPECT_EQ(3u, getError.invokeId);
  EXPECT_EQ(dlms::server::ServerStatus::AccessDenied, getError.status);
  EXPECT_FALSE(getError.hasData);
  EXPECT_TRUE(getError.data.empty());

  EXPECT_EQ(4u, getSuccess.invokeId);
  EXPECT_EQ(dlms::server::ServerStatus::Ok, getSuccess.status);
  EXPECT_TRUE(getSuccess.hasData);
  EXPECT_EQ(data, getSuccess.data);

  EXPECT_EQ(5u, setResponse.invokeId);
  EXPECT_EQ(dlms::server::ServerStatus::Ok, setResponse.status);

  EXPECT_EQ(6u, actionError.invokeId);
  EXPECT_EQ(dlms::server::ServerStatus::MethodNotFound, actionError.status);
  EXPECT_FALSE(actionError.hasData);
  EXPECT_TRUE(actionError.data.empty());

  EXPECT_EQ(7u, actionSuccess.invokeId);
  EXPECT_EQ(dlms::server::ServerStatus::Ok, actionSuccess.status);
  EXPECT_TRUE(actionSuccess.hasData);
  EXPECT_EQ(data, actionSuccess.data);
}

TEST(ServerStatus, NamesStableValues)
{
  EXPECT_STREQ(
    "NotAssociated",
    dlms::server::ServerStatusName(
      dlms::server::ServerStatus::NotAssociated));
}

TEST(ServerStatus, MapsCosemStatusValues)
{
  EXPECT_EQ(dlms::server::ServerStatus::Ok,
            dlms::server::MapCosemStatus(
              dlms::cosem::CosemStatus::Ok));
  EXPECT_EQ(dlms::server::ServerStatus::InvalidArgument,
            dlms::server::MapCosemStatus(
              dlms::cosem::CosemStatus::InvalidArgument));
  EXPECT_EQ(dlms::server::ServerStatus::ObjectNotFound,
            dlms::server::MapCosemStatus(
              dlms::cosem::CosemStatus::ObjectNotFound));
  EXPECT_EQ(dlms::server::ServerStatus::AccessDenied,
            dlms::server::MapCosemStatus(
              dlms::cosem::CosemStatus::AccessDenied));
  EXPECT_EQ(dlms::server::ServerStatus::AttributeNotFound,
            dlms::server::MapCosemStatus(
              dlms::cosem::CosemStatus::AttributeNotFound));
  EXPECT_EQ(dlms::server::ServerStatus::MethodNotFound,
            dlms::server::MapCosemStatus(
              dlms::cosem::CosemStatus::MethodNotFound));
  EXPECT_EQ(dlms::server::ServerStatus::ObjectError,
            dlms::server::MapCosemStatus(
              dlms::cosem::CosemStatus::ObjectError));
  EXPECT_EQ(dlms::server::ServerStatus::UnsupportedFeature,
            dlms::server::MapCosemStatus(
              dlms::cosem::CosemStatus::UnsupportedFeature));
  EXPECT_EQ(dlms::server::ServerStatus::InternalError,
            dlms::server::MapCosemStatus(
              dlms::cosem::CosemStatus::DuplicateObject));
  EXPECT_EQ(dlms::server::ServerStatus::InternalError,
            dlms::server::MapCosemStatus(
              dlms::cosem::CosemStatus::OutputBufferTooSmall));
  EXPECT_EQ(dlms::server::ServerStatus::InternalError,
            dlms::server::MapCosemStatus(
              dlms::cosem::CosemStatus::InternalError));
}

} // namespace
