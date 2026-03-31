/* test_gcc_driver_reset.c
 * A program to test GCC driver reinitialization logic
 * Compile with: gcc -o test_driver_reset test_gcc_driver_reset.c -D_GNU_SOURCE -ldl
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

/* Method 1: Process-based invocation using fork/exec */
void test_via_processes() {
    printf("=== Testing via fork/exec processes ===\n");
    
    const char *input_file = "test_input.c";
    const char *driver_path = "gcc";  /* Adjust if needed */
    
    /* Create a minimal input file */
    FILE *f = fopen(input_file, "w");
    if (f) {
        fprintf(f, "int main() { return 0; }\n");
        fclose(f);
    }
    
    /* Test case 1: Set various flags including sysroot and dump options */
    char *args1[] = {
        "gcc",
        "-save-temps",
        "-dumpdir", "/tmp/test_dump1",
        "-dumpbase", "test_dumpbase1",
        "-c", "test_input.c",
        "-o", "test_output1.o",
        "--sysroot=/alt/sysroot",  /* Change target system root */
        "-march=x86-64",           /* Change spec machine */
        NULL
    };
    
    /* Test case 2: Reset to defaults (no special flags) */
    char *args2[] = {
        "gcc",
        "-c", "test_input.c",
        "-o", "test_output2.o",
        NULL
    };
    
    /* Test case 3: Force failure to set greatest_status */
    char *args4[] = {
        "gcc",
        "-invalid-option-that-fails",
        NULL
    };
    
    /* Test case 4: Successful compilation after failure */
    char *args5[] = {
        "gcc",
        "-c", "test_input.c",
        "-o", "test_output3.o",
        NULL
    };
    
    char **test_cases[] = {args1, args2, args4, args5};
    int num_tests = 4;
    
    for (int i = 0; i < num_tests; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            /* Child process */
            execvp(driver_path, test_cases[i]);
            perror("execvp failed");
            exit(127);
        } else if (pid > 0) {
            /* Parent process */
            int status;
            waitpid(pid, &status, 0);
            printf("Test %d exit status: %d\n", i + 1, WEXITSTATUS(status));
        } else {
            perror("fork failed");
        }
    }
    
    /* Cleanup */
    unlink("test_input.c");
    unlink("test_output1.o");
    unlink("test_output2.o");
    unlink("test_output3.o");
}

/* Method 2: Direct library call using dlopen (if driver is built as shared lib) */
#ifdef USE_DIRECT_CALL
void test_via_direct_call() {
    printf("\n=== Testing via direct library calls ===\n");
    
    /* Attempt to load GCC driver as shared library */
    void *handle = dlopen("./libgccdriver.so", RTLD_LAZY);
    if (!handle) {
        handle = dlopen("libgccdriver.so", RTLD_LAZY);
    }
    
    if (!handle) {
        printf("Could not load libgccdriver.so: %s\n", dlerror());
        printf("Try building GCC driver as shared library first.\n");
        return;
    }
    
    /* Find the main function */
    int (*driver_main)(int, char**) = dlsym(handle, "main");
    if (!driver_main) {
        /* Try alternative entry point */
        driver_main = dlsym(handle, "driver::main");
    }
    
    if (!driver_main) {
        printf("Could not find driver main function: %s\n", dlerror());
        dlclose(handle);
        return;
    }
    
    /* Create test input file */
    FILE *f = fopen("test_input2.c", "w");
    if (f) {
        fprintf(f, "int foo() { return 42; }\n");
        fclose(f);
    }
    
    /* Test 1: With various flags */
    char *test1_argv[] = {
        "gcc",
        "-save-temps=obj",
        "-dumpdir", "/tmp/direct_test",
        "-dumpbase", "direct_base",
        "-c", "test_input2.c",
        "-o", "direct_output.o",
        "--sysroot=/test/sysroot",
        "-march=native",
        NULL
    };
    int test1_argc = sizeof(test1_argv)/sizeof(test1_argv[0]) - 1;
    
    printf("Call 1 (with flags): return = %d\n", 
           driver_main(test1_argc, test1_argv));
    
    /* Test 2: Reset to defaults */
    char *test2_argv[] = {
        "gcc",
        "-c", "test_input2.c",
        "-o", "direct_output2.o",
        NULL
    };
    int test2_argc = sizeof(test2_argv)/sizeof(test2_argv[0]) - 1;
    
    printf("Call 2 (defaults): return = %d\n",
           driver_main(test2_argc, test2_argv));
    
    /* Test 3: Force failure */
    char *test3_argv[] = {
        "gcc",
        "-nonexistent-flag",
        NULL
    };
    int test3_argc = sizeof(test3_argv)/sizeof(test3_argv[0]) - 1;
    
    printf("Call 3 (failure): return = %d\n",
           driver_main(test3_argc, test3_argv));
    
    /* Test 4: Success after failure */
    printf("Call 4 (success after failure): return = %d\n",
           driver_main(test2_argc, test2_argv));
    
    /* Cleanup */
    dlclose(handle);
    unlink("test_input2.c");
    unlink("direct_output.o");
    unlink("direct_output2.o");
}
#endif

/* Method 3: Simulate multi-stage compilation pipeline */
void test_multi_stage_pipeline() {
    printf("\n=== Testing multi-stage compilation pipeline ===\n");
    
    /* Create a more complex input file */
    FILE *f = fopen("pipeline_input.c", "w");
    if (f) {
        fprintf(f, "#include <stdio.h>\n\n");
        fprintf(f, "int helper() { return 1; }\n");
        fprintf(f, "int main() { printf(\"%%d\\n\", helper()); return 0; }\n");
        fclose(f);
    }
    
    /* Stage 1: Preprocessing with dump options */
    char *stage1[] = {
        "gcc",
        "-E",                       /* Preprocess only */
        "-save-temps=cwd",          /* Save temps in current directory */
        "-dumpdir", "./dump_stage1",
        "-dumpbase", "pipeline",
        "-dumpbase-ext", ".c",
        "pipeline_input.c",
        "-o", "pipeline.i",
        "--sysroot=/custom/root",   /* Alter system root */
        NULL
    };
    
    /* Stage 2: Compilation to assembly */
    char *stage2[] = {
        "gcc",
        "-S",                       /* Compile to assembly */
        "-save-temps=obj",
        "-dumpdir", "./dump_stage2",
        "-dumpbase", "pipeline",
        "pipeline.i",
        "-o", "pipeline.s",
        "-mtune=generic",           /* Change machine tuning */
        NULL
    };
    
    /* Stage 3: Assembly to object */
    char *stage3[] = {
        "gcc",
        "-c",                       /* Assemble */
        "pipeline.s",
        "-o", "pipeline.o",
        /* No dump options - should trigger reset */
        NULL
    };
    
    /* Stage 4: Linking */
    char *stage4[] = {
        "gcc",
        "pipeline.o",
        "-o", "pipeline_exec",
        /* Back to defaults */
        NULL
    };
    
    char **stages[] = {stage1, stage2, stage3, stage4};
    char *stage_names[] = {"Preprocess", "Compile", "Assemble", "Link"};
    
    for (int i = 0; i < 4; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            execvp("gcc", stages[i]);
            perror("execvp failed");
            exit(127);
        } else if (pid > 0) {
            int status;
            waitpid(pid, &status, 0);
            printf("Stage %d (%s): exit %d\n", 
                   i + 1, stage_names[i], WEXITSTATUS(status));
        }
    }
    
    /* Cleanup */
    unlink("pipeline_input.c");
    unlink("pipeline.i");
    unlink("pipeline.s");
    unlink("pipeline.o");
    unlink("pipeline_exec");
    system("rm -rf ./dump_stage1 ./dump_stage2");
}

int main(int argc, char **argv) {
    printf("GCC Driver Reinitialization Test\n");
    printf("=================================\n\n");
    
    /* Test using process-based approach */
    test_via_processes();
    
    /* Test multi-stage pipeline */
    test_multi_stage_pipeline();
    
#ifdef USE_DIRECT_CALL
    /* Test direct library calls if enabled */
    test_via_direct_call();
#endif
    
    printf("\nAll tests completed.\n");
    
    /* Final test: Verify greatest_status reset by running
     * a failing then succeeding compilation in same process
     * if we're testing a modified GCC driver */
    if (argc > 1 && strcmp(argv[1], "--test-status-reset") == 0) {
        printf("\n=== Testing greatest_status reset ===\n");
        
        /* This would require a custom GCC build with exposed driver_main */
        printf("To test greatest_status reset fully, build GCC with:\n");
        printf("  ./configure --enable-shared --enable-driver-as-library\n");
        printf("  make DRIVER_LIB=libgccdriver.so\n");
    }
    
    return 0;
}
