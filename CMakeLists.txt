# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)

idf_build_get_property(__hack_component_targets __COMPONENT_TARGETS)

if("esp-nimble-component" IN_LIST BUILD_COMPONENTS OR "__esp-nimble-component" IN_LIST __hack_component_targets)
    list(APPEND ESP_NIMBLE_PRIV_REQUIRES
        esp-nimble-component
    )
elseif("nimble" IN_LIST BUILD_COMPONENTS OR "__nimble" IN_LIST __hack_component_targets)
    list(APPEND ESP_NIMBLE_PRIV_REQUIRES
        nimble
    )
endif()

idf_component_register(
  REQUIRED_IDF_TARGETS
    "esp32"
    "esp32s3"
    "esp32c2"
    "esp32c3"
    "esp32c6"
  INCLUDE_DIRS
    "include"
  SRCS
    "src/Descriptor2904.cpp"
    "src/Address.cpp"
    "src/AdvertisedDevice.cpp"
    "src/Advertising.cpp"
    "src/Beacon.cpp"
    "src/Characteristic.cpp"
    "src/Client.cpp"
    "src/Descriptor.cpp"
    "src/Device.cpp"
    "src/EddystoneTLM.cpp"
    "src/EddystoneURL.cpp"
    "src/ExtendedAdvertising.cpp"
    "src/HIDDevice.cpp"
    "src/RemoteCharacteristic.cpp"
    "src/RemoteDescriptor.cpp"
    "src/RemoteService.cpp"
    "src/Scan.cpp"
    "src/Server.cpp"
    "src/Service.cpp"
    "src/Utils.cpp"
    "src/UUID.cpp"
  REQUIRES
    bt
    nvs_flash
    driver
  PRIV_REQUIRES
    ${ESP_NIMBLE_PRIV_REQUIRES}
)

