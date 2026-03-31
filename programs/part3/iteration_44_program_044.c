/* test-gcc-driver-reset.c
 *
 * This file contains a minimal C program. Its primary purpose is to serve as
 * a vehicle for testing GCC driver state reset logic, particularly the block
 * in gcc.cc that resets dumpdir, dumpbase, outbase, save_temps_flag,
 * spec_machine, and other global variables.
 *
 * The actual test is performed by a test harness that extracts the
 * compilation scenarios described in the #if 0 blocks below and executes
 * them as separate GCC invocations. The program itself compiles cleanly
 * if processed directly.
 */

int main(void) {
    return 0;
}

#if 0
/* ==================== SCENARIO A: Help/Version Reset ====================
 *
 * First invoke GCC with help or version flags, then compile the source.
 * This should trigger the reset block after printing help/version info
 * before proceeding to compilation.
 *
 * Test harness instructions:
 *   1. gcc --help=common
 *   2. gcc -v
 *   3. gcc -c test-gcc-driver-reset.c -O2 -o test1.o
 *   4. gcc test-gcc-driver-reset.c -O2 -o test1
 */
#endif

#if 0
/* ==================== SCENARIO B: Save Temps with Custom Dump ====================
 *
 * Use -save-temps with custom dumpdir/dumpbase, then compile without them.
 * This should trigger reset of dumpdir, dumpbase, dumpbase_ext, outbase,
 * and save_temps_flag.
 *
 * Test harness instructions:
 *   1. gcc -save-temps=obj -dumpdir ./mydumps/ -dumpbase myprog \
 *        test-gcc-driver-reset.c -O1 -o test2
 *   2. gcc test-gcc-driver-reset.c -O2 -o test3
 *   3. gcc -save-temps=cwd -dumpbase altbase test-gcc-driver-reset.c -o test4
 *   4. gcc test-gcc-driver-reset.c -o test5  # no save-temps
 */
#endif

#if 0
/* ==================== SCENARIO C: Multi-Stage & Specs ====================
 *
 * Exercise driver in different modes (compiler to assembly, assembler, linker)
 * with custom specs and -B options. This can affect spec_machine and
 * target_system_root variables.
 *
 * Test harness instructions:
 *   1. gcc -S -dumpbase asmout -o myasm.s test-gcc-driver-reset.c
 *   2. gcc -c myasm.s -o myasm.o
 *   3. gcc -specs=myspecs -B ./mylib/ myasm.o -o final1
 *   4. gcc -Wa,-adhln -c test-gcc-driver-reset.c -o list.o  # assembler mode
 *   5. gcc -Wl,--verbose -o final2 test-gcc-driver-reset.c  # linker mode
 */
#endif

#if 0
/* ==================== SCENARIO D: Dependency Generation ====================
 *
 * Use dependency generation options that interact with dumpbase/outbase.
 *
 * Test harness instructions:
 *   1. gcc -c -MF deps.d -MT target.o -dumpbase depgen \
 *        test-gcc-driver-reset.c -o depgen.o
 *   2. gcc -MMD -MP -dumpdir ./deps/ test-gcc-driver-reset.c -o mmd.o
 *   3. gcc -c -MQ 'special.o' -MF special.d test-gcc-driver-reset.c
 */
#endif

#if 0
/* ==================== SCENARIO E: Combined Coverage ====================
 *
 * A comprehensive command that combines many options to maximize coverage
 * of the reset block in a single invocation.
 *
 * Test harness instructions:
 *   1. gcc -save-temps=obj -dumpdir ./combined/ -dumpbase combined \
 *        -specs=myspecs -B /dummy/path -MF combined.d -MT combined.o \
 *        -gsplit-dwarf -g -O2 -v --help=common \
 *        test-gcc-driver-reset.c -o combined
 *   Note: --help=common will cause GCC to print help and exit, so it should
 *   be run separately. The -v flag alone triggers verbose output which
 *   shows driver stages and may exercise reset logic between phases.
 */
#endif

#if 0
/* ==================== SCENARIO F: Debug Split Dwarf ====================
 *
 * Use -gsplit-dwarf which creates multiple output files and interacts with
 * dumpbase/outbase logic.
 *
 * Test harness instructions:
 *   1. gcc -c -gsplit-dwarf -dumpbase splitdwarf \
 *        test-gcc-driver-reset.c -o splitdwarf.o
 *   2. gcc -c -g test-gcc-driver-reset.c -o normaldebug.o
 */
#endif

#if 0
/* ==================== SCENARIO G: Preprocessor Mode ====================
 *
 * Invoke GCC as a preprocessor (-E) which uses different driver mode.
 *
 * Test harness instructions:
 *   1. gcc -E -dD -dumpbase preproc test-gcc-driver-reset.c -o preproc.i
 *   2. gcc -c preproc.i -o from_preproc.o
 */
#endif
