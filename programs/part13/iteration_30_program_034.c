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

/* Create a minimal C source file for compilation */
void create_test_source(const char *filename) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        perror("Failed to create test source");
        exit(1);
    }
    
    fprintf(f, "/* Test source for driver reinitialization */\n");
    fprintf(f, "#include <stdio.h>\n\n");
    fprintf(f, "int main() {\n");
    fprintf(f, "    printf(\"Hello from test program\\n\");\n");
    fprintf(f, "    return 0;\n");
    fprintf(f, "}\n");
    
    fclose(f);
}

/* Method A: Process-based testing using fork/exec */
void test_via_processes() {
    printf("=== Testing via fork/exec (Process-based) ===\n");
    
    const char *input_file = "test_input.c";
    create_test_source(input_file);
    
    /* Array of test invocations with different flags */
    char *test_cases[][20] = {
        /* First: Set various flags including sysroot and dump options */
        {"gcc", "-save-temps", "-dumpdir", "/tmp/test_dump1", 
         "-dumpbase", "test_dumpbase", "-c", input_file, 
         "-o", "output1.o", "--sysroot=/usr/alt", 
         "-specs=/usr/share/gcc/specs/default.specs", NULL},
        
        /* Second: Minimal invocation to trigger reset to defaults */
        {"gcc", "-c", input_file, "-o", "output2.o", NULL},
        
        /* Third: Force failure with invalid option */
        {"gcc", "-invalid-option-that-does-not-exist", input_file, NULL},
        
        /* Fourth: Successful compilation again */
        {"gcc", "-c", input_file, "-o", "output3.o", 
         "-save-temps=obj", "-dumpdir", "/tmp/test_dump2", NULL},
        
        /* Fifth: Another minimal to ensure full reset */
        {"gcc", "-c", input_file, "-o", "output4.o", NULL}
    };
    
    int num_cases = sizeof(test_cases) / sizeof(test_cases[0]);
    int statuses[num_cases];
    
    for (int i = 0; i < num_cases; i++) {
        printf("\n--- Test case %d ---\n", i + 1);
        printf("Args: ");
        for (int j = 0; test_cases[i][j] != NULL; j++) {
            printf("%s ", test_cases[i][j]);
        }
        printf("\n");
        
        pid_t pid = fork();
        if (pid == 0) {
            /* Child process */
            execvp("gcc", test_cases[i]);
            /* If execvp fails */
            perror("execvp failed");
            exit(127);
        } else if (pid > 0) {
            /* Parent process */
            int status;
            waitpid(pid, &status, 0);
            
            if (WIFEXITED(status)) {
                statuses[i] = WEXITSTATUS(status);
                printf("Exit status: %d\n", statuses[i]);
            } else {
                statuses[i] = -1;
                printf("Process terminated abnormally\n");
            }
        } else {
            perror("fork failed");
            statuses[i] = -1;
        }
    }
    
    /* Analyze results */
    printf("\n=== Results Analysis ===\n");
    printf("Case 1 (with flags): %d\n", statuses[0]);
    printf("Case 2 (minimal): %d\n", statuses[1]);
    printf("Case 3 (invalid): %d (should be non-zero)\n", statuses[2]);
    printf("Case 4 (with flags again): %d\n", statuses[3]);
    printf("Case 5 (minimal again): %d\n", statuses[4]);
    
    /* Cleanup */
    unlink("output1.o");
    unlink("output2.o");
    unlink("output3.o");
    unlink("output4.o");
    unlink(input_file);
}

/* Method B: Direct library call using dlopen (if available) */
#ifdef WITH_LIBRARY_CALL
void test_via_library() {
    printf("\n=== Testing via direct library calls ===\n");
    
    /* Try to load GCC driver as a shared library */
    void *handle = dlopen("libgccdriver.so", RTLD_LAZY);
    if (!handle) {
        /* Try alternative names */
        handle = dlopen("/usr/lib/gcc/libgccdriver.so", RTLD_LAZY);
    }
    
    if (!handle) {
        printf("Could not load GCC driver library: %s\n", dlerror());
        printf("Skipping library-based test.\n");
        return;
    }
    
    /* Look for main or driver entry point */
    int (*driver_main)(int, char**) = dlsym(handle, "main");
    if (!driver_main) {
        driver_main = dlsym(handle, "gcc_driver_main");
    }
    
    if (!driver_main) {
        printf("Could not find driver entry point: %s\n", dlerror());
        dlclose(handle);
        return;
    }
    
    /* Test case 1: With various flags */
    char *args1[] = {
        "gcc", "-save-temps", "-dumpdir", "/tmp/lib_test1",
        "-dumpbase", "lib_test", "-c", "test_input.c",
        "-o", "lib_output1.o", "--sysroot=/usr",
        NULL
    };
    
    /* Test case 2: Minimal to trigger reset */
    char *args2[] = {
        "gcc", "-c", "test_input.c", "-o", "lib_output2.o",
        NULL
    };
    
    /* Test case 3: Invalid option to set greatest_status */
    char *args3[] = {
        "gcc", "-this-option-does-not-exist-12345",
        NULL
    };
    
    /* Test case 4: Successful compilation again */
    char *args4[] = {
        "gcc", "-c", "test_input.c", "-o", "lib_output3.o",
        "-save-temps", "-dumpdir", "/tmp/lib_test2",
        NULL
    };
    
    printf("Call 1: With flags\n");
    int result1 = driver_main(10, args1);
    printf("Result: %d\n", result1);
    
    printf("\nCall 2: Minimal (should trigger reset)\n");
    int result2 = driver_main(4, args2);
    printf("Result: %d\n", result2);
    
    printf("\nCall 3: Invalid option (should fail)\n");
    int result3 = driver_main(2, args3);
    printf("Result: %d\n", result3);
    
    printf("\nCall 4: With flags again\n");
    int result4 = driver_main(7, args4);
    printf("Result: %d\n", result4);
    
    dlclose(handle);
}
#endif

/* Alternative: Direct compilation with driver code */
#ifdef COMPILE_WITH_DRIVER
/* This would require linking with gcc driver object files */
#include "gcc.cc"  /* If we have access to the source */

void test_direct_call() {
    printf("\n=== Testing via direct function calls ===\n");
    
    /* Simulate the driver's global state reset by calling main multiple times */
    char *test_args[][10] = {
        {"test_gcc", "-save-temps", "-dumpdir", "/tmp/direct1", 
         "-dumpbase", "direct_test", "-c", "test_input.c", NULL},
        {"test_gcc", "-c", "test_input.c", "-o", "direct_out.o", NULL},
        {"test_gcc", "-invalid-flag-test", NULL},
        {"test_gcc", "-c", "test_input.c", NULL}
    };
    
    for (int i = 0; i < 4; i++) {
        printf("\nDirect call %d:\n", i + 1);
        /* This would call the actual driver::main() function */
        /* int result = main(sizeof(test_args[i])/sizeof(test_args[i][0]) - 1, test_args[i]); */
        /* printf("Result: %d\n", result); */
    }
}
#endif

/* Multi-stage compilation simulation */
void test_multi_stage() {
    printf("\n=== Testing multi-stage compilation ===\n");
    
    const char *input = "multi_stage.c";
    create_test_source(input);
    
    /* Simulate a full compilation pipeline with different flags per stage */
    char *stages[][15] = {
        /* Preprocessing stage */
        {"gcc", "-E", "-save-temps", "-dumpdir", "/tmp/stage1",
         "-dumpbase", "stage1", input, "-o", "stage1.i", NULL},
        
        /* Compilation stage */
        {"gcc", "-S", "-save-temps=obj", "-dumpdir", "/tmp/stage2",
         "-dumpbase", "stage2", "stage1.i", "-o", "stage2.s", NULL},
        
        /* Assembly stage (minimal, should trigger reset) */
        {"gcc", "-c", "stage2.s", "-o", "stage3.o", NULL},
        
        /* Linking stage with sysroot */
        {"gcc", "stage3.o", "-o", "final_program",
         "--sysroot=/usr/local", "-specs=/usr/share/gcc/specs/link.specs", NULL},
        
        /* Clean final compilation (should reset everything) */
        {"gcc", "-c", input, "-o", "final.o", NULL}
    };
    
    int num_stages = sizeof(stages) / sizeof(stages[0]);
    
    for (int i = 0; i < num_stages; i++) {
        printf("\nStage %d: ", i + 1);
        for (int j = 0; stages[i][j] != NULL; j++) {
            printf("%s ", stages[i][j]);
        }
        printf("\n");
        
        pid_t pid = fork();
        if (pid == 0) {
            execvp("gcc", stages[i]);
            perror("execvp failed");
            exit(127);
        } else if (pid > 0) {
            int status;
            waitpid(pid, &status, 0);
            if (WIFEXITED(status)) {
                printf("  Exit: %d\n", WEXITSTATUS(status));
            }
        }
    }
    
    /* Cleanup */
    unlink("stage1.i");
    unlink("stage2.s");
    unlink("stage3.o");
    unlink("final_program");
    unlink("final.o");
    unlink(input);
}

int main(int argc, char *argv[]) {
    printf("GCC Driver Reinitialization Test Program\n");
    printf("=========================================\n");
    
    /* Test 1: Process-based approach */
    test_via_processes();
    
    /* Test 2: Multi-stage compilation */
    test_multi_stage();
    
#ifdef WITH_LIBRARY_CALL
    /* Test 3: Library-based approach (if enabled) */
    test_via_library();
#endif
    
#ifdef COMPILE_WITH_DRIVER
    /* Test 4: Direct call approach (if enabled) */
    test_direct_call();
#endif
    
    printf("\n=== All tests completed ===\n");
    
    /* Final test: Create a wrapper that calls gcc multiple times in same process */
    /* This would require building gcc with special flags */
    if (argc > 1 && strcmp(argv[1], "--wrapper-test") == 0) {
        printf("\nRunning wrapper test...\n");
        
        /* This simulates what a build system might do */
        for (int i = 0; i < 3; i++) {
            char *wrapper_args[] = {
                "gcc", "-c", "wrapper_test.c",
                i == 0 ? "-save-temps" : 
                i == 1 ? "--sysroot=/opt/cross" : 
                "-dumpbase=wrapper",
                "-o", "wrapper_out.o",
                NULL
            };
            
            pid_t pid = fork();
            if (pid == 0) {
                execvp("gcc", wrapper_args);
                exit(1);
            } else {
                wait(NULL);
            }
        }
    }
    
    return 0;
}
