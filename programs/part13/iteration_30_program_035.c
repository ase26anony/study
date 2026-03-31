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

/* Define if we want to use direct library loading instead of fork/exec */
/* #define USE_DIRECT_LIBRARY_CALL */

/* Simple test C source file content */
const char *test_source = 
"#include <stdio.h>\n"
"int main() {\n"
"    printf(\"Hello from test program\\n\");\n"
"    return 0;\n"
"}\n";

/* Create a temporary test source file */
int create_test_file(const char *filename) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        perror("Failed to create test file");
        return -1;
    }
    fputs(test_source, f);
    fclose(f);
    return 0;
}

/* Method A: Process-based invocation using fork/exec */
int invoke_gcc_via_process(int invocation_num, char **argv) {
    pid_t pid = fork();
    
    if (pid == 0) {
        /* Child process */
        execvp("gcc", argv);
        /* If execvp fails */
        perror("execvp failed");
        exit(127);
    } else if (pid > 0) {
        /* Parent process */
        int status;
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status)) {
            printf("Invocation %d exited with status: %d\n", 
                   invocation_num, WEXITSTATUS(status));
            return WEXITSTATUS(status);
        } else if (WIFSIGNALED(status)) {
            printf("Invocation %d terminated by signal: %d\n", 
                   invocation_num, WTERMSIG(status));
            return -1;
        }
    } else {
        perror("fork failed");
        return -1;
    }
    return -1;
}

#ifdef USE_DIRECT_LIBRARY_CALL
/* Method B: Direct library loading (if GCC driver is built as shared library) */
int invoke_gcc_via_library(int invocation_num, char **argv) {
    static void *driver_lib = NULL;
    static int (*driver_main)(int, char**) = NULL;
    
    if (!driver_lib) {
        /* Try to load GCC driver as shared library */
        driver_lib = dlopen("./libgccdriver.so", RTLD_LAZY | RTLD_LOCAL);
        if (!driver_lib) {
            driver_lib = dlopen("libgccdriver.so", RTLD_LAZY | RTLD_LOCAL);
        }
        if (!driver_lib) {
            fprintf(stderr, "Failed to load GCC driver library: %s\n", dlerror());
            return -1;
        }
        
        /* Find the main function */
        driver_main = (int (*)(int, char**))dlsym(driver_lib, "main");
        if (!driver_main) {
            driver_main = (int (*)(int, char**))dlsym(driver_lib, "driver::main");
        }
        if (!driver_main) {
            fprintf(stderr, "Failed to find driver main function: %s\n", dlerror());
            dlclose(driver_lib);
            return -1;
        }
    }
    
    /* Count arguments */
    int argc = 0;
    while (argv[argc] != NULL) argc++;
    
    printf("Direct library invocation %d with %d args\n", invocation_num, argc);
    
    /* Call the driver main function */
    int result = driver_main(argc, argv);
    printf("Invocation %d returned: %d\n", invocation_num, result);
    
    return result;
}
#endif

/* Main test program */
int main(int argc, char **argv) {
    printf("=== GCC Driver Reinitialization Test ===\n");
    
    /* Create test source file */
    if (create_test_file("test_input.c") != 0) {
        return 1;
    }
    
    /* Create a temporary directory for sysroot testing */
    mkdir("/tmp/test_sysroot", 0755);
    mkdir("/tmp/test_sysroot/usr", 0755);
    mkdir("/tmp/test_sysroot/usr/include", 0755);
    
    /* Create a dummy spec file for testing */
    FILE *spec = fopen("/tmp/test_spec.spec", "w");
    if (spec) {
        fprintf(spec, "*cpp:\n");
        fprintf(spec, "-DSPEC_TEST\n");
        fclose(spec);
    }
    
    int overall_result = 0;
    
    /* INVOCATION 1: Complex setup with many flags to set state */
    printf("\n--- Invocation 1: Setting complex state ---\n");
    char *invocation1_args[] = {
        "gcc",
        "-save-temps",              /* Sets save_temps_flag */
        "-dumpdir", "/tmp/test_dump", /* Sets dumpdir */
        "-dumpbase", "test_dumpbase", /* Sets dumpbase */
        "-dumpbase-ext", ".c",      /* Sets dumpbase_ext */
        "--sysroot=/tmp/test_sysroot", /* Sets target_system_root */
        "-specs=/tmp/test_spec.spec", /* Alters spec machine */
        "-march=x86-64",            /* Changes machine spec */
        "-mtune=generic",
        "-c", "test_input.c",       /* Compile only */
        "-o", "output1.o",          /* Sets outbase */
        "-v",                       /* Verbose for debugging */
        NULL
    };
    
#ifdef USE_DIRECT_LIBRARY_CALL
    int result1 = invoke_gcc_via_library(1, invocation1_args);
#else
    int result1 = invoke_gcc_via_process(1, invocation1_args);
#endif
    
    /* Force a pause/sync to ensure driver state is cleared between invocations */
    sync();
    
    /* INVOCATION 2: Minimal invocation to trigger reset to defaults */
    printf("\n--- Invocation 2: Minimal (trigger reset) ---\n");
    char *invocation2_args[] = {
        "gcc",
        "-c", "test_input.c",       /* Simple compile */
        "-o", "output2.o",
        NULL
    };
    
#ifdef USE_DIRECT_LIBRARY_CALL
    int result2 = invoke_gcc_via_library(2, invocation2_args);
#else
    int result2 = invoke_gcc_via_process(2, invocation2_args);
#endif
    
    /* INVOCATION 3: Force failure to set greatest_status */
    printf("\n--- Invocation 3: Force failure ---\n");
    char *invocation3_args[] = {
        "gcc",
        "-invalid-option-that-does-not-exist", /* Will cause failure */
        "-c", "test_input.c",
        "-o", "output3.o",
        NULL
    };
    
#ifdef USE_DIRECT_LIBRARY_CALL
    int result3 = invoke_gcc_via_library(3, invocation3_args);
#else
    int result3 = invoke_gcc_via_process(3, invocation3_args);
#endif
    
    /* INVOCATION 4: Successful compilation after failure */
    printf("\n--- Invocation 4: Success after failure ---\n");
    char *invocation4_args[] = {
        "gcc",
        "-c", "test_input.c",
        "-o", "output4.o",
        NULL
    };
    
#ifdef USE_DIRECT_LIBRARY_CALL
    int result4 = invoke_gcc_via_library(4, invocation4_args);
#else
    int result4 = invoke_gcc_via_process(4, invocation4_args);
#endif
    
    /* INVOCATION 5: Test save-temps with different dumpdir */
    printf("\n--- Invocation 5: Different save-temps config ---\n");
    char *invocation5_args[] = {
        "gcc",
        "-save-temps=obj",          /* Different save_temps_flag value */
        "-dumpdir", "/tmp/another_dump",
        "-dumpbase", "another_base",
        "-c", "test_input.c",
        "-o", "output5.o",
        NULL
    };
    
#ifdef USE_DIRECT_LIBRARY_CALL
    int result5 = invoke_gcc_via_library(5, invocation5_args);
#else
    int result5 = invoke_gcc_via_process(5, invocation5_args);
#endif
    
    /* INVOCATION 6: Back to minimal to trigger another reset */
    printf("\n--- Invocation 6: Another minimal (final reset) ---\n");
    char *invocation6_args[] = {
        "gcc",
        "-c", "test_input.c",
        "-o", "output6.o",
        NULL
    };
    
#ifdef USE_DIRECT_LIBRARY_CALL
    int result6 = invoke_gcc_via_library(6, invocation6_args);
#else
    int result6 = invoke_gcc_via_process(6, invocation6_args);
#endif
    
    /* Summary */
    printf("\n=== Test Summary ===\n");
    printf("Invocation 1 (complex): %d\n", result1);
    printf("Invocation 2 (minimal): %d\n", result2);
    printf("Invocation 3 (failure): %d\n", result3);
    printf("Invocation 4 (success): %d\n", result4);
    printf("Invocation 5 (different): %d\n", result5);
    printf("Invocation 6 (minimal): %d\n", result6);
    
    /* Cleanup */
    unlink("test_input.c");
    unlink("/tmp/test_spec.spec");
    /* Note: Not removing /tmp/test_sysroot as it might be in use */
    
    /* Check if greatest_status reset worked */
    if (result3 != 0 && result4 == 0) {
        printf("\n✓ greatest_status reset test: FAILURE then SUCCESS detected\n");
    } else {
        printf("\n✗ greatest_status reset may not have been tested properly\n");
        overall_result = 1;
    }
    
    return overall_result;
}
