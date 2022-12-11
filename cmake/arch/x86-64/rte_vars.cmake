#
# arch:
#
#   - define ARCH variable (overridden by cmdline or by previous
#     optional define in machine .mk)
#   - define CROSS variable (overridden by cmdline or previous define
#     in machine .mk)
#   - define CPU_CFLAGS variable (overridden by cmdline or previous
#     define in machine .mk)
#   - define CPU_LDFLAGS variable (overridden by cmdline or previous
#     define in machine .mk)
#   - define CPU_ASFLAGS variable (overridden by cmdline or previous
#     define in machine .mk)
#   - may override any previously defined variable
#
# examples for CONFIG_RTE_ARCH: i686, x86_64, x86_64_32
#

set(RTE_ARCH x86_64)
set(RTE_CPU_CFLAGS -m64)
set(RTE_CPU_ASFLAGS -felf64)

set(RTE_OBJCOPY_TARGET elf64-x86-64)
set(RTE_OBJCOPY_ARCH i386:x86-64)