/* test_gcc_driver_reset.c
 *
 * This file contains a trivial C program. Its main purpose is to serve as
 * a vehicle for testing GCC driver state reset logic, particularly the
 * uncovered lines in gcc.cc that reset dumpdir, dumpbase, outbase,
 * save_temps_flag, spec_machine, and other global variables.
 *
 * The actual test scenarios are described in the #if 0 blocks below.
 * A test harness should extract and execute these GCC invocations in
 * sequence to trigger the target reset logic.
 */

int main(void) {
    return 0;
}

#if 0
/* ==================== SCENARIO A: Help/Version Reset ====================
 *
 * First invoke GCC with help or version flags, then compile the source.
 * This should trigger reset of print_help_list, print_version,
 * print_subprocess_help, and other state before the actual compilation.
 *
 * Test harness steps:
 *   1. gcc --help=common
 *   2. gcc -v
 *   3. gcc -c test_gcc_driver_reset.c -O2 -o test1.o
 *   4. gcc test1.o -o test1
 */

/* Command 1: Print help for common options */
// gcc --help=common

/* Command 2: Print version information */
// gcc -v

/* Command 3: Compile after help/version output (triggers state reset) */
// gcc -c test_gcc_driver_reset.c -O2 -o test1.o

/* Command 4: Link the object file */
// gcc test1.o -o test1

#endif

#if 0
/* ==================== SCENARIO B: Save Temps & Dump Options ====================
 *
 * Use -save-temps with custom -dumpdir/-dumpbase, then compile without them.
 * This should trigger reset of save_temps_flag, dumpdir, dumpbase, dumpbase_ext,
 * outbase, and related length variables.
 *
 * Test harness steps:
 *   1. gcc -save-temps=obj -dumpdir ./dump1 -dumpbase myapp -O1 -c test_gcc_driver_reset.c -o test2.o
 *   2. gcc test2.o -o test2
 *   3. gcc -c test_gcc_driver_reset.c -O2 -o test3.o  (no save-temps/dump options)
 *   4. gcc test3.o -o test3
 */

/* Command 1: Compile with save-temps and custom dump options */
// gcc -save-temps=obj -dumpdir ./dump1 -dumpbase myapp -O1 -c test_gcc_driver_reset.c -o test2.o

/* Command 2: Link with the same dumpbase context */
// gcc test2.o -o test2

/* Command 3: Compile without save-temps (triggers reset to SAVE_TEMPS_NONE) */
// gcc -c test_gcc_driver_reset.c -O2 -o test3.o

/* Command 4: Link the second object */
// gcc test3.o -o test3

#endif

#if 0
/* ==================== SCENARIO C: Multi-Stage & Specs ====================
 *
 * Exercise the driver in different modes (compiler to assembly, assembler,
 * linker) with -specs and -B options. This may trigger reset of spec_machine,
 * target_system_root, and other driver mode state.
 *
 * Test harness steps:
 *   1. gcc -S -dumpbase asm -o my.s test_gcc_driver_reset.c
 *   2. gcc -c my.s -o my.o
 *   3. gcc -specs=myspecs -B ./mylib/ my.o -o final
 *
 * Note: Create a dummy myspecs file and ./mylib/ directory for testing.
 */

/* Command 1: Generate assembly with dumpbase */
// gcc -S -dumpbase asm -o my.s test_gcc_driver_reset.c

/* Command 2: Assemble the generated assembly */
// gcc -c my.s -o my.o

/* Command 3: Link with custom specs and prefix */
// gcc -specs=myspecs -B ./mylib/ my.o -o final

#endif

#if 0
/* ==================== SCENARIO D: Dependency Generation ====================
 *
 * Use dependency generation options with dumpbase, exercising output naming
 * infrastructure and subsequent reset.
 *
 * Test harness steps:
 *   1. gcc -c -MF deps.d -MT target.o -dumpbase depgen -O1 test_gcc_driver_reset.c -o target.o
 *   2. gcc -c test_gcc_driver_reset.c -O2 -o plain.o  (no dependency options)
 */

/* Command 1: Compile with dependency file generation */
// gcc -c -MF deps.d -MT target.o -dumpbase depgen -O1 test_gcc_driver_reset.c -o target.o

/* Command 2: Compile without dependency options (triggers reset) */
// gcc -c test_gcc_driver_reset.c -O2 -o plain.o

#endif

#if 0
/* ==================== SCENARIO E: Combined Verbose & Dump Options ====================
 *
 * Use -v (verbose) with save-temps and dump options to expose driver stages
 * and increase chance of state resets between phases.
 *
 * Test harness steps:
 *   1. gcc -save-temps=obj -dumpdir ./testdump -dumpbase mytest -O2 -v -c test_gcc_driver_reset.c -o verbose.o
 *   2. gcc verbose.o -o verbose
 */

/* Command 1: Verbose compilation with all dump options */
// gcc -save-temps=obj -dumpdir ./testdump -dumpbase mytest -O2 -v -c test_gcc_driver_reset.c -o verbose.o

/* Command 2: Link the verbose compilation output */
// gcc verbose.o -o verbose

#endif

#if 0
/* ==================== SCENARIO F: Split DWARF & Debug Info ====================
 *
 * Use split debug information options which create multiple output files
 * and interact with output base name logic.
 *
 * Test harness steps:
 *   1. gcc -gsplit-dwarf -g -dumpbase splitdwarf -O1 -c test_gcc_driver_reset.c -o split.o
 *   2. gcc split.o -o split
 */

/* Command 1: Compile with split DWARF debug info */
// gcc -gsplit-dwarf -g -dumpbase splitdwarf -O1 -c test_gcc_driver_reset.c -o split.o

/* Command 2: Link the split debug object */
// gcc split.o -o split

#endif

#if 0
/* ==================== SCENARIO G: Driver Mode Switches ====================
 *
 * Invoke GCC as different tools via -Wl, and -Wa, options, potentially
 * triggering driver mode changes and state resets.
 *
 * Test harness steps:
 *   1. gcc -Wl,--verbose -c test_gcc_driver_reset.c -o linker_mode.o
 *   2. gcc -Wa,--verbose -c test_gcc_driver_reset.c -o assembler_mode.o
 *   3. gcc -E -dD -dumpbase preproc test_gcc_driver_reset.c > preprocessed.i
 */

/* Command 1: Invoke linker with verbose flag through driver */
// gcc -Wl,--verbose -c test_gcc_driver_reset.c -o linker_mode.o

/* Command 2: Invoke assembler with verbose flag through driver */
// gcc -Wa,--verbose -c test_gcc_driver_reset.c -o assembler_mode.o

/* Command 3: Preprocess only with dumpbase */
// gcc -E -dD -dumpbase preproc test_gcc_driver_reset.c > preprocessed.i

#endif
