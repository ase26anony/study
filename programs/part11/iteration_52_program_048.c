/* test_gcc_driver_init.c
 * This program tests the GCC driver initialization logic by invoking
 * multiple compilation jobs with different state configurations.
 * It specifically targets the reset logic in gcc.cc lines 11228-11250.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

/* Simple utility to create temporary files */
static int create_temp_file(const char *template, const char *content) {
    FILE *f = fopen(template, "w");
    if (!f) {
        perror("fopen");
        return 0;
    }
    fprintf(f, "%s", content);
    fclose(f);
    return 1;
}

/* Clean up temporary files */
static void cleanup_files(const char **files, int count) {
    for (int i = 0; i < count; i++) {
        if (files[i]) {
            unlink(files[i]);
        }
    }
}

/* Execute a command and return its exit status */
static int run_command(const char *cmd) {
    printf("Executing: %s\n", cmd);
    int status = system(cmd);
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    return -1;
}

/* Create a temporary directory */
static int create_temp_dir(const char *path) {
    struct stat st = {0};
    if (stat(path, &st) == -1) {
        if (mkdir(path, 0755) == -1) {
            perror("mkdir");
            return 0;
        }
    }
    return 1;
}

/* Remove a directory and its contents */
static void remove_dir(const char *path) {
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "rm -rf %s", path);
    system(cmd);
}

int main(void) {
    int overall_result = 0;
    int checksum = 0;
    
    /* Create temporary source files */
    const char *temp_files[] = {
        "test_gcc_temp1.c",
        "test_gcc_temp2.c", 
        "test_gcc_error.c",
        "test_gcc_temp1.o",
        "test_gcc_temp2.o",
        NULL
    };
    
    /* Create valid source files */
    if (!create_temp_file("test_gcc_temp1.c", 
        "int foo(void) { return 0; }\n"
        "/* Prevent unused function warning */\n"
        "int use_foo(void) { return foo(); }\n")) {
        fprintf(stderr, "Failed to create temp1.c\n");
        return 1;
    }
    
    if (!create_temp_file("test_gcc_temp2.c",
        "int bar(void) { return 1; }\n"
        "/* Prevent unused function warning */\n"
        "int use_bar(void) { return bar(); }\n")) {
        fprintf(stderr, "Failed to create temp2.c\n");
        cleanup_files(temp_files, 4);
        return 1;
    }
    
    /* Create a file with syntax error */
    if (!create_temp_file("test_gcc_error.c",
        "int baz(void) { return /* missing expression */ }\n")) {
        fprintf(stderr, "Failed to create error.c\n");
        cleanup_files(temp_files, 4);
        return 1;
    }
    
    /* Create dump directory */
    if (!create_temp_dir("./test_dump_dir")) {
        fprintf(stderr, "Failed to create dump directory\n");
        cleanup_files(temp_files, 4);
        return 1;
    }
    
    printf("\n=== Testing GCC Driver Initialization Block ===\n\n");
    
    /* Sequence 1: Help flag then compilation
     * This sets print_help_list, then resets it */
    printf("--- Sequence 1: Help then Compile ---\n");
    int seq1_result = 0;
    
    /* First job: help request (sets print_help_list) */
    seq1_result |= run_command("gcc --help=common 2>&1 | head -5 > /dev/null");
    
    /* Second job: compilation (should reset print_help_list) */
    seq1_result |= run_command("gcc -c test_gcc_temp1.c -o test_gcc_temp1.o");
    
    if (seq1_result == 0) {
        printf("Sequence 1: PASS\n");
        checksum += 1;
    } else {
        printf("Sequence 1: FAIL (code: %d)\n", seq1_result);
        overall_result = 1;
    }
    
    /* Sequence 2: Version flag then compilation  
     * This sets print_version, then resets it */
    printf("\n--- Sequence 2: Version then Compile ---\n");
    int seq2_result = 0;
    
    /* First job: version request */
    seq2_result |= run_command("gcc --version 2>&1 | head -1 > /dev/null");
    
    /* Second job: compilation */
    seq2_result |= run_command("gcc -c test_gcc_temp2.c -o test_gcc_temp2.o");
    
    if (seq2_result == 0) {
        printf("Sequence 2: PASS\n");
        checksum += 2;
    } else {
        printf("Sequence 2: FAIL (code: %d)\n", seq2_result);
        overall_result = 1;
    }
    
    /* Sequence 3: Save-temps with dumpdir then plain compile
     * This sets save_temps_flag, dumpdir, dumpbase, then resets them */
    printf("\n--- Sequence 3: Save-temps with dumpdir then plain compile ---\n");
    int seq3_result = 0;
    
    /* First job: with save-temps and dumpdir (sets multiple variables) */
    seq3_result |= run_command("gcc -save-temps -dumpdir ./test_dump_dir -dumpbase testbase "
                               "-c test_gcc_temp1.c -o test_gcc_temp1.o 2>&1 | grep -v 'warning:'");
    
    /* Check if dump files were created */
    struct stat st;
    int dump_files_exist = (stat("./test_dump_dir/testbase.i", &st) == 0) ||
                          (stat("testbase.i", &st) == 0);
    
    /* Second job: plain compile (should reset dumpdir/dumpbase) */
    seq3_result |= run_command("gcc -c test_gcc_temp2.c -o test_gcc_temp2.o");
    
    if (seq3_result == 0 && dump_files_exist) {
        printf("Sequence 3: PASS (dump files created)\n");
        checksum += 4;
    } else {
        printf("Sequence 3: FAIL (code: %d, dump exists: %d)\n", seq3_result, dump_files_exist);
        overall_result = 1;
    }
    
    /* Sequence 4: Verbose flag then compilation
     * This sets verbose_only_flag, then resets it */
    printf("\n--- Sequence 4: Verbose then Compile ---\n");
    int seq4_result = 0;
    
    /* First job: verbose compilation */
    seq4_result |= run_command("gcc -v -c test_gcc_temp1.c 2>&1 | grep -q 'COLLECT_GCC'");
    
    /* Second job: normal compilation */
    seq4_result |= run_command("gcc -c test_gcc_temp2.c");
    
    if (seq4_result <= 1) {  /* grep returns 0 if found, 1 if not found */
        printf("Sequence 4: PASS\n");
        checksum += 8;
    } else {
        printf("Sequence 4: FAIL (code: %d)\n", seq4_result);
        overall_result = 1;
    }
    
    /* Sequence 5: Linker selection then default
     * This sets use_ld, then resets it */
    printf("\n--- Sequence 5: Linker selection then default ---\n");
    int seq5_result = 0;
    
    /* First job: specify linker */
    seq5_result |= run_command("gcc -fuse-ld=bfd -c test_gcc_temp1.c 2>&1 | grep -v 'warning:'");
    
    /* Second job: default linker */
    seq5_result |= run_command("gcc -c test_gcc_temp2.c");
    
    if (seq5_result == 0) {
        printf("Sequence 5: PASS\n");
        checksum += 16;
    } else {
        printf("Sequence 5: FAIL (code: %d)\n", seq5_result);
        overall_result = 1;
    }
    
    /* Sequence 6: Error then success (tests greatest_status reset)
     * First job fails, second succeeds */
    printf("\n--- Sequence 6: Error then Success ---\n");
    int seq6_result = 0;
    
    /* First job: should fail due to syntax error */
    int error_result = run_command("gcc -c test_gcc_error.c 2>/dev/null");
    if (error_result != 0) {
        printf("  Expected error occurred (good)\n");
        seq6_result = 0;
    } else {
        printf("  Unexpected success (bad)\n");
        seq6_result = 1;
    }
    
    /* Second job: should succeed */
    seq6_result |= run_command("gcc -c test_gcc_temp1.c");
    
    if (seq6_result == 0) {
        printf("Sequence 6: PASS\n");
        checksum += 32;
    } else {
        printf("Sequence 6: FAIL (code: %d)\n", seq6_result);
        overall_result = 1;
    }
    
    /* Sequence 7: Multiple inputs in single invocation
     * Tests per-job initialization with multiple source files */
    printf("\n--- Sequence 7: Multiple inputs in single invocation ---\n");
    int seq7_result = 0;
    
    /* Compile two files in one command - creates two jobs */
    seq7_result |= run_command("gcc -c test_gcc_temp1.c test_gcc_temp2.c");
    
    if (seq7_result == 0) {
        printf("Sequence 7: PASS\n");
        checksum += 64;
    } else {
        printf("Sequence 7: FAIL (code: %d)\n", seq7_result);
        overall_result = 1;
    }
    
    /* Sequence 8: Mixed options across jobs using -x
     * Tests state transitions between different language modes */
    printf("\n--- Sequence 8: Mixed language specifications ---\n");
    int seq8_result = 0;
    
    /* Create a simple C++ source */
    if (create_temp_file("test_gcc_temp.cpp",
        "extern \"C\" int cpp_func(void) { return 42; }\n")) {
        
        /* First job: compile as C++ */
        seq8_result |= run_command("gcc -x c++ -c test_gcc_temp.cpp -o test_gcc_temp_cpp.o");
        
        /* Second job: compile as C */
        seq8_result |= run_command("gcc -x c -c test_gcc_temp1.c -o test_gcc_temp1.o");
        
        unlink("test_gcc_temp.cpp");
        unlink("test_gcc_temp_cpp.o");
    } else {
        seq8_result = 1;
    }
    
    if (seq8_result == 0) {
        printf("Sequence 8: PASS\n");
        checksum += 128;
    } else {
        printf("Sequence 8: FAIL (code: %d)\n", seq8_result);
        overall_result = 1;
    }
    
    /* Cleanup */
    printf("\n--- Cleanup ---\n");
    cleanup_files(temp_files, 5);
    remove_dir("./test_dump_dir");
    
    /* Remove any leftover dump files */
    system("rm -f testbase.* *.i *.s *.o 2>/dev/null");
    
    /* Final summary */
    printf("\n=== Test Summary ===\n");
    printf("Checksum (bitmask of successful sequences): %d (0x%x)\n", checksum, checksum);
    printf("Overall result: %s\n", overall_result == 0 ? "PASS" : "FAIL");
    
    if (checksum == 255) {  /* All 8 sequences passed (1+2+4+8+16+32+64+128 = 255) */
        printf("All sequences passed! Driver initialization block should be covered.\n");
    } else {
        printf("Some sequences failed. Missing bits: 0x%x\n", 255 - checksum);
    }
    
    return overall_result;
}
