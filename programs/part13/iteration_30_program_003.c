/* test_driver_reinit.c - Test program to trigger gcc.cc lines 11228-11250 */
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

/* Create test input file */
void create_test_file(const char *filename) {
    FILE *f = fopen(filename, "w");
    if (f) {
        fputs(test_source, f);
        fclose(f);
    }
}

/* Method A: Process-based invocation using fork/exec */
void test_via_processes() {
    printf("=== Testing via fork/exec (Process-based) ===\n");
    
    const char *input_file = "test_input.c";
    create_test_file(input_file);
    
    /* Array of test invocations with different flags */
    char *test_cases[][20] = {
        /* First: Set various flags including sysroot and dump options */
        {"gcc", "-save-temps", "-dumpdir", "/tmp/test_dump1", 
         "-dumpbase", "test_dumpbase", "-c", input_file, 
         "-o", "output1.o", "--sysroot=/usr/alt", 
         "-march=x86-64", "-mtune=generic", NULL},
        
        /* Second: Minimal flags to trigger reset to defaults */
        {"gcc", "-c", input_file, "-o", "output2.o", NULL},
        
        /* Third: Force failure with invalid option */
        {"gcc", "-invalid-option-that-does-not-exist", input_file, NULL},
        
        /* Fourth: Successful compilation again */
        {"gcc", "-c", input_file, "-o", "output3.o", 
         "-specs=/usr/lib/gcc/x86_64-linux-gnu/*/specs", NULL},
        
        /* Fifth: Another with different dump options */
        {"gcc", "-save-temps=obj", "-dumpdir", "/tmp/test_dump2/",
         "-dumpbase", "another_base", "-c", input_file,
         "-o", "output4.o", NULL},
         
        /* Sixth: Back to minimal to trigger reset again */
        {"gcc", "-c", input_file, "-o", "output5.o", NULL}
    };
    
    int num_cases = sizeof(test_cases) / sizeof(test_cases[0]);
    int statuses[num_cases];
    
    for (int i = 0; i < num_cases; i++) {
        pid_t pid = fork();
        
        if (pid == 0) {
            /* Child process */
            printf("Test %d: Executing:", i+1);
            for (int j = 0; test_cases[i][j] != NULL; j++) {
                printf(" %s", test_cases[i][j]);
            }
            printf("\n");
            
            execvp("gcc", test_cases[i]);
            
            /* If execvp fails */
            perror("execvp failed");
            exit(EXIT_FAILURE);
        } else if (pid > 0) {
            /* Parent process */
            waitpid(pid, &statuses[i], 0);
            printf("Test %d exited with status: %d\n\n", 
                   i+1, WEXITSTATUS(statuses[i]));
        } else {
            perror("fork failed");
        }
        
        /* Small delay to ensure clean separation */
        usleep(10000);
    }
    
    /* Cleanup */
    unlink(input_file);
    for (int i = 1; i <= 5; i++) {
        char fname[20];
        snprintf(fname, sizeof(fname), "output%d.o", i);
        unlink(fname);
    }
    
    printf("Process-based test completed.\n");
}

/* Method B: Direct library call using dlopen/dlsym */
#ifdef DRIVER_TEST
void test_via_direct_call() {
    printf("\n=== Testing via direct library call ===\n");
    
    /* Try to load GCC driver as a shared library */
    void *handle = dlopen("libgccdriver.so", RTLD_LAZY | RTLD_LOCAL);
    if (!handle) {
        /* Try alternative names */
        handle = dlopen("/usr/lib/gcc/x86_64-linux-gnu/*/libgccdriver.so", 
                       RTLD_LAZY | RTLD_LOCAL);
    }
    
    if (!handle) {
        printf("Could not load GCC driver library: %s\n", dlerror());
        printf("Skipping direct library test.\n");
        return;
    }
    
    /* Look for driver entry point - this varies by GCC version */
    typedef int (*driver_main_t)(int, char**);
    driver_main_t driver_main = NULL;
    
    /* Try different symbol names */
    const char *symbols[] = {
        "main",
        "driver::main",
        "gcc_driver_main",
        "_Z6driver4main",
        NULL
    };
    
    for (int i = 0; symbols[i] != NULL; i++) {
        driver_main = (driver_main_t)dlsym(handle, symbols[i]);
        if (driver_main) {
            printf("Found driver entry point: %s\n", symbols[i]);
            break;
        }
    }
    
    if (!driver_main) {
        printf("Could not find driver entry point: %s\n", dlerror());
        dlclose(handle);
        return;
    }
    
    /* Create test invocations */
    char *argv1[] = {
        "gcc", "-save-temps", "-dumpdir", "/tmp/direct_test",
        "-dumpbase", "direct_base", "-c", "test_input.c",
        "-o", "direct_output.o", "--sysroot=/usr",
        NULL
    };
    
    char *argv2[] = {
        "gcc", "-c", "test_input.c", "-o", "direct_output2.o",
        NULL
    };
    
    char *argv3[] = {
        "gcc", "-invalid-option-force-failure",
        NULL
    };
    
    char *argv4[] = {
        "gcc", "-c", "test_input.c", "-o", "direct_output3.o",
        NULL
    };
    
    /* Execute driver multiple times in same process */
    printf("Calling driver with full flags...\n");
    int result1 = driver_main(10, argv1);
    printf("Result 1: %d\n", result1);
    
    printf("Calling driver with minimal flags (should trigger reset)...\n");
    int result2 = driver_main(5, argv2);
    printf("Result 2: %d\n", result2);
    
    printf("Calling driver with invalid option...\n");
    int result3 = driver_main(2, argv3);
    printf("Result 3: %d\n", result3);
    
    printf("Calling driver again (testing greatest_status reset)...\n");
    int result4 = driver_main(5, argv4);
    printf("Result 4: %d\n", result4);
    
    dlclose(handle);
    printf("Direct library test completed.\n");
}
#endif

/* Method C: Simulate multi-stage compilation pipeline */
void test_multi_stage_pipeline() {
    printf("\n=== Testing multi-stage compilation pipeline ===\n");
    
    const char *input_file = "pipeline_input.c";
    create_test_file(input_file);
    
    /* Simulate: preprocess -> compile -> assemble -> link */
    char *stages[][15] = {
        /* Stage 1: Preprocess with dump options */
        {"gcc", "-E", "-save-temps", "-dumpdir", "/tmp/pipeline",
         "-dumpbase", "stage1", input_file, "-o", "stage1.i", NULL},
        
        /* Stage 2: Compile to assembly */
        {"gcc", "-S", "-dumpbase", "stage2", "stage1.i", 
         "-o", "stage2.s", NULL},
        
        /* Stage 3: Assemble */
        {"gcc", "-c", "stage2.s", "-o", "stage3.o", NULL},
        
        /* Stage 4: Link with different options */
        {"gcc", "stage3.o", "-o", "pipeline_output",
         "-Wl,--verbose", NULL},
         
        /* Stage 5: Clean minimal compilation */
        {"gcc", "-c", input_file, "-o", "final.o", NULL}
    };
    
    int num_stages = sizeof(stages) / sizeof(stages[0]);
    
    for (int i = 0; i < num_stages; i++) {
        pid_t pid = fork();
        
        if (pid == 0) {
            printf("Pipeline stage %d: ", i+1);
            for (int j = 0; stages[i][j] != NULL; j++) {
                printf("%s ", stages[i][j]);
            }
            printf("\n");
            
            execvp("gcc", stages[i]);
            perror("execvp failed");
            exit(EXIT_FAILURE);
        } else if (pid > 0) {
            int status;
            waitpid(pid, &status, 0);
            printf("Stage %d completed with status: %d\n", 
                   i+1, WEXITSTATUS(status));
        }
        
        usleep(5000); /* Small delay between stages */
    }
    
    /* Cleanup pipeline files */
    unlink(input_file);
    unlink("stage1.i");
    unlink("stage2.s");
    unlink("stage3.o");
    unlink("pipeline_output");
    unlink("final.o");
    
    printf("Pipeline test completed.\n");
}

int main(int argc, char *argv[]) {
    printf("GCC Driver Reinitialization Test Program\n");
    printf("Targeting uncovered lines 11228-11250 in gcc.cc\n\n");
    
    /* Test using multiple processes (most reliable) */
    test_via_processes();
    
    /* Test multi-stage pipeline */
    test_multi_stage_pipeline();
    
    /* Test direct library call if enabled */
    #ifdef DRIVER_TEST
    test_via_direct_call();
    #endif
    
    printf("\n=== All tests completed ===\n");
    
    /* Verify that we can still compile normally after all tests */
    printf("\nFinal verification compile:\n");
    create_test_file("verify.c");
    
    pid_t pid = fork();
    if (pid == 0) {
        char *args[] = {"gcc", "-c", "verify.c", "-o", "verify.o", NULL};
        execvp("gcc", args);
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
        printf("Verification compile status: %d\n", WEXITSTATUS(status));
        unlink("verify.c");
        unlink("verify.o");
    }
    
    return 0;
}
