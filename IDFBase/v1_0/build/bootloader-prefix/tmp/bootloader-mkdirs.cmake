# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "C:/Users/aksha/esp/esp-idf/components/bootloader/subproject"
  "D:/movio/codes/v1_0/build/bootloader"
  "D:/movio/codes/v1_0/build/bootloader-prefix"
  "D:/movio/codes/v1_0/build/bootloader-prefix/tmp"
  "D:/movio/codes/v1_0/build/bootloader-prefix/src/bootloader-stamp"
  "D:/movio/codes/v1_0/build/bootloader-prefix/src"
  "D:/movio/codes/v1_0/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "D:/movio/codes/v1_0/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
