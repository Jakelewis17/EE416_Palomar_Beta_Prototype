# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "C:/Users/zwhar/esp/esp-idf/components/bootloader/subproject"
  "C:/git/EE416_Palomar_Beta_Prototype/Software/harrington-test-code/build/bootloader"
  "C:/git/EE416_Palomar_Beta_Prototype/Software/harrington-test-code/build/bootloader-prefix"
  "C:/git/EE416_Palomar_Beta_Prototype/Software/harrington-test-code/build/bootloader-prefix/tmp"
  "C:/git/EE416_Palomar_Beta_Prototype/Software/harrington-test-code/build/bootloader-prefix/src/bootloader-stamp"
  "C:/git/EE416_Palomar_Beta_Prototype/Software/harrington-test-code/build/bootloader-prefix/src"
  "C:/git/EE416_Palomar_Beta_Prototype/Software/harrington-test-code/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "C:/git/EE416_Palomar_Beta_Prototype/Software/harrington-test-code/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "C:/git/EE416_Palomar_Beta_Prototype/Software/harrington-test-code/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
