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
        return -1;
    }
    fputs(test_source, f);
    fclose(f);
    return 0;
}

/* Method A: Process-based invocation using fork/exec */
void test_process_based() {
    printf("=== Testing via Process-Based Invocation ===\n");
    
    char *gcc_path = "gcc";
    char *input_file = "test_input.c";
    
    /* Create test source file */
    if (create_test_file(input_file) != 0) {
        return;
    }
    
    /* Array of test invocations with different flags */
    char *test_invocations[][20] = {
        /* First: Set various flags including sysroot and dump options */
        {gcc_path, "-save-temps", "-dumpdir", "/tmp/test_dump1", 
         "-dumpbase", "test_dumpbase1", "-c", input_file, 
         "-o", "output1.o", "--sysroot=/usr/alt", 
         "-march=x86-64", NULL},
        
        /* Second: Minimal invocation to trigger reset to defaults */
        {gcc_path, "-c", input_file, "-o", "output2.o", NULL},
        
        /* Third: Invalid option to cause failure */
        {gcc_path, "-invalid-option-that-does-not-exist", input_file, NULL},
        
        /* Fourth: Successful compilation after failure */
        {gcc_path, "-c", input_file, "-o", "output3.o", NULL},
        
        /* Fifth: Another with different dump options */
        {gcc_path, "-save-temps=obj", "-dumpdir", "/tmp/test_dump2",
         "-dumpbase", "test_dumpbase2", "-fdump-tree-original",
         "-c", input_file, "-o", "output4.o", NULL},
        
        /* Sixth: Back to minimal to trigger reset again */
        {gcc_path, "-c", input_file, "-o", "output5.o", NULL}
    };
    
    int num_tests = sizeof(test_invocations) / sizeof(test_invocations[0]);
    int statuses[num_tests];
    
    for (int i = 0; i < num_tests; i++) {
        printf("\nTest %d: ", i + 1);
        for (int j = 0; test_invocations[i][j] != NULL; j++) {
            printf("%s ", test_invocations[i][j]);
        }
        printf("\n");
        
        pid_t pid = fork();
        if (pid == 0) {
            /* Child process */
            execvp(gcc_path, test_invocations[i]);
            /* If exec fails */
            perror("execvp failed");
            exit(127);
        } else if (pid > 0) {
            /* Parent process */
            waitpid(pid, &statuses[i], 0);
            if (WIFEXITED(statuses[i])) {
                printf("Exit status: %d\n", WEXITSTATUS(statuses[i]));
            } else {
                printf("Process terminated abnormally\n");
            }
        } else {
            perror("fork failed");
        }
        
        /* Small delay to ensure cleanup */
        usleep(10000);
    }
    
    /* Cleanup temporary files */
    unlink(input_file);
    for (int i = 1; i <= 5; i++) {
        char filename[20];
        snprintf(filename, sizeof(filename), "output%d.o", i);
        unlink(filename);
    }
    
    printf("\n=== Process-based test complete ===\n");
}

/* Method B: Direct library call using dlopen (if available) */
void test_library_based() {
    printf("\n=== Testing via Library-Based Invocation ===\n");
    
#ifdef DRIVER_TEST
    /* Try to load GCC driver as a shared library */
    void *handle = dlopen("libgccdriver.so", RTLD_LAZY | RTLD_LOCAL);
    if (!handle) {
        /* Try alternative names */
        handle = dlopen("./libgccdriver.so", RTLD_LAZY | RTLD_LOCAL);
    }
    
    if (!handle) {
        printf("Could not load libgccdriver.so: %s\n", dlerror());
        printf("Skipping library-based test (driver may not be built as shared library)\n");
        return;
    }
    
    /* Look for driver entry point - this is highly implementation specific */
    int (*driver_main)(int, char**) = dlsym(handle, "main");
    if (!driver_main) {
        driver_main = dlsym(handle, "driver::main");
    }
    
    if (!driver_main) {
        printf("Could not find driver entry point: %s\n", dlerror());
        dlclose(handle);
        return;
    }
    
    /* Create test source file */
    char *input_file = "test_input_lib.c";
    if (create_test_file(input_file) != 0) {
        dlclose(handle);
        return;
    }
    
    /* Test invocations with different argument vectors */
    char *argv1[] = {"gcc", "-save-temps", "-dumpdir", "/tmp/libtest1",
                     "-dumpbase", "libbase1", "-c", input_file,
                     "-o", "libout1.o", "--sysroot=/opt/special",
                     "-march=native", NULL};
    
    char *argv2[] = {"gcc", "-c", input_file, "-o", "libout2.o", NULL};
    
    char *argv3[] = {"gcc", "-nonexistent-flag", input_file, NULL};
    
    char *argv4[] = {"gcc", "-c", input_file, "-o", "libout3.o", NULL};
    
    printf("Invocation 1 (with special flags):\n");
    int result1 = driver_main(12, argv1);
    printf("Result: %d\n", result1);
    
    printf("\nInvocation 2 (minimal, should trigger reset):\n");
    int result2 = driver_main(5, argv2);
    printf("Result: %d\n", result2);
    
    printf("\nInvocation 3 (should fail):\n");
    int result3 = driver_main(3, argv3);
    printf("Result: %d\n", result3);
    
    printf("\nInvocation 4 (should succeed):\n");
    int result4 = driver_main(5, argv4);
    printf("Result: %d\n", result4);
    
    /* Cleanup */
    unlink(input_file);
    dlclose(handle);
    
    printf("\n=== Library-based test complete ===\n");
#else
    printf("DRIVER_TEST not defined, skipping library-based test\n");
#endif
}

/* Method C: Direct function call if linking with driver object files */
#ifdef LINK_WITH_DRIVER
/* These would be declared in gcc.h or similar */
extern int driver_main(int argc, char **argv);
extern void driver_initialize(void);
extern void driver_finalize(void);

void test_direct_call() {
    printf("\n=== Testing via Direct Function Call ===\n");
    
    char *input_file = "test_input_direct.c";
    if (create_test_file(input_file) != 0) {
        return;
    }
    
    /* Multiple invocations with different states */
    for (int i = 0; i < 3; i++) {
        printf("\n--- Iteration %d ---\n", i + 1);
        
        /* Vary flags each iteration */
        char *argv[10];
        int argc;
        
        switch (i) {
            case 0:
                argv[0] = "gcc";
                argv[1] = "-save-temps";
                argv[2] = "-dumpdir";
                argv[3] = "/tmp/iter1";
                argv[4] = "-c";
                argv[5] = input_file;
                argv[6] = "-o";
                argv[7] = "iter1.o";
                argv[8] = "--sysroot=/custom/root";
                argv[9] = NULL;
                argc = 9;
                break;
                
            case 1:
                argv[0] = "gcc";
                argv[1] = "-c";
                argv[2] = input_file;
                argv[3] = "-o";
                argv[4] = "iter2.o";
                argv[5] = NULL;
                argc = 5;
                break;
                
            case 2:
                argv[0] = "gcc";
                argv[1] = "-invalid-flag-for-failure";
                argv[2] = input_file;
                argv[3] = NULL;
                argc = 3;
                break;
        }
        
        int result = driver_main(argc, argv);
        printf("Driver returned: %d\n", result);
        
        /* Force cleanup between calls if possible */
        if (i < 2) {
            driver_finalize();
            driver_initialize();
        }
    }
    
    unlink(input_file);
    printf("\n=== Direct call test complete ===\n");
}
#endif

int main(int argc, char **argv) {
    printf("GCC Driver Reinitialization Test Program\n");
    printf("=========================================\n");
    
    /* Test process-based invocation (most reliable) */
    test_process_based();
    
    /* Test library-based if supported */
    test_library_based();
    
#ifdef LINK_WITH_DRIVER
    /* Test direct call if linked with driver */
    test_direct_call();
#endif
    
    /* Additional test: Simulate multi-stage compilation pipeline */
    printf("\n=== Testing Multi-Stage Compilation Pipeline ===\n");
    
    /* Create pipeline test source */
    char *pipeline_source = "pipeline_test.c";
    if (create_test_file(pipeline_source) == 0) {
        /* Simulate: preprocess -> compile -> assemble -> link */
        char *stages[][10] = {
            {"gcc", "-E", pipeline_source, "-o", "pipeline.i", 
             "-save-temps", "-dumpdir", "/tmp/pipeline", NULL},
            {"gcc", "-S", "pipeline.i", "-o", "pipeline.s", NULL},
            {"gcc", "-c", "pipeline.s", "-o", "pipeline.o", 
             "-dumpbase", "pipeline_stage", NULL},
            {"gcc", "pipeline.o", "-o", "pipeline_exe", NULL}
        };
        
        for (int i = 0; i < 4; i++) {
            printf("\nStage %d: ", i + 1);
            for (int j = 0; stages[i][j] != NULL; j++) {
                printf("%s ", stages[i][j]);
            }
            printf("\n");
            
            pid_t pid = fork();
            if (pid == 0) {
                execvp("gcc", stages[i]);
                perror("execvp failed in pipeline");
                exit(127);
            } else if (pid > 0) {
                int status;
                waitpid(pid, &status, 0);
                printf("Stage completed with status: %d\n", 
                       WIFEXITED(status) ? WEXITSTATUS(status) : -1);
            }
            usleep(5000); /* Small delay between stages */
        }
        
        /* Cleanup pipeline files */
        unlink(pipeline_source);
        unlink("pipeline.i");
        unlink("pipeline.s");
        unlink("pipeline.o");
        unlink("pipeline_exe");
    }
    
    printf("\n=== All tests completed ===\n");
    return 0;
}
