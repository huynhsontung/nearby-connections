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

#ifndef CORE_INTERNAL_ENDPOINT_CHANNEL_H_
#define CORE_INTERNAL_ENDPOINT_CHANNEL_H_

#include <cstdint>
#include <string>

#include "securegcm/d2d_connection_context_v1.h"
#include "absl/time/clock.h"
#include "analytics/analytics_recorder.h"
#include "platform/base/byte_array.h"
#include "platform/base/exception.h"
#include "platform/public/mutex.h"
#include "proto/connections_enums.pb.h"

namespace location {
namespace nearby {
namespace connections {

class EndpointChannel {
 public:
  virtual ~EndpointChannel() = default;

  using EncryptionContext = ::securegcm::D2DConnectionContextV1;

  virtual ExceptionOr<ByteArray>
  Read() = 0;  // throws Exception::IO, Exception::INTERRUPTED

  virtual Exception Write(const ByteArray& data) = 0;  // throws Exception::IO

  // Closes this EndpointChannel, without tracking the closure in analytics.
  virtual void Close() = 0;

  // Closes this EndpointChannel and records the closure with the given reason.
  virtual void Close(proto::connections::DisconnectionReason reason) = 0;

  // Returns a one-word type descriptor for the concrete EndpointChannel
  // implementation that can be used in log messages; eg: BLUETOOTH, BLE, WIFI.
  virtual std::string GetType() const = 0;

  // Returns the name of the EndpointChannel.
  virtual std::string GetName() const = 0;

  // Returns the analytics enum representing the medium of this EndpointChannel.
  virtual proto::connections::Medium GetMedium() const = 0;

  // Returns the used BLE or WiFi technology of this EndpointChannel.
  virtual proto::connections::ConnectionTechnology GetTechnology() const = 0;

  // Returns the used wifi band of this EndpointChannel.
  virtual proto::connections::ConnectionBand GetBand() const = 0;

  // Returns the used wifi frequency of this EndpointChannel.
  virtual int GetFrequency() const = 0;

  // Returns the try counts of this EndpointChannel.
  virtual int GetTryCount() const = 0;

  // Returns the maximum supported transmit packet size(MTU) for the underlying
  // transport.
  virtual int GetMaxTransmitPacketSize() const = 0;

  // Enables encryption on the EndpointChannel.
  virtual void EnableEncryption(std::shared_ptr<EncryptionContext> context) = 0;

  // Disables encryption on the EndpointChannel.
  virtual void DisableEncryption() = 0;

  // True if the EndpointChannel is currently pausing all writes.
  virtual bool IsPaused() const = 0;

  // Pauses all writes on this EndpointChannel until resume() is called.
  virtual void Pause() = 0;

  // Resumes any writes on this EndpointChannel that were suspended when pause()
  // was called.
  virtual void Resume() = 0;

  // Returns the timestamp of the last read from this endpoint, or -1 if no
  // reads have occurred.
  virtual absl::Time GetLastReadTimestamp() const = 0;

  // Returns the timestamp of the last write to this endpoint, or -1 if no
  // writes have occurred.
  virtual absl::Time GetLastWriteTimestamp() const = 0;

  // Sets the AnalyticsRecorder instance for analytics.
  virtual void SetAnalyticsRecorder(
      analytics::AnalyticsRecorder* analytics_recorder,
      const std::string& endpoint_id) = 0;
};

inline bool operator==(const EndpointChannel& lhs, const EndpointChannel& rhs) {
  return (lhs.GetType() == rhs.GetType()) && (lhs.GetName() == rhs.GetName()) &&
         (lhs.GetMedium() == rhs.GetMedium());
}

inline bool operator!=(const EndpointChannel& lhs, const EndpointChannel& rhs) {
  return !(lhs == rhs);
}

}  // namespace connections
}  // namespace nearby
}  // namespace location

#endif  // CORE_INTERNAL_ENDPOINT_CHANNEL_H_
