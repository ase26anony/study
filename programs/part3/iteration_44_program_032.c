/* test-gcc-driver-reset.c
 * 
 * This file contains a minimal C program. Its primary purpose is to serve
 * as a vehicle for testing the GCC driver's internal state reset logic,
 * particularly the block in gcc.cc that resets dumpdir, dumpbase, outbase,
 * save_temps_flag, spec_machine, and other global variables.
 *
 * The actual test sequences are described in the #if 0 blocks below.
 * A test harness should extract and execute these GCC invocations in order.
 */

int main(void) {
    return 0;
}

#if 0
/* ==================== SCENARIO A: Help/Version Reset ====================
 * 
 * First invoke GCC with help or version flags, then compile the source.
 * This should trigger the reset of print_help_list, print_version, and
 * print_subprocess_help before the actual compilation begins.
 */

/* Step A1: Print help for common options */
gcc --help=common

/* Step A2: Print version information */
gcc -v

/* Step A3: Compile the source file with optimization */
gcc -O2 -o test1 test-gcc-driver-reset.c

#endif

#if 0
/* ==================== SCENARIO B: Save Temps & Dump Options ====================
 *
 * Use -save-temps with custom dumpdir/dumpbase, then compile without them.
 * This exercises the reset of save_temps_flag, dumpdir, dumpbase, dumpbase_ext,
 * outbase, and their associated length variables.
 */

/* Step B1: Compile with save-temps and custom dump options */
gcc -save-temps=obj -dumpdir ./mydumps/ -dumpbase myprog -O1 -o test2 test-gcc-driver-reset.c

/* Step B2: Compile again without save-temps (triggers reset to SAVE_TEMPS_NONE) */
gcc -O2 -o test3 test-gcc-driver-reset.c

/* Step B3: Another variation with different dumpbase and output */
gcc -save-temps=cwd -dumpbase alt -dumpbase-ext .foo -o test4 test-gcc-driver-reset.c

#endif

#if 0
/* ==================== SCENARIO C: Multi-Stage & Specs ====================
 *
 * Invoke GCC in different driver modes (compiler to assembly, assembler, linker)
 * with custom specs and -B prefix. This triggers re-initialization of
 * spec_machine and target_system_root variables.
 */

/* Step C1: Compile to assembly with dumpbase */
gcc -S -dumpbase asm -o my.s test-gcc-driver-reset.c

/* Step C2: Assemble the generated assembly (driver in assembler mode) */
gcc -c my.s

/* Step C3: Link with custom spec file and -B option */
gcc -specs=myspecs -B ./mylib/ my.o -o final

/* Step C4: Use -Wa and -Wl to invoke assembler/linker subprocesses */
gcc -Wa,-aln=my.lst -Wl,--verbose -o test5 test-gcc-driver-reset.c

#endif

#if 0
/* ==================== SCENARIO D: Dependency Generation ====================
 *
 * Use dependency generation options that interact with dumpbase/outbase.
 */

/* Step D1: Generate dependencies with custom dumpbase */
gcc -c -MF deps.d -MT target.o -dumpbase depgen test-gcc-driver-reset.c

/* Step D2: With split dwarf (creates .dwo files using output base logic) */
gcc -c -g -gsplit-dwarf -dumpdir ./dwarf/ -o split.o test-gcc-driver-reset.c

#endif

#if 0
/* ==================== SCENARIO E: Combined Verbose & Reset ====================
 *
 * Use -v (verbose) which shows subprocess help and may trigger multiple
 * internal resets between driver stages.
 */

/* Step E1: Verbose compilation with save-temps and dump options */
gcc -save-temps=obj -dumpdir ./verbose_dump/ -dumpbase verbose -O2 -v -o test6 test-gcc-driver-reset.c

/* Step E2: Preprocessor-only with dumpbase */
gcc -E -dD -dumpbase preproc test-gcc-driver-reset.c > preproc.out

#endif

#if 0
/* ==================== SCENARIO F: Sysroot & Target Options ====================
 *
 * Exercise target_system_root and related variables via -specs and -B.
 */

/* Step F1: Compile with a spec file that may redefine sysroot */
gcc -specs=myspecs -B /custom/path/ -o test7 test-gcc-driver-reset.c

/* Step F2: Change sysroot configuration between invocations */
gcc --sysroot=/alt/sysroot -o test8 test-gcc-driver-reset.c

#endif
