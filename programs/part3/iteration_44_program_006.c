/* test-gcc-driver-reset.c
 * 
 * This file contains a minimal C program. Its primary purpose is to serve
 * as a vehicle for testing the GCC driver's internal state reset logic,
 * particularly the code block that resets dumpdir, dumpbase, outbase,
 * save_temps_flag, spec_machine, and other global variables.
 *
 * The actual test scenarios are described in the #if 0 blocks below.
 * A test harness should extract and execute these GCC command lines
 * in sequence to trigger the uncovered reset logic in gcc.cc.
 *
 * Compile this file directly with default flags to verify it's valid C:
 *   gcc -o test-default test-gcc-driver-reset.c
 */

int main(void) {
    return 0;
}

#if 0
/* ==================== SCENARIO A: Help/Version Reset ====================
 * 
 * First invoke GCC with help or version flags, then compile normally.
 * This should trigger the reset of print_help_list, print_version,
 * print_subprocess_help, and other state variables before the actual
 * compilation begins.
 *
 * Test sequence:
 * 1. gcc --help=common
 * 2. gcc -v
 * 3. gcc -c test-gcc-driver-reset.c -O2 -o test1.o
 * 4. gcc test-gcc-driver-reset.c -O2 -o test1
 */

/* Command 1: Print help for common options */
gcc --help=common

/* Command 2: Print version information */
gcc -v

/* Command 3: Compile to object file after help/version */
gcc -c test-gcc-driver-reset.c -O2 -o test1.o

/* Command 4: Compile executable after version display */
gcc test-gcc-driver-reset.c -O2 -o test1
#endif

#if 0
/* ==================== SCENARIO B: Save Temps with Custom Dump ====================
 *
 * Use -save-temps with custom dumpdir/dumpbase, then compile without them.
 * This should trigger the reset of save_temps_flag, dumpdir, dumpbase,
 * dumpbase_ext, outbase, and related variables.
 *
 * Test sequence:
 * 1. gcc -save-temps=obj -dumpdir ./mydumps/ -dumpbase myprog \
 *      test-gcc-driver-reset.c -O1 -o test2
 * 2. gcc test-gcc-driver-reset.c -O2 -o test3
 * 3. gcc -save-temps=cwd -dumpbase plain -dumpdir "" \
 *      test-gcc-driver-reset.c -o test4
 * 4. gcc -save-temps=none test-gcc-driver-reset.c -o test5
 */

/* Command 1: Save temps with custom dump directory and base name */
gcc -save-temps=obj -dumpdir ./mydumps/ -dumpbase myprog \
    test-gcc-driver-reset.c -O1 -o test2

/* Command 2: Compile without save-temps (triggers reset) */
gcc test-gcc-driver-reset.c -O2 -o test3

/* Command 3: Save temps in current directory with empty dumpdir */
gcc -save-temps=cwd -dumpbase plain -dumpdir "" \
    test-gcc-driver-reset.c -o test4

/* Command 4: Explicitly disable save temps */
gcc -save-temps=none test-gcc-driver-reset.c -o test5
#endif

#if 0
/* ==================== SCENARIO C: Multi-Stage & Specs ====================
 *
 * Exercise the driver in different modes (compiler, assembler, linker)
 * with custom specs and search paths. This should trigger reset of
 * spec_machine, target_system_root, and driver mode flags.
 *
 * Test sequence:
 * 1. gcc -S -dumpbase asmout -o my.s test-gcc-driver-reset.c
 * 2. gcc -c my.s -o my.o
 * 3. gcc -specs=myspecs -B ./mylib/ my.o -o final
 * 4. gcc -Wa,-al=my.lst -c test-gcc-driver-reset.c -o wa_test.o
 * 5. gcc -Wl,-Map=output.map test-gcc-driver-reset.c -o map_test
 */

/* Command 1: Generate assembly with dumpbase */
gcc -S -dumpbase asmout -o my.s test-gcc-driver-reset.c

/* Command 2: Assemble the generated assembly */
gcc -c my.s -o my.o

/* Command 3: Link with custom specs and library path */
gcc -specs=myspecs -B ./mylib/ my.o -o final

/* Command 4: Use assembler options (Wa) */
gcc -Wa,-al=my.lst -c test-gcc-driver-reset.c -o wa_test.o

/* Command 5: Use linker options (Wl) */
gcc -Wl,-Map=output.map test-gcc-driver-reset.c -o map_test
#endif

#if 0
/* ==================== SCENARIO D: Dependency Generation ====================
 *
 * Use dependency generation options that interact with dumpbase/outbase.
 * Also test split debug info which creates multiple output files.
 *
 * Test sequence:
 * 1. gcc -c -MF deps.d -MT target.o -dumpbase depgen \
 *      test-gcc-driver-reset.c -o target.o
 * 2. gcc -g -gsplit-dwarf -dumpbase splitdwarf \
 *      test-gcc-driver-reset.c -o split_test
 * 3. gcc -c -MD -MP -MF deps2.d -dumpbase "" \
 *      test-gcc-driver-reset.c -o nodump.o
 */

/* Command 1: Generate dependencies with custom dumpbase */
gcc -c -MF deps.d -MT target.o -dumpbase depgen \
    test-gcc-driver-reset.c -o target.o

/* Command 2: Generate split debug info */
gcc -g -gsplit-dwarf -dumpbase splitdwarf \
    test-gcc-driver-reset.c -o split_test

/* Command 3: Generate dependencies with empty dumpbase */
gcc -c -MD -MP -MF deps2.d -dumpbase "" \
    test-gcc-driver-reset.c -o nodump.o
#endif

#if 0
/* ==================== SCENARIO E: Combined Complex Invocation ====================
 *
 * A complex command that combines many features, followed by a simple one.
 * This maximizes the state that needs to be reset.
 *
 * Test sequence:
 * 1. gcc -save-temps=obj -dumpdir ./complex/ -dumpbase complex -outbase complex \
 *      -specs=myspecs -B /dummy/path -g -gsplit-dwarf -MF complex.d \
 *      -v --help=common test-gcc-driver-reset.c -o complex.out
 * 2. gcc test-gcc-driver-reset.c -o simple.out
 */

/* Command 1: Complex invocation with many options */
gcc -save-temps=obj -dumpdir ./complex/ -dumpbase complex -outbase complex \
    -specs=myspecs -B /dummy/path -g -gsplit-dwarf -MF complex.d \
    -v test-gcc-driver-reset.c -o complex.out

/* Command 2: Simple compilation (triggers full reset) */
gcc test-gcc-driver-reset.c -o simple.out
#endif
