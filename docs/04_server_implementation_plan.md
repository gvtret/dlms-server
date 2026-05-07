# dlms-server Implementation Plan

## Phase 0. Documentation

Deliverables:

- requirements;
- API contract;
- architecture diagrams;
- test plan;
- implementation plan;
- empty standalone repository registered as root submodule.

Commit message:

```text
docs(server): define server dispatch layer
```

## Phase 1. Types And Server Context

Deliverables:

- CMake project and GoogleTest harness;
- `ServerStatus`;
- request and response value types;
- `ServerContext`;
- COSEM-to-server status mapper;
- tests for context, type defaults, and status mapping.

Commit message:

```text
feat(server): add dispatch context and service types
```

## Phase 2. GET Dispatch

Deliverables:

- `CosemServiceDispatcher::HandleGet`;
- association and logical-device gates;
- descriptor validation;
- COSEM read dispatch;
- response construction;
- tests for success, invalid, denied, missing, and object-error paths.

Commit message:

```text
feat(server): dispatch get requests to cosem
```

## Phase 3. SET And ACTION Dispatch

Deliverables:

- `CosemServiceDispatcher::HandleSet`;
- `CosemServiceDispatcher::HandleAction`;
- write/invoke dispatch;
- response construction;
- tests for success, invalid, denied, missing, and object-error paths.

Commit message:

```text
feat(server): dispatch set and action requests
```

## Phase 4. Server Facade

Deliverables:

- `DlmsServer` facade;
- forwarding tests;
- README update with minimal usage sample.

Commit message:

```text
feat(server): add minimal server facade
```

## Phase 5. Root Integration

Deliverables:

- root CMake subdirectory wiring;
- root submodule pointer update;
- root test run with `dlms_server_tests`;
- deferred note for full APDU path until server-side xDLMS decoding exists.

Commit message:

```text
build: add dlms-server to root build
```
