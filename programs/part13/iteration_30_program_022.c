/* gcc_driver_reinit_test.c - Test program to trigger GCC driver reinitialization logic */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <dlfcn.h>
#include <errno.h>

/* Simple test source file content */
const char *test_source = 
"#include <stdio.h>\n"
"int main() {\n"
"    printf(\"Hello from test program\\n\");\n"
"    return 0;\n"
"}\n";

/* Create test source file */
int create_test_file(const char *filename) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        perror("Failed to create test file");
        return 0;
    }
    fputs(test_source, f);
    fclose(f);
    return 1;
}

/* Method A: Process-based invocation using fork/exec */
void test_process_based(const char *gcc_path) {
    printf("=== Testing via Process-Based Invocation ===\n");
    
    const char *input_file = "test_input.c";
    if (!create_test_file(input_file)) {
        return;
    }
    
    /* Array of test invocations with different flags */
    char *test_invocations[][20] = {
        /* First: Set various flags including sysroot and dump options */
        {(char *)gcc_path, "-save-temps", "-dumpdir", "/tmp/gcc_test1", 
         "-dumpbase", "foo", "-c", input_file, "-o", "output1.o", 
         "--sysroot=/alt/sysroot", "-v", NULL},
        
        /* Second: Minimal invocation to trigger reset to defaults */
        {(char *)gcc_path, "-c", input_file, "-o", "output2.o", NULL},
        
        /* Third: Force failure with invalid option */
        {(char *)gcc_path, "-invalid-option-that-does-not-exist", input_file, NULL},
        
        /* Fourth: Successful compilation again */
        {(char *)gcc_path, "-c", input_file, "-o", "output3.o", NULL},
        
        /* Fifth: Test with different machine specs */
        {(char *)gcc_path, "-march=native", "-mtune=generic", "-c", 
         input_file, "-o", "output4.o", NULL},
        
        /* Sixth: Back to defaults */
        {(char *)gcc_path, "-c", input_file, "-o", "output5.o", NULL}
    };
    
    int num_tests = sizeof(test_invocations) / sizeof(test_invocations[0]);
    
    for (int i = 0; i < num_tests; i++) {
        printf("\n--- Invocation %d ---\n", i + 1);
        
        /* Print command */
        printf("Command: ");
        for (int j = 0; test_invocations[i][j] != NULL; j++) {
            printf("%s ", test_invocations[i][j]);
        }
        printf("\n");
        
        pid_t pid = fork();
        if (pid == 0) {
            /* Child process */
            execv(gcc_path, test_invocations[i]);
            /* If execv fails */
            perror("execv failed");
            exit(127);
        } else if (pid > 0) {
            /* Parent process */
            int status;
            waitpid(pid, &status, 0);
            
            if (WIFEXITED(status)) {
                printf("Exit status: %d\n", WEXITSTATUS(status));
            } else {
                printf("Process terminated abnormally\n");
            }
        } else {
            perror("fork failed");
        }
    }
    
    /* Cleanup */
    unlink(input_file);
    for (int i = 1; i <= 5; i++) {
        char fname[32];
        snprintf(fname, sizeof(fname), "output%d.o", i);
        unlink(fname);
    }
}

/* Method B: Direct library call using dlopen */
#ifdef DRIVER_LIBRARY_TEST
void test_library_based(const char *driver_lib_path) {
    printf("\n=== Testing via Direct Library Call ===\n");
    
    void *handle = dlopen(driver_lib_path, RTLD_LAZY | RTLD_LOCAL);
    if (!handle) {
        fprintf(stderr, "Failed to load driver library: %s\n", dlerror());
        return;
    }
    
    /* Look for driver entry point - this depends on GCC's internal structure */
    typedef int (*driver_main_t)(int, char**, char**);
    driver_main_t driver_main = (driver_main_t)dlsym(handle, "main");
    
    if (!driver_main) {
        /* Try alternative names */
        driver_main = (driver_main_t)dlsym(handle, "gcc_main");
        driver_main = (driver_main_t)dlsym(handle, "driver::main");
    }
    
    if (!driver_main) {
        fprintf(stderr, "Could not find driver entry point: %s\n", dlerror());
        dlclose(handle);
        return;
    }
    
    /* Create test invocations */
    char *argv1[] = {
        "gcc", "-save-temps", "-dumpdir", "/tmp/lib_test",
        "-dumpbase", "libfoo", "-c", "test_input.c",
        "-o", "liboutput1.o", "--sysroot=/alt/sysroot", NULL
    };
    
    char *argv2[] = {
        "gcc", "-c", "test_input.c", "-o", "liboutput2.o", NULL
    };
    
    char *argv3[] = {
        "gcc", "-invalid-option-for-failure", "test_input.c", NULL
    };
    
    char *argv4[] = {
        "gcc", "-c", "test_input.c", "-o", "liboutput3.o", NULL
    };
    
    /* Execute multiple times to trigger reinitialization */
    printf("\nFirst invocation (with special flags):\n");
    int result1 = driver_main(11, argv1, NULL);
    printf("Result: %d\n", result1);
    
    printf("\nSecond invocation (minimal, triggers reset):\n");
    int result2 = driver_main(5, argv2, NULL);
    printf("Result: %d\n", result2);
    
    printf("\nThird invocation (forced failure):\n");
    int result3 = driver_main(3, argv3, NULL);
    printf("Result: %d\n", result3);
    
    printf("\nFourth invocation (success after failure):\n");
    int result4 = driver_main(5, argv4, NULL);
    printf("Result: %d\n", result4);
    
    dlclose(handle);
}
#endif

/* Method C: Direct compilation with driver source */
#ifdef COMPILE_WITH_DRIVER
/* This would require linking with gcc.o and other driver objects */
extern int driver_main(int argc, char **argv, char **envp);

void test_direct_link() {
    printf("\n=== Testing via Direct Function Call ===\n");
    
    /* Create argument arrays */
    char *arg0 = "test_program";
    char *arg1 = "-save-temps";
    char *arg2 = "-dumpdir";
    char *arg3 = "/tmp/direct_test";
    char *arg4 = "-dumpbase";
    char *arg5 = "direct";
    char *arg6 = "-c";
    char *arg7 = "test_input.c";
    char *arg8 = "-o";
    char *arg9 = "direct1.o";
    char *arg10 = "--sysroot=/alt/sysroot";
    
    char *argv1[] = {arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, NULL};
    char *argv2[] = {arg0, arg6, arg7, arg8, "direct2.o", NULL};
    char *argv3[] = {arg0, "-invalid-arg-for-failure", arg7, NULL};
    char *argv4[] = {arg0, arg6, arg7, arg8, "direct3.o", NULL};
    
    printf("First call (with flags):\n");
    int r1 = driver_main(11, argv1, NULL);
    printf("Result: %d\n", r1);
    
    printf("\nSecond call (reset to defaults):\n");
    int r2 = driver_main(5, argv2, NULL);
    printf("Result: %d\n", r2);
    
    printf("\nThird call (force failure):\n");
    int r3 = driver_main(3, argv3, NULL);
    printf("Result: %d\n", r3);
    
    printf("\nFourth call (success after failure):\n");
    int r4 = driver_main(5, argv4, NULL);
    printf("Result: %d\n", r4);
}
#endif

int main(int argc, char **argv) {
    printf("GCC Driver Reinitialization Test Program\n");
    printf("========================================\n");
    
    const char *gcc_path = "gcc";
    const char *input_file = "test_input.c";
    
    /* Check if we should use a different gcc path */
    if (argc > 1) {
        gcc_path = argv[1];
    }
    
    /* Create test source file */
    if (!create_test_file(input_file)) {
        return 1;
    }
    
    /* Test using process-based method (most reliable) */
    test_process_based(gcc_path);
    
    /* Try library-based method if requested */
    #ifdef DRIVER_LIBRARY_TEST
    if (argc > 2) {
        test_library_based(argv[2]);
    }
    #endif
    
    /* Try direct link method if compiled with driver */
    #ifdef COMPILE_WITH_DRIVER
    test_direct_link();
    #endif
    
    /* Cleanup */
    unlink(input_file);
    
    printf("\n=== Test Complete ===\n");
    return 0;
}
