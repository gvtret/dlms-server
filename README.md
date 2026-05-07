# dlms-server

`dlms-server` is the server-side DLMS/COSEM orchestration layer.

The repository owns the boundary between decoded server-side xDLMS service
requests and the COSEM object model. It does not implement lower protocol
codecs, transport I/O, persistent object storage, or cryptographic primitives.

Phase 0 is documentation-only. The initial implementation plan is:

1. server request and response contracts;
2. GET dispatch to `dlms-cosem`;
3. SET and ACTION dispatch;
4. association/session gate;
5. root integration.

See `docs/` for requirements, API, architecture, test plan, and implementation
plan.

Minimal request-model usage:

```cpp
dlms::server::ServerContext context;
context.SetAssociated(true);
context.AttachLogicalDevice(&logicalDevice);

dlms::server::DlmsServer server(context);
dlms::server::ServerGetResponse response =
  server.HandleGet(getRequest);
```
