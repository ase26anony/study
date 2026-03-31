/* test_gcc_driver_reset.c
 * A program to test GCC driver reinitialization logic by invoking it multiple times
 * with different configurations to trigger state resets.
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <dlfcn.h>
#include <errno.h>

/* Define this to use direct library calls instead of fork/exec */
/* #define USE_DIRECT_LIBRARY_CALL */

/* Path to GCC driver */
#ifndef GCC_PATH
#define GCC_PATH "gcc"
#endif

/* Test input file */
#define TEST_INPUT_FILE "test_input.c"
#define TEST_SPEC_FILE "test.specs"

/* Create a minimal test C source file */
void create_test_input(void) {
    FILE *f = fopen(TEST_INPUT_FILE, "w");
    if (!f) {
        perror("Failed to create test input file");
        exit(1);
    }
    
    fprintf(f, "/* Test input for GCC driver reset test */\n");
    fprintf(f, "#include <stdio.h>\n\n");
    fprintf(f, "int main(void) {\n");
    fprintf(f, "    printf(\"Hello from test program\\n\");\n");
    fprintf(f, "    return 0;\n");
    fprintf(f, "}\n");
    
    fclose(f);
}

/* Create a test spec file to alter machine specs */
void create_test_spec_file(void) {
    FILE *f = fopen(TEST_SPEC_FILE, "w");
    if (!f) {
        perror("Failed to create test spec file");
        exit(1);
    }
    
    fprintf(f, "*link:\n");
    fprintf(f, "-Wl,--verbose\n\n");
    
    fprintf(f, "*cpp:\n");
    fprintf(f, "-DSPEC_TEST=1\n");
    
    fclose(f);
}

/* Clean up test files */
void cleanup_test_files(void) {
    unlink(TEST_INPUT_FILE);
    unlink(TEST_SPEC_FILE);
    unlink("output1.o");
    unlink("output2.o");
    unlink("output3.o");
    unlink("output4.o");
    /* Clean up any temp files from -save-temps */
    system("rm -f *.i *.s *.ii test-* 2>/dev/null");
}

/* Method A: Process-based invocation using fork/exec */
int invoke_gcc_via_process(const char **argv) {
    pid_t pid = fork();
    
    if (pid == 0) {
        /* Child process */
        execvp(GCC_PATH, (char *const *)argv);
        perror("execvp failed");
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
        perror("fork failed");
        return -1;
    }
}

#ifdef USE_DIRECT_LIBRARY_CALL
/* Method B: Direct library call using dlopen/dlsym */
int invoke_gcc_via_library(const char **argv) {
    static void *gcc_lib = NULL;
    static int (*gcc_main)(int, const char **) = NULL;
    
    if (!gcc_lib) {
        gcc_lib = dlopen("libgccdriver.so", RTLD_LAZY);
        if (!gcc_lib) {
            /* Try alternative names */
            gcc_lib = dlopen("./libgccdriver.so", RTLD_LAZY);
        }
        if (!gcc_lib) {
            fprintf(stderr, "Failed to load GCC driver library: %s\n", dlerror());
            return -1;
        }
        
        gcc_main = (int (*)(int, const char **))dlsym(gcc_lib, "main");
        if (!gcc_main) {
            gcc_main = (int (*)(int, const char **))dlsym(gcc_lib, "driver::main");
        }
        if (!gcc_main) {
            fprintf(stderr, "Failed to find GCC driver entry point: %s\n", dlerror());
            dlclose(gcc_lib);
            return -1;
        }
    }
    
    /* Count arguments */
    int argc = 0;
    while (argv[argc] != NULL) argc++;
    
    /* Call the driver's main function */
    return gcc_main(argc, argv);
}
#endif

/* Run the test sequence */
int main(int argc, char **argv) {
    printf("=== GCC Driver Reinitialization Test ===\n");
    
    /* Create test files */
    create_test_input();
    create_test_spec_file();
    
    int overall_result = 0;
    
    /* Test 1: First invocation with various flags to set state */
    printf("\n--- Test 1: Setting driver state ---\n");
    const char *test1_args[] = {
        GCC_PATH,
        "-save-temps",              /* Sets save_temps_flag */
        "-dumpdir", "/tmp/gcc_test", /* Sets dumpdir */
        "-dumpbase", "test_dump",   /* Sets dumpbase */
        "-o", "output1.o",          /* Sets outbase */
        "--sysroot=/alt/sysroot",   /* Sets target_system_root */
        "-specs=" TEST_SPEC_FILE,   /* Alters spec machine */
        "-march=x86-64",            /* Further modifies machine spec */
        "-v",                       /* Sets verbose flag */
        "-c", TEST_INPUT_FILE,      /* Compile only */
        NULL
    };
    
    int result1;
#ifdef USE_DIRECT_LIBRARY_CALL
    result1 = invoke_gcc_via_library(test1_args);
#else
    result1 = invoke_gcc_via_process(test1_args);
#endif
    printf("Test 1 exit status: %d\n", result1);
    
    /* Test 2: Second invocation without special flags to trigger reset */
    printf("\n--- Test 2: Resetting to defaults ---\n");
    const char *test2_args[] = {
        GCC_PATH,
        "-c", TEST_INPUT_FILE,      /* Simple compile */
        "-o", "output2.o",          /* Different output */
        NULL
    };
    
    int result2;
#ifdef USE_DIRECT_LIBRARY_CALL
    result2 = invoke_gcc_via_library(test2_args);
#else
    result2 = invoke_gcc_via_process(test2_args);
#endif
    printf("Test 2 exit status: %d\n", result2);
    
    /* Test 3: Third invocation with invalid option to force failure */
    printf("\n--- Test 3: Forcing failure to set greatest_status ---\n");
    const char *test3_args[] = {
        GCC_PATH,
        "-invalid-option-that-does-not-exist",  /* Will cause failure */
        "-c", TEST_INPUT_FILE,
        "-o", "output3.o",
        NULL
    };
    
    int result3;
#ifdef USE_DIRECT_LIBRARY_CALL
    result3 = invoke_gcc_via_library(test3_args);
#else
    result3 = invoke_gcc_via_process(test3_args);
#endif
    printf("Test 3 exit status: %d (expected non-zero)\n", result3);
    
    /* Test 4: Fourth invocation should succeed, testing status reset */
    printf("\n--- Test 4: Successful compilation after failure ---\n");
    const char *test4_args[] = {
        GCC_PATH,
        "-c", TEST_INPUT_FILE,
        "-o", "output4.o",
        NULL
    };
    
    int result4;
#ifdef USE_DIRECT_LIBRARY_CALL
    result4 = invoke_gcc_via_library(test4_args);
#else
    result4 = invoke_gcc_via_process(test4_args);
#endif
    printf("Test 4 exit status: %d (expected 0)\n", result4);
    
    /* Test 5: Additional test with different dumpbase/dumpdir combinations */
    printf("\n--- Test 5: Testing dumpdir/dumpbase reset ---\n");
    const char *test5_args[] = {
        GCC_PATH,
        "-save-temps=obj",          /* Different save_temps value */
        "-dumpdir", "different_dir",
        "-dumpbase", "different_base",
        "-dumpbase_ext", ".extra",
        "-c", TEST_INPUT_FILE,
        "-o", "output5.o",
        NULL
    };
    
    int result5;
#ifdef USE_DIRECT_LIBRARY_CALL
    result5 = invoke_gcc_via_library(test5_args);
#else
    result5 = invoke_gcc_via_process(test5_args);
#endif
    printf("Test 5 exit status: %d\n", result5);
    
    /* Summary */
    printf("\n=== Test Summary ===\n");
    printf("Test 1 (set state): %d\n", result1);
    printf("Test 2 (reset): %d\n", result2);
    printf("Test 3 (force failure): %d\n", result3);
    printf("Test 4 (recover): %d\n", result4);
    printf("Test 5 (alternate flags): %d\n", result5);
    
    /* Verify expected behavior */
    if (result3 == 0) {
        printf("WARNING: Test 3 should have failed but succeeded\n");
        overall_result = 1;
    }
    
    if (result4 != 0) {
        printf("ERROR: Test 4 should have succeeded but failed\n");
        overall_result = 1;
    }
    
    /* Cleanup */
    cleanup_test_files();
    
    printf("\nTest %s\n", overall_result == 0 ? "PASSED" : "FAILED");
    return overall_result;
}
