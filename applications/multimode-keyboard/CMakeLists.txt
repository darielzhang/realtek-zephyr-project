# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(multimode-keyboard)

INCLUDE_DIRECTORIES(
    inc
    inc/led
    inc/usb
    inc/key
)
FILE(GLOB app_sources src/key/*.c src/usb/*.c src/led/*.c src/*.c)
target_sources(app PRIVATE ${app_sources})


