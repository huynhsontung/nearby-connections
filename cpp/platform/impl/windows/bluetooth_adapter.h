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

#ifndef PLATFORM_IMPL_WINDOWS_BLUETOOTH_ADAPTER_H_
#define PLATFORM_IMPL_WINDOWS_BLUETOOTH_ADAPTER_H_

#include <guiddef.h>

#include <string>
#include <functional>

#include "platform/api/bluetooth_adapter.h"
#include "platform/impl/windows/generated/winrt/base.h"
#include "platform/impl/windows/generated/winrt/Windows.Devices.Bluetooth.h"
#include "platform/impl/windows/generated/winrt/Windows.Devices.Radios.h"

namespace location {
namespace nearby {
namespace windows {

// Represents a Bluetooth adapter.
// https://docs.microsoft.com/en-us/uwp/api/windows.devices.bluetooth.bluetoothadapter?view=winrt-20348
using winrt::Windows::Devices::Bluetooth::IBluetoothAdapter;

// Represents a radio device on the system.
// https://docs.microsoft.com/en-us/uwp/api/windows.devices.radios.radio?view=winrt-20348
using winrt::Windows::Devices::Radios::IRadio;

// Enumeration that describes possible radio states.
// https://docs.microsoft.com/en-us/uwp/api/windows.devices.radios.radiostate?view=winrt-20348
using winrt::Windows::Devices::Radios::RadioState;

// https://developer.android.com/reference/android/bluetooth/BluetoothAdapter.html
class BluetoothAdapter : public api::BluetoothAdapter {
 public:
  BluetoothAdapter();

  ~BluetoothAdapter() override = default;

  typedef std::function<void(api::BluetoothAdapter::ScanMode)> ScanModeCallback;

  // Synchronously sets the status of the BluetoothAdapter to 'status', and
  // returns true if the operation was a success.
  bool SetStatus(Status status) override;

  // Returns true if the BluetoothAdapter's current status is
  // Status::Value::kEnabled.
  bool IsEnabled() const override;

  // https://developer.android.com/reference/android/bluetooth/BluetoothAdapter.html#getScanMode()
  // Returns ScanMode::kUnknown on error.
  ScanMode GetScanMode() const override;

  // Synchronously sets the scan mode of the adapter, and returns true if the
  // operation was a success.
  bool SetScanMode(ScanMode scan_mode) override;

  // https://developer.android.com/reference/android/bluetooth/BluetoothAdapter.html#getName()
  // Returns an empty string on error
  std::string GetName() const override;

  // https://developer.android.com/reference/android/bluetooth/BluetoothAdapter.html#setName(java.lang.String)
  bool SetName(absl::string_view name) override;

  // Returns BT MAC address assigned to this adapter.
  std::string GetMacAddress() const override;

  void SetOnScanModeChanged(ScanModeCallback callback) {
    if (scan_mode_changed_ == nullptr) {
      scan_mode_changed_ = callback;
    }
  }

 private:
  IBluetoothAdapter windows_bluetooth_adapter_;

  IRadio windows_bluetooth_radio_;
  char *GetGenericBluetoothAdapterInstanceID(void) const;
  void find_and_replace(char *source, const char *strFind,
                        const char *strReplace) const;
  ScanMode scan_mode_ = ScanMode::kNone;
  ScanModeCallback scan_mode_changed_ = nullptr;
};

}  // namespace windows
}  // namespace nearby
}  // namespace location

#endif  // PLATFORM_IMPL_WINDOWS_BLUETOOTH_ADAPTER_H_
