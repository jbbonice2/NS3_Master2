# Install script for directory: /home/bonice/Bureau/knowledge/ns-allinone-3.42/ns-3.42/src/lorawan

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "default")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set default install directory permissions.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/usr/bin/objdump")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libns3.42-lorawan-default.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libns3.42-lorawan-default.so")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libns3.42-lorawan-default.so"
         RPATH "/usr/local/lib:$ORIGIN/:$ORIGIN/../lib:/usr/local/lib64:$ORIGIN/:$ORIGIN/../lib64")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES "/home/bonice/Bureau/knowledge/ns-allinone-3.42/ns-3.42/build/lib/libns3.42-lorawan-default.so")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libns3.42-lorawan-default.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libns3.42-lorawan-default.so")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libns3.42-lorawan-default.so"
         OLD_RPATH "/home/bonice/Bureau/knowledge/ns-allinone-3.42/ns-3.42/build/lib:::::::::::::::::"
         NEW_RPATH "/usr/local/lib:$ORIGIN/:$ORIGIN/../lib:/usr/local/lib64:$ORIGIN/:$ORIGIN/../lib64")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libns3.42-lorawan-default.so")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/ns3" TYPE FILE FILES
    "/home/bonice/Bureau/knowledge/ns-allinone-3.42/ns-3.42/src/lorawan/model/lora-net-device.h"
    "/home/bonice/Bureau/knowledge/ns-allinone-3.42/ns-3.42/src/lorawan/model/lorawan-mac.h"
    "/home/bonice/Bureau/knowledge/ns-allinone-3.42/ns-3.42/src/lorawan/model/lora-phy.h"
    "/home/bonice/Bureau/knowledge/ns-allinone-3.42/ns-3.42/src/lorawan/model/building-penetration-loss.h"
    "/home/bonice/Bureau/knowledge/ns-allinone-3.42/ns-3.42/src/lorawan/model/correlated-shadowing-propagation-loss-model.h"
    "/home/bonice/Bureau/knowledge/ns-allinone-3.42/ns-3.42/src/lorawan/model/lora-channel.h"
    "/home/bonice/Bureau/knowledge/ns-allinone-3.42/ns-3.42/src/lorawan/model/lora-interference-helper.h"
    "/home/bonice/Bureau/knowledge/ns-allinone-3.42/ns-3.42/src/lorawan/model/gateway-lorawan-mac.h"
    "/home/bonice/Bureau/knowledge/ns-allinone-3.42/ns-3.42/src/lorawan/model/end-device-lorawan-mac.h"
    "/home/bonice/Bureau/knowledge/ns-allinone-3.42/ns-3.42/src/lorawan/model/class-a-end-device-lorawan-mac.h"
    "/home/bonice/Bureau/knowledge/ns-allinone-3.42/ns-3.42/src/lorawan/model/gateway-lora-phy.h"
    "/home/bonice/Bureau/knowledge/ns-allinone-3.42/ns-3.42/src/lorawan/model/end-device-lora-phy.h"
    "/home/bonice/Bureau/knowledge/ns-allinone-3.42/ns-3.42/src/lorawan/model/simple-end-device-lora-phy.h"
    "/home/bonice/Bureau/knowledge/ns-allinone-3.42/ns-3.42/src/lorawan/model/simple-gateway-lora-phy.h"
    "/home/bonice/Bureau/knowledge/ns-allinone-3.42/ns-3.42/src/lorawan/model/sub-band.h"
    "/home/bonice/Bureau/knowledge/ns-allinone-3.42/ns-3.42/src/lorawan/model/logical-lora-channel.h"
    "/home/bonice/Bureau/knowledge/ns-allinone-3.42/ns-3.42/src/lorawan/model/logical-lora-channel-helper.h"
    "/home/bonice/Bureau/knowledge/ns-allinone-3.42/ns-3.42/src/lorawan/model/periodic-sender.h"
    "/home/bonice/Bureau/knowledge/ns-allinone-3.42/ns-3.42/src/lorawan/model/one-shot-sender.h"
    "/home/bonice/Bureau/knowledge/ns-allinone-3.42/ns-3.42/src/lorawan/model/forwarder.h"
    "/home/bonice/Bureau/knowledge/ns-allinone-3.42/ns-3.42/src/lorawan/model/lorawan-mac-header.h"
    "/home/bonice/Bureau/knowledge/ns-allinone-3.42/ns-3.42/src/lorawan/model/lora-frame-header.h"
    "/home/bonice/Bureau/knowledge/ns-allinone-3.42/ns-3.42/src/lorawan/model/mac-command.h"
    "/home/bonice/Bureau/knowledge/ns-allinone-3.42/ns-3.42/src/lorawan/model/lora-device-address.h"
    "/home/bonice/Bureau/knowledge/ns-allinone-3.42/ns-3.42/src/lorawan/model/lora-device-address-generator.h"
    "/home/bonice/Bureau/knowledge/ns-allinone-3.42/ns-3.42/src/lorawan/model/lora-tag.h"
    "/home/bonice/Bureau/knowledge/ns-allinone-3.42/ns-3.42/src/lorawan/model/network-server.h"
    "/home/bonice/Bureau/knowledge/ns-allinone-3.42/ns-3.42/src/lorawan/model/network-status.h"
    "/home/bonice/Bureau/knowledge/ns-allinone-3.42/ns-3.42/src/lorawan/model/network-controller.h"
    "/home/bonice/Bureau/knowledge/ns-allinone-3.42/ns-3.42/src/lorawan/model/network-controller-components.h"
    "/home/bonice/Bureau/knowledge/ns-allinone-3.42/ns-3.42/src/lorawan/model/network-scheduler.h"
    "/home/bonice/Bureau/knowledge/ns-allinone-3.42/ns-3.42/src/lorawan/model/end-device-status.h"
    "/home/bonice/Bureau/knowledge/ns-allinone-3.42/ns-3.42/src/lorawan/model/gateway-status.h"
    "/home/bonice/Bureau/knowledge/ns-allinone-3.42/ns-3.42/src/lorawan/model/lora-radio-energy-model.h"
    "/home/bonice/Bureau/knowledge/ns-allinone-3.42/ns-3.42/src/lorawan/model/lora-tx-current-model.h"
    "/home/bonice/Bureau/knowledge/ns-allinone-3.42/ns-3.42/src/lorawan/model/lora-utils.h"
    "/home/bonice/Bureau/knowledge/ns-allinone-3.42/ns-3.42/src/lorawan/model/adr-component.h"
    "/home/bonice/Bureau/knowledge/ns-allinone-3.42/ns-3.42/src/lorawan/model/hex-grid-position-allocator.h"
    "/home/bonice/Bureau/knowledge/ns-allinone-3.42/ns-3.42/src/lorawan/model/tow-mab-selector.h"
    "/home/bonice/Bureau/knowledge/ns-allinone-3.42/ns-3.42/src/lorawan/model/ee-mab-selector.h"
    "/home/bonice/Bureau/knowledge/ns-allinone-3.42/ns-3.42/src/lorawan/helper/lora-radio-energy-model-helper.h"
    "/home/bonice/Bureau/knowledge/ns-allinone-3.42/ns-3.42/src/lorawan/helper/lora-helper.h"
    "/home/bonice/Bureau/knowledge/ns-allinone-3.42/ns-3.42/src/lorawan/helper/lora-phy-helper.h"
    "/home/bonice/Bureau/knowledge/ns-allinone-3.42/ns-3.42/src/lorawan/helper/lorawan-mac-helper.h"
    "/home/bonice/Bureau/knowledge/ns-allinone-3.42/ns-3.42/src/lorawan/helper/periodic-sender-helper.h"
    "/home/bonice/Bureau/knowledge/ns-allinone-3.42/ns-3.42/src/lorawan/helper/one-shot-sender-helper.h"
    "/home/bonice/Bureau/knowledge/ns-allinone-3.42/ns-3.42/src/lorawan/helper/forwarder-helper.h"
    "/home/bonice/Bureau/knowledge/ns-allinone-3.42/ns-3.42/src/lorawan/helper/network-server-helper.h"
    "/home/bonice/Bureau/knowledge/ns-allinone-3.42/ns-3.42/src/lorawan/helper/lora-packet-tracker.h"
    "/home/bonice/Bureau/knowledge/ns-allinone-3.42/ns-3.42/src/lorawan/test/utilities.h"
    "/home/bonice/Bureau/knowledge/ns-allinone-3.42/ns-3.42/build/include/ns3/lorawan-module.h"
    )
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("/home/bonice/Bureau/knowledge/ns-allinone-3.42/ns-3.42/cmake-cache/src/lorawan/examples/cmake_install.cmake")

endif()

