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

/* Define this if testing with direct library calls */
#define USE_DIRECT_LIBRARY_CALL 0

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
void test_via_processes() {
    printf("=== Testing via Process Invocation ===\n");
    
    /* Create test source file */
    if (!create_test_file("test_input.c")) {
        return;
    }
    
    /* Array of test invocations with different flags */
    char *test_cases[][10] = {
        /* First: Set various flags including sysroot and dump options */
        {"gcc", "-save-temps", "-dumpdir", "/tmp/test_dump1", 
         "-dumpbase", "test_dumpbase1", "-c", "test_input.c", 
         "-o", "test_output1.o", NULL},
        
        /* Second: Reset to defaults (no special flags) */
        {"gcc", "-c", "test_input.c", "-o", "test_output2.o", NULL},
        
        /* Third: Force failure with invalid option */
        {"gcc", "-invalid-option-that-does-not-exist", NULL},
        
        /* Fourth: Successful compilation again */
        {"gcc", "-c", "test_input.c", "-o", "test_output3.o", NULL},
        
        /* Fifth: Test with sysroot and machine spec */
        {"gcc", "--sysroot=/tmp/fake_sysroot", "-march=x86-64", 
         "-c", "test_input.c", "-o", "test_output4.o", NULL},
        
        /* Sixth: Reset again (no sysroot, default machine) */
        {"gcc", "-c", "test_input.c", "-o", "test_output5.o", NULL}
    };
    
    int num_cases = sizeof(test_cases) / sizeof(test_cases[0]);
    int status;
    
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
    for (int i = 1; i <= 5; i++) {
        char filename[32];
        snprintf(filename, sizeof(filename), "test_output%d.o", i);
        unlink(filename);
        snprintf(filename, sizeof(filename), "test_output%d.s", i);
        unlink(filename);
        snprintf(filename, sizeof(filename), "test_output%d.i", i);
        unlink(filename);
    }
}

#if USE_DIRECT_LIBRARY_CALL
/* Method B: Direct library call using dlopen/dlsym */
void test_via_library() {
    printf("\n=== Testing via Direct Library Calls ===\n");
    
    /* Try to load GCC driver as a shared library */
    void *handle = dlopen("libgccdriver.so", RTLD_LAZY | RTLD_LOCAL);
    if (!handle) {
        /* Try alternative names */
        handle = dlopen("./libgccdriver.so", RTLD_LAZY | RTLD_LOCAL);
    }
    
    if (!handle) {
        printf("Could not load libgccdriver.so: %s\n", dlerror());
        printf("Falling back to process-based testing\n");
        test_via_processes();
        return;
    }
    
    /* Find the main function */
    int (*gcc_main)(int, char**) = dlsym(handle, "main");
    if (!gcc_main) {
        /* Try alternative entry point */
        gcc_main = dlsym(handle, "driver::main");
    }
    
    if (!gcc_main) {
        printf("Could not find driver entry point: %s\n", dlerror());
        dlclose(handle);
        return;
    }
    
    /* Create test source file */
    if (!create_test_file("test_lib_input.c")) {
        dlclose(handle);
        return;
    }
    
    /* Test cases for direct library calls */
    char *test1[] = {"gcc", "-save-temps", "-dumpdir", "/tmp/lib_test", 
                     "-dumpbase", "lib_dump", "-c", "test_lib_input.c", 
                     "-o", "test_lib_output.o", NULL};
    char *test2[] = {"gcc", "-c", "test_lib_input.c", 
                     "-o", "test_lib_output2.o", NULL};
    char *test3[] = {"gcc", "-invalid-option", NULL};
    char *test4[] = {"gcc", "-c", "test_lib_input.c", 
                     "-o", "test_lib_output3.o", NULL};
    
    /* Execute test cases */
    printf("Test 1: With dump flags\n");
    int result1 = gcc_main(10, test1);
    printf("Result: %d\n", result1);
    
    printf("\nTest 2: Reset to defaults\n");
    int result2 = gcc_main(5, test2);
    printf("Result: %d\n", result2);
    
    printf("\nTest 3: Force failure\n");
    int result3 = gcc_main(2, test3);
    printf("Result: %d\n", result3);
    
    printf("\nTest 4: Successful after failure\n");
    int result4 = gcc_main(5, test4);
    printf("Result: %d\n", result4);
    
    /* Cleanup */
    unlink("test_lib_input.c");
    unlink("test_lib_output.o");
    unlink("test_lib_output2.o");
    unlink("test_lib_output3.o");
    
    dlclose(handle);
}
#endif

/* Alternative: Link directly with driver object files */
#ifdef DRIVER_TEST
/* This would be used when compiling with -DDRIVER_TEST and linking
   directly with gcc driver object files */
extern int driver_main(int argc, char **argv);

void test_direct_link() {
    printf("\n=== Testing via Direct Link ===\n");
    
    if (!create_test_file("test_direct_input.c")) {
        return;
    }
    
    /* Multiple invocations to trigger state reset */
    char *invocations[][8] = {
        {"gcc", "-save-temps", "-dumpdir", "/tmp/direct", 
         "-c", "test_direct_input.c", "-o", "test_direct1.o"},
        {"gcc", "-c", "test_direct_input.c", "-o", "test_direct2.o"},
        {"gcc", "-invalid-flag"},
        {"gcc", "-c", "test_direct_input.c", "-o", "test_direct3.o"}
    };
    
    for (int i = 0; i < 4; i++) {
        printf("Invocation %d: ", i + 1);
        int argc = (i == 2) ? 2 : 8;
        int result = driver_main(argc, invocations[i]);
        printf("Result: %d\n", result);
    }
    
    unlink("test_direct_input.c");
}
#endif

/* Main test orchestrator */
int main(int argc, char **argv) {
    printf("GCC Driver Reinitialization Test Program\n");
    printf("=========================================\n\n");
    
    /* Check if we should use a specific method */
    if (argc > 1 && strcmp(argv[1], "--library") == 0) {
#if USE_DIRECT_LIBRARY_CALL
        test_via_library();
#else
        printf("Direct library calls not enabled in this build.\n");
        printf("Compile with -DUSE_DIRECT_LIBRARY_CALL=1 and -ldl\n");
#endif
    } 
#ifdef DRIVER_TEST
    else if (argc > 1 && strcmp(argv[1], "--direct") == 0) {
        test_direct_link();
    }
#endif
    else {
        /* Default: use process-based testing */
        test_via_processes();
    }
    
    printf("\n=== Test Complete ===\n");
    
    /* Additional test: Simulate multi-stage compilation pipeline */
    printf("\n=== Testing Multi-Stage Compilation ===\n");
    
    if (create_test_file("pipeline.c")) {
        /* Simulate: preprocess -> compile -> assemble -> link */
        char *stages[][10] = {
            /* Preprocess with dump options */
            {"gcc", "-E", "-dumpdir", "/tmp/pipe", "-dumpbase", "stage1",
             "pipeline.c", "-o", "pipeline.i", NULL},
            
            /* Compile to assembly (reset dumpdir) */
            {"gcc", "-S", "pipeline.c", "-o", "pipeline.s", NULL},
            
            /* Assemble with different output */
            {"gcc", "-c", "pipeline.s", "-o", "pipeline.o", NULL},
            
            /* Link final executable */
            {"gcc", "pipeline.o", "-o", "pipeline.exe", NULL}
        };
        
        for (int i = 0; i < 4; i++) {
            pid_t pid = fork();
            if (pid == 0) {
                execvp("gcc", stages[i]);
                perror("execvp failed");
                exit(127);
            } else if (pid > 0) {
                int status;
                waitpid(pid, &status, 0);
                printf("Stage %d completed with status: %d\n", 
                       i + 1, WEXITSTATUS(status));
            }
        }
        
        /* Cleanup pipeline files */
        unlink("pipeline.c");
        unlink("pipeline.i");
        unlink("pipeline.s");
        unlink("pipeline.o");
        unlink("pipeline.exe");
    }
    
    return 0;
}
