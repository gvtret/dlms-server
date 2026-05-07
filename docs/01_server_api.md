# dlms-server API

## 1. Public Headers

Planned headers:

```text
include/dlms/server/server_status.hpp
include/dlms/server/server_types.hpp
include/dlms/server/server_context.hpp
include/dlms/server/service_dispatcher.hpp
include/dlms/server/dlms_server.hpp
```

No C ABI is planned for the first implementation.

## 2. Status Contract

`ServerStatus` shall contain:

- `Ok`
- `InvalidArgument`
- `NotAssociated`
- `NoLogicalDevice`
- `ObjectNotFound`
- `AccessDenied`
- `AttributeNotFound`
- `MethodNotFound`
- `ObjectError`
- `UnsupportedFeature`
- `EncodeRequired`
- `InternalError`

The dispatcher maps `dlms-cosem::CosemStatus` to `ServerStatus` without losing
meaning where possible.

## 3. Request Types

`ServerGetRequest`:

- `invokeId`;
- `CosemAttributeDescriptor descriptor`.

`ServerSetRequest`:

- `invokeId`;
- `CosemAttributeDescriptor descriptor`;
- `CosemByteBuffer data`.

`ServerActionRequest`:

- `invokeId`;
- `CosemMethodDescriptor descriptor`;
- `CosemByteBuffer parameter`.

The first implementation uses request models, not APDU byte views.

## 4. Response Types

`ServerGetResponse`:

- `invokeId`;
- `ServerStatus status`;
- `bool hasData`;
- `CosemByteBuffer data`.

`ServerSetResponse`:

- `invokeId`;
- `ServerStatus status`.

`ServerActionResponse`:

- `invokeId`;
- `ServerStatus status`;
- `bool hasData`;
- `CosemByteBuffer data`.

## 5. Server Context

```cpp
class ServerContext
{
public:
  void SetAssociated(bool associated);
  bool IsAssociated() const;

  void SetAccessContext(const dlms::cosem::CosemAccessContext& context);
  dlms::cosem::CosemAccessContext AccessContext() const;

  void AttachLogicalDevice(dlms::cosem::LogicalDevice* logicalDevice);
  dlms::cosem::LogicalDevice* LogicalDevice();
  const dlms::cosem::LogicalDevice* LogicalDevice() const;
};
```

The context does not own the logical device.

## 6. Service Dispatcher

```cpp
class CosemServiceDispatcher
{
public:
  explicit CosemServiceDispatcher(ServerContext& context);

  ServerGetResponse HandleGet(const ServerGetRequest& request) const;
  ServerSetResponse HandleSet(const ServerSetRequest& request);
  ServerActionResponse HandleAction(const ServerActionRequest& request);
};
```

## 7. Server Facade

`DlmsServer` is a thin facade planned after the dispatcher is stable:

```cpp
class DlmsServer
{
public:
  explicit DlmsServer(ServerContext& context);

  ServerGetResponse HandleGet(const ServerGetRequest& request);
  ServerSetResponse HandleSet(const ServerSetRequest& request);
  ServerActionResponse HandleAction(const ServerActionRequest& request);
};
```

It shall not own an event loop in the first implementation.

## 8. Module Diagram

```mermaid
classDiagram
  class ServerContext {
    -bool associated
    -CosemAccessContext access
    -LogicalDevice* logicalDevice
    +SetAssociated()
    +IsAssociated()
    +AttachLogicalDevice()
  }

  class CosemServiceDispatcher {
    -ServerContext& context
    +HandleGet()
    +HandleSet()
    +HandleAction()
  }

  class DlmsServer {
    -CosemServiceDispatcher dispatcher
    +HandleGet()
    +HandleSet()
    +HandleAction()
  }

  class LogicalDevice {
    +ReadAttribute()
    +WriteAttribute()
    +InvokeMethod()
  }

  DlmsServer --> CosemServiceDispatcher
  CosemServiceDispatcher --> ServerContext
  ServerContext --> LogicalDevice
```
