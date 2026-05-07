# dlms-server Architecture

## 1. Layer Position

```mermaid
flowchart TD
  App["Meter application"]
  Server["dlms-server"]
  Association["dlms-association"]
  XDlms["dlms-xdlms"]
  Cosem["dlms-cosem"]
  Security["Future dlms-security"]
  Apdu["dlms-apdu"]
  Profile["dlms-profile"]

  App --> Server
  Server --> Association
  Server --> XDlms
  Server --> Cosem
  Server -. security context .-> Security
  Association --> Apdu
  XDlms --> Apdu
  Association --> Profile
  XDlms --> Profile
```

`dlms-server` sits above APDU/profile/association concerns and above the COSEM
object model. The first implementation deliberately starts with decoded request
models, so the dispatcher can be tested without a transport stack.

## 2. Dependency Rules

Allowed first-phase dependencies:

- `dlms-cosem`.

Active adapter dependencies:

- `dlms-xdlms` for server-side normal GET indication and handler contracts.

Deferred dependencies:

- `dlms-association` for server association state when stable;
- `dlms-apdu` for APDU response encoding;
- `dlms-security` for protected association contexts.

Forbidden dependencies:

- `dlms-transport`;
- direct `dlms-hdlc`, `dlms-llc`, or `dlms-wrapper`;
- application-specific object storage.

## 3. GET Flow

```mermaid
sequenceDiagram
  participant Caller as xDLMS/APDU server boundary
  participant Dispatcher as CosemServiceDispatcher
  participant Context as ServerContext
  participant Device as LogicalDevice
  participant Object as ICosemObject

  Caller->>Dispatcher: HandleGet(request)
  Dispatcher->>Context: IsAssociated()
  Dispatcher->>Context: LogicalDevice()
  Dispatcher->>Device: ReadAttribute(descriptor, accessContext)
  Device->>Object: ReadAttribute(attributeId)
  Object-->>Device: CosemStatus + data
  Device-->>Dispatcher: CosemStatus + data
  Dispatcher-->>Caller: ServerGetResponse
```

## 4. SET Flow

```mermaid
sequenceDiagram
  participant Caller as xDLMS/APDU server boundary
  participant Dispatcher as CosemServiceDispatcher
  participant Context as ServerContext
  participant Device as LogicalDevice

  Caller->>Dispatcher: HandleSet(request)
  Dispatcher->>Context: IsAssociated()
  Dispatcher->>Context: LogicalDevice()
  Dispatcher->>Device: WriteAttribute(descriptor, accessContext, data)
  Device-->>Dispatcher: CosemStatus
  Dispatcher-->>Caller: ServerSetResponse
```

## 5. ACTION Flow

```mermaid
sequenceDiagram
  participant Caller as xDLMS/APDU server boundary
  participant Dispatcher as CosemServiceDispatcher
  participant Context as ServerContext
  participant Device as LogicalDevice

  Caller->>Dispatcher: HandleAction(request)
  Dispatcher->>Context: IsAssociated()
  Dispatcher->>Context: LogicalDevice()
  Dispatcher->>Device: InvokeMethod(descriptor, accessContext, parameter)
  Device-->>Dispatcher: CosemStatus + data
  Dispatcher-->>Caller: ServerActionResponse
```

## 6. State Model

The first state model is intentionally small:

```mermaid
stateDiagram-v2
  [*] --> Detached
  Detached --> Ready: AttachLogicalDevice
  Ready --> Associated: SetAssociated(true)
  Associated --> Ready: SetAssociated(false)
  Ready --> Detached: AttachLogicalDevice(null)
```

Requests are accepted only in `Associated` with a logical device attached.

## 7. xDLMS GET Adapter Flow

```mermaid
sequenceDiagram
  participant XD as XdlmsServerDispatcher
  participant Adapter as XdlmsServerAdapter
  participant Facade as DlmsServer
  participant Dispatch as CosemServiceDispatcher
  participant Device as LogicalDevice

  XD->>Adapter: HandleGet(GetIndication)
  Adapter->>Adapter: Map descriptor to ServerGetRequest
  Adapter->>Facade: HandleGet(request)
  Facade->>Dispatch: HandleGet(request)
  Dispatch->>Device: ReadAttribute(descriptor, access)
  Device-->>Dispatch: CosemStatus + data
  Dispatch-->>Facade: ServerGetResponse
  Facade-->>Adapter: ServerGetResponse
  Adapter-->>XD: XdlmsStatus + GetResult
```

`XdlmsServerAdapter` is the only `dlms-server` module that knows about
`dlms-xdlms`. The core dispatcher remains testable with server request models.

## 8. Class Interaction

```mermaid
classDiagram
  class ServerContext {
    -bool associated
    -LogicalDevice* logicalDevice
    -CosemAccessContext accessContext
  }

  class CosemServiceDispatcher {
    +HandleGet()
    +HandleSet()
    +HandleAction()
  }

  class DlmsServer {
    +HandleGet()
    +HandleSet()
    +HandleAction()
  }

  class ServerStatusMapper {
    +MapCosemStatus()
  }

  class ResponseFactory {
    +MakeGetResponse()
    +MakeSetResponse()
    +MakeActionResponse()
  }

  class LogicalDevice {
    +ReadAttribute()
    +WriteAttribute()
    +InvokeMethod()
  }

  class XdlmsServerAdapter {
    +HandleGet()
  }

  class IXdlmsServerHandler {
    +HandleGet()
  }

  XdlmsServerAdapter ..|> IXdlmsServerHandler
  XdlmsServerAdapter --> DlmsServer
  DlmsServer --> CosemServiceDispatcher
  CosemServiceDispatcher --> ServerContext
  CosemServiceDispatcher --> ServerStatusMapper
  CosemServiceDispatcher --> ResponseFactory
  ServerContext --> LogicalDevice
```

## 9. Error Model

The layer returns `ServerStatus` in every service response. Runtime paths do
not throw exceptions. COSEM object errors are mapped deterministically to
server statuses.

Output data is committed to a response only when the underlying operation
returns success.

The xDLMS adapter maps object access failures into data-access-result response
models and maps infrastructure failures, such as missing association or missing
logical device, into `XdlmsStatus` values.

## 10. Root Integration Strategy

Root integration keeps `add_subdirectory(lib/dlms-xdlms)` before
`add_subdirectory(lib/dlms-server)` so the adapter can link to the xDLMS
contract without creating a cycle. End-to-end APDU tests are deferred until
request/response encoding contracts are stable.
