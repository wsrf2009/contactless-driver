cmd_/opt/acr910/pcd/mod/crc.o := /opt/DevKit8500/arm-eabi-4.4.0/bin/arm-eabi-gcc -Wp,-MD,/opt/acr910/pcd/mod/.crc.o.d  -nostdinc -isystem /opt/DevKit8500/arm-eabi-4.4.0/bin/../lib/gcc/arm-eabi/4.4.0/include -Iinclude  -I/opt/DevKit8500/linux-2.6.32/arch/arm/include -include include/linux/autoconf.h -D__KERNEL__ -mlittle-endian -Iarch/arm/mach-omap2/include -Iarch/arm/plat-omap/include -Wall -Wundef -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -Werror-implicit-function-declaration -Wno-format-security -fno-delete-null-pointer-checks -Os -marm -mabi=aapcs-linux -mno-thumb-interwork -funwind-tables -D__LINUX_ARM_ARCH__=7 -march=armv7-a -msoft-float -Uarm -Wframe-larger-than=1024 -fno-stack-protector -fomit-frame-pointer -g -Wdeclaration-after-statement -Wno-pointer-sign -fno-strict-overflow -fno-dwarf2-cfi-asm -fconserve-stack -O0 -g -DEXPORT_SYMTAB  -DMODULE -D"KBUILD_STR(s)=\#s" -D"KBUILD_BASENAME=KBUILD_STR(crc)"  -D"KBUILD_MODNAME=KBUILD_STR(pcdm)"  -c -o /opt/acr910/pcd/mod/.tmp_crc.o /opt/acr910/pcd/mod/src/crc.c

deps_/opt/acr910/pcd/mod/crc.o := \
  /opt/acr910/pcd/mod/src/crc.c \
  /opt/acr910/pcd/mod/src/common.h \
  include/linux/bitops.h \
    $(wildcard include/config/generic/find/first/bit.h) \
    $(wildcard include/config/generic/find/last/bit.h) \
    $(wildcard include/config/generic/find/next/bit.h) \
  /opt/DevKit8500/linux-2.6.32/arch/arm/include/asm/types.h \
  include/asm-generic/int-ll64.h \
  /opt/DevKit8500/linux-2.6.32/arch/arm/include/asm/bitsperlong.h \
  include/asm-generic/bitsperlong.h \
    $(wildcard include/config/64bit.h) \
  /opt/DevKit8500/linux-2.6.32/arch/arm/include/asm/bitops.h \
    $(wildcard include/config/smp.h) \
  include/linux/compiler.h \
    $(wildcard include/config/trace/branch/profiling.h) \
    $(wildcard include/config/profile/all/branches.h) \
    $(wildcard include/config/enable/must/check.h) \
    $(wildcard include/config/enable/warn/deprecated.h) \
  include/linux/compiler-gcc.h \
    $(wildcard include/config/arch/supports/optimized/inlining.h) \
    $(wildcard include/config/optimize/inlining.h) \
  include/linux/compiler-gcc4.h \
  /opt/DevKit8500/linux-2.6.32/arch/arm/include/asm/system.h \
    $(wildcard include/config/cpu/xsc3.h) \
    $(wildcard include/config/cpu/fa526.h) \
    $(wildcard include/config/cpu/sa1100.h) \
    $(wildcard include/config/cpu/sa110.h) \
    $(wildcard include/config/cpu/32v6k.h) \
  include/linux/linkage.h \
  /opt/DevKit8500/linux-2.6.32/arch/arm/include/asm/linkage.h \
  include/linux/irqflags.h \
    $(wildcard include/config/trace/irqflags.h) \
    $(wildcard include/config/irqsoff/tracer.h) \
    $(wildcard include/config/preempt/tracer.h) \
    $(wildcard include/config/trace/irqflags/support.h) \
  include/linux/typecheck.h \
  /opt/DevKit8500/linux-2.6.32/arch/arm/include/asm/irqflags.h \
  /opt/DevKit8500/linux-2.6.32/arch/arm/include/asm/ptrace.h \
    $(wildcard include/config/cpu/endian/be8.h) \
    $(wildcard include/config/arm/thumb.h) \
  /opt/DevKit8500/linux-2.6.32/arch/arm/include/asm/hwcap.h \
  include/asm-generic/cmpxchg-local.h \
  include/linux/types.h \
    $(wildcard include/config/uid16.h) \
    $(wildcard include/config/lbdaf.h) \
    $(wildcard include/config/phys/addr/t/64bit.h) \
  include/linux/posix_types.h \
  include/linux/stddef.h \
  /opt/DevKit8500/linux-2.6.32/arch/arm/include/asm/posix_types.h \
  include/asm-generic/bitops/non-atomic.h \
  include/asm-generic/bitops/fls64.h \
  include/asm-generic/bitops/sched.h \
  include/asm-generic/bitops/hweight.h \
  include/asm-generic/bitops/lock.h \

/opt/acr910/pcd/mod/crc.o: $(deps_/opt/acr910/pcd/mod/crc.o)

$(deps_/opt/acr910/pcd/mod/crc.o):
