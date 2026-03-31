/* test_driver_reinit.c
 * A program to test GCC driver reinitialization logic
 * Specifically targets lines 11228-11250 in gcc.cc
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

/* Simple test C source file content */
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
void test_with_processes() {
    printf("=== Testing with fork/exec (Process-based) ===\n");
    
    const char *input_file = "test_input.c";
    if (!create_test_file(input_file)) {
        return;
    }
    
    /* Array of test invocations with different flags */
    char *test_invocations[][10] = {
        /* First: Set various flags including sysroot and dump options */
        {"gcc", "-save-temps", "-dumpdir", "/tmp/test_dump1", 
         "-dumpbase", "test_dumpbase", "-c", input_file, 
         "-o", "output1.o", NULL},
        
        /* Second: Minimal invocation to trigger reset to defaults */
        {"gcc", "-c", input_file, "-o", "output2.o", NULL},
        
        /* Third: Invalid option to force failure and set greatest_status */
        {"gcc", "-invalid-option-that-does-not-exist", NULL},
        
        /* Fourth: Successful compilation after failure */
        {"gcc", "-c", input_file, "-o", "output3.o", NULL},
        
        /* Fifth: Test with sysroot and machine spec changes */
        {"gcc", "--sysroot=/tmp/fake_sysroot", "-march=native",
         "-c", input_file, "-o", "output4.o", NULL},
        
        /* Sixth: Back to defaults to trigger reset */
        {"gcc", "-c", input_file, "-o", "output5.o", NULL},
    };
    
    int num_tests = sizeof(test_invocations) / sizeof(test_invocations[0]);
    int statuses[num_tests];
    
    for (int i = 0; i < num_tests; i++) {
        printf("\n--- Invocation %d ---\n", i + 1);
        printf("Args: ");
        for (int j = 0; test_invocations[i][j] != NULL; j++) {
            printf("%s ", test_invocations[i][j]);
        }
        printf("\n");
        
        pid_t pid = fork();
        if (pid == 0) {
            /* Child process */
            execvp("gcc", test_invocations[i]);
            /* If execvp fails */
            perror("execvp failed");
            exit(127);
        } else if (pid > 0) {
            /* Parent process */
            int status;
            waitpid(pid, &status, 0);
            statuses[i] = status;
            
            if (WIFEXITED(status)) {
                printf("Exit status: %d\n", WEXITSTATUS(status));
            } else {
                printf("Process terminated abnormally\n");
            }
        } else {
            perror("fork failed");
            statuses[i] = -1;
        }
    }
    
    printf("\n=== Summary of exit statuses ===\n");
    for (int i = 0; i < num_tests; i++) {
        printf("Test %d: %d\n", i + 1, 
               WIFEXITED(statuses[i]) ? WEXITSTATUS(statuses[i]) : -1);
    }
    
    /* Cleanup */
    unlink(input_file);
    for (int i = 1; i <= 5; i++) {
        char filename[20];
        snprintf(filename, sizeof(filename), "output%d.o", i);
        unlink(filename);
    }
}

/* Method B: Direct library call using dlopen (if available) */
#ifdef TEST_WITH_DLOPEN
void test_with_dlopen() {
    printf("\n=== Testing with dlopen (Library-based) ===\n");
    
    /* This method requires GCC driver built as a shared library */
    void *handle = dlopen("./libgccdriver.so", RTLD_LAZY | RTLD_LOCAL);
    if (!handle) {
        printf("dlopen failed: %s\n", dlerror());
        printf("Skipping dlopen test (libgccdriver.so not found)\n");
        return;
    }
    
    /* Look for the driver's main function */
    int (*driver_main)(int, char**) = dlsym(handle, "main");
    if (!driver_main) {
        /* Try alternative entry point names */
        driver_main = dlsym(handle, "gcc_main");
        if (!driver_main) {
            driver_main = dlsym(handle, "driver::main");
        }
    }
    
    if (!driver_main) {
        printf("Could not find driver entry point: %s\n", dlerror());
        dlclose(handle);
        return;
    }
    
    /* Create test source file */
    const char *input_file = "test_input_lib.c";
    if (!create_test_file(input_file)) {
        dlclose(handle);
        return;
    }
    
    /* Test argument vectors for multiple invocations */
    char *argv1[] = {
        "gcc", "-save-temps", "-dumpdir", "/tmp/lib_test",
        "-dumpbase", "lib_dump", "-c", "test_input_lib.c",
        "-o", "lib_output1.o", NULL
    };
    
    char *argv2[] = {
        "gcc", "-c", "test_input_lib.c", "-o", "lib_output2.o", NULL
    };
    
    char *argv3[] = {
        "gcc", "--sysroot=/tmp/fake_root", "-c", 
        "test_input_lib.c", "-o", "lib_output3.o", NULL
    };
    
    char *argv4[] = {
        "gcc", "-c", "test_input_lib.c", "-o", "lib_output4.o", NULL
    };
    
    char *argv5[] = {
        "gcc", "-invalid-option", NULL
    };
    
    char *argv6[] = {
        "gcc", "-c", "test_input_lib.c", "-o", "lib_output5.o", NULL
    };
    
    /* Execute multiple invocations in the same process */
    int argc;
    int results[6];
    
    printf("\nInvocation 1 (with save-temps and dumpdir):\n");
    argc = sizeof(argv1)/sizeof(argv1[0]) - 1;
    results[0] = driver_main(argc, argv1);
    
    printf("\nInvocation 2 (minimal, should reset to defaults):\n");
    argc = sizeof(argv2)/sizeof(argv2[0]) - 1;
    results[1] = driver_main(argc, argv2);
    
    printf("\nInvocation 3 (with sysroot):\n");
    argc = sizeof(argv3)/sizeof(argv3[0]) - 1;
    results[2] = driver_main(argc, argv3);
    
    printf("\nInvocation 4 (back to defaults):\n");
    argc = sizeof(argv4)/sizeof(argv4[0]) - 1;
    results[3] = driver_main(argc, argv4);
    
    printf("\nInvocation 5 (invalid option to set greatest_status):\n");
    argc = sizeof(argv5)/sizeof(argv5[0]) - 1;
    results[4] = driver_main(argc, argv5);
    
    printf("\nInvocation 6 (successful after failure):\n");
    argc = sizeof(argv6)/sizeof(argv6[0]) - 1;
    results[5] = driver_main(argc, argv6);
    
    printf("\n=== Library call results ===\n");
    for (int i = 0; i < 6; i++) {
        printf("Call %d returned: %d\n", i + 1, results[i]);
    }
    
    /* Cleanup */
    unlink(input_file);
    for (int i = 1; i <= 5; i++) {
        char filename[30];
        snprintf(filename, sizeof(filename), "lib_output%d.o", i);
        unlink(filename);
    }
    
    dlclose(handle);
}
#endif

/* Method C: Simulate multi-stage compilation pipeline */
void test_multi_stage_pipeline() {
    printf("\n=== Testing Multi-Stage Compilation Pipeline ===\n");
    
    const char *input_file = "pipeline_input.c";
    if (!create_test_file(input_file)) {
        return;
    }
    
    /* Simulate a full compilation pipeline with different flags per stage */
    struct {
        char *args[15];
        char *description;
    } stages[] = {
        {
            {"gcc", "-E", "-save-temps", "-dumpdir", "/tmp/stage1",
             "-dumpbase", "preprocess", input_file, 
             "-o", "preprocessed.i", NULL},
            "Preprocessing with save-temps"
        },
        {
            {"gcc", "-S", "-save-temps=obj", "-dumpdir", "/tmp/stage2",
             "-dumpbase", "compile", "preprocessed.i",
             "-o", "compiled.s", NULL},
            "Compilation with different save-temps mode"
        },
        {
            {"gcc", "-c", "-dumpbase", "assemble", "compiled.s",
             "-o", "assembled.o", NULL},
            "Assembly without save-temps (should reset flag)"
        },
        {
            {"gcc", "--sysroot=/tmp/custom_root", "-v",
             "assembled.o", "-o", "final_executable", NULL},
            "Linking with sysroot and verbose"
        },
        {
            {"gcc", "assembled.o", "-o", "final_default", NULL},
            "Linking without sysroot (should reset to default)"
        }
    };
    
    int num_stages = sizeof(stages) / sizeof(stages[0]);
    
    for (int i = 0; i < num_stages; i++) {
        printf("\n--- Stage %d: %s ---\n", i + 1, stages[i].description);
        
        pid_t pid = fork();
        if (pid == 0) {
            execvp("gcc", stages[i].args);
            perror("execvp failed");
            exit(127);
        } else if (pid > 0) {
            int status;
            waitpid(pid, &status, 0);
            if (WIFEXITED(status)) {
                printf("Exit: %d\n", WEXITSTATUS(status));
            }
        }
    }
    
    /* Cleanup pipeline files */
    unlink(input_file);
    unlink("preprocessed.i");
    unlink("compiled.s");
    unlink("assembled.o");
    unlink("final_executable");
    unlink("final_default");
}

int main(int argc, char *argv[]) {
    printf("GCC Driver Reinitialization Test Program\n");
    printf("Targeting uncovered lines 11228-11250 in gcc.cc\n\n");
    
    /* Test with process-based invocations */
    test_with_processes();
    
    /* Test multi-stage compilation pipeline */
    test_multi_stage_pipeline();
    
#ifdef TEST_WITH_DLOPEN
    /* Test with direct library calls if enabled */
    test_with_dlopen();
#endif
    
    printf("\n=== Test Complete ===\n");
    printf("Check coverage data to verify lines 11228-11250 were executed.\n");
    
    return 0;
}
