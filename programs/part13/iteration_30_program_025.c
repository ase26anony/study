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
        {"gcc", "-save-temps", "-dumpdir", "/tmp/gcc_test1", 
         "-dumpbase", "testdump", "-c", "test_input.c", 
         "-o", "output1.o", NULL},
        
        /* Second: Reset to defaults (no special flags) */
        {"gcc", "-c", "test_input.c", "-o", "output2.o", NULL},
        
        /* Third: Force failure with invalid option */
        {"gcc", "-invalid-option-that-does-not-exist", NULL},
        
        /* Fourth: Successful compilation again */
        {"gcc", "-c", "test_input.c", "-o", "output3.o", NULL},
        
        /* Fifth: Another with different sysroot and machine spec */
        {"gcc", "--sysroot=/usr/alt", "-march=native", 
         "-save-temps=obj", "-c", "test_input.c", 
         "-o", "output4.o", NULL},
        
        /* Sixth: Back to defaults */
        {"gcc", "-c", "test_input.c", "-o", "output5.o", NULL}
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
        char fname[20];
        snprintf(fname, sizeof(fname), "output%d.o", i);
        unlink(fname);
    }
    
    printf("\n=== Process-based test complete ===\n\n");
}

/* Method B: Direct library call using dlopen/dlsym */
#ifdef DRIVER_TEST
void test_via_direct_call() {
    printf("=== Testing via direct library calls ===\n");
    
    /* Try to load GCC driver as a library */
    void *handle = dlopen("libgccdriver.so", RTLD_LAZY | RTLD_LOCAL);
    if (!handle) {
        /* Try alternative names */
        handle = dlopen("./libgccdriver.so", RTLD_LAZY | RTLD_LOCAL);
    }
    
    if (!handle) {
        printf("Could not load libgccdriver.so: %s\n", dlerror());
        printf("Skipping direct library test.\n");
        return;
    }
    
    /* Look for driver entry point - this is compiler-specific */
    int (*driver_main)(int, char**) = dlsym(handle, "main");
    if (!driver_main) {
        driver_main = dlsym(handle, "gcc_main");
    }
    
    if (!driver_main) {
        printf("Could not find driver entry point: %s\n", dlerror());
        dlclose(handle);
        return;
    }
    
    /* Create test source file */
    FILE *fp = fopen("test_input2.c", "w");
    if (!fp) {
        perror("Failed to create test_input2.c");
        dlclose(handle);
        return;
    }
    fputs(test_source, fp);
    fclose(fp);
    
    /* Test invocations with different argument vectors */
    char *argv1[] = {"gcc", "-save-temps", "-dumpdir", "/tmp/direct_test",
                     "-dumpbase", "direct", "-c", "test_input2.c", 
                     "-o", "direct1.o", NULL};
    char *argv2[] = {"gcc", "-c", "test_input2.c", "-o", "direct2.o", NULL};
    char *argv3[] = {"gcc", "-invalid-flag", NULL};
    char *argv4[] = {"gcc", "-c", "test_input2.c", "-o", "direct3.o", NULL};
    
    printf("\nDirect call 1 (with dump options):\n");
    int result1 = driver_main(10, argv1);
    printf("Result: %d\n", result1);
    
    printf("\nDirect call 2 (defaults):\n");
    int result2 = driver_main(4, argv2);
    printf("Result: %d\n", result2);
    
    printf("\nDirect call 3 (invalid - should fail):\n");
    int result3 = driver_main(2, argv3);
    printf("Result: %d\n", result3);
    
    printf("\nDirect call 4 (successful again):\n");
    int result4 = driver_main(4, argv4);
    printf("Result: %d\n", result4);
    
    /* Cleanup */
    unlink("test_input2.c");
    unlink("direct1.o");
    unlink("direct2.o");
    unlink("direct3.o");
    
    dlclose(handle);
    printf("\n=== Direct library test complete ===\n\n");
}
#endif

/* Method C: Simulate multi-stage compilation pipeline */
void test_multi_stage_pipeline() {
    printf("=== Testing multi-stage compilation pipeline ===\n");
    
    /* Create a more complex test file */
    const char *complex_source = 
    "#define TEST_MACRO 42\n"
    "int add(int a, int b) { return a + b + TEST_MACRO; }\n"
    "int main() { return add(1, 2); }\n";
    
    FILE *fp = fopen("pipeline.c", "w");
    if (!fp) {
        perror("Failed to create pipeline.c");
        return;
    }
    fputs(complex_source, fp);
    fclose(fp);
    
    /* Stage 1: Preprocessing with save-temps */
    printf("\nStage 1: Preprocessing with -save-temps\n");
    char *stage1[] = {"gcc", "-E", "-save-temps", "-dumpdir", "/tmp/pipeline",
                      "-dumpbase", "stage1", "pipeline.c", 
                      "-o", "pipeline.i", NULL};
    pid_t pid1 = fork();
    if (pid1 == 0) {
        execvp("gcc", stage1);
        perror("execvp stage1");
        exit(127);
    }
    waitpid(pid1, NULL, 0);
    
    /* Stage 2: Compilation with different dumpbase */
    printf("\nStage 2: Compilation with new dumpbase\n");
    char *stage2[] = {"gcc", "-S", "-save-temps=obj", "-dumpbase", "stage2",
                      "pipeline.i", "-o", "pipeline.s", NULL};
    pid_t pid2 = fork();
    if (pid2 == 0) {
        execvp("gcc", stage2);
        perror("execvp stage2");
        exit(127);
    }
    waitpid(pid2, NULL, 0);
    
    /* Stage 3: Assembly with sysroot change */
    printf("\nStage 3: Assembly with sysroot\n");
    char *stage3[] = {"gcc", "-c", "--sysroot=/usr", "pipeline.s",
                      "-o", "pipeline.o", NULL};
    pid_t pid3 = fork();
    if (pid3 == 0) {
        execvp("gcc", stage3);
        perror("execvp stage3");
        exit(127);
    }
    waitpid(pid3, NULL, 0);
    
    /* Stage 4: Linking back to defaults */
    printf("\nStage 4: Linking (defaults)\n");
    char *stage4[] = {"gcc", "pipeline.o", "-o", "pipeline", NULL};
    pid_t pid4 = fork();
    if (pid4 == 0) {
        execvp("gcc", stage4);
        perror("execvp stage4");
        exit(127);
    }
    waitpid(pid4, NULL, 0);
    
    /* Stage 5: Clean compilation with failure */
    printf("\nStage 5: Invalid compilation (should fail)\n");
    char *stage5[] = {"gcc", "-nonexistent-flag", "pipeline.c", NULL};
    pid_t pid5 = fork();
    int status5;
    if (pid5 == 0) {
        execvp("gcc", stage5);
        perror("execvp stage5");
        exit(127);
    }
    waitpid(pid5, &status5, 0);
    printf("Failure exit status: %d\n", WEXITSTATUS(status5));
    
    /* Stage 6: Successful compilation again */
    printf("\nStage 6: Successful compilation again\n");
    char *stage6[] = {"gcc", "-c", "pipeline.c", "-o", "final.o", NULL};
    pid_t pid6 = fork();
    if (pid6 == 0) {
        execvp("gcc", stage6);
        perror("execvp stage6");
        exit(127);
    }
    waitpid(pid6, NULL, 0);
    
    /* Cleanup */
    unlink("pipeline.c");
    unlink("pipeline.i");
    unlink("pipeline.s");
    unlink("pipeline.o");
    unlink("pipeline");
    unlink("final.o");
    
    printf("\n=== Multi-stage pipeline test complete ===\n\n");
}

int main(int argc, char *argv[]) {
    printf("GCC Driver Reinitialization Test Program\n");
    printf("=========================================\n\n");
    
    /* Test using process-based method (most reliable) */
    test_via_processes();
    
    /* Test multi-stage pipeline */
    test_multi_stage_pipeline();
    
    /* Test direct library call if enabled */
#ifdef DRIVER_TEST
    test_via_direct_call();
#endif
    
    printf("\nAll tests completed.\n");
    
    /* Verify that we can still compile a simple program */
    printf("\nFinal verification: Simple compilation\n");
    char *final_test[] = {"gcc", "--version", NULL};
    pid_t pid = fork();
    if (pid == 0) {
        execvp("gcc", final_test);
        exit(127);
    }
    waitpid(pid, NULL, 0);
    
    return 0;
}
