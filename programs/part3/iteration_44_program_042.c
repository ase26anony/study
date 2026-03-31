/* test-gcc-driver-reset.c
 *
 * This file contains a minimal C program. Its primary purpose is to serve
 * as a vehicle for testing the GCC driver's internal state reset logic,
 * particularly the block in gcc.cc that resets dumpdir, dumpbase, outbase,
 * save_temps_flag, spec_machine, and other global variables.
 *
 * The actual test sequences are described in the #if 0 blocks below.
 * A test harness should extract and execute these GCC command lines
 * in the order they appear, possibly in separate subprocesses or with
 * a wrapper that re-invokes GCC with different flags.
 *
 * The program itself is valid and portable C.
 */

int main(void) {
    return 0;
}

#if 0
/* ==================== SCENARIO A: Help/Version Reset ====================
 *
 * First, invoke GCC with a help or version flag to set print_help_list,
 * print_version, or print_subprocess_help. Then, immediately compile
 * this file. The driver should reset these flags and other state before
 * the actual compilation.
 *
 * Test sequence for harness:
 *   1. gcc --help=common
 *   2. gcc -v
 *   3. gcc -c test-gcc-driver-reset.c -O2 -o test1.o
 *   4. gcc test-gcc-driver-reset.c -O2 -o test1
 */

#endif

#if 0
/* ==================== SCENARIO B: Save Temps & Dump Options ====================
 *
 * This sequence tests the reset of save_temps_flag, dumpdir, dumpbase,
 * dumpbase_ext, and outbase. First, compile with -save-temps and custom
 * dump options, then compile again without them to trigger the reset.
 *
 * Test sequence for harness:
 *   1. gcc -save-temps=obj -dumpdir ./dump1 -dumpbase myapp \
 *        -dumpbase-ext .c -o test2 test-gcc-driver-reset.c
 *   2. gcc -O2 -o test3 test-gcc-driver-reset.c
 *   3. gcc -save-temps=cwd -dumpdir ./dump2/ -dumpbase another \
 *        -o test4 test-gcc-driver-reset.c
 *   4. gcc -save-temps=none -o test5 test-gcc-driver-reset.c
 */

#endif

#if 0
/* ==================== SCENARIO C: Multi-Stage & Specs ====================
 *
 * Exercise the driver in different modes (compiler to assembly, assembler,
 * linker) and with -specs and -B options that may affect target_system_root
 * and spec_machine.
 *
 * Test sequence for harness:
 *   1. gcc -S -dumpbase asm -o my.s test-gcc-driver-reset.c
 *   2. gcc -c my.s -o my.o
 *   3. gcc -specs=myspecs -B ./mylib/ my.o -o final
 *   4. gcc -Wa,-aln=my.lst -c test-gcc-driver-reset.c -o wa.o
 *   5. gcc -Wl,-Map=output.map test-gcc-driver-reset.c -o wl.out
 */

#endif

#if 0
/* ==================== SCENARIO D: Dependency Generation ====================
 *
 * Use -MF, -MT, -MQ options which interact with dumpbase and output naming.
 * Also test -gsplit-dwarf which creates multiple output files.
 *
 * Test sequence for harness:
 *   1. gcc -c -MF deps.d -MT target.o -dumpbase depgen \
 *        test-gcc-driver-reset.c -o depgen.o
 *   2. gcc -c -gsplit-dwarf -dumpbase split -o split.o test-gcc-driver-reset.c
 *   3. gcc -c -M -dumpdir ./deps/ -dumpbase mgen test-gcc-driver-reset.c
 */

#endif

#if 0
/* ==================== SCENARIO E: Combined Verbose & Dump ====================
 *
 * Use -v (verbose) which may print help/version-like output and show
 * subprocess commands, combined with dump options to maximize coverage
 * of the reset block.
 *
 * Test sequence for harness:
 *   1. gcc -save-temps=obj -dumpdir ./verbose_dump -dumpbase vtest \
 *        -v -O2 test-gcc-driver-reset.c -o vtest.out
 *   2. gcc -E -dD -dumpbase preproc -v test-gcc-driver-reset.c > preproc.i
 *   3. gcc -print-subprocess-help -v
 *   4. gcc -c -dumpbase after_help test-gcc-driver-reset.c -o after.o
 */

#endif

#if 0
/* ==================== SCENARIO F: Sysroot & Suffix Changes ====================
 *
 * Exercise target_system_root, target_sysroot_suffix, and
 * target_sysroot_hdrs_suffix via -specs and -B options.
 *
 * Test sequence for harness:
 *   1. gcc -specs=myspecs -B /dummy/path -c test-gcc-driver-reset.c -o spec.o
 *   2. gcc -c --sysroot=/alternate/root test-gcc-driver-reset.c -o sysroot.o
 *   3. gcc -c -isysroot /another/root test-gcc-driver-reset.c -o isysroot.o
 */

#endif
