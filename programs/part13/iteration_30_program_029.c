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
#include <fcntl.h>
#include <dlfcn.h>
#include <errno.h>

/* Define if we want to use direct library loading instead of fork/exec */
/* #define USE_DIRECT_LIBRARY_CALL */

/* Create a minimal C source file for compilation */
void create_test_source(const char *filename) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        perror("fopen");
        exit(1);
    }
    
    fprintf(f, "/* Test source file for GCC driver reset testing */\n");
    fprintf(f, "#include <stdio.h>\n\n");
    fprintf(f, "int main() {\n");
    fprintf(f, "    printf(\"Hello from test program!\\n\");\n");
    fprintf(f, "    return 0;\n");
    fprintf(f, "}\n");
    
    fclose(f);
    printf("Created test source file: %s\n", filename);
}

/* Create a dummy spec file to test spec machine reset */
void create_dummy_spec(const char *filename) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        perror("fopen");
        exit(1);
    }
    
    fprintf(f, "*link:\n");
    fprintf(f, "-ldummy_test_lib\n\n");
    fprintf(f, "*cpp:\n");
    fprintf(f, "-DDUMMY_TEST=1\n");
    
    fclose(f);
    printf("Created dummy spec file: %s\n", filename);
}

/* Method A: Process-based invocation using fork/exec */
int invoke_gcc_via_process(const char **argv) {
    pid_t pid = fork();
    
    if (pid == 0) {
        /* Child process */
        execvp("gcc", (char * const *)argv);
        /* If execvp fails */
        perror("execvp");
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
        perror("fork");
        return -1;
    }
}

#ifdef USE_DIRECT_LIBRARY_CALL
/* Method B: Direct library call using dlopen/dlsym */
int invoke_gcc_via_library(const char **argv) {
    static void *gcc_lib = NULL;
    static int (*gcc_main)(int, const char **) = NULL;
    
    if (!gcc_lib) {
        /* Try to load GCC driver as a shared library */
        gcc_lib = dlopen("libgccdriver.so", RTLD_LAZY);
        if (!gcc_lib) {
            /* Try alternative names */
            gcc_lib = dlopen("./libgccdriver.so", RTLD_LAZY);
        }
        if (!gcc_lib) {
            fprintf(stderr, "Failed to load GCC driver library: %s\n", dlerror());
            return -1;
        }
        
        /* Find the main function */
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

/* Run the test sequence to trigger all uncovered lines */
void run_comprehensive_test() {
    int status;
    const char *input_file = "test_input.c";
    const char *spec_file = "dummy.specs";
    
    /* Create necessary files */
    create_test_source(input_file);
    create_dummy_spec(spec_file);
    
    printf("\n=== Starting GCC Driver Reinitialization Test ===\n\n");
    
    /* Test 1: First invocation with various flags to set state */
    printf("Test 1: Setting driver state with various flags\n");
    printf("------------------------------------------------\n");
    const char *test1_args[] = {
        "gcc",
        "-save-temps",          /* Sets save_temps_flag */
        "-dumpdir", "/tmp/gcc_test_dump",  /* Sets dumpdir */
        "-dumpbase", "test_dumpbase",      /* Sets dumpbase */
        "-dumpbase-ext", ".c",  /* Sets dumpbase_ext */
        "--sysroot=/alt/sysroot", /* Changes target_system_root */
        "-specs", spec_file,    /* Changes spec_machine */
        "-march=native",        /* Additional machine spec change */
        "-c", input_file,       /* Compile only */
        "-o", "output1.o",      /* Output file */
        "-v",                   /* Verbose to trigger verbose_only_flag */
        NULL
    };
    
#ifdef USE_DIRECT_LIBRARY_CALL
    status = invoke_gcc_via_library(test1_args);
#else
    status = invoke_gcc_via_process(test1_args);
#endif
    printf("Exit status: %d\n\n", status);
    
    /* Test 2: Second invocation with minimal flags to trigger reset */
    printf("Test 2: Minimal invocation to trigger state reset\n");
    printf("------------------------------------------------\n");
    const char *test2_args[] = {
        "gcc",
        "-c", input_file,       /* Simple compile */
        "-o", "output2.o",      /* Different output */
        NULL
    };
    
#ifdef USE_DIRECT_LIBRARY_CALL
    status = invoke_gcc_via_library(test2_args);
#else
    status = invoke_gcc_via_process(test2_args);
#endif
    printf("Exit status: %d\n\n", status);
    
    /* Test 3: Third invocation with error to set greatest_status */
    printf("Test 3: Invalid invocation to set greatest_status\n");
    printf("------------------------------------------------\n");
    const char *test3_args[] = {
        "gcc",
        "-invalid-option",      /* Will cause failure */
        "-c", input_file,
        NULL
    };
    
#ifdef USE_DIRECT_LIBRARY_CALL
    status = invoke_gcc_via_library(test3_args);
#else
    status = invoke_gcc_via_process(test3_args);
#endif
    printf("Exit status: %d (should be non-zero)\n\n", status);
    
    /* Test 4: Fourth invocation to test greatest_status reset */
    printf("Test 4: Valid invocation after error\n");
    printf("------------------------------------------------\n");
    const char *test4_args[] = {
        "gcc",
        "-c", input_file,
        "-o", "output3.o",
        NULL
    };
    
#ifdef USE_DIRECT_LIBRARY_CALL
    status = invoke_gcc_via_library(test4_args);
#else
    status = invoke_gcc_via_process(test4_args);
#endif
    printf("Exit status: %d (should be zero)\n\n", status);
    
    /* Test 5: Additional test with different dumpdir/dumpbase combinations */
    printf("Test 5: Testing dumpdir/dumpbase reset with trailing dash\n");
    printf("------------------------------------------------\n");
    const char *test5_args[] = {
        "gcc",
        "-save-temps=obj",
        "-dumpdir", "/tmp/test-",  /* Trailing dash to test dumpdir_trailing_dash_added */
        "-dumpbase", "complex.base",
        "-dumpbase-ext", ".ext",
        "-c", input_file,
        "-o", "output4.o",
        NULL
    };
    
#ifdef USE_DIRECT_LIBRARY_CALL
    status = invoke_gcc_via_library(test5_args);
#else
    status = invoke_gcc_via_process(test5_args);
#endif
    printf("Exit status: %d\n\n", status);
    
    /* Test 6: Final clean invocation to ensure complete reset */
    printf("Test 6: Final clean invocation\n");
    printf("------------------------------------------------\n");
    const char *test6_args[] = {
        "gcc",
        "-c", input_file,
        "-o", "output5.o",
        NULL
    };
    
#ifdef USE_DIRECT_LIBRARY_CALL
    status = invoke_gcc_via_library(test6_args);
#else
    status = invoke_gcc_via_process(test6_args);
#endif
    printf("Exit status: %d\n\n", status);
    
    /* Cleanup */
    unlink(input_file);
    unlink(spec_file);
    unlink("output1.o");
    unlink("output2.o");
    unlink("output3.o");
    unlink("output4.o");
    unlink("output5.o");
    
    printf("=== Test Complete ===\n");
}

/* Alternative: Direct compilation with driver code included */
#ifdef COMPILE_WITH_DRIVER
/* This would require linking with gcc driver object files */
extern int driver_main(int argc, const char **argv);

void test_direct_driver_call() {
    printf("\n=== Testing Direct Driver Calls ===\n\n");
    
    /* Create argument arrays */
    const char *args1[] = {"gcc", "-save-temps", "-dumpdir", "/tmp", 
                          "-c", "test.c", "-o", "test.o", NULL};
    const char *args2[] = {"gcc", "-c", "test.c", "-o", "test2.o", NULL};
    const char *args3[] = {"gcc", "-invalid-opt", NULL};
    const char *args4[] = {"gcc", "-c", "test.c", NULL};
    
    /* Call driver multiple times */
    printf("Call 1 (with flags): %d\n", driver_main(7, args1));
    printf("Call 2 (minimal): %d\n", driver_main(5, args2));
    printf("Call 3 (error): %d\n", driver_main(2, args3));
    printf("Call 4 (success): %d\n", driver_main(3, args4));
}
#endif

int main(int argc, char **argv) {
    printf("GCC Driver Reinitialization Test Program\n");
    printf("========================================\n\n");
    
    /* Check if we should use direct library calls */
    int use_lib = 0;
    if (argc > 1 && strcmp(argv[1], "--use-lib") == 0) {
        use_lib = 1;
    }
    
    if (use_lib) {
#ifdef USE_DIRECT_LIBRARY_CALL
        printf("Using direct library call method\n");
        run_comprehensive_test();
#else
        printf("Direct library call not enabled in this build\n");
        printf("Recompile with -DUSE_DIRECT_LIBRARY_CALL\n");
#endif
    } else {
        printf("Using fork/exec method\n");
        run_comprehensive_test();
    }
    
#ifdef COMPILE_WITH_DRIVER
    if (argc > 1 && strcmp(argv[1], "--direct") == 0) {
        test_direct_driver_call();
    }
#endif
    
    return 0;
}
