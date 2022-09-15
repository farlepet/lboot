cmake_minimum_required(VERSION 3.7)

project(lboot
        LANGUAGES C ASM)
ENABLE_LANGUAGE(ASM-ATT)

set(CMAKE_SYSTEM_PROCESSOR i386)
set(triple i386-none-eabi)
set(CMAKE_C_COMPILER_TARGET   ${tiple})
set(CMAKE_ASM_COMPILER_TARGET ${tiple})

if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_ASM-ATT_COMPILER ${CMAKE_C_COMPILER})
    add_compile_options($<$<COMPILE_LANGUAGE:ASM-ATT>:-c>)
    
    set(CMAKE_C_LINK_EXECUTABLE "ld")
else()
    add_compile_options($<$<COMPILE_LANGUAGE:ASM-ATT>:-x$<SEMICOLON>assembler-with-cpp>)
endif()

set(STAGE2_NAME "stage2")
set(STAGE2_LIB  "libstage2.a")
set(STAGE2_ELF  "stage2.elf")
set(STAGE2_BIN  "stage2.bin")

add_compile_options(-m32 -march=i386 -fno-pic)
add_compile_options(-Wall -Wextra -Werror)
add_compile_options(-Os)

add_compile_options(-fno-stack-protector -fdata-sections -ffunction-sections)
add_compile_options(-nostdlib -nostdinc -ffreestanding)
#add_compile_options(-include "${CMAKE_CURRENT_LIST_DIR}/inc/config.h")
add_compile_options(-include ${CMAKE_CURRENT_BINARY_DIR}/inc/config.h)
add_compile_options($<$<COMPILE_LANG_AND_ID:C,Clang>:-Weverything$<SEMICOLON>-Wno-unknown-warning-option$<SEMICOLON>-Wno-padded>)

#
# Options
#

# Creates a cache variable of "string" type (also applicable to numbers)
function(str_option var desc val)
    set(${var} ${val} CACHE STRING ${desc})
endfunction()

# Features
option(USE_SERIAL              "Use serial rather than VGA for status output"                      OFF)
option(FEATURE_VERBOSE_PANIC   "Include file and line number in panic messages"                    OFF)
option(FEATURE_STATUSBAR       "Enable status bar showing current operations, only applies to VGA" ON)
option(FEATURE_WORKINGSTATUS   "Show working status indicator during longer operations"            ON)
option(FEATURE_PROTOCOL_XMODEM "Enable XMODEM protocol"                                            ON)
# Debug
option(DEBUG                     "Enable debugging symbols"        OFF)
str_option(DEBUG_CONFIG          "Set CONFIG debug level"          0)
str_option(DEBUG_STORAGE_BIOS    "Set STORAGE_BIOS debug level"    0)
str_option(DEBUG_FS_FAT          "Set FS_FAT debug level"          0)
str_option(DEBUG_EXEC            "Set EXEC debug level"            0)
str_option(DEBUG_EXEC_MULTIBOOT  "Set EXEC_MULTIBOOT debug level"  0)
str_option(DEBUG_EXEC_ELF        "Set EXEC_ELF debug level"        0)
str_option(DEBUG_PROTOCOL_XMODEM "Set PROTOCOL_XMODEM debug level" 0)
# Serial
str_option(SERIAL_FIFO_SIZE "Set serial FIFO size (2^n)" 8)
str_option(SERIAL_BAUDRATE  "Set serial baud rate"       115200)

if(DEBUG)
    message(STATUS "Enabling debug symbols")
    add_compile_options(-g)
endif()

if(USE_SERIAL AND CONFIG_DEBUG_PROTOCOL_XMODEM)
    message(SEND_ERROR "COMFIG_DEBUG_PROTOCOL_XMODEM debug is incompatible with USE_SERIAL")
endif()

set(FEATURE_PROTOCOL OFF)
if(FEATURE_PROTOCOL_XMODEM)
    set(FEATURE_PROTOCOL ON)
endif()

configure_file(${CMAKE_CURRENT_LIST_DIR}/inc/config.h.in
               ${CMAKE_CURRENT_BINARY_DIR}/inc/config.h)


add_library(${STAGE2_NAME})

target_sources(
    ${STAGE2_NAME} PRIVATE
    src/startup/startup.s
	src/startup/cstart.c
	src/bios/bios_asm.s
	src/bios/bios.c
	src/config/config.c
	src/data/fifo.c
	src/exec/exec.c
	src/exec/multiboot.c
	src/exec/fmt/elf.c
	src/exec/fmt/flat.c
	src/intr/idt.c
	src/intr/interrupts.c
	src/intr/int_wrappers.s
	src/intr/pic.c
	src/io/output.c
	src/io/serial.c
	src/io/vga.c
	src/mm/alloc.c
	src/stdlib/ctype.c
	src/stdlib/string.c
	src/stdlib/stdlib.c
	src/storage/bios.c
	src/storage/file.c
	src/storage/fs/fs.c
	src/storage/fs/fat.c
    $<$<BOOL:FEATURE_PROTOCOL>:src/storage/protocol/protocol.c>
    $<$<BOOL:FEATURE_PROTOCOL_XMODEM>:src/storage/protocol/xmodem.c>
	src/time/pit.c
	src/time/time.c
)

include_directories(${STAGE2_NAME} inc/)

#target_link_options(${STAGE2_NAME} PRIVATE "-Wl,-T,${CMAKE_CURRENT_LIST_DIR}/stage2.ld" -m32 -march=i386 -ffreestanding -nostartfiles -fno-pic -nostdlib -nostdinc)
#target_link_options(${STAGE2_NAME} PRIVATE -T "${CMAKE_CURRENT_LIST_DIR}/stage2.ld" -melf_i386)

add_custom_command(
    TARGET ${STAGE2_NAME}
    POST_BUILD
    COMMAND ${CMAKE_LINKER} ARGS -T "${CMAKE_CURRENT_LIST_DIR}/stage2.ld" -melf_i386 --gc-sections -o ${STAGE2_ELF} ${STAGE2_LIB}
)

add_custom_command(
    TARGET ${STAGE2_NAME}
    POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} ARGS -O binary --only-section=.text --only-section=.rodata --only-section=.data ${STAGE2_ELF} ${STAGE2_BIN})

