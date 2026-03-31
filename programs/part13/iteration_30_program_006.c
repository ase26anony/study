/* test_driver_reset.c - Program to test GCC driver reinitialization logic */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <dlfcn.h>
#include <errno.h>

/* Define this if you want to use direct library calls instead of fork/exec */
/* #define USE_DIRECT_LIBRARY_CALL */

/* Simple test C source file content */
const char *test_source = 
"#include <stdio.h>\n"
"int main() {\n"
"    printf(\"Hello from test program\\n\");\n"
"    return 0;\n"
"}\n";

/* Create a temporary source file for compilation */
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
int invoke_gcc_via_process(const char **argv) {
    pid_t pid = fork();
    
    if (pid == 0) {
        /* Child process */
        execvp("gcc", (char * const *)argv);
        /* If execvp fails */
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
        /* Try to load the GCC driver as a shared library */
        gcc_lib = dlopen("./libgccdriver.so", RTLD_LAZY);
        if (!gcc_lib) {
            gcc_lib = dlopen("libgccdriver.so", RTLD_LAZY);
        }
        if (!gcc_lib) {
            fprintf(stderr, "Failed to load libgccdriver.so: %s\n", dlerror());
            return -1;
        }
        
        /* Find the main function */
        gcc_main = (int (*)(int, const char **))dlsym(gcc_lib, "main");
        if (!gcc_main) {
            /* Try alternative entry point */
            gcc_main = (int (*)(int, const char **))dlsym(gcc_lib, "driver::main");
        }
        if (!gcc_main) {
            fprintf(stderr, "Failed to find driver entry point: %s\n", dlerror());
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

/* Test case 1: Multi-stage compilation with save-temps and dumpdir */
int test_multi_stage_compilation() {
    printf("=== Test 1: Multi-stage compilation with state reset ===\n");
    
    /* Create test source file */
    if (create_test_file("test_input.c") != 0) {
        return -1;
    }
    
    int overall_status = 0;
    
    /* First invocation: Set various flags that should be reset */
    const char *invocation1[] = {
        "gcc",
        "-save-temps",          /* Sets save_temps_flag */
        "-dumpdir", "/tmp/test_dump",  /* Sets dumpdir */
        "-dumpbase", "test_dumpbase",  /* Sets dumpbase */
        "-dumpbase-ext", ".c",  /* Sets dumpbase_ext */
        "--sysroot=/alt/sysroot", /* Alters target_system_root */
        "-specs=test.specs",    /* May affect spec_machine */
        "-march=native",        /* May affect spec_machine */
        "-c", "test_input.c",
        "-o", "output1.o",
        NULL
    };
    
    printf("Invocation 1: With save-temps, dumpdir, sysroot\n");
    int status1 = invoke_gcc_via_process(invocation1);
    printf("Exit status: %d\n", status1);
    overall_status |= (status1 != 0);
    
    /* Second invocation: Minimal flags to trigger reset to defaults */
    const char *invocation2[] = {
        "gcc",
        "-c", "test_input.c",
        "-o", "output2.o",
        NULL
    };
    
    printf("\nInvocation 2: Minimal flags (should trigger reset)\n");
    int status2 = invoke_gcc_via_process(invocation2);
    printf("Exit status: %d\n", status2);
    overall_status |= (status2 != 0);
    
    /* Clean up intermediate files */
    unlink("test_input.c");
    unlink("output1.o");
    unlink("output2.o");
    
    return overall_status;
}

/* Test case 2: Error handling and greatest_status reset */
int test_error_handling() {
    printf("\n=== Test 2: Error handling and greatest_status reset ===\n");
    
    /* Create test source file */
    if (create_test_file("test_input2.c") != 0) {
        return -1;
    }
    
    /* Third invocation: Force failure with invalid option */
    const char *invocation3[] = {
        "gcc",
        "-invalid-option-that-does-not-exist",
        "-c", "test_input2.c",
        "-o", "output3.o",
        NULL
    };
    
    printf("Invocation 3: Invalid option (should fail)\n");
    int status3 = invoke_gcc_via_process(invocation3);
    printf("Exit status: %d (non-zero expected)\n", status3);
    
    /* Fourth invocation: Successful compilation after failure */
    const char *invocation4[] = {
        "gcc",
        "-c", "test_input2.c",
        "-o", "output4.o",
        NULL
    };
    
    printf("\nInvocation 4: Valid compilation after failure\n");
    int status4 = invoke_gcc_via_process(invocation4);
    printf("Exit status: %d (zero expected)\n", status4);
    
    /* Clean up */
    unlink("test_input2.c");
    unlink("output3.o");
    unlink("output4.o");
    
    return (status4 != 0); /* Return non-zero if successful compilation failed */
}

/* Test case 3: Complex pipeline simulation */
int test_complex_pipeline() {
    printf("\n=== Test 3: Complex compilation pipeline ===\n");
    
    if (create_test_file("pipeline_input.c") != 0) {
        return -1;
    }
    
    /* Simulate preprocessing stage */
    const char *preprocess[] = {
        "gcc",
        "-E",                   /* Preprocess only */
        "-save-temps=obj",      /* Different save_temps mode */
        "-dumpdir", "pipeline_dir/",
        "-dumpbase", "pipeline",
        "-o", "pipeline.i",
        "pipeline_input.c",
        NULL
    };
    
    printf("Stage 1: Preprocessing with save-temps\n");
    int status1 = invoke_gcc_via_process(preprocess);
    
    /* Simulate compilation stage */
    const char *compile[] = {
        "gcc",
        "-S",                   /* Compile to assembly */
        "-dumpbase", "stage2",  /* Different dumpbase */
        "-o", "pipeline.s",
        "pipeline.i",
        NULL
    };
    
    printf("\nStage 2: Compilation (different dumpbase)\n");
    int status2 = invoke_gcc_via_process(compile);
    
    /* Simulate assembly stage - minimal flags */
    const char *assemble[] = {
        "gcc",
        "-c",
        "-o", "pipeline.o",
        "pipeline.s",
        NULL
    };
    
    printf("\nStage 3: Assembly (minimal flags, triggers reset)\n");
    int status3 = invoke_gcc_via_process(assemble);
    
    /* Clean up */
    unlink("pipeline_input.c");
    unlink("pipeline.i");
    unlink("pipeline.s");
    unlink("pipeline.o");
    
    return status1 | status2 | status3;
}

/* Alternative: Direct driver linking test */
#ifdef DRIVER_TEST
/* This would require linking with gcc driver object files */
extern int driver_main(int argc, const char **argv);

int test_direct_linking() {
    printf("\n=== Testing via direct driver linking ===\n");
    
    /* First call with special options */
    const char *args1[] = {"gcc", "-save-temps", "-dumpdir", "/tmp", "-c", "test.c", NULL};
    int argc1 = 5;
    printf("First driver call with save-temps\n");
    int result1 = driver_main(argc1, args1);
    
    /* Second call without special options */
    const char *args2[] = {"gcc", "-c", "test.c", NULL};
    int argc2 = 3;
    printf("\nSecond driver call (minimal)\n");
    int result2 = driver_main(argc2, args2);
    
    return result1 | result2;
}
#endif

int main(int argc, char **argv) {
    printf("GCC Driver Reinitialization Test Program\n");
    printf("=========================================\n\n");
    
    int overall_result = 0;
    
    /* Run test cases */
    overall_result |= test_multi_stage_compilation();
    overall_result |= test_error_handling();
    overall_result |= test_complex_pipeline();
    
#ifdef DRIVER_TEST
    overall_result |= test_direct_linking();
#endif
    
    printf("\n=========================================\n");
    printf("All tests completed. Overall result: %s\n", 
           overall_result == 0 ? "PASS" : "FAIL");
    
    return overall_result;
}
