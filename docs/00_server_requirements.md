# dlms-server Requirements

## 1. Scope

`dlms-server` dispatches incoming server-side DLMS/COSEM service requests to a
COSEM logical device and returns service response models that can later be
encoded by APDU/xDLMS layers.

The first implementation targets logical-name referencing, public-client
no-security operation, and normal GET/SET/ACTION service dispatch.

The next implementation phase adds an adapter from `dlms-xdlms`
`IXdlmsServerHandler` to this server dispatcher. The adapter is still inside
`dlms-server` because the dependency direction is server -> xDLMS -> APDU,
while xDLMS must not depend back on server or COSEM.

## 2. Standards Alignment Notes

Doc-rag alignment used for Phase 0:

- Association LN models an application association between a client and server
  using logical-name referencing.
- Association LN `object_list` contains visible COSEM objects with `class_id`,
  `version`, `logical_name`, and attribute/method access rights.
- Access-right descriptors include all implemented attributes and methods, with
  authenticated access modes available for protected associations.
- Every COSEM logical device shall expose a logical device name object and the
  current Association object for read access.
- The management logical device shall support public-client no-security
  association and expose SAP Assignment information for logical devices.
- Protected requests and responses must be processed only after the protection
  has been verified and removed by a security layer.

## 3. In Scope

- Server request and response value types.
- Server-side service status model.
- GET normal dispatch to `dlms-cosem::LogicalDevice`.
- SET normal dispatch to `dlms-cosem::LogicalDevice`.
- ACTION normal dispatch to `dlms-cosem::LogicalDevice`.
- Association/session gate for "associated" versus "not associated" checks.
- Access-context forwarding to `dlms-cosem`.
- Response model construction without APDU byte encoding.
- xDLMS server normal GET handler adapter.
- Tests with fake logical devices and fake object implementations.

## 4. Out Of Scope

- TCP, UDP, serial, Wrapper, HDLC, or LLC I/O.
- ACSE BER and xDLMS A-XDR byte encoding.
- Client-side xDLMS request orchestration.
- COSEM object storage implementation.
- Cryptographic primitive implementation.
- Long-running server event loop.
- Block transfer.
- Selective access.
- Short-name referencing.
- xDLMS SET/ACTION adapter until the xDLMS layer exposes stable server-side
  SET/ACTION contracts.

## 5. Functional Requirements

### 5.1 Server Context

The layer shall hold a server context with:

- association state;
- selected logical device;
- access context;
- negotiated max PDU size placeholder;
- conformance placeholder.

The initial implementation shall support a simple "associated" boolean. Full
Association Server state integration is deferred until `dlms-association`
exposes a server-side stable contract.

### 5.2 GET

The layer shall:

- reject invalid GET descriptors before object lookup;
- reject service dispatch when the server context is not associated;
- call `LogicalDevice::ReadAttribute` when associated;
- preserve COSEM access and object errors in response status;
- copy returned xDLMS data bytes only on success.

### 5.3 SET

The layer shall:

- reject invalid SET descriptors before object lookup;
- reject service dispatch when the server context is not associated;
- call `LogicalDevice::WriteAttribute` when associated;
- return a status-only response.

### 5.4 ACTION

The layer shall:

- reject invalid ACTION descriptors before object lookup;
- reject service dispatch when the server context is not associated;
- call `LogicalDevice::InvokeMethod` when associated;
- copy returned action data bytes only on success.

### 5.5 Association View

The layer shall not build Association LN object lists itself. It shall rely on
`dlms-cosem` metadata and object dispatch results. Encoding the Association LN
`object_list` to xDLMS data is a future COSEM object concern.

### 5.6 xDLMS Server GET Adapter

The layer shall:

- implement `dlms::xdlms::IXdlmsServerHandler`;
- translate `dlms::xdlms::GetIndication` to `ServerGetRequest`;
- call `DlmsServer::HandleGet`;
- translate successful server data to `dlms::xdlms::GetResult`;
- translate server access failures to xDLMS data-access-result values;
- translate infrastructure failures to `dlms::xdlms::XdlmsStatus`;
- preserve the xDLMS invoke id supplied by the dispatcher.

## 6. Non-Functional Requirements

- C++11.
- CMake 3.16+.
- GoogleTest.
- Runtime APIs return status codes; no public/runtime exceptions.
- Downward dependencies only.
- No dependency on transport or profile layers in the dispatch module.
- The xDLMS adapter depends only on `dlms-xdlms`, `dlms-cosem`, and local
  server APIs.
- Output buffers remain caller-owned and are changed only on successful
  responses.

## 7. Acceptance Criteria

- Standalone repository builds with MinGW.
- Unit tests cover invalid input, not-associated dispatch, object not found,
  access denied, object errors, and successful GET/SET/ACTION.
- Unit tests cover xDLMS GET adapter success, access-result mapping, and
  infrastructure failure propagation.
- Root integration can add the repository without dependency cycles.
