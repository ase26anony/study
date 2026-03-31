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

/* Define this to use direct library loading instead of fork/exec */
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
        return 0;
    }
    fputs(test_source, f);
    fclose(f);
    return 1;
}

/* Method A: Process-based invocation using fork/exec */
int invoke_gcc_via_process(const char **argv) {
    pid_t pid = fork();
    if (pid == 0) {
        /* Child process */
        execvp("gcc", (char *const *)argv);
        perror("execvp failed");
        exit(127);
    } else if (pid > 0) {
        /* Parent process */
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)) {
            return WEXITSTATUS(status);
        }
        return -1;
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
        gcc_lib = dlopen("libgccdriver.so", RTLD_LAZY | RTLD_LOCAL);
        if (!gcc_lib) {
            /* Try alternative names */
            gcc_lib = dlopen("./libgccdriver.so", RTLD_LAZY | RTLD_LOCAL);
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
    while (argv[argc]) argc++;
    
    /* Call the driver's main function */
    return gcc_main(argc, argv);
}
#endif

/* Run a series of GCC invocations to trigger the uncovered reset logic */
int run_gcc_reset_test(const char *input_file) {
    int overall_status = 0;
    
    printf("=== Starting GCC Driver Reset Test ===\n\n");
    
    /* Stage 1: Invoke with various flags to set global state */
    printf("Stage 1: Setting driver state with special flags\n");
    const char *stage1_args[] = {
        "gcc",
        "-save-temps",          /* Sets save_temps_flag */
        "-dumpdir", "/tmp/gcc_test_dump",  /* Sets dumpdir */
        "-dumpbase", "test_dumpbase",      /* Sets dumpbase */
        "-dumpbase-ext", ".c",  /* Sets dumpbase_ext */
        "--sysroot=/alt/sysroot", /* Changes target_system_root */
        "-specs=test.specs",    /* Changes spec_machine (if spec file exists) */
        "-march=native",        /* May affect spec machine */
        "-c", input_file,
        "-o", "output1.o",
        NULL
    };
    
#ifdef USE_DIRECT_LIBRARY_CALL
    int status1 = invoke_gcc_via_library(stage1_args);
#else
    int status1 = invoke_gcc_via_process(stage1_args);
#endif
    printf("Stage 1 exit status: %d\n\n", status1);
    
    /* Stage 2: Invoke with minimal flags to trigger reset to defaults */
    printf("Stage 2: Triggering reset to default values\n");
    const char *stage2_args[] = {
        "gcc",
        "-c", input_file,
        "-o", "output2.o",
        NULL
    };
    
#ifdef USE_DIRECT_LIBRARY_CALL
    int status2 = invoke_gcc_via_library(stage2_args);
#else
    int status2 = invoke_gcc_via_process(stage2_args);
#endif
    printf("Stage 2 exit status: %d\n\n", status2);
    
    /* Stage 3: Force failure to set greatest_status */
    printf("Stage 3: Forcing failure to set greatest_status\n");
    const char *stage3_args[] = {
        "gcc",
        "-invalid-option-that-does-not-exist",
        "-c", input_file,
        NULL
    };
    
#ifdef USE_DIRECT_LIBRARY_CALL
    int status3 = invoke_gcc_via_library(stage3_args);
#else
    int status3 = invoke_gcc_via_process(stage3_args);
#endif
    printf("Stage 3 exit status: %d (expected non-zero)\n\n", status3);
    
    /* Stage 4: Successful compilation to test greatest_status reset */
    printf("Stage 4: Testing greatest_status reset with successful compilation\n");
    const char *stage4_args[] = {
        "gcc",
        "-c", input_file,
        "-o", "output4.o",
        NULL
    };
    
#ifdef USE_DIRECT_LIBRARY_CALL
    int status4 = invoke_gcc_via_library(stage4_args);
#else
    int status4 = invoke_gcc_via_process(stage4_args);
#endif
    printf("Stage 4 exit status: %d (expected 0)\n\n", status4);
    
    /* Stage 5: Test save_temps overrides with dumpdir */
    printf("Stage 5: Testing save_temps_flag and dumpdir interactions\n");
    const char *stage5_args[] = {
        "gcc",
        "-save-temps=obj",
        "-dumpdir", "/tmp/override_test",
        "-dumpbase", "override",
        "-c", input_file,
        "-o", "output5.o",
        NULL
    };
    
#ifdef USE_DIRECT_LIBRARY_CALL
    int status5 = invoke_gcc_via_library(stage5_args);
#else
    int status5 = invoke_gcc_via_process(stage5_args);
#endif
    printf("Stage 5 exit status: %d\n\n", status5);
    
    /* Summary */
    printf("=== Test Summary ===\n");
    printf("Stage 1 (set state): %d\n", status1);
    printf("Stage 2 (reset): %d\n", status2);
    printf("Stage 3 (force fail): %d\n", status3);
    printf("Stage 4 (success after fail): %d\n", status4);
    printf("Stage 5 (save-temps test): %d\n", status5);
    
    /* Verify the pattern we expect for greatest_status reset */
    if (status3 != 0 && status4 == 0) {
        printf("\n✓ greatest_status reset test PASSED (failure then success)\n");
    } else {
        printf("\n✗ greatest_status reset test FAILED\n");
        overall_status = 1;
    }
    
    return overall_status;
}

/* Alternative: Direct compilation with driver code included */
#ifdef COMPILE_WITH_DRIVER
/* This would require linking with gcc driver object files */
extern int driver_main(int argc, const char **argv);

int test_direct_driver_calls() {
    printf("Testing via direct driver_main calls\n");
    
    /* Create argument arrays */
    const char *args1[] = {"gcc", "-save-temps", "-dumpdir", "/tmp/direct", 
                          "-c", "test.c", "-o", "test.o", NULL};
    const char *args2[] = {"gcc", "-c", "test.c", "-o", "test2.o", NULL};
    const char *args3[] = {"gcc", "-invalid-flag", NULL};
    const char *args4[] = {"gcc", "-c", "test.c", "-o", "test3.o", NULL};
    
    /* Call driver_main multiple times */
    int status1 = driver_main(8, args1);
    int status2 = driver_main(5, args2);
    int status3 = driver_main(2, args3);
    int status4 = driver_main(5, args4);
    
    printf("Direct calls: %d, %d, %d, %d\n", status1, status2, status3, status4);
    
    return (status3 != 0 && status4 == 0) ? 0 : 1;
}
#endif

int main(int argc, char **argv) {
    const char *input_file = "test_input.c";
    
    /* Create test input file */
    if (!create_test_file(input_file)) {
        return 1;
    }
    
    /* Create a dummy spec file for testing */
    FILE *spec = fopen("test.specs", "w");
    if (spec) {
        fprintf(spec, "*link:\n+%%{!static:-dynamic-linker /lib/ld-linux.so.2}\n");
        fclose(spec);
    }
    
    int result;
    
#ifdef COMPILE_WITH_DRIVER
    /* Use direct driver calls if compiled with driver code */
    result = test_direct_driver_calls();
#else
    /* Use process-based or library-based invocation */
    result = run_gcc_reset_test(input_file);
#endif
    
    /* Cleanup */
    unlink(input_file);
    unlink("test.specs");
    unlink("output1.o");
    unlink("output2.o");
    unlink("output4.o");
    unlink("output5.o");
    
    /* Clean up any temp files created by -save-temps */
    system("rm -f test_input.i test_input.s test_input.o 2>/dev/null");
    system("rm -f /tmp/gcc_test_dump/* /tmp/override_test/* 2>/dev/null");
    
    return result;
}
