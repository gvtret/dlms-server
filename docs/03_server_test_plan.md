# dlms-server Test Plan

## 1. Unit Tests

### Phase 1. Types And Context

- default context is not associated;
- default context has public access;
- context accepts a logical-device pointer;
- request helpers preserve invoke id and descriptors;
- response helpers preserve invoke id and status;
- COSEM status mapper handles all first-phase statuses.

### Phase 2. GET Dispatch

- invalid descriptor returns `InvalidArgument`;
- not-associated context returns `NotAssociated`;
- missing logical device returns `NoLogicalDevice`;
- missing object maps to `ObjectNotFound`;
- access denied maps to `AccessDenied`;
- missing attribute maps to `AttributeNotFound`;
- object error maps to `ObjectError`;
- success copies returned data;
- failed GET does not copy partial data into response.

### Phase 3. SET And ACTION Dispatch

- SET follows the same gate checks as GET;
- SET calls `WriteAttribute` only after access checks;
- ACTION follows the same gate checks as GET;
- ACTION calls `InvokeMethod` only after access checks;
- ACTION success copies returned data;
- ACTION failure does not copy partial data into response.

### Phase 4. Server Facade

- `DlmsServer` forwards GET/SET/ACTION to the dispatcher;
- facade does not own the logical device;
- facade preserves context changes.

## 2. Integration Tests

Standalone integration:

- fake COSEM logical device with a simple object;
- public associated context;
- normal GET/SET/ACTION request models pass through dispatcher.

Root integration, after CMake wiring:

- root build includes `dlms_server_tests`;
- existing lower-layer tests remain green;
- no dependency cycles are introduced.

End-to-end integration, deferred:

```text
Wrapper/TCP or fake APDU channel
  -> dlms-association server state
  -> xDLMS request decode
  -> dlms-server
  -> dlms-cosem LogicalDevice
  -> response APDU
```

## 3. Verification Commands

Standalone:

```text
cmake -S . -B build-mingw64 -G "MinGW Makefiles"
cmake --build build-mingw64
ctest --test-dir build-mingw64 --output-on-failure
```

Root:

```text
cmake -S . -B build-mingw64 -G "MinGW Makefiles"
cmake --build build-mingw64
ctest --test-dir build-mingw64 --output-on-failure
```
