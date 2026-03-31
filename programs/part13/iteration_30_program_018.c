/* test_driver_reinit.c - Test program to trigger gcc driver reinitialization logic */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <dlfcn.h>
#include <errno.h>

/* Simple test C source file content */
const char *test_source = 
"#include <stdio.h>\n"
"int main() {\n"
"    printf(\"Hello from test program\\n\");\n"
"    return 0;\n"
"}\n";

/* Method A: Process-based invocation using fork/exec */
void test_via_processes() {
    printf("=== Testing via fork/exec processes ===\n");
    
    /* Create test source file */
    FILE *fp = fopen("test_input.c", "w");
    if (!fp) {
        perror("Failed to create test_input.c");
        return;
    }
    fputs(test_source, fp);
    fclose(fp);
    
    /* Array of test invocations with different flags */
    char *test_invocations[][10] = {
        /* First: Set various flags including sysroot and dump options */
        {"gcc", "-save-temps", "-dumpdir", "/tmp/test_dump1", 
         "-dumpbase", "test_dumpbase", "-c", "test_input.c", 
         "-o", "test_output1.o", NULL},
        
        /* Second: Minimal invocation to trigger reset to defaults */
        {"gcc", "-c", "test_input.c", "-o", "test_output2.o", NULL},
        
        /* Third: Force failure with invalid option */
        {"gcc", "-invalid-option-that-does-not-exist", NULL},
        
        /* Fourth: Successful compilation again */
        {"gcc", "-c", "test_input.c", "-o", "test_output3.o", NULL},
        
        /* Fifth: With sysroot to test target_system_root reset */
        {"gcc", "--sysroot=/tmp/fake_sysroot", "-c", "test_input.c", 
         "-o", "test_output4.o", NULL},
        
        /* Sixth: Without sysroot to trigger default reset */
        {"gcc", "-c", "test_input.c", "-o", "test_output5.o", NULL}
    };
    
    int num_tests = sizeof(test_invocations) / sizeof(test_invocations[0]);
    int status;
    pid_t pid;
    
    for (int i = 0; i < num_tests; i++) {
        printf("\nTest %d: ", i + 1);
        for (int j = 0; test_invocations[i][j]; j++) {
            printf("%s ", test_invocations[i][j]);
        }
        printf("\n");
        
        pid = fork();
        if (pid == 0) {
            /* Child process */
            execvp("gcc", test_invocations[i]);
            /* If exec fails */
            perror("execvp failed");
            exit(127);
        } else if (pid > 0) {
            /* Parent process */
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
    unlink("test_input.c");
    unlink("test_output1.o");
    unlink("test_output2.o");
    unlink("test_output3.o");
    unlink("test_output4.o");
    unlink("test_output5.o");
    unlink("/tmp/test_dump1/test_dumpbase.*");
}

/* Method B: Direct library call using dlopen/dlsym */
#ifdef DRIVER_TEST
void test_via_dlopen() {
    printf("\n=== Testing via dlopen/dlsym ===\n");
    
    /* Try to load gcc driver as shared library */
    void *handle = dlopen("libgccdriver.so", RTLD_LAZY);
    if (!handle) {
        /* Try alternative names */
        handle = dlopen("./libgccdriver.so", RTLD_LAZY);
        if (!handle) {
            printf("Could not load libgccdriver.so: %s\n", dlerror());
            printf("Skipping dlopen test - driver may not be built as shared library\n");
            return;
        }
    }
    
    /* Look for main function */
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
    
    /* Create test source file for library testing */
    FILE *fp = fopen("libtest_input.c", "w");
    if (!fp) {
        perror("Failed to create libtest_input.c");
        dlclose(handle);
        return;
    }
    fputs(test_source, fp);
    fclose(fp);
    
    /* Test invocations for direct library calls */
    char *test1_args[] = {
        "gcc", "-save-temps", "-dumpdir", "/tmp/libtest1",
        "-dumpbase", "libdump", "-c", "libtest_input.c",
        "-o", "liboutput1.o", NULL
    };
    
    char *test2_args[] = {
        "gcc", "-c", "libtest_input.c", "-o", "liboutput2.o", NULL
    };
    
    char *test3_args[] = {
        "gcc", "-invalid-option-force-failure", NULL
    };
    
    char *test4_args[] = {
        "gcc", "-c", "libtest_input.c", "-o", "liboutput3.o", NULL
    };
    
    /* Execute driver multiple times in same process */
    printf("\nFirst invocation with special flags:\n");
    int result1 = driver_main(10, test1_args);
    printf("Driver returned: %d\n", result1);
    
    printf("\nSecond invocation (minimal, should trigger reset):\n");
    int result2 = driver_main(5, test2_args);
    printf("Driver returned: %d\n", result2);
    
    printf("\nThird invocation (should fail):\n");
    int result3 = driver_main(2, test3_args);
    printf("Driver returned: %d\n", result3);
    
    printf("\nFourth invocation (should succeed):\n");
    int result4 = driver_main(5, test4_args);
    printf("Driver returned: %d\n", result4);
    
    /* Cleanup */
    dlclose(handle);
    unlink("libtest_input.c");
    unlink("liboutput1.o");
    unlink("liboutput2.o");
    unlink("liboutput3.o");
}
#endif

/* Method C: Simulate multi-stage compilation pipeline */
void test_multi_stage_pipeline() {
    printf("\n=== Testing multi-stage compilation pipeline ===\n");
    
    /* Create a more complex test file */
    const char *complex_source = 
    "#define TEST_MACRO 42\n"
    "int add(int a, int b) { return a + b; }\n"
    "int main() { return add(1, TEST_MACRO); }\n";
    
    FILE *fp = fopen("pipeline_input.c", "w");
    if (!fp) {
        perror("Failed to create pipeline_input.c");
        return;
    }
    fputs(complex_source, fp);
    fclose(fp);
    
    /* Simulate compilation pipeline with different flags at each stage */
    char *pipeline_stages[][15] = {
        /* Stage 1: Preprocessing with save-temps and dumpdir */
        {"gcc", "-E", "-save-temps=obj", "-dumpdir", "/tmp/pipeline",
         "-dumpbase", "stage1", "pipeline_input.c", 
         "-o", "pipeline.i", NULL},
        
        /* Stage 2: Compilation to assembly with different dumpbase */
        {"gcc", "-S", "-save-temps", "-dumpbase", "stage2",
         "pipeline_input.c", "-o", "pipeline.s", NULL},
        
        /* Stage 3: Assembly to object with sysroot */
        {"gcc", "-c", "--sysroot=/tmp/fake_root",
         "pipeline.s", "-o", "pipeline.o", NULL},
        
        /* Stage 4: Linking without special flags (triggers reset) */
        {"gcc", "pipeline.o", "-o", "pipeline.out", NULL},
        
        /* Stage 5: Clean compilation with all defaults */
        {"gcc", "pipeline_input.c", "-o", "pipeline_final", NULL}
    };
    
    int num_stages = sizeof(pipeline_stages) / sizeof(pipeline_stages[0]);
    
    for (int i = 0; i < num_stages; i++) {
        printf("\nPipeline Stage %d: ", i + 1);
        for (int j = 0; pipeline_stages[i][j]; j++) {
            printf("%s ", pipeline_stages[i][j]);
        }
        printf("\n");
        
        pid_t pid = fork();
        if (pid == 0) {
            execvp("gcc", pipeline_stages[i]);
            perror("execvp failed");
            exit(127);
        } else if (pid > 0) {
            int status;
            waitpid(pid, &status, 0);
            if (WIFEXITED(status)) {
                printf("Exit status: %d\n", WEXITSTATUS(status));
            }
        }
    }
    
    /* Execute the final program if successful */
    if (access("pipeline_final", X_OK) == 0) {
        printf("\nExecuting final program:\n");
        system("./pipeline_final");
    }
    
    /* Cleanup */
    unlink("pipeline_input.c");
    unlink("pipeline.i");
    unlink("pipeline.s");
    unlink("pipeline.o");
    unlink("pipeline.out");
    unlink("pipeline_final");
}

/* Method D: Test with spec machine and march options */
void test_spec_machine_reset() {
    printf("\n=== Testing spec machine and march reset ===\n");
    
    FILE *fp = fopen("spec_test.c", "w");
    if (!fp) {
        perror("Failed to create spec_test.c");
        return;
    }
    fputs("int main() { return 0; }\n", fp);
    fclose(fp);
    
    /* Test different machine specifications */
    char *spec_tests[][10] = {
        /* With march and mtune */
        {"gcc", "-march=x86-64", "-mtune=generic", "-c", 
         "spec_test.c", "-o", "spec1.o", NULL},
        
        /* With different march */
        {"gcc", "-march=nocona", "-c", "spec_test.c", 
         "-o", "spec2.o", NULL},
        
        /* Without march (should trigger reset to default) */
        {"gcc", "-c", "spec_test.c", "-o", "spec3.o", NULL},
        
        /* With specs file if available */
        {"gcc", "-specs=/usr/lib/gcc/x86_64-linux-gnu/*/specs",
         "-c", "spec_test.c", "-o", "spec4.o", NULL},
        
        /* Again without specs (reset) */
        {"gcc", "-c", "spec_test.c", "-o", "spec5.o", NULL}
    };
    
    int num_tests = sizeof(spec_tests) / sizeof(spec_tests[0]);
    
    for (int i = 0; i < num_tests; i++) {
        printf("\nSpec test %d: ", i + 1);
        for (int j = 0; spec_tests[i][j]; j++) {
            printf("%s ", spec_tests[i][j]);
        }
        printf("\n");
        
        pid_t pid = fork();
        if (pid == 0) {
            execvp("gcc", spec_tests[i]);
            perror("execvp failed");
            exit(127);
        } else if (pid > 0) {
            int status;
            waitpid(pid, &status, 0);
            if (WIFEXITED(status)) {
                printf("Exit status: %d\n", WEXITSTATUS(status));
            }
        }
    }
    
    /* Cleanup */
    unlink("spec_test.c");
    unlink("spec1.o");
    unlink("spec2.o");
    unlink("spec3.o");
    unlink("spec4.o");
    unlink("spec5.o");
}

int main(int argc, char *argv[]) {
    printf("GCC Driver Reinitialization Test Program\n");
    printf("=========================================\n");
    
    /* Test all methods */
    test_via_processes();
    
#ifdef DRIVER_TEST
    test_via_dlopen();
#endif
    
    test_multi_stage_pipeline();
    test_spec_machine_reset();
    
    printf("\n=== All tests completed ===\n");
    
    /* Final test: Verify greatest_status behavior */
    printf("\n=== Testing greatest_status reset ===\n");
    printf("This test demonstrates that after a failed compilation,\n");
    printf("a subsequent successful one should work correctly.\n");
    
    /* Create final test file */
    FILE *fp = fopen("final_test.c", "w");
    if (fp) {
        fputs("int main() { return 0; }\n", fp);
        fclose(fp);
        
        /* First, force a failure */
        printf("\n1. Forcing failure with invalid option:\n");
        if (system("gcc -this-option-does-not-exist 2>/dev/null") != 0) {
            printf("   (Expected failure occurred)\n");
        }
        
        /* Then, successful compilation */
        printf("\n2. Successful compilation (should reset greatest_status):\n");
        int result = system("gcc -c final_test.c -o final_test.o 2>&1");
        printf("   Compilation result: %s\n", 
               WEXITSTATUS(result) == 0 ? "SUCCESS" : "FAILURE");
        
        /* Cleanup */
        unlink("final_test.c");
        unlink("final_test.o");
    }
    
    return 0;
}
