/* test_gcc_driver_reset.c
 * A program to test GCC driver reinitialization logic
 * Specifically targets lines 11228-11250 in gcc.cc
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <dlfcn.h>
#include <errno.h>

/* Define if we want to use direct library calls */
#define USE_DIRECT_LIBRARY_CALL 0

/* Path to GCC driver */
#ifndef GCC_PATH
#define GCC_PATH "gcc"
#endif

/* Test input file */
#define TEST_INPUT_FILE "test_input.c"
#define TEST_SPEC_FILE "test.specs"

/* Create a minimal test input file */
void create_test_input(void) {
    FILE *f = fopen(TEST_INPUT_FILE, "w");
    if (!f) {
        perror("Failed to create test input file");
        exit(1);
    }
    
    fprintf(f, "/* Minimal test file for GCC driver reset testing */\n");
    fprintf(f, "int main(void) {\n");
    fprintf(f, "    return 0;\n");
    fprintf(f, "}\n");
    fclose(f);
    
    printf("Created test input file: %s\n", TEST_INPUT_FILE);
}

/* Create a test spec file to alter spec machine */
void create_test_specs(void) {
    FILE *f = fopen(TEST_SPEC_FILE, "w");
    if (!f) {
        perror("Failed to create test spec file");
        exit(1);
    }
    
    fprintf(f, "*cpp:\n");
    fprintf(f, "-DSPEC_TEST=1\n");
    fclose(f);
    
    printf("Created test spec file: %s\n", TEST_SPEC_FILE);
}

/* Method A: Process-based invocation using fork/exec */
int invoke_gcc_process(const char **argv) {
    pid_t pid = fork();
    
    if (pid == 0) {
        /* Child process */
        execv(GCC_PATH, (char * const *)argv);
        /* If execv returns, it failed */
        perror("execv failed");
        exit(127);
    } else if (pid > 0) {
        /* Parent process */
        int status;
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status)) {
            return WEXITSTATUS(status);
        } else {
            return -1;
        }
    } else {
        /* Fork failed */
        perror("fork failed");
        return -1;
    }
}

/* Method B: Direct library call (if GCC driver is built as shared library) */
#if USE_DIRECT_LIBRARY_CALL
int invoke_gcc_library(int argc, const char **argv) {
    static void *handle = NULL;
    static int (*gcc_main)(int, const char **) = NULL;
    
    if (!handle) {
        handle = dlopen("./libgccdriver.so", RTLD_LAZY | RTLD_LOCAL);
        if (!handle) {
            fprintf(stderr, "dlopen failed: %s\n", dlerror());
            return -1;
        }
        
        gcc_main = (int (*)(int, const char **))dlsym(handle, "main");
        if (!gcc_main) {
            fprintf(stderr, "dlsym failed: %s\n", dlerror());
            dlclose(handle);
            return -1;
        }
    }
    
    return gcc_main(argc, argv);
}
#endif

/* Test case 1: Multi-stage compilation with save-temps and dump options */
int test_multi_stage(void) {
    printf("\n=== Test 1: Multi-stage compilation with save-temps ===\n");
    
    /* Stage 1: Compile with save-temps, dumpdir, dumpbase, and sysroot */
    const char *stage1_args[] = {
        GCC_PATH,
        "-save-temps",
        "-dumpdir", "/tmp/gcc_test_dump",
        "-dumpbase", "test_dumpbase",
        "-dumpbase-ext", ".c",
        "--sysroot=/tmp/alt_sysroot",
        "-mtune=generic",
        "-c", TEST_INPUT_FILE,
        "-o", "output1.o",
        NULL
    };
    
    printf("Stage 1: With save-temps, dumpdir, sysroot\n");
    int status1 = invoke_gcc_process(stage1_args);
    printf("Exit status: %d\n", status1);
    
    /* Stage 2: Simple compilation - should trigger reset of all stage1 options */
    const char *stage2_args[] = {
        GCC_PATH,
        "-c", TEST_INPUT_FILE,
        "-o", "output2.o",
        NULL
    };
    
    printf("\nStage 2: Simple compilation (triggers reset)\n");
    int status2 = invoke_gcc_process(stage2_args);
    printf("Exit status: %d\n", status2);
    
    return status1 | status2;
}

/* Test case 2: Spec machine and target system root reset */
int test_spec_reset(void) {
    printf("\n=== Test 2: Spec machine and sysroot reset ===\n");
    
    /* First with custom specs and sysroot */
    const char *stage1_args[] = {
        GCC_PATH,
        "-specs", TEST_SPEC_FILE,
        "--sysroot=/tmp/custom_sysroot",
        "-march=x86-64",
        "-c", TEST_INPUT_FILE,
        "-o", "spec_output1.o",
        NULL
    };
    
    printf("Stage 1: With custom specs and sysroot\n");
    int status1 = invoke_gcc_process(stage1_args);
    printf("Exit status: %d\n", status1);
    
    /* Second without - should reset to defaults */
    const char *stage2_args[] = {
        GCC_PATH,
        "-c", TEST_INPUT_FILE,
        "-o", "spec_output2.o",
        NULL
    };
    
    printf("\nStage 2: Without specs/sysroot (triggers reset)\n");
    int status2 = invoke_gcc_process(stage2_args);
    printf("Exit status: %d\n", status2);
    
    return status1 | status2;
}

/* Test case 3: Greatest status reset */
int test_status_reset(void) {
    printf("\n=== Test 3: Greatest status reset ===\n");
    
    /* First: Invalid option - should fail */
    const char *stage1_args[] = {
        GCC_PATH,
        "-invalid-option-that-does-not-exist",
        NULL
    };
    
    printf("Stage 1: Invalid option (should fail)\n");
    int status1 = invoke_gcc_process(stage1_args);
    printf("Exit status: %d (non-zero expected)\n", status1);
    
    /* Second: Valid compilation - should succeed */
    const char *stage2_args[] = {
        GCC_PATH,
        "-c", TEST_INPUT_FILE,
        "-o", "status_output.o",
        NULL
    };
    
    printf("\nStage 2: Valid compilation (should succeed)\n");
    int status2 = invoke_gcc_process(stage2_args);
    printf("Exit status: %d (zero expected)\n", status2);
    
    /* Third: Another failure */
    const char *stage3_args[] = {
        GCC_PATH,
        "-nonexistent-flag",
        NULL
    };
    
    printf("\nStage 3: Another failure\n");
    int status3 = invoke_gcc_process(stage3_args);
    printf("Exit status: %d (non-zero expected)\n", status3);
    
    /* Fourth: Success again */
    const char *stage4_args[] = {
        GCC_PATH,
        "-c", TEST_INPUT_FILE,
        "-o", "status_output2.o",
        NULL
    };
    
    printf("\nStage 4: Success again\n");
    int status4 = invoke_gcc_process(stage4_args);
    printf("Exit status: %d (zero expected)\n", status4);
    
    return (status1 != 0 && status2 == 0 && status3 != 0 && status4 == 0) ? 0 : 1;
}

/* Test case 4: Combined test with all resets */
int test_combined_reset(void) {
    printf("\n=== Test 4: Combined reset test ===\n");
    
    /* Complex first invocation with many options */
    const char *stage1_args[] = {
        GCC_PATH,
        "-save-temps=obj",
        "-dumpdir", "/tmp/combined",
        "-dumpbase", "combined",
        "--sysroot=/tmp/mysysroot",
        "-specs", TEST_SPEC_FILE,
        "-mtune=native",
        "-v",  /* verbose */
        "-c", TEST_INPUT_FILE,
        "-o", "combined1.o",
        NULL
    };
    
    printf("Stage 1: Complex invocation\n");
    int status1 = invoke_gcc_process(stage1_args);
    
    /* Simple second invocation - triggers all resets */
    const char *stage2_args[] = {
        GCC_PATH,
        "-c", TEST_INPUT_FILE,
        "-o", "combined2.o",
        NULL
    };
    
    printf("\nStage 2: Simple invocation (triggers full reset)\n");
    int status2 = invoke_gcc_process(stage2_args);
    
    return status1 | status2;
}

int main(int argc, char **argv) {
    printf("GCC Driver Reset Test Program\n");
    printf("Targeting lines 11228-11250 in gcc.cc\n");
    
    /* Create necessary test files */
    create_test_input();
    create_test_specs();
    
    int overall_result = 0;
    
    /* Run all test cases */
    overall_result |= test_multi_stage();
    overall_result |= test_spec_reset();
    overall_result |= test_status_reset();
    overall_result |= test_combined_reset();
    
    /* Cleanup */
    unlink(TEST_INPUT_FILE);
    unlink(TEST_SPEC_FILE);
    unlink("output1.o");
    unlink("output2.o");
    unlink("spec_output1.o");
    unlink("spec_output2.o");
    unlink("status_output.o");
    unlink("status_output2.o");
    unlink("combined1.o");
    unlink("combined2.o");
    
    printf("\n=== Test Summary ===\n");
    printf("Overall result: %s\n", overall_result == 0 ? "PASS" : "FAIL");
    printf("\nTargeted reset variables:\n");
    printf("- save_temps_flag (SAVE_TEMPS_NONE)\n");
    printf("- dumpdir, dumpbase, dumpbase_ext, outbase (NULL)\n");
    printf("- target_system_root (DEFAULT_TARGET_SYSTEM_ROOT)\n");
    printf("- spec_machine (DEFAULT_TARGET_MACHINE)\n");
    printf("- greatest_status (1)\n");
    
    return overall_result;
}
