/* test_gcc_driver_reset.c
 * A program to test GCC driver reinitialization logic by invoking it multiple times
 * with different flags to trigger state resets between invocations.
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <errno.h>

/* Define this to use direct library calls instead of fork/exec */
/* #define USE_DIRECT_LIBRARY_CALL */

/* Create a minimal C source file for compilation */
void create_test_source(const char *filename) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        perror("Failed to create test source");
        exit(1);
    }
    
    fprintf(f, "/* Test source for GCC driver reset testing */\n");
    fprintf(f, "#include <stdio.h>\n\n");
    fprintf(f, "int main(void) {\n");
    fprintf(f, "    printf(\"Hello from test program\\n\");\n");
    fprintf(f, "    return 0;\n");
    fprintf(f, "}\n");
    
    fclose(f);
}

/* Create a dummy spec file to test spec machine reset */
void create_test_spec(const char *filename) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        perror("Failed to create test spec");
        exit(1);
    }
    
    fprintf(f, "*cpp:\n");
    fprintf(f, "+ -DSPEC_TEST=1\n\n");
    fprintf(f, "*cc1:\n");
    fprintf(f, "+ -fverbose-asm\n\n");
    fprintf(f, "*link:\n");
    fprintf(f, "+ -Wl,--verbose\n");
    
    fclose(f);
}

/* Method A: Process-based invocation using fork/exec */
int invoke_gcc_via_process(const char **argv, const char *description) {
    printf("\n=== Invocation: %s ===\n", description);
    printf("Command:");
    for (int i = 0; argv[i] != NULL; i++) {
        printf(" %s", argv[i]);
    }
    printf("\n");
    
    pid_t pid = fork();
    if (pid == 0) {
        /* Child process */
        execvp(argv[0], (char * const *)argv);
        perror("execvp failed");
        exit(127);
    } else if (pid > 0) {
        /* Parent process */
        int status;
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status)) {
            printf("Exit status: %d\n", WEXITSTATUS(status));
            return WEXITSTATUS(status);
        } else {
            printf("Process terminated abnormally\n");
            return -1;
        }
    } else {
        perror("fork failed");
        return -1;
    }
}

#ifdef USE_DIRECT_LIBRARY_CALL
/* Method B: Direct library call using dlopen/dlsym */
int invoke_gcc_via_library(const char **argv, const char *description) {
    printf("\n=== Direct Library Invocation: %s ===\n", description);
    
    static void *gcc_lib = NULL;
    static int (*gcc_main)(int, const char **) = NULL;
    
    if (!gcc_lib) {
        /* Try to load GCC driver as a shared library */
        gcc_lib = dlopen("./libgccdriver.so", RTLD_LAZY | RTLD_LOCAL);
        if (!gcc_lib) {
            gcc_lib = dlopen("libgccdriver.so", RTLD_LAZY | RTLD_LOCAL);
        }
        if (!gcc_lib) {
            fprintf(stderr, "Failed to load GCC driver library: %s\n", dlerror());
            fprintf(stderr, "Falling back to process-based invocation\n");
            return invoke_gcc_via_process(argv, description);
        }
        
        gcc_main = (int (*)(int, const char **))dlsym(gcc_lib, "main");
        if (!gcc_main) {
            gcc_main = (int (*)(int, const char **))dlsym(gcc_lib, "driver::main");
        }
        if (!gcc_main) {
            fprintf(stderr, "Failed to find GCC main function: %s\n", dlerror());
            dlclose(gcc_lib);
            return invoke_gcc_via_process(argv, description);
        }
    }
    
    /* Count arguments */
    int argc = 0;
    while (argv[argc] != NULL) argc++;
    
    printf("Calling GCC main with %d arguments\n", argc);
    int result = gcc_main(argc, argv);
    printf("Return value: %d\n", result);
    
    return result;
}
#endif

/* Main test sequence */
int main(int argc, char *argv[]) {
    printf("=== GCC Driver Reinitialization Test ===\n");
    
    /* Create necessary files */
    create_test_source("test_reset.c");
    create_test_spec("test_spec.spec");
    
    /* Create alternative sysroot directory structure */
    mkdir("alt_sysroot", 0755);
    mkdir("alt_sysroot/usr", 0755);
    mkdir("alt_sysroot/usr/include", 0755);
    
    /* Prepare argument arrays for different invocations */
    const char *gcc_path = "gcc";
    
    /* Test 1: First invocation with various flags to set state */
    const char *test1_args[] = {
        gcc_path,
        "-save-temps",           /* Sets save_temps_flag */
        "-dumpdir", "test_dump", /* Sets dumpdir */
        "-dumpbase", "test_base", /* Sets dumpbase */
        "--sysroot=alt_sysroot", /* Sets target_system_root */
        "-specs=test_spec.spec", /* May affect spec_machine */
        "-march=native",         /* May affect spec_machine */
        "-c", "test_reset.c",
        "-o", "output1.o",
        "-v",                    /* Verbose to see what's happening */
        NULL
    };
    
    /* Test 2: Second invocation with minimal flags to trigger reset */
    const char *test2_args[] = {
        gcc_path,
        "-c", "test_reset.c",
        "-o", "output2.o",
        NULL
    };
    
    /* Test 3: Third invocation that fails to set greatest_status */
    const char *test3_args[] = {
        gcc_path,
        "-invalid-option-that-does-not-exist",
        "-c", "test_reset.c",
        "-o", "output3.o",
        NULL
    };
    
    /* Test 4: Fourth invocation that succeeds to test status reset */
    const char *test4_args[] = {
        gcc_path,
        "-c", "test_reset.c",
        "-o", "output4.o",
        NULL
    };
    
    /* Test 5: Fifth invocation with different dump settings */
    const char *test5_args[] = {
        gcc_path,
        "-save-temps=obj",
        "-dumpdir", "test_dump2",
        "-dumpbase", "test_base2",
        "-dumpbase-ext", ".ext",
        "-c", "test_reset.c",
        "-o", "output5.o",
        NULL
    };
    
    /* Test 6: Sixth invocation to trigger another reset */
    const char *test6_args[] = {
        gcc_path,
        "-c", "test_reset.c",
        "-o", "output6.o",
        NULL
    };
    
    int status1, status2, status3, status4, status5, status6;
    
#ifdef USE_DIRECT_LIBRARY_CALL
    /* Use direct library calls if enabled */
    status1 = invoke_gcc_via_library(test1_args, "First - with save-temps, dumpdir, sysroot");
    status2 = invoke_gcc_via_library(test2_args, "Second - minimal (trigger reset)");
    status3 = invoke_gcc_via_library(test3_args, "Third - should fail");
    status4 = invoke_gcc_via_library(test4_args, "Fourth - should succeed");
    status5 = invoke_gcc_via_library(test5_args, "Fifth - different dump settings");
    status6 = invoke_gcc_via_library(test6_args, "Sixth - minimal again");
#else
    /* Use process-based invocation (default) */
    status1 = invoke_gcc_via_process(test1_args, "First - with save-temps, dumpdir, sysroot");
    status2 = invoke_gcc_via_process(test2_args, "Second - minimal (trigger reset)");
    status3 = invoke_gcc_via_process(test3_args, "Third - should fail");
    status4 = invoke_gcc_via_process(test4_args, "Fourth - should succeed");
    status5 = invoke_gcc_via_process(test5_args, "Fifth - different dump settings");
    status6 = invoke_gcc_via_process(test6_args, "Sixth - minimal again");
#endif
    
    printf("\n=== Test Summary ===\n");
    printf("Test 1 (with flags): %d\n", status1);
    printf("Test 2 (minimal): %d\n", status2);
    printf("Test 3 (should fail): %d\n", status3);
    printf("Test 4 (should succeed): %d\n", status4);
    printf("Test 5 (different dump): %d\n", status5);
    printf("Test 6 (minimal again): %d\n", status6);
    
    /* Cleanup */
    unlink("test_reset.c");
    unlink("test_spec.spec");
    unlink("output1.o");
    unlink("output2.o");
    unlink("output4.o");
    unlink("output5.o");
    unlink("output6.o");
    
    /* Clean up dump directories */
    system("rm -rf test_dump test_dump2");
    system("rm -rf alt_sysroot");
    
    printf("\n=== Test Complete ===\n");
    
    /* Return success if all process invocations worked (even if some GCC calls failed) */
    return 0;
}
