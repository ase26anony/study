/* test_driver_reinit.c - Test program to trigger gcc.cc driver reinitialization logic */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <dlfcn.h>
#include <errno.h>

/* Define this if we want to test direct library loading */
#define USE_DIRECT_LIBRARY_CALL 0

/* Simple test C source file content */
const char *test_source = 
"#include <stdio.h>\n"
"int main() {\n"
"    printf(\"Hello from test program\\n\");\n"
"    return 0;\n"
"}\n";

/* Create a temporary source file */
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
void test_via_process() {
    printf("=== Testing via Process Fork/Exec ===\n");
    
    /* Create test source file */
    if (!create_test_file("test_input.c")) {
        return;
    }
    
    /* Array of test invocations with different flags */
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
        
        /* Fifth: Another with different sysroot and machine spec */
        {"gcc", "--sysroot=/tmp/fake_sysroot", "-march=x86-64", 
         "-mtune=generic", "-c", "test_input.c", "-o", "test_output4.o", NULL},
        
        /* Sixth: Back to minimal to trigger reset again */
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
            /* If exec fails */
            perror("execvp failed");
            exit(127);
        } else if (pid > 0) {
            /* Parent process */
            waitpid(pid, &statuses[i], 0);
            printf("Exit status: %d\n", WEXITSTATUS(statuses[i]));
        } else {
            perror("fork failed");
            statuses[i] = -1;
        }
    }
    
    /* Cleanup temporary files */
    unlink("test_input.c");
    for (int i = 1; i <= 5; i++) {
        char filename[32];
        snprintf(filename, sizeof(filename), "test_output%d.o", i);
        unlink(filename);
    }
    
    printf("\n=== Process-based test complete ===\n");
}

#if USE_DIRECT_LIBRARY_CALL
/* Method B: Direct library call using dlopen/dlsym */
void test_via_library() {
    printf("=== Testing via Direct Library Call ===\n");
    
    /* Try to load gcc driver as shared library */
    void *handle = dlopen("libgccdriver.so", RTLD_LAZY | RTLD_GLOBAL);
    if (!handle) {
        /* Try alternative names */
        handle = dlopen("/usr/lib/gcc/x86_64-linux-gnu/11/libgccdriver.so", RTLD_LAZY);
    }
    
    if (!handle) {
        printf("Could not load gcc driver library: %s\n", dlerror());
        printf("Falling back to process-based testing\n");
        test_via_process();
        return;
    }
    
    /* Find the main function */
    int (*driver_main)(int, char**) = dlsym(handle, "main");
    if (!driver_main) {
        /* Try alternative entry point */
        driver_main = dlsym(handle, "gcc_main");
    }
    
    if (!driver_main) {
        printf("Could not find driver entry point: %s\n", dlerror());
        dlclose(handle);
        return;
    }
    
    /* Create test source file */
    if (!create_test_file("test_input.c")) {
        dlclose(handle);
        return;
    }
    
    /* Test invocations similar to process-based but called directly */
    char *argv1[] = {"gcc", "-save-temps", "-dumpdir", "/tmp/lib_test1",
                     "-dumpbase", "lib_dumpbase", "-c", "test_input.c",
                     "-o", "lib_output.o", NULL};
    char *argv2[] = {"gcc", "-c", "test_input.c", "-o", "lib_output2.o", NULL};
    char *argv3[] = {"gcc", "-invalid-option", NULL};
    char *argv4[] = {"gcc", "-c", "test_input.c", "-o", "lib_output3.o", NULL};
    
    printf("\nFirst call with special flags:\n");
    int result1 = driver_main(10, argv1);
    printf("Result: %d\n", result1);
    
    printf("\nSecond call (minimal, should trigger reset):\n");
    int result2 = driver_main(4, argv2);
    printf("Result: %d\n", result2);
    
    printf("\nThird call (should fail):\n");
    int result3 = driver_main(2, argv3);
    printf("Result: %d\n", result3);
    
    printf("\nFourth call (should succeed):\n");
    int result4 = driver_main(4, argv4);
    printf("Result: %d\n", result4);
    
    /* Cleanup */
    unlink("test_input.c");
    unlink("lib_output.o");
    unlink("lib_output2.o");
    unlink("lib_output3.o");
    dlclose(handle);
    
    printf("\n=== Library-based test complete ===\n");
}
#endif

/* Alternative: Compile with gcc driver object files directly */
#ifdef DRIVER_TEST
/* This would require linking with gcc driver object files */
extern int driver_main(int argc, char **argv);

void test_via_direct_link() {
    printf("=== Testing via Direct Object Link ===\n");
    
    if (!create_test_file("test_input.c")) {
        return;
    }
    
    /* Multiple invocations to trigger reinitialization */
    char *argv_sets[][8] = {
        {"gcc", "-save-temps=obj", "-dumpdir", "./dumps", 
         "-dumpbase", "test", "-c", "test_input.c"},
        {"gcc", "-c", "test_input.c", "-o", "simple.o"},
        {"gcc", "-B", "/nonexistent/path", "-c", "test_input.c"},
        {"gcc", "-c", "test_input.c", "-o", "final.o"}
    };
    
    int argc_sets[] = {8, 5, 4, 5};
    
    for (int i = 0; i < 4; i++) {
        printf("\nInvocation %d:\n", i + 1);
        int result = driver_main(argc_sets[i], argv_sets[i]);
        printf("Driver returned: %d\n", result);
        
        /* Small delay to ensure cleanup */
        usleep(10000);
    }
    
    unlink("test_input.c");
}
#endif

int main(int argc, char **argv) {
    printf("GCC Driver Reinitialization Test Program\n");
    printf("=========================================\n");
    
    /* Check if we should use a specific method */
    if (argc > 1 && strcmp(argv[1], "--library") == 0) {
#if USE_DIRECT_LIBRARY_CALL
        test_via_library();
#else
        printf("Library call method not enabled in this build\n");
        test_via_process();
#endif
    }
#ifdef DRIVER_TEST
    else if (argc > 1 && strcmp(argv[1], "--direct") == 0) {
        test_via_direct_link();
    }
#endif
    else {
        /* Default: process-based testing */
        test_via_process();
    }
    
    /* Additional test: Simulate multi-stage compilation pipeline */
    printf("\n=== Testing Multi-Stage Compilation Pipeline ===\n");
    
    if (create_test_file("pipeline.c")) {
        /* Simulate separate stages with different dump options */
        char *pipeline_stages[][8] = {
            {"gcc", "-E", "-dumpdir", "/tmp/stage1", 
             "-dumpbase", "preprocess", "pipeline.c", "-o", "pipeline.i"},
            {"gcc", "-S", "-dumpdir", "/tmp/stage2", 
             "-dumpbase", "compile", "pipeline.i", "-o", "pipeline.s"},
            {"gcc", "-c", "-dumpdir", "/tmp/stage3", 
             "-dumpbase", "assemble", "pipeline.s", "-o", "pipeline.o"},
            {"gcc", "-dumpdir", "/tmp/stage4", 
             "-dumpbase", "link", "pipeline.o", "-o", "pipeline"}
        };
        
        for (int i = 0; i < 4; i++) {
            printf("\nStage %d: ", i + 1);
            pid_t pid = fork();
            if (pid == 0) {
                execvp("gcc", pipeline_stages[i]);
                exit(127);
            } else if (pid > 0) {
                int status;
                waitpid(pid, &status, 0);
                printf("Exit: %d\n", WEXITSTATUS(status));
            }
        }
        
        /* Cleanup pipeline files */
        unlink("pipeline.c");
        unlink("pipeline.i");
        unlink("pipeline.s");
        unlink("pipeline.o");
        unlink("pipeline");
    }
    
    printf("\n=== All tests completed ===\n");
    return 0;
}
