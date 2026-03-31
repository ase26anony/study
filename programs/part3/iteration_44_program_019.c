/* test-gcc-driver-reset.c
 *
 * This file contains a minimal C program. Its primary purpose is to serve as
 * a vehicle for testing GCC driver state reset logic, particularly the block
 * in gcc.cc that resets dumpdir, dumpbase, outbase, save_temps_flag,
 * spec_machine, and other global variables.
 *
 * The actual test scenarios are described in the #if 0 blocks below.
 * A test harness should extract and execute these GCC command lines
 * sequentially, possibly in the same process or in a controlled environment
 * that mimics the driver's behavior across multiple invocations.
 */

int main(void) {
    return 0;
}

#if 0
/* ==================== SCENARIO A: Help/Version Reset ====================
 *
 * First, invoke GCC with a help or version flag to set print_help_list,
 * print_version, or print_subprocess_help. Then immediately compile the
 * source file. The driver should reset these flags and global state before
 * proceeding with compilation.
 *
 * Test sequence for a harness:
 *   1. gcc --help=common
 *   2. gcc -v
 *   3. gcc -c test-gcc-driver-reset.c -O2 -o test1.o
 *
 * The third command should trigger the reset block after processing the
 * help/version flags from previous invocations (if the driver retains state
 * across calls in the test environment).
 */
#endif

#if 0
/* ==================== SCENARIO B: Save Temps & Dump Options =============
 *
 * This scenario exercises save_temps_flag, dumpdir, dumpbase, and outbase.
 * First, compile with -save-temps=obj and custom dump options, then compile
 * again without them. The second compilation should reset the dump-related
 * variables and save_temps_flag.
 *
 * Test sequence:
 *   1. gcc -save-temps=obj -dumpdir ./mydumps/ -dumpbase myprog \
 *        -dumpbase-ext .c -o test2 test-gcc-driver-reset.c
 *   2. gcc -O2 -o test3 test-gcc-driver-reset.c
 *
 * The second command should clear dumpdir, dumpbase, dumpbase_ext, outbase,
 * and set save_temps_flag to SAVE_TEMPS_NONE.
 */
#endif

#if 0
/* ==================== SCENARIO C: Multi-Stage & Specs ===================
 *
 * Exercise the driver in different modes (compiler to assembly, assembler,
 * linker) and with spec files that may affect target_system_root.
 * Transitioning between modes can trigger re-initialization of spec_machine
 * and other state.
 *
 * Test sequence:
 *   1. gcc -S -dumpbase asm -o my.s test-gcc-driver-reset.c
 *   2. gcc -c my.s
 *   3. gcc -specs=myspecs -B ./mylib/ my.o -o final
 *
 * The spec file "myspecs" could contain:
 *   *%R: /custom/sysroot
 *   *sysroot: %R
 * This may set target_system_root and target_system_root_changed.
 * The -B option adds a prefix to search paths, interacting with sysroot logic.
 */
#endif

#if 0
/* ==================== SCENARIO D: Dependency Generation =================
 *
 * Use -MF, -MT, -MQ options which interact with dumpbase and output naming.
 * Also include -gsplit-dwarf to generate multiple auxiliary outputs.
 *
 * Test sequence:
 *   1. gcc -c -MF deps.d -MT target.o -dumpbase depgen \
 *        -gsplit-dwarf -o depgen.o test-gcc-driver-reset.c
 *   2. gcc -c -MF deps2.d -MQ 'target2.o' -o simple.o test-gcc-driver-reset.c
 *
 * The second command should reset dumpbase and related state from the first.
 */
#endif

#if 0
/* ==================== SCENARIO E: Combined Verbose & Dump ===============
 *
 * Use -v (verbose) which may print help-like information and subprocess help,
 * combined with dump options. This can trigger print_subprocess_help and
 * verbose_only_flag, which are reset in the target block.
 *
 * Test sequence:
 *   1. gcc -save-temps=obj -dumpdir ./verbose_dump/ -dumpbase verbose \
 *        -O2 -v -o verbose_test test-gcc-driver-reset.c
 *   2. gcc -O1 -o normal.o test-gcc-driver-reset.c
 *
 * The verbose flag may cause the driver to print subprocess commands and
 * version info, setting print_subprocess_help and verbose_only_flag.
 */
#endif

#if 0
/* ==================== SCENARIO F: Sysroot Suffix Changes ================
 *
 * Use -isysroot and -B options to change target_system_root and its suffixes.
 * The reset block clears target_sysroot_suffix and target_sysroot_hdrs_suffix.
 *
 * Test sequence:
 *   1. gcc -isysroot /custom/sysroot -B /custom/bin/ \
 *        -c -o sysroot1.o test-gcc-driver-reset.c
 *   2. gcc -c -o sysroot2.o test-gcc-driver-reset.c
 *
 * The second command should reset the sysroot suffix variables to 0.
 */
#endif

/* End of test scenarios. */
