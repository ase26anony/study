/* test_gcc_driver_reset.c
 * A test program to trigger driver reinitialization logic in gcc.cc
 * Specifically targets lines 11228-11250 for coverage
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

/* Define this to use direct library loading instead of fork/exec */
/* #define USE_DIRECT_LIBRARY_CALL */

/* Simple test C source file content */
const char *test_source = 
"#include <stdio.h>\n"
"int main() {\n"
"    printf(\"Hello from test program\\n\");\n"
"    return 0;\n"
"}\n";

/* Create a test source file */
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
    printf("=== Testing via fork/exec (Process-based) ===\n");
    
    /* Create test source file */
    if (!create_test_file("test_input.c")) {
        return;
    }
    
    /* Array of test invocations with different flags to trigger resets */
    char *test_invocations[][10] = {
        /* First: Set various flags including sysroot and dump options */
        {"gcc", "-save-temps", "-dumpdir", "/tmp/test_dump1", 
         "-dumpbase", "test_dumpbase1", "-c", "test_input.c", 
         "-o", "test_output1.o", NULL},
        
        /* Second: Minimal invocation to trigger reset to defaults */
        {"gcc", "-c", "test_input.c", "-o", "test_output2.o", NULL},
        
        /* Third: Force failure with invalid option */
        {"gcc", "-invalid-option-that-does-not-exist", NULL},
        
        /* Fourth: Successful compilation again */
        {"gcc", "-c", "test_input.c", "-o", "test_output3.o", NULL},
        
        /* Fifth: Another with different dump options */
        {"gcc", "-save-temps=obj", "-dumpdir", "/tmp/test_dump2",
         "-dumpbase", "test_dumpbase2", "-c", "test_input.c",
         "-o", "test_output4.o", NULL},
        
        /* Sixth: Back to minimal to trigger another reset */
        {"gcc", "-c", "test_input.c", "-o", "test_output5.o", NULL}
    };
    
    int num_tests = sizeof(test_invocations) / sizeof(test_invocations[0]);
    int statuses[num_tests];
    
    for (int i = 0; i < num_tests; i++) {
        printf("\n--- Test %d: ", i + 1);
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
            } else if (WIFSIGNALED(status)) {
                printf("Terminated by signal: %d\n", WTERMSIG(status));
            }
        } else {
            perror("fork failed");
            statuses[i] = -1;
        }
        
        /* Small delay to ensure clean separation between invocations */
        usleep(10000);
    }
    
    printf("\n=== Summary of exit statuses ===\n");
    for (int i = 0; i < num_tests; i++) {
        printf("Test %d: %d\n", i + 1, 
               WIFEXITED(statuses[i]) ? WEXITSTATUS(statuses[i]) : -1);
    }
    
    /* Cleanup */
    unlink("test_input.c");
    for (int i = 1; i <= 5; i++) {
        char filename[32];
        snprintf(filename, sizeof(filename), "test_output%d.o", i);
        unlink(filename);
    }
}

#ifdef USE_DIRECT_LIBRARY_CALL
/* Method B: Direct library loading (if gcc driver is built as shared library) */
void test_via_direct_library() {
    printf("=== Testing via direct library loading ===\n");
    
    /* Try to load gcc driver as shared library */
    void *handle = dlopen("libgccdriver.so", RTLD_LAZY | RTLD_LOCAL);
    if (!handle) {
        /* Try alternative names */
        handle = dlopen("libgcc.so", RTLD_LAZY | RTLD_LOCAL);
    }
    
    if (!handle) {
        fprintf(stderr, "Could not load gcc driver library: %s\n", dlerror());
        fprintf(stderr, "Falling back to process-based testing\n");
        test_via_processes();
        return;
    }
    
    /* Look for main function */
    int (*driver_main)(int, char**) = dlsym(handle, "main");
    if (!driver_main) {
        /* Try alternative entry point */
        driver_main = dlsym(handle, "driver::main");
    }
    
    if (!driver_main) {
        fprintf(stderr, "Could not find driver entry point: %s\n", dlerror());
        dlclose(handle);
        return;
    }
    
    /* Create test source file */
    if (!create_test_file("test_input_lib.c")) {
        dlclose(handle);
        return;
    }
    
    /* Test argument vectors for multiple invocations */
    char *argv1[] = {
        "gcc", "-save-temps", "-dumpdir", "/tmp/lib_test1",
        "-dumpbase", "lib_dumpbase", "-c", "test_input_lib.c",
        "-o", "lib_output1.o", NULL
    };
    
    char *argv2[] = {
        "gcc", "-c", "test_input_lib.c", "-o", "lib_output2.o", NULL
    };
    
    char *argv3[] = {
        "gcc", "-invalid-option-force-failure", NULL
    };
    
    char *argv4[] = {
        "gcc", "-c", "test_input_lib.c", "-o", "lib_output3.o", NULL
    };
    
    printf("\nInvocation 1 (with dump options):\n");
    int result1 = driver_main(10, argv1);
    printf("Result: %d\n", result1);
    
    printf("\nInvocation 2 (minimal, should trigger reset):\n");
    int result2 = driver_main(4, argv2);
    printf("Result: %d\n", result2);
    
    printf("\nInvocation 3 (force failure):\n");
    int result3 = driver_main(2, argv3);
    printf("Result: %d\n", result3);
    
    printf("\nInvocation 4 (successful again):\n");
    int result4 = driver_main(4, argv4);
    printf("Result: %d\n", result4);
    
    /* Cleanup */
    dlclose(handle);
    unlink("test_input_lib.c");
    unlink("lib_output1.o");
    unlink("lib_output2.o");
    unlink("lib_output3.o");
}
#endif

/* Alternative: Link directly with gcc driver object files */
#ifdef LINK_WITH_DRIVER_OBJ
/* This would require compiling with gcc driver object files */
extern int driver_main(int argc, char **argv);

void test_via_direct_link() {
    printf("=== Testing via direct object linking ===\n");
    
    if (!create_test_file("test_input_direct.c")) {
        return;
    }
    
    /* Multiple invocations to trigger state resets */
    char *argv_sets[][8] = {
        {"gcc", "-save-temps", "-dumpdir", "/tmp/direct1", 
         "-c", "test_input_direct.c", "-o", "direct1.o"},
        {"gcc", "-c", "test_input_direct.c", "-o", "direct2.o"},
        {"gcc", "-nonexistent-flag", NULL},
        {"gcc", "-c", "test_input_direct.c", "-o", "direct3.o"}
    };
    
    int arg_counts[] = {8, 4, 2, 4};
    
    for (int i = 0; i < 4; i++) {
        printf("\nDirect call %d:\n", i + 1);
        int result = driver_main(arg_counts[i], argv_sets[i]);
        printf("Return value: %d\n", result);
    }
    
    /* Cleanup */
    unlink("test_input_direct.c");
    unlink("direct1.o");
    unlink("direct2.o");
    unlink("direct3.o");
}
#endif

int main(int argc, char **argv) {
    printf("GCC Driver Reinitialization Test Program\n");
    printf("Targeting uncovered lines 11228-11250 in gcc.cc\n\n");
    
    /* Check if we should use a specific method */
    if (argc > 1 && strcmp(argv[1], "--direct") == 0) {
#ifdef USE_DIRECT_LIBRARY_CALL
        test_via_direct_library();
#else
        printf("Direct library call not enabled in this build\n");
        test_via_processes();
#endif
    } else if (argc > 1 && strcmp(argv[1], "--link") == 0) {
#ifdef LINK_WITH_DRIVER_OBJ
        test_via_direct_link();
#else
        printf("Direct linking not enabled in this build\n");
        test_via_processes();
#endif
    } else {
        /* Default: use process-based testing */
        test_via_processes();
    }
    
    printf("\n=== Test complete ===\n");
    
    /* Additional test: Create a pipeline simulation */
    printf("\n=== Testing compilation pipeline simulation ===\n");
    
    /* Create a more complex test to trigger all reset conditions */
    if (create_test_file("pipeline_test.c")) {
        /* Simulate multi-stage compilation with different flags */
        char *pipeline_cmds[][15] = {
            /* Stage 1: Preprocess with sysroot and dump options */
            {"gcc", "-E", "-dD", "--sysroot=/tmp/fake_sysroot",
             "-dumpdir", "/tmp/pipe1", "-dumpbase", "stage1",
             "pipeline_test.c", "-o", "pipeline.i", NULL},
            
            /* Stage 2: Compile with different options */
            {"gcc", "-S", "-save-temps", "-dumpdir", "/tmp/pipe2",
             "pipeline.i", "-o", "pipeline.s", NULL},
            
            /* Stage 3: Assemble with minimal options (trigger reset) */
            {"gcc", "-c", "pipeline.s", "-o", "pipeline.o", NULL},
            
            /* Stage 4: Link with yet another set of options */
            {"gcc", "-v", "-dumpbase", "final", "pipeline.o",
             "-o", "pipeline_exe", NULL},
            
            /* Stage 5: Clean minimal invocation */
            {"gcc", "--version", NULL}
        };
        
        for (int i = 0; i < 5; i++) {
            printf("\nPipeline stage %d: ", i + 1);
            for (int j = 0; pipeline_cmds[i][j] != NULL; j++) {
                printf("%s ", pipeline_cmds[i][j]);
            }
            printf("\n");
            
            if (fork() == 0) {
                execvp("gcc", pipeline_cmds[i]);
                perror("execvp");
                exit(1);
            }
            wait(NULL);
            usleep(5000);
        }
        
        /* Cleanup pipeline files */
        unlink("pipeline_test.c");
        unlink("pipeline.i");
        unlink("pipeline.s");
        unlink("pipeline.o");
        unlink("pipeline_exe");
    }
    
    return 0;
}
