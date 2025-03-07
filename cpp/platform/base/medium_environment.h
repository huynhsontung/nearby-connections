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

#ifndef PLATFORM_BASE_MEDIUM_ENVIRONMENT_H_
#define PLATFORM_BASE_MEDIUM_ENVIRONMENT_H_

#include <atomic>
#include <memory>

#include "absl/container/flat_hash_map.h"
#include "absl/strings/string_view.h"
#include "platform/api/ble.h"
#include "platform/api/bluetooth_adapter.h"
#include "platform/api/bluetooth_classic.h"
#include "platform/api/webrtc.h"
#include "platform/api/wifi_lan.h"
#include "platform/base/byte_array.h"
#include "platform/base/feature_flags.h"
#include "platform/base/listeners.h"
#include "platform/base/nsd_service_info.h"
#include "platform/public/single_thread_executor.h"

namespace location {
namespace nearby {

// Environment config that can control availability of certain mediums for
// testing.
struct EnvironmentConfig {
  // Control whether WEB_RTC medium is enabled in the environment.
  // This is currently set to false, due to http://b/139734036 that would lead
  // to flaky tests.
  bool webrtc_enabled = false;
};

// MediumEnvironment is a simulated environment which allows multiple instances
// of simulated HW devices to "work" together as if they are physical.
// For each medium type it provides necessary methods to implement
// advertising, discovery and establishment of a data link.
// NOTE: this code depends on public:types target.
class MediumEnvironment {
 public:
  using BluetoothDiscoveryCallback =
      api::BluetoothClassicMedium::DiscoveryCallback;
  using BleDiscoveredPeripheralCallback =
      api::BleMedium::DiscoveredPeripheralCallback;
  using BleAcceptedConnectionCallback =
      api::BleMedium::AcceptedConnectionCallback;
  using OnSignalingMessageCallback =
      api::WebRtcSignalingMessenger::OnSignalingMessageCallback;
  using OnSignalingCompleteCallback =
      api::WebRtcSignalingMessenger::OnSignalingCompleteCallback;
  using WifiLanDiscoveredServiceCallback =
      api::WifiLanMedium::DiscoveredServiceCallback;

  MediumEnvironment(const MediumEnvironment&) = delete;
  MediumEnvironment& operator=(const MediumEnvironment&) = delete;

  // Creates and returns a reference to the global test environment instance.
  static MediumEnvironment& Instance();

  // Global ON/OFF switch for medium environment.
  // Start & Stop work as On/Off switch for this object.
  // Default state (after creation) is ON, to make it compatible with early
  // tests that are already using it and relying on it being ON.

  // Enables Medium environment.
  void Start(EnvironmentConfig config = EnvironmentConfig());
  // Disables Medium environment.
  void Stop();

  // Clears state. No notifications are sent.
  void Reset();

  // Waits for all previously scheduled jobs to finish.
  // This method works as a barrier that guarantees that after it returns, all
  // the activities that started before it was called, or while it was running
  // are ended. This means that system is at the state of relaxation when this
  // code returns. It requires external stimulus to get out of relaxation state.
  //
  // If enable_notifications is true (default), simulation environment
  // will send all future notification events to all registered objects,
  // whenever protocol requires that. This is expected behavior.
  // If enabled_notifications is false, future event notifications will not be
  // sent to registered instances. This is useful for protocol shutdown,
  // where we no longer care about notifications, and where notifications may
  // otherwise be delivered after the notification source or target lifeteme has
  // ended, and cause undefined behavior.
  void Sync(bool enable_notifications = true);

  // Adds an adapter to internal container.
  // Notify BluetoothClassicMediums if any that adapter state has changed.
  void OnBluetoothAdapterChangedState(api::BluetoothAdapter& adapter,
                                      api::BluetoothDevice& adapter_device,
                                      std::string name, bool enabled,
                                      api::BluetoothAdapter::ScanMode mode);

  // Adds medium-related info to allow for adapter discovery to work.
  // This provides acccess to this medium from other mediums, when protocol
  // expects they should communicate.
  void RegisterBluetoothMedium(api::BluetoothClassicMedium& medium,
                               api::BluetoothAdapter& medium_adapter);

  // Updates callback info to allow for dispatch of discovery events.
  //
  // Invokes callback asynchronously when any changes happen to discoverable
  // devices, or if the defice is turned off, whether or not it is discoverable,
  // if it was ever reported as discoverable.
  //
  // This should be called when discoverable state changes.
  // with user-specified callback when discovery is enabled, and with default
  // (empty) callback otherwise.
  void UpdateBluetoothMedium(api::BluetoothClassicMedium& medium,
                             BluetoothDiscoveryCallback callback);

  // Removes medium-related info. This should correspond to device power off.
  void UnregisterBluetoothMedium(api::BluetoothClassicMedium& medium);

  // Returns a Bluetooth Device object matching given mac address to nullptr.
  api::BluetoothDevice* FindBluetoothDevice(const std::string& mac_address);

  const EnvironmentConfig& GetEnvironmentConfig();

  // Registers |message_callback| to receive messages sent to device with id
  // |self_id|, and |complete_callback| to notify when signaling is complete.
  void RegisterWebRtcSignalingMessenger(
      absl::string_view self_id, OnSignalingMessageCallback message_callback,
      OnSignalingCompleteCallback complete_callback);

  // Unregisters the callback listening to incoming messages for |self_id|.
  void UnregisterWebRtcSignalingMessenger(absl::string_view self_id);

  // Simulates sending a signaling message |message| to device with id
  // |peer_id|.
  void SendWebRtcSignalingMessage(absl::string_view peer_id,
                                  const ByteArray& message);

  // Simulates sending an "signaling complete" signal to the WebRTC medium.
  void SendWebRtcSignalingComplete(absl::string_view peer_id, bool success);

  // Used to set if WebRtcMedium should use a valid peer connection or nullptr
  // in tests.
  void SetUseValidPeerConnection(bool use_valid_peer_connection);

  bool GetUseValidPeerConnection();

  // Used to set latency when creating the peer connection in tests.
  void SetPeerConnectionLatency(absl::Duration peer_connection_latency);

  absl::Duration GetPeerConnectionLatency();

  // Adds medium-related info to allow for scanning/advertising to work.
  // This provides acccess to this medium from other mediums, when protocol
  // expects they should communicate.
  void RegisterBleMedium(api::BleMedium& medium);

  // Updates advertising info to indicate the current medium is exposing
  // advertising event.
  void UpdateBleMediumForAdvertising(api::BleMedium& medium,
                                     api::BlePeripheral& peripheral,
                                     const std::string& service_id,
                                     bool fast_advertisement, bool enabled);

  // Updates discovery callback info to allow for dispatch of discovery events.
  //
  // Invokes callback asynchronously when any changes happen to discoverable
  // devices, or if the defice is turned off, whether or not it is discoverable,
  // if it was ever reported as discoverable.
  //
  // This should be called when discoverable state changes.
  // with user-specified callback when discovery is enabled, and with default
  // (empty) callback otherwise.
  void UpdateBleMediumForScanning(
      api::BleMedium& medium, const std::string& service_id,
      const std::string& fast_advertisement_service_uuid,
      BleDiscoveredPeripheralCallback callback, bool enabled);

  // Updates Accepted connection callback info to allow for dispatch of
  // advertising events.
  void UpdateBleMediumForAcceptedConnection(
      api::BleMedium& medium, const std::string& service_id,
      BleAcceptedConnectionCallback callback);

  // Removes medium-related info. This should correspond to device power off.
  void UnregisterBleMedium(api::BleMedium& medium);

  // Call back when advertising has created the server socket and is ready for
  // connect.
  void CallBleAcceptedConnectionCallback(api::BleMedium& medium,
                                         api::BleSocket& socket,
                                         const std::string& service_id);

  // Adds medium-related info to allow for discovery/advertising to work.
  // This provides acccess to this medium from other mediums, when protocol
  // expects they should communicate.
  void RegisterWifiLanMedium(api::WifiLanMedium& medium);

  // Updates advertising info to indicate the current medium is exposing
  // advertising event.
  void UpdateWifiLanMediumForAdvertising(api::WifiLanMedium& medium,
                                         const NsdServiceInfo& nsd_service_info,
                                         bool enabled);

  // Updates discovery callback info to allow for dispatch of discovery events.
  //
  // This should be called when discoverable state changes.
  // with user-specified callback when discovery is enabled, and with default
  // (empty) callback otherwise.
  void UpdateWifiLanMediumForDiscovery(
      api::WifiLanMedium& medium, WifiLanDiscoveredServiceCallback callback,
      const std::string& service_type, bool enabled);

  // Gets Fake IP address for WifiLan medium.
  std::string GetFakeIPAddress() const;

  // Gets Fake port number for WifiLan medium.
  int GetFakePort() const;

  // Removes medium-related info. This should correspond to device power off.
  void UnregisterWifiLanMedium(api::WifiLanMedium& medium);

  // Returns WifiLan medium whose advertising service matching IP address and
  // port, or nullptr.
  api::WifiLanMedium* GetWifiLanMedium(const std::string& ip_address, int port);

  void SetFeatureFlags(const FeatureFlags::Flags& flags);

 private:
  struct BluetoothMediumContext {
    BluetoothDiscoveryCallback callback;
    api::BluetoothAdapter* adapter = nullptr;
    // discovered device vs device name map.
    absl::flat_hash_map<api::BluetoothDevice*, std::string> devices;
  };

  struct BleMediumContext {
    BleDiscoveredPeripheralCallback discovery_callback;
    BleAcceptedConnectionCallback accepted_connection_callback;
    api::BlePeripheral* ble_peripheral = nullptr;
    bool advertising = false;
    bool fast_advertisement = false;
  };

  struct WifiLanMediumContext {
    // advertising service type vs NsdServiceInfo map.
    absl::flat_hash_map<std::string, NsdServiceInfo> advertising_services;
    // discovered service type vs callback map.
    absl::flat_hash_map<std::string, WifiLanDiscoveredServiceCallback>
        discovered_callbacks;
    // discovered service vs service type map.
    absl::flat_hash_map<std::string, NsdServiceInfo> discovered_services;
  };

  // This is a singleton object, for which destructor will never be called.
  // Constructor will be invoked once from Instance() static method.
  // Object is create in-place (with a placement new) to guarantee that
  // destructor is not scheduled for execution at exit.
  MediumEnvironment() = default;
  ~MediumEnvironment() = default;

  void OnBluetoothDeviceStateChanged(BluetoothMediumContext& info,
                                     api::BluetoothDevice& device,
                                     const std::string& name,
                                     api::BluetoothAdapter::ScanMode mode,
                                     bool enabled);

  void OnBlePeripheralStateChanged(BleMediumContext& info,
                                   api::BlePeripheral& peripheral,
                                   const std::string& service_id,
                                   bool fast_advertisement, bool enabled);

  void OnWifiLanServiceStateChanged(WifiLanMediumContext& info,
                                    const NsdServiceInfo& service_info,
                                    bool enabled);

  void RunOnMediumEnvironmentThread(std::function<void()> runnable);

  std::atomic_bool enabled_ = true;
  std::atomic_int job_count_ = 0;
  std::atomic_bool enable_notifications_ = false;
  SingleThreadExecutor executor_;
  EnvironmentConfig config_;

  // The following data members are accessed in the context of a private
  // executor_ thread.
  absl::flat_hash_map<api::BluetoothAdapter*, api::BluetoothDevice*>
      bluetooth_adapters_;
  absl::flat_hash_map<api::BluetoothClassicMedium*, BluetoothMediumContext>
      bluetooth_mediums_;

  absl::flat_hash_map<api::BleMedium*, BleMediumContext> ble_mediums_;

  // Maps peer id to callback for receiving signaling messages.
  absl::flat_hash_map<std::string, OnSignalingMessageCallback>
      webrtc_signaling_message_callback_;

  // Maps peer id to callback for signaling complete events.
  absl::flat_hash_map<std::string, OnSignalingCompleteCallback>
      webrtc_signaling_complete_callback_;

  absl::flat_hash_map<api::WifiLanMedium*, WifiLanMediumContext>
      wifi_lan_mediums_;

  bool use_valid_peer_connection_ = true;
  absl::Duration peer_connection_latency_ = absl::ZeroDuration();
};

}  // namespace nearby
}  // namespace location

#endif  // PLATFORM_BASE_MEDIUM_ENVIRONMENT_H_
