cmd_/opt/acr910/pcd/mod/typeB.o := /opt/DevKit8500/arm-eabi-4.4.0/bin/arm-eabi-gcc -Wp,-MD,/opt/acr910/pcd/mod/.typeB.o.d  -nostdinc -isystem /opt/DevKit8500/arm-eabi-4.4.0/bin/../lib/gcc/arm-eabi/4.4.0/include -Iinclude  -I/opt/DevKit8500/linux-2.6.32/arch/arm/include -include include/linux/autoconf.h -D__KERNEL__ -mlittle-endian -Iarch/arm/mach-omap2/include -Iarch/arm/plat-omap/include -Wall -Wundef -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -Werror-implicit-function-declaration -Wno-format-security -fno-delete-null-pointer-checks -Os -marm -mabi=aapcs-linux -mno-thumb-interwork -funwind-tables -D__LINUX_ARM_ARCH__=7 -march=armv7-a -msoft-float -Uarm -Wframe-larger-than=1024 -fno-stack-protector -fomit-frame-pointer -g -Wdeclaration-after-statement -Wno-pointer-sign -fno-strict-overflow -fno-dwarf2-cfi-asm -fconserve-stack -O0 -g -DEXPORT_SYMTAB  -DMODULE -D"KBUILD_STR(s)=\#s" -D"KBUILD_BASENAME=KBUILD_STR(typeB)"  -D"KBUILD_MODNAME=KBUILD_STR(pcdm)"  -c -o /opt/acr910/pcd/mod/.tmp_typeB.o /opt/acr910/pcd/mod/src/typeB.c

deps_/opt/acr910/pcd/mod/typeB.o := \
  /opt/acr910/pcd/mod/src/typeB.c \
  include/linux/string.h \
    $(wildcard include/config/binary/printf.h) \
  include/linux/compiler.h \
    $(wildcard include/config/trace/branch/profiling.h) \
    $(wildcard include/config/profile/all/branches.h) \
    $(wildcard include/config/enable/must/check.h) \
    $(wildcard include/config/enable/warn/deprecated.h) \
  include/linux/compiler-gcc.h \
    $(wildcard include/config/arch/supports/optimized/inlining.h) \
    $(wildcard include/config/optimize/inlining.h) \
  include/linux/compiler-gcc4.h \
  include/linux/types.h \
    $(wildcard include/config/uid16.h) \
    $(wildcard include/config/lbdaf.h) \
    $(wildcard include/config/phys/addr/t/64bit.h) \
    $(wildcard include/config/64bit.h) \
  /opt/DevKit8500/linux-2.6.32/arch/arm/include/asm/types.h \
  include/asm-generic/int-ll64.h \
  /opt/DevKit8500/linux-2.6.32/arch/arm/include/asm/bitsperlong.h \
  include/asm-generic/bitsperlong.h \
  include/linux/posix_types.h \
  include/linux/stddef.h \
  /opt/DevKit8500/linux-2.6.32/arch/arm/include/asm/posix_types.h \
  /opt/DevKit8500/arm-eabi-4.4.0/bin/../lib/gcc/arm-eabi/4.4.0/include/stdarg.h \
  /opt/DevKit8500/linux-2.6.32/arch/arm/include/asm/string.h \
  /opt/acr910/pcd/mod/src/common.h \
  include/linux/bitops.h \
    $(wildcard include/config/generic/find/first/bit.h) \
    $(wildcard include/config/generic/find/last/bit.h) \
    $(wildcard include/config/generic/find/next/bit.h) \
  /opt/DevKit8500/linux-2.6.32/arch/arm/include/asm/bitops.h \
    $(wildcard include/config/smp.h) \
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
  include/asm-generic/bitops/non-atomic.h \
  include/asm-generic/bitops/fls64.h \
  include/asm-generic/bitops/sched.h \
  include/asm-generic/bitops/hweight.h \
  include/asm-generic/bitops/lock.h \
  /opt/acr910/pcd/mod/src/typeB.h \
  /opt/acr910/pcd/mod/src/picc.h \
  include/linux/semaphore.h \
  include/linux/list.h \
    $(wildcard include/config/debug/list.h) \
  include/linux/poison.h \
  include/linux/prefetch.h \
  /opt/DevKit8500/linux-2.6.32/arch/arm/include/asm/processor.h \
    $(wildcard include/config/mmu.h) \
  /opt/DevKit8500/linux-2.6.32/arch/arm/include/asm/cache.h \
    $(wildcard include/config/arm/l1/cache/shift.h) \
    $(wildcard include/config/aeabi.h) \
  include/linux/spinlock.h \
    $(wildcard include/config/debug/spinlock.h) \
    $(wildcard include/config/generic/lockbreak.h) \
    $(wildcard include/config/preempt.h) \
    $(wildcard include/config/debug/lock/alloc.h) \
  include/linux/preempt.h \
    $(wildcard include/config/debug/preempt.h) \
    $(wildcard include/config/preempt/notifiers.h) \
  include/linux/thread_info.h \
    $(wildcard include/config/compat.h) \
  /opt/DevKit8500/linux-2.6.32/arch/arm/include/asm/thread_info.h \
    $(wildcard include/config/arm/thumbee.h) \
  /opt/DevKit8500/linux-2.6.32/arch/arm/include/asm/fpstate.h \
    $(wildcard include/config/vfpv3.h) \
    $(wildcard include/config/iwmmxt.h) \
  /opt/DevKit8500/linux-2.6.32/arch/arm/include/asm/domain.h \
    $(wildcard include/config/io/36.h) \
  include/linux/kernel.h \
    $(wildcard include/config/preempt/voluntary.h) \
    $(wildcard include/config/debug/spinlock/sleep.h) \
    $(wildcard include/config/prove/locking.h) \
    $(wildcard include/config/printk.h) \
    $(wildcard include/config/dynamic/debug.h) \
    $(wildcard include/config/ring/buffer.h) \
    $(wildcard include/config/tracing.h) \
    $(wildcard include/config/numa.h) \
    $(wildcard include/config/ftrace/mcount/record.h) \
  include/linux/log2.h \
    $(wildcard include/config/arch/has/ilog2/u32.h) \
    $(wildcard include/config/arch/has/ilog2/u64.h) \
  include/linux/dynamic_debug.h \
  /opt/DevKit8500/linux-2.6.32/arch/arm/include/asm/byteorder.h \
  include/linux/byteorder/little_endian.h \
  include/linux/swab.h \
  /opt/DevKit8500/linux-2.6.32/arch/arm/include/asm/swab.h \
  include/linux/byteorder/generic.h \
  /opt/DevKit8500/linux-2.6.32/arch/arm/include/asm/bug.h \
    $(wildcard include/config/bug.h) \
    $(wildcard include/config/debug/bugverbose.h) \
  include/asm-generic/bug.h \
    $(wildcard include/config/generic/bug.h) \
    $(wildcard include/config/generic/bug/relative/pointers.h) \
  /opt/DevKit8500/linux-2.6.32/arch/arm/include/asm/div64.h \
  include/linux/stringify.h \
  include/linux/bottom_half.h \
  include/linux/spinlock_types.h \
  include/linux/spinlock_types_up.h \
  include/linux/lockdep.h \
    $(wildcard include/config/lockdep.h) \
    $(wildcard include/config/lock/stat.h) \
    $(wildcard include/config/generic/hardirqs.h) \
  include/linux/rwlock_types.h \
  include/linux/spinlock_up.h \
  include/linux/rwlock.h \
  include/linux/spinlock_api_up.h \
  /opt/DevKit8500/linux-2.6.32/arch/arm/include/asm/atomic.h \
  include/asm-generic/atomic-long.h \
  /opt/acr910/pcd/mod/src/ccid.h \
  /opt/acr910/pcd/mod/src/part4.h \
  /opt/acr910/pcd/mod/src/pn512.h \
  /opt/acr910/pcd/mod/src/pn512app.h \
  /opt/acr910/pcd/mod/src/delay.h \
  /opt/acr910/pcd/mod/src/debug.h \

/opt/acr910/pcd/mod/typeB.o: $(deps_/opt/acr910/pcd/mod/typeB.o)

$(deps_/opt/acr910/pcd/mod/typeB.o):
