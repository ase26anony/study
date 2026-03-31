/* test_driver_reinit.c - Test GCC driver reinitialization logic */
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
const char* test_source = 
"#include <stdio.h>\n"
"int main() {\n"
"    printf(\"Hello from test program\\n\");\n"
"    return 0;\n"
"}\n";

/* Method A: Process-based testing using fork/exec */
void test_with_processes() {
    printf("=== Testing with fork/exec ===\n");
    
    /* Write test source file */
    FILE* f = fopen("test_input.c", "w");
    if (!f) {
        perror("Failed to create test_input.c");
        return;
    }
    fputs(test_source, f);
    fclose(f);
    
    /* Array of test invocations */
    char* test_invocations[][10] = {
        /* First: Set various flags including sysroot */
        {"gcc", "-save-temps", "-dumpdir", "/tmp/test_dump1", 
         "-dumpbase", "test_dumpbase", "-c", "test_input.c", 
         "-o", "test_output1.o", NULL},
        
        /* Second: Reset to defaults (no special flags) */
        {"gcc", "-c", "test_input.c", "-o", "test_output2.o", NULL},
        
        /* Third: Force failure with invalid option */
        {"gcc", "-invalid-option-that-does-not-exist", NULL},
        
        /* Fourth: Successful compilation again */
        {"gcc", "-c", "test_input.c", "-o", "test_output3.o", NULL},
        
        /* Fifth: Test with sysroot and machine specs */
        {"gcc", "--sysroot=/tmp/fake_sysroot", "-march=x86-64", 
         "-mtune=generic", "-c", "test_input.c", "-o", "test_output4.o", NULL},
        
        /* Sixth: Back to defaults */
        {"gcc", "-c", "test_input.c", "-o", "test_output5.o", NULL}
    };
    
    int num_tests = sizeof(test_invocations) / sizeof(test_invocations[0]);
    int statuses[num_tests];
    
    for (int i = 0; i < num_tests; i++) {
        printf("\nTest %d: ", i + 1);
        for (int j = 0; test_invocations[i][j]; j++) {
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
    
    /* Cleanup */
    unlink("test_input.c");
    for (int i = 1; i <= 5; i++) {
        char filename[32];
        snprintf(filename, sizeof(filename), "test_output%d.o", i);
        unlink(filename);
    }
    
    printf("\n=== Process-based test complete ===\n");
}

/* Method B: Direct library call using dlopen (if available) */
#ifdef TEST_WITH_DLOPEN
void test_with_dlopen() {
    printf("\n=== Testing with dlopen ===\n");
    
    /* Try to load GCC driver as a shared library */
    void* handle = dlopen("libgccdriver.so", RTLD_LAZY);
    if (!handle) {
        /* Try alternative names */
        handle = dlopen("/usr/lib/gcc/libgccdriver.so", RTLD_LAZY);
    }
    
    if (!handle) {
        printf("Could not load libgccdriver.so: %s\n", dlerror());
        printf("Skipping dlopen test (driver may not be built as shared library)\n");
        return;
    }
    
    /* Look for main function */
    int (*driver_main)(int, char**) = dlsym(handle, "main");
    if (!driver_main) {
        /* Try alternative entry point */
        driver_main = dlsym(handle, "driver::main");
    }
    
    if (!driver_main) {
        printf("Could not find driver entry point: %s\n", dlerror());
        dlclose(handle);
        return;
    }
    
    /* Create test source file */
    FILE* f = fopen("dlopen_test.c", "w");
    if (f) {
        fputs(test_source, f);
        fclose(f);
    }
    
    /* Test invocations for direct calls */
    char* test1[] = {"gcc", "-save-temps", "-dumpdir", "/tmp/dlopen_test",
                     "-c", "dlopen_test.c", "-o", "dlopen_test1.o", NULL};
    char* test2[] = {"gcc", "-c", "dlopen_test.c", "-o", "dlopen_test2.o", NULL};
    char* test3[] = {"gcc", "-invalid-option", NULL};
    char* test4[] = {"gcc", "-c", "dlopen_test.c", "-o", "dlopen_test3.o", NULL};
    
    printf("Call 1: With save-temps and dumpdir\n");
    int ret1 = driver_main(8, test1);
    printf("Return: %d\n", ret1);
    
    printf("\nCall 2: Reset to defaults\n");
    int ret2 = driver_main(5, test2);
    printf("Return: %d\n", ret2);
    
    printf("\nCall 3: Force failure\n");
    int ret3 = driver_main(2, test3);
    printf("Return: %d\n", ret3);
    
    printf("\nCall 4: Successful compilation\n");
    int ret4 = driver_main(5, test4);
    printf("Return: %d\n", ret4);
    
    /* Cleanup */
    unlink("dlopen_test.c");
    unlink("dlopen_test1.o");
    unlink("dlopen_test2.o");
    unlink("dlopen_test3.o");
    
    dlclose(handle);
    printf("\n=== Dlopen test complete ===\n");
}
#endif

/* Method C: Direct function call by linking with driver object files */
#ifdef LINK_WITH_DRIVER
/* This would require including gcc.cc and linking with all driver objects */
extern int driver_main(int argc, char** argv);

void test_direct_linking() {
    printf("\n=== Testing with direct linking ===\n");
    
    /* Create test source */
    FILE* f = fopen("direct_test.c", "w");
    if (f) {
        fputs(test_source, f);
        fclose(f);
    }
    
    /* Multiple invocations to trigger reinitialization */
    char* invocations[][8] = {
        {"gcc", "-save-temps", "-dumpbase", "test", 
         "--sysroot=/tmp/sysroot", "-c", "direct_test.c", NULL},
        {"gcc", "-c", "direct_test.c", NULL},
        {"gcc", "-nonexistent-flag", NULL},
        {"gcc", "-c", "direct_test.c", "-o", "final.o", NULL}
    };
    
    for (int i = 0; i < 4; i++) {
        printf("\nInvocation %d:\n", i + 1);
        int argc = 0;
        while (invocations[i][argc]) argc++;
        
        int result = driver_main(argc, invocations[i]);
        printf("Result: %d\n", result);
    }
    
    /* Cleanup */
    unlink("direct_test.c");
    unlink("final.o");
}
#endif

int main(int argc, char** argv) {
    printf("GCC Driver Reinitialization Test\n");
    printf("=================================\n");
    
    /* Test with processes (always works) */
    test_with_processes();
    
    /* Conditionally test with dlopen */
#ifdef TEST_WITH_DLOPEN
    test_with_dlopen();
#endif
    
    /* Conditionally test with direct linking */
#ifdef LINK_WITH_DRIVER
    test_direct_linking();
#endif
    
    /* Additional test: Simulate multi-stage compilation pipeline */
    printf("\n=== Testing multi-stage compilation pipeline ===\n");
    
    /* Create pipeline test source */
    FILE* pipe_f = fopen("pipeline.c", "w");
    if (pipe_f) {
        fprintf(pipe_f, "#define TEST_MACRO 42\n");
        fprintf(pipe_f, "int test_func() { return TEST_MACRO; }\n");
        fclose(pipe_f);
    }
    
    /* Simulate: preprocess -> compile -> assemble -> link */
    char* stages[][10] = {
        /* Stage 1: Preprocess with dump options */
        {"gcc", "-E", "-save-temps", "-dumpdir", "/tmp/pipeline",
         "-dumpbase", "pipeline", "pipeline.c", "-o", "pipeline.i", NULL},
        
        /* Stage 2: Compile to assembly */
        {"gcc", "-S", "pipeline.i", "-o", "pipeline.s", NULL},
        
        /* Stage 3: Assemble */
        {"gcc", "-c", "pipeline.s", "-o", "pipeline.o", NULL},
        
        /* Stage 4: Link with different options */
        {"gcc", "pipeline.o", "-o", "pipeline_exec", NULL}
    };
    
    for (int i = 0; i < 4; i++) {
        printf("\nStage %d: ", i + 1);
        for (int j = 0; stages[i][j]; j++) {
            printf("%s ", stages[i][j]);
        }
        printf("\n");
        
        if (fork() == 0) {
            execvp("gcc", stages[i]);
            perror("execvp");
            exit(1);
        }
        wait(NULL);
    }
    
    /* Cleanup pipeline files */
    unlink("pipeline.c");
    unlink("pipeline.i");
    unlink("pipeline.s");
    unlink("pipeline.o");
    unlink("pipeline_exec");
    
    printf("\n=== All tests completed ===\n");
    return 0;
}
