# Detailed Design of ConfigDaemon

## Table of Contents
- [1. Introduction](#1-introduction)
- [2. Description of Interfaces](#2-description-of-interfaces)
  - [2.1 Internal Process Communication Interfaces](#21-internal-process-communication-interfaces)
  - [2.3 C++ Interfaces](#23-c--interfaces)
- [3. Static Architecture](#3-static-architecture)
  - [3.1 Central Database](#31-central-database)
  - [3.2 IPC Interface](#32-ipc-interface)
    - [3.2.1 mw::com Shared-Memory Allocation](#321-mwcom-shared-memory-allocation)
    - [3.2.2 IPC Serialization](#322-ipc-serialization)
  - [3.3 Plugin Mechanism](#33-plugin-mechanism)
  - [3.4 Shared Resources and Fault Event Reporter](#34-shared-resources-and-fault-event-reporter)
- [4. Dynamic Architecture](#4-dynamic-architecture)
  - [4.1 Execution Stages](#41-execution-stages)
  - [4.2 IPC communication](#42-ipc-communication)
- [5. External Dependencies](#5-external-dependencies)
- [6. Security Policy](#6-security-policy)

## 1. Introduction

Embedded software typically requires vehicle-specific configuration parameters, such as geometry and geographical region. `ConfigDaemon` application and [`ConfigProvider` library](../../common/ConfigProvider/detailed_design/README.md) together implement a configuration management approach that centralizes storage, verification, and updates, and provides configuration data to client applications.

The diagram below demonstrates the composition principle of `ConfigDaemon` and User Adaptive Applications. The entire communication path between the business logic using a parameter and a parameter stored is encapsulated by `ConfigDaemon` and `ConfigProvider` library. Thus, a user is not directly confronted with the kind of IPC implementation or parameter representation and handling by the interface `IInternalConfigProvider`.

<details>
<summary>Click to expand SW component view</summary>

<img src="./component_diagrams/svg/component_view.svg" alt="W component view" width="800">

</details>

The design uses a centralized on-target database for all parameters used in an ECU. Clients have read-only access to the database through a generic interface. This supports use cases that rely on flexible [runtime dependencies](./use_cases/README.md#go-for-runtime-dependencies-instead-of-compile-dependencies) rather than static, build-time bindings: an Adaptive Application can access parameters via a generic key–value interface. Compared with statically defined interfaces for each parameter (which must be resolved at build time), this approach reduces build time and avoids architecture model (e.g. FRANCA) changes and rebuilds when parameters change.

Updates to the database are performed exclusively through plugins, which are extensible.

## 2. Description of Interfaces

### 2.1 Internal Process Communication Interfaces

All clients use the generic interface `InternalConfigProvider` to get or subscribe to parameters offered by `ConfigDaemon`.
Find more in [Common InternalConfigProvider description](./README.md#32-ipc-interface).

The interface `ConfigCalibration` provides write access to parameter sets for a `User Adaptive Application` in a generic manner using a key-value principle with `parameter_set_name` as a key and `parameter_set` as a value. See the [fidl](../adaptive_model/interfaces/ConfigCalibration.fidl) file for the interface definition.
`ConfigDaemon` exposes it via mw::com. Therefore, a proxy will be generated on the application side.

The class diagram below depict the relationship between the classes that are used in creating the `ConfigCalibration` service:

<details>
<summary>Click to expand config calibration service skeleton</summary>

<img alt="config calibration service skeleton" src="https://www.plantuml.com/plantuml/proxy?src=https://raw.githubusercontent.com/eclipse-score/baselibs/refs/heads/main/score/config_management/config_daemon/detailed_design/calibration/assets/calibration_plugin_class_diagram.puml">

</details>

### 2.3 C++ Interfaces
C++ interface usage of `ConfigDaemon` is summarized in Chapter [External dependencies](#7-external-dependencies).

## 3. Static Architecture

A general static overview of `ConfigDaemon`:
The core application entry point is the class `ConfigDaemon`. It owns central data storage, offers generic interfaces to client applications, and manages plugins.
It mainly consists of four parts:

1. `Factory`: A factory pattern to improve testability.

2. `ParameterSetCollection`: Encapsulates the central database and stores `ParameterSet` instances as key–value pairs.

3. `InternalConfigProviderService`: The generic service that client applications use to obtain read-only access to the `ParameterSetCollection`.

4. `Plugin`: Components that update the `ParameterSetCollection` according to specific logic.

<details>
<summary>ConfigDaemon Static Architecture</summary>

<img src="./class_diagrams/generated/svg/config_daemon_sa.svg" alt="ConfigDaemon Static Architecture" width="1200">

</details>

### 3.1 Central Database

`Parameter` represents a configuration value and contains its content.
Some `Parameter` instances that are closely related are bundled into a `ParameterSet`. A `ParameterSet` is the smallest unit for reading or updating the central database. Each `ParameterSet` also contains a `ParameterSetQualifier`, which indicates the qualification status of the `ParameterSet`. `ParameterSetCollection` encapsulates the in-memory database and provides interfaces to read and update `ParameterSet` instances.

<details>
<summary>ConfigDaemon DataBase Static Architecture</summary>

<img src="./class_diagrams/generated/svg/config_daemon_data_base_sa.svg" alt="ConfigDaemon DataBase Static Architecture" width="800">

</details>


### 3.2 IPC Interface

`InternalConfigProviderService` binds the underlying IPC technology (e.g. `mw::com`) with composition of class `InternalConfigProviderSkeleton`, which does static polymorphism to template class `mw::com::AsSkeleton` from `mw::com` library. It also owns `InternalConfigProviderServiceReactor`, which implements business logic to retrieve `ParameterSet` instances from the `ParameterSetCollection` via the `IReadOnlyParameterSetCollection` interface.

Besides, `InternalConfigProviderService` also updates the `InitialQualifierState`, see [Details for InitialQualifierState](./plugins/coding/README.md#315-output-interfaces).
The most critical state is `Qualified`, when `InitialQualifierState` assumes this value, applications consuming the data are allowed to assume parameters are safe to be used. As stated in our Safety Goal, we shall never provide corrupted or disqualified data when `InitialQualifierState`=`Qualified`

`ConfigDaemon` uses `Factory` to create `InternalConfigProviderService` and manages it via `mw::service::ProvidedServicesContainer` (from the `mw::service` library), which starts a service as soon as it is added. If the service cannot be started, it is not added and `ConfigDaemon` cannot continue; see the risk described in [4.1 Execution Stages](#41-execution-stages). Client applications use `ConfigProvider` to obtain or subscribe to `ParameterSet` instances from `ConfigDaemon`.

<details>
<summary>InternalConfigProviderService Static Architecture</summary>

<img src="./class_diagrams/generated/svg/config_daemon_icp_sa.svg" alt="InternalConfigProviderService Static Architecture" width="800">

</details>

#### 3.2.1 mw::com Shared-Memory Allocation

`mw::com` IPC is backed by shared memory (SHM): for a method with dynamically-sized arguments, such as `get_parameter_set` (whose `ParameterSetName` and `ParameterSetContent` arguments vary in length), the proxy allocates SHM for both the method's argument and its return value when it is created. Within that allocation, the actual buffer capacity is only reserved lazily, the first time the method is used - via `ReserveOnce()` - and whatever size is reserved at that point can never be grown afterwards; later calls only `Clear()` and reuse the same buffer. On the proxy side, `InternalConfigProvider::GetParameterSet()` reserves the `ParameterSetName` argument buffer via `ReserveOnce(sizing.max_name_length)` on the first call. On the server side, `InternalConfigProviderService`'s method handler reserves the return-value buffer via `result.ReserveOnce(call_max_payload_size)` on the first call it handles. `ConfigProvider` determines the sizes used for these reservations - `sizing.max_name_length` and `sizing.max_payload_size` - from `buffer_size_report.json` (see [3.2.2](#322-ipc-serialization) below).

- `max_name_length`: the length, in bytes (excluding the null terminator), of the longest `ParameterSetName` among the parameter sets `ConfigProvider` was configured with (see `initial_parameter_set_name_list` in [`buffer_size_report.h`](../../config_provider/code/config_provider/factory/buffer_size_report.h)). It bounds the proxy-side SHM argument buffer used to pass the requested parameter set's name to `ConfigDaemon`. Since each client application configures `ConfigProvider` with its own subset of parameter set names, this value is computed independently per client and can differ between applications.
- `call_max_payload_size`: the worst-case serialized size, in bytes, of a `ParameterSet`'s content, as computed at build time by `buffer_size_calculator` (see [3.2.2](#322-ipc-serialization) below) and reported as `max_payload_size` in `buffer_size_report.json`. It bounds the server-side SHM return-value buffer `InternalConfigProviderService` reserves to hold the `ParameterSetContent` it writes back to the caller, and is passed into the method call by the proxy (as the `call_max_payload_size` argument) so the server knows how much space it has to write into. Like `max_name_length`, this is derived from the calling client's own `initial_parameter_set_name_list`, so it also varies per client application rather than being a single system-wide constant.

Two runtime checks then ensure a single oversized call can never overrun either fixed-size reservation:

1. **Proxy side** (`InternalConfigProvider::GetParameterSet()`): before allocating/populating the method's `ParameterSetName` argument, it checks the requested `set_name` length against `sizing.max_name_length` and fails fast (`ConfigProviderError::kParameterSetNameTooLong`) instead of writing into the SHM pool.
2. **Server side** (`InternalConfigProviderService::Start()`'s method handler): receives the caller's reserved payload size as the `call_max_payload_size` method argument (set by the proxy from the same `shm_sizing`), and compares the parameter set content's size against it before writing the return value; if it would exceed the pool, the handler returns a defined `kParameterSetPayloadTooLargeContent` sentinel instead of overflowing, which the proxy maps to `ConfigProviderError::kParameterSetPayloadTooLarge`.

#### 3.2.2 IPC Serialization

`ParameterSet` instances exchanged over the `InternalConfigProvider` and `ConfigCalibration` interfaces are serialized as JSON. Since `mw::com` is shared-memory-based, the maximum serialized buffer size of a `ParameterSet` must be known at build time and never exceeded at runtime, which in turn requires the serialized size of each value to be a pure function of its compile-time-known data type and array dimensions rather than of its concrete runtime value.

For example, a `uint8` parameter is always budgeted its worst-case width (3 characters, for `255`) regardless of its actual value, so the buffer size of a whole `ParameterSet` becomes the sum of these fixed per-type budgets over its layout, computable at build time without knowing any value. See the [Parameter Set JSON Format Contract](../../docs/ipc_encoding_concepts/parameter_set_json_format_contract.md) for the full size formula and the exact rules this budget relies on.

Three components work together to put this into practice:

- **`type_mapping.json`**: a build-time input file, authored per component, that declares the layout of every `ParameterSet` the component uses - i.e. for each parameter, its `data_type` and `dimensions`. See this [example](../../tools/buffer_size_calculator/example_type_mapping.json).
- **`buffer_size_calculator`**: a Bazel-generated build-time tool that reads one or more `type_mapping.json` files and computes the worst-case serialized buffer size of each declared `ParameterSet`. See its [README](../../tools/buffer_size_calculator/README.md).
- **`ParameterSetJsonSerializer`**: the runtime component that serializes/deserializes `ParameterSet` instances to/from the JSON wire format described above; its output for any value is contractually guaranteed to never exceed the budget computed by `buffer_size_calculator` for the same layout. See [`ParameterSetJsonWriter` vs. Hex Encoding: Safety, Robustness, and Effort Analysis](../../docs/ipc_encoding_concepts/parameter_set_json_writer_analysis.md) for the design rationale.

Computing a build-time budget does not, by itself, guarantee that the shared-memory slot sized to it can never be overrun at runtime - a mismatch between what the size calculator assumes and what the serializer actually writes could still cause an overflow. This is prevented by two independent safeguards: (1) the serializer and the build-time size calculator share the exact same byte-layout rules (number formatting, escaping, array framing), so the computed budget always describes what is actually written, and (2) `ParameterSetJsonSerializer` additionally guards every write at runtime against the remaining buffer space and fails safe - with a defined, detectable error, never a partial or overflowing write - if a mismatch were ever to occur. See [Section 6.1](../../docs/ipc_encoding_concepts/parameter_set_json_writer_analysis.md#61-valid-json-is-not-the-same-as-safe-bounded-serialization) for the full argument.


### 3.3 Plugin Mechanism

`Plugin` components populate or modify the contents of the `ParameterSetCollection`. One `Plugin` is processing parameters of one kind, e.g. either coding parameters or calibration parameters.

`ConfigDaemon` creates plugins indirectly: it uses `Factory` to instantiate a `PluginCollector`, which holds a collection of `PluginCreator` instances. The collectors trigger creators to instantiate the actual `Plugin` objects used by `ConfigDaemon`. This design simplifies adding or removing plugins by changing `PluginCollector` implementations.

`ConfigDaemon` manages plugins through two primary APIs. During initialization, `ConfigDaemon` calls the `Initialize()` function of each plugin to instantiate internal components in a non-blocking manner. During runtime, `ConfigDaemon` calls the `Run()` function of each plugin, passing handles to the `ParameterSetCollection` and to `InternalConfigProviderService` so plugins can update the database and notify clients about parameter and qualifier changes.

<details>
<summary>Plugin Static Architecture</summary>

<img src="./class_diagrams/generated/svg/config_daemon_plugin_sa.svg" alt="Plugin Static Architecture" width="800">

</details>

### 3.4 Shared Resources and Fault Event Reporter

Sometimes `Plugin`s inside `ConfigDaemon` need to share some resource, for example proxy instance or service instance. In the current design of `ConfigDaemon`, these resources need to be instantiated by `ConfigDaemon` and distributed to `Plugin`s via parameters in `Run` function.

`FaultEventReporter` is one of such resource. It is used to report Hw/Sw DTC to Degradation Handler via `FaultEventInterfaceRPort`. `FaultEventReporter` class takes care of managing the request from both plugins and forwards the request to `FaultEventProxy` class which in turn reports the DTC to Degradation Handler.

<details>
<summary>FaultEventReporter Static Architecture</summary>

<img src="./class_diagrams/generated/svg/fault_event_reporter_sa.svg" alt="FaultEventReporter Static Architecture" width="800">

</details>

## 4. Dynamic Architecture

### 4.1 Execution Stages

`ConfigDaemon`'s lifetime can be divided into three stages.

1. During construction, the `main` function creates a `Factory` and passes it to the `ConfigDaemon` constructor. Inside the constructor, `ConfigDaemon` uses the factory to create the `ParameterSetCollection`.
2. The `main` function then runs `ConfigDaemon` via the `LifecycleManager` by calling its `Initialize()` function. At this stage, `ConfigDaemon` creates plugins via the factory (as described in 3.3) and triggers non-blocking `Initialize()` calls on them.
3. After successful initialization, `LifecycleManager` calls the `Run()` method of `ConfigDaemon` with a `stop_token` argument. `ConfigDaemon` loads the `ParameterSetCollection` from storage and creates `InternalConfigProviderService` via the factory, which starts and offers it. It then executes the `Run()` functions of the plugins (created during initialization) sequentially. The process blocks indefinitely; `LifecycleManager` can unblock execution using the `stop_token` to enter shutdown sequence. Before returning to `main`, `ConfigDaemon` stops offering `InternalConfigProviderService` and calls the `Deinitialize()` function of the plugins.

If `InternalConfigProviderService` cannot be created or started (for example because offering the service fails), the container returned by the factory is empty. `ConfigDaemon::Run()` then logs an error and returns a failure code before the `Run()` function of any plugin is called. This fail-fast behavior is a deliberate design decision: `InternalConfigProviderService` is mandatory for `ConfigDaemon`, which does not continue without it.

**Risk:** because no plugin is run in this case, the `CodingPlugin` is not executed and coding of the ECU is not possible for as long as the service cannot be offered. The failure ends the whole `ConfigDaemon` process, so the calibration plugin is not run either. This risk is accepted as part of the design decision above.

<details>
<summary>Execution Sequence Diagram</summary>

<img src="./sequence_diagrams/generated/svg/execution_sequence_diagram.svg" alt="Execution Sequence Diagram" width="800">

</details>


### 4.2 IPC communication
A client application can request a `ParameterSet` with a timeout in a polling manner. `ConfigProvider` returns the cached value if available; otherwise, it uses the `InternalConfigProviderProxy` to perform IPC with `ConfigDaemon` and retrieve the desired `ParameterSet` from the database.

<details>
<summary>InternalConfigProviderService Get Sequence Diagram</summary>

<img src="./sequence_diagrams/generated/svg/cfgd_icp_get_sequence_diagram.svg" alt="InternalConfigProviderService Get Sequence Diagram" width="800">

</details>

A client application can also subscribe to a named `ParameterSet` with a callback. If a plugin at the `ConfigDaemon` side updates the `ParameterSet`, an IPC event is sent to `ConfigProvider`. Before invoking the registered callback, `ConfigProvider` instructs the `InternalConfigProviderProxy` to poll the updated `ParameterSet` and update its cache.

<details>
<summary>InternalConfigProviderService Get Sequence Diagram</summary>

<img src="./sequence_diagrams/generated/svg/cfgd_icp_subscribe_sequence_diagram.svg" alt="InternalConfigProviderService Subscribe Sequence Diagram" width="800">

</details>

## 5. External Dependencies
<!-- TODO: Update the score lib path -->

## 6. Security Policy

A security policy (secpol) file defines the OS-level capabilities and IPC connection permissions granted to `ConfigDaemon` by the QNX security framework. It follows the principle of least privilege: only the abilities actually required for the process to function are granted.

The secpol file is located at [`config_daemon.secpol`](../../../../../ecu/xpad/abc-lmn/config/ipnext/isoc/config_management/config_daemon/config_daemon.secpol).

### Abilities

| Ability | Why it is needed |
|---|---|
| `nonroot` | By default, a process gets the abilities specified in an allow statement only if it is root. To grant them when the process is non-root as well, the nonroot option is required. |
| `amsr/safe-process:asil_b` | MICROSAR Adaptive (Vector stack) safety-process classification. It marks `ConfigDaemon` as ASIL-B for Adaptive safety/runtime integration and corresponding supervision behavior. |
| `map_fixed` | Required by the OS to load and run the application itself. Without this ability the process cannot start. This is a base policy requirement for all QNX applications (QNX 7.1 and QNX 8.0). |
| `pathspace` | Provides the process access to the `procnto` pathname prefix space. Required for `mw::log` (DLT logging), to register named IPC server endpoints, and to attach the message passing channel (`/mw_com/message_passing/logging.CfgD.1012`). |
| `prot_exec` | Required by the dynamic linker to map shared libraries with execute permission during process startup. |
| `public_channel` | Allows the process to create public channels, a QNX mechanism for inter-process communication (IPC). Required to attach the DLT logging channel (`/mw_com/message_passing/logging.CfgD.1012`). |
| `xthread_threadctl:11` | Required for calling `pthread_setname_np()` (thread naming), which on QNX uses the ThreadCtl(_NTO_TCTL_NAME) kernel call with subcommand value 11. ConfigDaemon's dependencies (mw::com, score::concurrency, mw::log) spawn threads that call `pthread_setname_np()`. Without this ability, each naming call produces a secpol violation. The `:11` subrange restricts permission to only the thread-naming subcommand, following the principle of least privilege. |

### Channel connect permissions

`ConfigDaemon` is permitted to initiate IPC connections to the following processes:

| Process | Reason |
|---|---|
| `Bs_t` | Required by the coding plugin to call BasicSecurity services used during coding checks: the `crypto` proxy (`VerifyNcdSignature`) for NCD signature/integrity validation and additional BasicSecurity-backed services (e.g., secure feature registry and secure VIN related proxies). |
| `CalibrationServer_t` | CalibrationServer is a client of both services provided by ConfigD: `ConfigCalibration` and `InternalConfigProvider`, and ConfigD connects to CalibrationServer for IPC transport. |
| `datarouter_t` | Required to use the DLT `mw::log` framework for logging. |
| `devb_loopback_t` | Only on development and ITF targets, where the [`install_writable_overlay`](../../../../../ecu/xpad/abc-lmn/config/ipnext/isoc/os/fs/system_qdrive/scripts/install_writable_overlay) script mounts a writable overlay over `/opt` via `devb-loopback` so tests can override `./etc/param_set_mapping.json` and `./etc/default_parameter_set_collection.json`. `ConfigDaemon` then needs a channel to `devb-loopback` to read them. |
| `devb_ufs_qualcomm_t` | Required for ConfigD persistent-storage functionality on Qualcomm UFS-backed partitions: storing/verifying the parameter-set collection state and hash (`/persistent/trusted/ConfigDaemon/...`), reading flash counter (`/persistent/untrusted/ConfigProvider/flash_counter`), and flushing KVS data (`/persistent/ConfigDaemon/nvmblock/key_value_storage`). |
| `devb_virtio_t` | Implementation detail of qemu testing. |
| `DiagnosticManagerSwc_t` | Diagnostic Manager — used for coding diagnostic jobs and DTC reporting |
| `execution_manager_t` | AUTOSAR Execution Manager — manages process lifecycle |
| `fsevmgr_t` | For managing file system events (needed for LoLa service discovery). |
| `IPCServiceDiscoveryDaemon_t` | Required by Vector mw::com IPC service discovery. During startup, ConfigDaemon proxy creation/`FindService` resolves service instances (`FaultEventProxy`, `CryptoProxy`, `SecureFeatureRegistryProxy`, `VINProxy`, `VPCProxy`, `ProgIdProxy`, `SoftwareIdProviderProxy`) via this daemon. |
| `lifecycle_state_machine_t` | LSM uses the `InternalConfigProvider` interface to read coding parameters from ConfigD — specifically the `FasCountryVariants` coding parameter, which LSM uses to determine the current software variant and drive the `CountryVariant` function group state. |
| `PhmHeartBeatProxy_t` | Platform Health Management — ConfigD reports hardware and software fault events via `FaultEventProxy`. The events are used by PhmHeartBeatProxy to set primary DTCs. |
| `pipe_t` | Communication through pipes (needed by LoLa's `abortable_blocking_reader`). |
| `qtsafefsd_t` | QNX safe filesystem — coding and calibration parameter files are stored under `/opt` which is integrity-protected by the read-only qtsafefs (ASIL-B). |
| `secured_t` | Secure daemon — ConfigD subscribes to the `SecureDebug` interface provided by `secured` to determine whether to offer the `ConfigCalibrationService` (toggled off in field mode, on in engineering mode). |
| `SoftwareIdProvider_t` | Provides SWID for ECU programming detection |
| `SoftwareUpdate_t` | Provides `ProgId` used by the coding plugin to detect whether the ECU has been reprogrammed. |
| `someipd_posix_t` | SOME/IP daemon — required for receiving VIN and VPC (Vehicle Profile Checksum), consumed by the coding plugin for qualification. |

### Named path attachments

`ConfigDaemon` is permitted to attach to the following named paths:

| Path | Purpose |
|---|---|
| `/dev/name/local/amsr/amsr_ipc_server-0000039180_0000000101` | `ConfigCalibration` service endpoint (service ID 39180) — ConfigDaemon registers this skeleton to expose the `ConfigCalibration` interface, which allows CalibrationServer to write/update parameter sets. |
| `/dev/name/local/amsr/amsr_ipc_server-0000037832_0000000101` | `SvkCafId` service endpoint (service ID 37832) - ConfigDaemon's coding plugin registers this skeleton to expose the current CAF IDs (`SgbmId` array), which `SoftwareUpdate` consumes to identify the active coding software versions. |
| `/dev/name/local/amsr/amsr_ipc_server-0000016578_0000000101` | `InternalConfigProvider` service endpoint (service ID 16578) — ConfigDaemon registers this skeleton to expose the `InternalConfigProvider` interface, which provides read-only access to configuration parameters for all client applications. |
| `/dev/name/local/amsr/amsr_ipc_server-0000061020_0000002273` | Vector AMSR IPC infrastructure server - ConfigDaemon connects to this during `mw::core::Initialize()` to register itself in the `mw::com` runtime, enabling it to offer its services (`ConfigCalibration`, `InternalConfigProvider`) and discover/connect to other services. Required by all processes using the Vector adaptive stack. |
| `/mw_com/message_passing/logging.CfgD.1012` | DLT logging channel for ConfigDaemon |
| `/mw_com/message_passing/LoLa*` | Needed for message_passing use in LoLa. |
