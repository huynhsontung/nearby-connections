// Copyright 2020 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#ifndef CORE_OPTIONS_H_
#define CORE_OPTIONS_H_
#include <string>

#include "core/medium_selector.h"
#include "core/strategy.h"
#include "platform/base/byte_array.h"
#include "proto/connections_enums.pb.h"

namespace location {
namespace nearby {
namespace connections {

// Feature On/Off switch for mediums.
using BooleanMediumSelector = MediumSelector<bool>;

// Represents the various power levels that can be used, on mediums that support
// it.
enum class PowerLevel {
  kHighPower = 0,
  kLowPower = 1,
};

// Connection Options: used for both Advertising and Discovery.
// All fields are mutable, to make the type copy-assignable.
struct ConnectionOptions {
  Strategy strategy;
  BooleanMediumSelector allowed{BooleanMediumSelector().SetAll(true)};
  bool auto_upgrade_bandwidth;
  bool enforce_topology_constraints;
  bool low_power;
  bool enable_bluetooth_listening;
  bool enable_webrtc_listening;

  // Whether this is intended to be used in conjunction with InjectEndpoint().
  bool is_out_of_band_connection = false;
  ByteArray remote_bluetooth_mac_address;
  std::string fast_advertisement_service_uuid;
  int keep_alive_interval_millis = 0;
  int keep_alive_timeout_millis = 0;

  // Verify if  ConnectionOptions is in a not-initialized (Empty) state.
  bool Empty() const;

  // Bring  ConnectionOptions to a not-initialized (Empty) state.
  void Clear();

  // Returns a copy and normalizes allowed mediums:
  // (1) If is_out_of_band_connection is true, verifies that there is only one
  //     medium allowed, defaulting to only Bluetooth if unspecified.
  // (2) If no mediums are allowed, allow all mediums.
  ConnectionOptions CompatibleOptions() const;

  std::vector<Medium> GetMediums() const;

  // This call follows the standard Microsoft calling pattern of calling first
  // to get the size of the array. Caller then allocates memory for the array,
  // and makes this call again to copy the array into the provided location.
  void GetMediums(location::nearby::proto::connections::Medium* mediums,
                  uint32_t* mediumsSize);
};

// Metadata injected to facilitate out-of-band connections. The medium field is
// required, and the other fields are only specified for a specific medium.
// Currently, Bluetooth is the only supported medium for out-of-band
// connections.
struct OutOfBandConnectionMetadata {
  // Medium to use for the out-of-band connection.
  Medium medium;

  // Endpoint ID to use for the injected connection; will be included in the
  // endpoint_found_cb callback. Must be exactly 4 bytes and should be randomly-
  // generated such that no two IDs are identical.
  std::string endpoint_id;

  // Endpoint info to use for the injected connection; will be included in the
  // endpoint_found_cb callback. Should uniquely identify the InjectEndpoint()
  // call so that the client which made the call can verify the endpoint
  // that was found is the one that was injected.
  //
  // Cannot be empty, and must be <131 bytes.
  ByteArray endpoint_info;

  // Used for Bluetooth connections.
  ByteArray remote_bluetooth_mac_address;
};

}  // namespace connections
}  // namespace nearby
}  // namespace location

#endif  // CORE_OPTIONS_H_
