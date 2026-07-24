# Migration Guide: From Local Persistent Caching to Centrally Provided ParameterSet Collection Files

## Overview

This guide explains how to migrate from the local persistent parameter caching approach (KVS-only) to the new centrally provided collection file approach introduced via the `initial_parameter_set_name_list` argument of `ConfigProviderFactory::Create()`.

---

## Background

### Old approach - local persistent parameter caching (KVS-only)

Previously, when persistent caching was required, each application was responsible for managing its own KVS (Key-Value Storage) entries. ParameterSets were stored per application into a private KVS during lifecycle execution and reloaded from there on the next lifecycle start.

The application would create a `Persistency` instance backed by `mw::per::KeyValueStorage` and pass it to `ConfigProviderFactory::Create()`. No pre-loading from shared system files took place; all cached data was application-local and had to be accumulated over successive lifecycles.

### New approach - centrally provided ParameterSet collection files

ConfigDaemon maintains two central JSON collection files that represent the **complete ParameterSet collection** for the ECU:

| File | Purpose |
|------|---------|
| `/persistent/trusted/ConfigDaemon/actual_parameter_set_collection.json` | Current (post-coding) collection. Updated by ConfigDaemon after a successful coding procedure. |
| `/opt/ConfigDaemon/etc/default_parameter_set_collection.json` | Shipped default collection. Used as fallback when a flash has occurred since the last calibration (i.e. the flash counter changed). |

When the application passes `initial_parameter_set_name_list` to `ConfigProviderFactory::Create()`, ConfigProvider reads the specified ParameterSets **directly from these centrally managed files** at startup — before any communication with ConfigDaemon takes place. This eliminates the need for applications to individually accumulate and manage their own per-ParameterSet KVS entries.

---

## What changes for you

| Aspect | Old (KVS-only) | New (collection file + `initial_parameter_set_name_list`) |
|--------|---------------|-----------------------------------------------------------|
| Initial data source | Application-private KVS entries, accumulated over lifecycles | Centrally maintained JSON collection files managed by ConfigDaemon |
| Application KVS setup | Required (open handle, configure `mw::per::KeyValueStorage`) | Required (same `PersistencyFactory::Create` flow, also needed for flash counter coupling) |
| secpol setup | Usually only KVS-related access | KVS-related access plus read access to centrally provided collection files |
| Specify required ParameterSets | Not needed | Pass `initial_parameter_set_name_list` to `Create()` |
| First lifecycle (no cached data yet) | No data available until ConfigDaemon serves it | Default collection file provides initial values immediately |
| Flash detection | Application-managed | Handled automatically by `PersistencyImpl` via flash counter at `/persistent/untrusted/ConfigProvider/flash_counter` |
| Factory overload | `Create(token, persistency, memory_resource, callback)` | `Create(token, persistency, initial_parameter_set_name_list, memory_resource, callback)` |

---

## Step-by-step migration

### Step 1 - Add `initial_parameter_set_name_list` to the factory call

Replace your existing `Create()` call that passes only a `persistency` instance with the overload that also accepts the name list.

**Before:**

```cpp
score::config_management::config_provider::ConfigProviderFactory config_provider_factory;
auto config_provider = config_provider_factory.Create<Port>(
    stop_token,
    std::move(persistency),
    score::cpp::pmr::get_default_resource(),
    std::move(callback));
```

**After:**

```cpp
score::cpp::pmr::vector<std::string_view> initial_parameter_set_name_list{score::cpp::pmr::get_default_resource()};
initial_parameter_set_name_list.emplace_back("MyParameterSet1");
initial_parameter_set_name_list.emplace_back("MyParameterSet2");

score::config_management::config_provider::ConfigProviderFactory config_provider_factory;
auto config_provider = config_provider_factory.Create<Port>(
    stop_token,
    std::move(persistency),
    initial_parameter_set_name_list,
    score::cpp::pmr::get_default_resource(),
    std::move(callback));
```

The `persistency` instance is created the same way as before (see [Persistency setup](#step-2---persistency-setup-unchanged)).

### Step 2 - Persistency setup (unchanged)

The `Persistency` instance is created using the same `PersistencyFactory::Create<KeyValueStorageType>()` method as before. No changes are needed here:

```cpp
score::config_management::config_provider::PersistencyFactory persistency_factory;
const auto kvs_is = ::mw::core::InstanceSpecifier{KeyValueStorageRPort};
auto kvs_handle = ::mw::per::OpenKeyValueStorage(kvs_is);
auto persistency = persistency_factory.Create<mw::per::KeyValueStorage>(
    kvs_handle, score::cpp::pmr::get_default_resource());
```

### Step 3 - Configure secpol for centrally provided files

The migrated setup needs secpol permissions for reading the centrally provided ParameterSet collection files in addition to KVS access.

Ensure your application security policy grants read access for:

- `/persistent/trusted/ConfigDaemon/actual_parameter_set_collection.json`
- `/opt/ConfigDaemon/etc/default_parameter_set_collection.json`
- parent directories needed to resolve these paths

Also keep the existing permissions needed to access KVS and to maintain the flash counter coupling logic.

Without these file read permissions, ConfigProvider cannot pre-load initial ParameterSets from the central collection files and startup behavior falls back to service-only retrieval.

In addition to secpol configuration, the application shall be configured with the supplementary groups required by the persistency and flash-counter flow:

- `10007`
- `11001`
- `16007`

These supplementary groups are required for the EES30 migration path because `PersistencyImpl` depends on KVS-backed state and flash-counter handling in addition to access to the central collection files.

Concrete secpol snippet:

```secpol
allow MyApp_t self:ability {
    pathspace
};

allow MyApp_t {
    qtsafefsd_t
    devb_ufs_qualcomm_t
}:channel connect;

allow_attach MyApp_t {
    /persistent/trusted/ConfigDaemon/actual_parameter_set_collection.json
    /bmw/platform/opt/ConfigDaemon/etc/default_parameter_set_collection.json
};
```

Notes:

- In secpol, use the mounted path `/bmw/platform/opt/ConfigDaemon/etc/default_parameter_set_collection.json` for the default collection file.
- If your target or test image mounts storage via additional filesystem components, you may also need extra `:channel connect` entries such as `devb_virtio_t` or `devb_loopback_t`.
- The policy above grants path access needed for file reads. Effective read-only behavior is still enforced by the underlying filesystem and ownership/mode settings.

### Step 4 - Choose the ParameterSet names to declare

The `initial_parameter_set_name_list` must contain **exactly the names of the ParameterSets your application requires**. These names must match the keys present in the central collection files. Only the listed ParameterSets will be pre-loaded; all others are ignored.

If a listed name is not found in the collection file, it is silently skipped — no error is returned. The ParameterSet will still be retrievable from ConfigDaemon at runtime once the service becomes available.

---

## Full example

```cpp
#include "score/config_management/config_provider/code/config_provider/factory/factory_socal_r20_11.h"
#include "score/config_management/config_provider/code/persistency/factory/persistency_factory.h"

// --- Persistency setup (same as before) ---
score::config_management::config_provider::PersistencyFactory persistency_factory;
const auto kvs_is = ::mw::core::InstanceSpecifier{KeyValueStorageRPort};
auto kvs_handle = ::mw::per::OpenKeyValueStorage(kvs_is);
auto persistency = persistency_factory.Create<mw::per::KeyValueStorage>(
    kvs_handle, score::cpp::pmr::get_default_resource());

// --- Declare required ParameterSets ---
score::cpp::pmr::vector<std::string_view> initial_parameter_set_name_list{score::cpp::pmr::get_default_resource()};
initial_parameter_set_name_list.emplace_back("MyParameterSet1");
initial_parameter_set_name_list.emplace_back("MyParameterSet2");

// --- Create ConfigProvider with the new overload ---
score::config_management::config_provider::ConfigProviderFactory config_provider_factory;

bool callback_called{false};
score::config_management::config_provider::IsAvailableNotificationCallback callback{
    [&callback_called]() noexcept { callback_called = true; }};

auto config_provider = config_provider_factory.Create<Port>(
    stop_token,
    std::move(persistency),
    initial_parameter_set_name_list,
    score::cpp::pmr::get_default_resource(),
    std::move(callback));

// --- ParameterSets are now available immediately from the collection file ---
auto parameter_set_result = config_provider->GetParameterSet("MyParameterSet1");
if (parameter_set_result.has_value())
{
    auto qualifier = parameter_set_result.value()->GetQualifier();
    // NOTE: qualifier may be kUnqualified until an update is received from ConfigDaemon.
    // Evaluate qualifier before trusting the values.
}
```

---

## Qualifier semantics

ParameterSets pre-loaded from the centrally provided collection files carry the qualifier value that is **stored in the JSON file itself AND is overridden by ConfigProvider**. The actual collection file reflects the last qualified state set by ConfigDaemon. The default collection file reflects shipped defaults and may have `kUnqualified` values.

Always check `ParameterSet::GetQualifier()` before relying on the parameter values, regardless of their source.

---

## Fallback behaviour (flash detection)

`PersistencyImpl` automatically detects whether a new flash has occurred by comparing a stored flash counter against `/persistent/untrusted/ConfigProvider/flash_counter`. If the counter changed, the **default collection** (`/opt/ConfigDaemon/etc/default_parameter_set_collection.json`) is used instead of the actual collection. This ensures stale post-calibration data is never used after an image update.

No application-side action is required to support this behaviour.

---

## No-KVS variant

No-KVS operation is not supported for this migration target.

Even if your application does not need traditional per-ParameterSet local caching, a `Persistency` instance backed by KVS is still required because flash counter handling is mandatorily coupled to central collection file selection.

`PersistencyImpl` compares and stores the flash counter state and uses that information to decide whether to read from:

- `/persistent/trusted/ConfigDaemon/actual_parameter_set_collection.json`, or
- `/opt/ConfigDaemon/etc/default_parameter_set_collection.json`

As a result, use the persistency-based factory overloads for EES30 migration and do not rely on timeout-only overloads for startup pre-loading behavior.

---

## Factory overload reference

| Overload | Persistent caching | Pre-loads from collection file | EES30 migration suitability |
|----------|--------------------|-------------------------------|-----------------------------|
| `Create(token, timeout, memory_resource, callback)` | No | No | Not suitable |
| `Create(token, persistency, memory_resource, callback)` | Yes (KVS only) | No | Partial (no initial list pre-load) |
| `Create(token, timeout, initial_parameter_set_name_list, memory_resource, callback)` | No | No (`PersistencyImpl`) | Not suitable |
| `Create(token, persistency, initial_parameter_set_name_list, memory_resource, callback)` | Yes (KVS + collection file) | **Yes** | Recommended |
| `Create(token, timeout, max_samples_limit, polling_cycle_interval, memory_resource, callback)` | No | No | Not suitable |
| `Create(token, persistency, max_samples_limit, polling_cycle_interval, memory_resource, callback)` | Yes (KVS only) | No | Partial (no initial list pre-load) |

---

## Contact

Michael Saborov: michael.saborov@bmw.de
Wei Zhaoxiong: zhaoxiong.wei@bmw.de
