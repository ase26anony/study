/* test_gcc_driver_reinit.c
 * Tests the driver reinitialization logic in gcc.cc lines 11228-11250
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

/* Create a minimal C source file for compilation */
void create_test_source(const char *filename) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        perror("Failed to create test source");
        exit(1);
    }
    fprintf(f, "/* Test source for driver reinitialization */\n");
    fprintf(f, "int main() {\n");
    fprintf(f, "    return 0;\n");
    fprintf(f, "}\n");
    fclose(f);
}

/* Method A: Process-based invocation using fork/exec */
void test_via_processes(const char *gcc_path, const char *input_file) {
    printf("=== Testing via fork/exec (Process-based) ===\n");
    
    /* First invocation: Set various flags including sysroot and dump options */
    char *argv1[] = {
        (char *)gcc_path,
        "-save-temps",
        "-dumpdir", "/tmp/test_dump1",
        "-dumpbase", "test_dumpbase1",
        "-c",
        (char *)input_file,
        "-o", "output1.o",
        "--sysroot=/tmp/fake_sysroot",
        "-march=x86-64",
        NULL
    };
    
    /* Second invocation: Minimal flags to trigger reset to defaults */
    char *argv2[] = {
        (char *)gcc_path,
        "-c",
        (char *)input_file,
        "-o", "output2.o",
        NULL
    };
    
    /* Third invocation: Force failure with invalid option */
    char *argv3[] = {
        (char *)gcc_path,
        "-invalid-option-that-does-not-exist",
        (char *)input_file,
        NULL
    };
    
    /* Fourth invocation: Successful compilation after failure */
    char *argv4[] = {
        (char *)gcc_path,
        "-c",
        (char *)input_file,
        "-o", "output3.o",
        "-O2",
        NULL
    };
    
    char *argvs[] = {argv1[0], argv2[0], argv3[0], argv4[0]};
    char ***all_argv = (char ***)malloc(4 * sizeof(char **));
    all_argv[0] = argv1;
    all_argv[1] = argv2;
    all_argv[2] = argv3;
    all_argv[3] = argv4;
    
    int statuses[4];
    
    for (int i = 0; i < 4; i++) {
        printf("\n--- Invocation %d ---\n", i + 1);
        printf("Args: ");
        for (int j = 0; all_argv[i][j]; j++) {
            printf("%s ", all_argv[i][j]);
        }
        printf("\n");
        
        pid_t pid = fork();
        if (pid == 0) {
            /* Child process */
            execv(gcc_path, all_argv[i]);
            perror("execv failed");
            exit(127);
        } else if (pid > 0) {
            /* Parent process */
            waitpid(pid, &statuses[i], 0);
            printf("Exit status: %d\n", WEXITSTATUS(statuses[i]));
        } else {
            perror("fork failed");
            exit(1);
        }
    }
    
    free(all_argv);
    
    printf("\n=== Process-based test complete ===\n");
    printf("Expected: Invocation 3 should fail, others should succeed\n");
}

/* Method B: Direct library call using dlopen/dlsym */
#ifdef TEST_DIRECT_CALL
void test_via_direct_call(const char *driver_lib_path) {
    printf("\n=== Testing via direct library call ===\n");
    
    void *handle = dlopen(driver_lib_path, RTLD_LAZY | RTLD_LOCAL);
    if (!handle) {
        fprintf(stderr, "dlopen failed: %s\n", dlerror());
        return;
    }
    
    /* Look for driver entry point - could be main() or driver::main() */
    int (*driver_main)(int, char **) = dlsym(handle, "main");
    if (!driver_main) {
        /* Try alternative names */
        driver_main = dlsym(handle, "_Z6driveriPPc");  // mangled name for driver::main
    }
    
    if (!driver_main) {
        fprintf(stderr, "Could not find driver entry point: %s\n", dlerror());
        dlclose(handle);
        return;
    }
    
    /* Create test argument vectors */
    char arg0[] = "gcc";
    char arg1[] = "-save-temps";
    char arg2[] = "-dumpdir";
    char arg3[] = "/tmp/test_direct";
    char arg4[] = "-dumpbase";
    char arg5[] = "direct_base";
    char arg6[] = "-c";
    char arg7[] = "input.c";
    char arg8[] = "-o";
    char arg9[] = "direct_output.o";
    char arg10[] = "--sysroot=/tmp/alt_root";
    
    char *argv1[] = {arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, NULL};
    
    char arg11[] = "-c";
    char arg12[] = "input.c";
    char arg13[] = "-o";
    char arg14[] = "direct_output2.o";
    
    char *argv2[] = {arg0, arg11, arg12, arg13, arg14, NULL};
    
    char arg15[] = "-invalid-option-force-failure";
    
    char *argv3[] = {arg0, arg15, arg12, NULL};
    
    char arg16[] = "-O3";
    char *argv4[] = {arg0, arg11, arg12, arg13, arg14, arg16, NULL};
    
    printf("First call with special flags...\n");
    int ret1 = driver_main(11, argv1);
    printf("Return: %d\n", ret1);
    
    printf("\nSecond call with minimal flags (should trigger reset)...\n");
    int ret2 = driver_main(5, argv2);
    printf("Return: %d\n", ret2);
    
    printf("\nThird call with invalid option (should fail)...\n");
    int ret3 = driver_main(3, argv3);
    printf("Return: %d\n", ret3);
    
    printf("\nFourth call after failure (should succeed)...\n");
    int ret4 = driver_main(6, argv4);
    printf("Return: %d\n", ret4);
    
    dlclose(handle);
    printf("\n=== Direct call test complete ===\n");
}
#endif

/* Method C: Simulate multi-stage compilation pipeline */
void test_multi_stage_pipeline(const char *gcc_path, const char *input_file) {
    printf("\n=== Testing multi-stage compilation pipeline ===\n");
    
    /* Stage 1: Preprocessing with save-temps and custom dumpdir */
    char *stage1_args[] = {
        (char *)gcc_path,
        "-E",  /* Preprocess only */
        "-save-temps",
        "-dumpdir", "/tmp/pipeline_stage1",
        "-dumpbase", "pipeline",
        (char *)input_file,
        "-o", "output.i",
        "--sysroot=/tmp/stage1_sysroot",
        NULL
    };
    
    /* Stage 2: Compilation to assembly with different dumpdir */
    char *stage2_args[] = {
        (char *)gcc_path,
        "-S",  /* Compile to assembly */
        "-save-temps=obj",
        "-dumpdir", "/tmp/pipeline_stage2",
        "-dumpbase", "pipeline_asm",
        (char *)input_file,
        "-o", "output.s",
        "-march=native",  /* Change machine spec */
        NULL
    };
    
    /* Stage 3: Assembly to object (no special flags - trigger reset) */
    char *stage3_args[] = {
        (char *)gcc_path,
        "-c",
        (char *)input_file,
        "-o", "output.o",
        NULL
    };
    
    /* Stage 4: Failed linking attempt */
    char *stage4_args[] = {
        (char *)gcc_path,
        "nonexistent_file.o",
        "-o", "program",
        NULL
    };
    
    /* Stage 5: Successful linking */
    char *stage5_args[] = {
        (char *)gcc_path,
        "output.o",
        "-o", "program",
        NULL
    };
    
    char ***stages[] = {stage1_args, stage2_args, stage3_args, stage4_args, stage5_args};
    int stage_counts[] = {9, 9, 4, 4, 4};
    
    for (int i = 0; i < 5; i++) {
        printf("\n--- Stage %d ---\n", i + 1);
        printf("Args: ");
        for (int j = 0; stages[i][j]; j++) {
            printf("%s ", stages[i][j]);
        }
        printf("\n");
        
        pid_t pid = fork();
        if (pid == 0) {
            execv(gcc_path, stages[i]);
            perror("execv failed");
            exit(127);
        } else if (pid > 0) {
            int status;
            waitpid(pid, &status, 0);
            printf("Exit status: %d\n", WEXITSTATUS(status));
        } else {
            perror("fork failed");
        }
    }
    
    printf("\n=== Multi-stage pipeline test complete ===\n");
}

int main(int argc, char **argv) {
    const char *input_file = "test_input.c";
    const char *gcc_path = "gcc";
    
    /* Override with command line arguments if provided */
    if (argc > 1) {
        gcc_path = argv[1];
    }
    if (argc > 2) {
        input_file = argv[2];
    }
    
    printf("Testing GCC driver reinitialization logic\n");
    printf("GCC path: %s\n", gcc_path);
    printf("Input file: %s\n", input_file);
    
    /* Create test source file */
    create_test_source(input_file);
    
    /* Create fake sysroot directory for testing */
    mkdir("/tmp/fake_sysroot", 0755);
    mkdir("/tmp/alt_root", 0755);
    mkdir("/tmp/stage1_sysroot", 0755);
    
    /* Test 1: Process-based invocations */
    test_via_processes(gcc_path, input_file);
    
    /* Test 2: Multi-stage pipeline */
    test_multi_stage_pipeline(gcc_path, input_file);
    
#ifdef TEST_DIRECT_CALL
    /* Test 3: Direct library call (if enabled) */
    if (argc > 3) {
        test_via_direct_call(argv[3]);
    }
#endif
    
    /* Cleanup */
    unlink(input_file);
    unlink("output1.o");
    unlink("output2.o");
    unlink("output3.o");
    unlink("output.i");
    unlink("output.s");
    unlink("output.o");
    unlink("program");
    
    printf("\n=== All tests completed ===\n");
    return 0;
}
