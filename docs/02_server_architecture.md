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

Deferred dependencies:

- `dlms-xdlms` for server-side service request models when stable;
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

## 7. Class Interaction

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

  CosemServiceDispatcher --> ServerContext
  CosemServiceDispatcher --> ServerStatusMapper
  CosemServiceDispatcher --> ResponseFactory
  ServerContext --> LogicalDevice
```

## 8. Error Model

The layer returns `ServerStatus` in every service response. Runtime paths do
not throw exceptions. COSEM object errors are mapped deterministically to
server statuses.

Output data is committed to a response only when the underlying operation
returns success.

## 9. Root Integration Strategy

Root integration shall initially add the submodule and later add
`add_subdirectory(lib/dlms-server)` after the repository has a stable CMake
target. End-to-end APDU tests are deferred until request/response encoding
contracts are stable.
