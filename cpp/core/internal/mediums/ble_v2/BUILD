# Copyright 2020 Google LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     https://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
licenses(["notice"])

cc_library(
    name = "ble_v2",
    srcs = [
        "advertisement_read_result.cc",
        "ble_advertisement.cc",
        "ble_advertisement_header.cc",
        "ble_packet.cc",
    ],
    hdrs = [
        "advertisement_read_result.h",
        "ble_advertisement.h",
        "ble_advertisement_header.h",
        "ble_packet.h",
        "ble_peripheral.h",
        "discovered_peripheral_callback.h",
    ],
    # compatible_with = ["//buildenv/target:non_prod"],
    copts = ["-DCORE_ADAPTER_DLL"],
    visibility = [
        "//core/internal:__subpackages__",
    ],
    deps = [
        "//core:core_types",
        "//platform/base",
        "//platform/base:util",
        "//platform/public:logging",
        "//platform/public:types",
        "@abseil//absl/container:flat_hash_map",
        "@abseil//absl/container:flat_hash_set",
        "@abseil//absl/strings",
        "@abseil//absl/time",
    ],
)

cc_test(
    name = "ble_v2_test",
    srcs = [
        "advertisement_read_result_test.cc",
        "ble_advertisement_header_test.cc",
        "ble_advertisement_test.cc",
        "ble_packet_test.cc",
        "ble_peripheral_test.cc",
    ],
    deps = [
        ":ble_v2",
        "//testing/base/public:gunit_main",
        "//platform/base",
        "//platform/impl/g3",  # buildcleaner: keep
        "//platform/public:comm",
    ],
)
