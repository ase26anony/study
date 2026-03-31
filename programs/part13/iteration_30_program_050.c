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

/* Method A: Process-based invocation using fork/exec */
void test_via_processes() {
    printf("=== Testing via fork/exec processes ===\n");
    
    /* Create test source file */
    FILE* fp = fopen("test_input.c", "w");
    if (!fp) {
        perror("Failed to create test_input.c");
        return;
    }
    fputs(test_source, fp);
    fclose(fp);
    
    /* Array of test invocations with different flags */
    char* test_invocations[][10] = {
        /* First: Set various flags including sysroot and dump options */
        {"gcc", "-save-temps", "-dumpdir", "/tmp/test_dump1", 
         "-dumpbase", "test_dumpbase", "-c", "test_input.c", 
         "-o", "test_output1.o", NULL},
        
        /* Second: Reset to defaults (no special flags) */
        {"gcc", "-c", "test_input.c", "-o", "test_output2.o", NULL},
        
        /* Third: Force failure with invalid option */
        {"gcc", "-invalid-option-that-does-not-exist", NULL},
        
        /* Fourth: Successful compilation again */
        {"gcc", "-c", "test_input.c", "-o", "test_output3.o", NULL},
        
        /* Fifth: Another with different sysroot and machine spec */
        {"gcc", "--sysroot=/tmp/fake_sysroot", "-march=native",
         "-c", "test_input.c", "-o", "test_output4.o", NULL},
        
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
            /* If execvp fails */
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
        char fname[50];
        sprintf(fname, "test_output%d.o", i);
        unlink(fname);
    }
    
    printf("\n=== Process-based test complete ===\n");
}

/* Method B: Direct library call using dlopen/dlsym */
#ifdef DRIVER_LIBRARY_TEST
void test_via_library() {
    printf("\n=== Testing via direct library calls ===\n");
    
    /* Try to load GCC driver as a shared library */
    void* handle = dlopen("libgccdriver.so", RTLD_LAZY | RTLD_LOCAL);
    if (!handle) {
        /* Try alternative names */
        handle = dlopen("./gcc_driver.so", RTLD_LAZY | RTLD_LOCAL);
    }
    
    if (!handle) {
        printf("Could not load GCC driver library: %s\n", dlerror());
        printf("Skipping library test. Ensure GCC is built with -shared -fPIC\n");
        return;
    }
    
    /* Look for driver entry point - this could be main() or a wrapper */
    int (*driver_main)(int, char**) = dlsym(handle, "main");
    if (!driver_main) {
        driver_main = dlsym(handle, "gcc_driver_main");
    }
    
    if (!driver_main) {
        printf("Could not find driver entry point: %s\n", dlerror());
        dlclose(handle);
        return;
    }
    
    /* Create test source file */
    FILE* fp = fopen("libtest_input.c", "w");
    if (!fp) {
        perror("Failed to create libtest_input.c");
        dlclose(handle);
        return;
    }
    fputs(test_source, fp);
    fclose(fp);
    
    /* Test invocations through direct function calls */
    char* test1[] = {"gcc", "-save-temps", "-dumpdir", "/tmp/libtest1",
                     "-dumpbase", "libdump", "-c", "libtest_input.c",
                     "-o", "liboutput1.o", NULL};
    char* test2[] = {"gcc", "-c", "libtest_input.c", "-o", "liboutput2.o", NULL};
    char* test3[] = {"gcc", "-invalid-option-force-failure", NULL};
    char* test4[] = {"gcc", "--sysroot=/tmp/alt_root", "-specs=test.specs",
                     "-c", "libtest_input.c", "-o", "liboutput3.o", NULL};
    char* test5[] = {"gcc", "-c", "libtest_input.c", "-o", "liboutput4.o", NULL};
    
    int argc;
    int results[5];
    
    /* Test 1: With dump options */
    argc = sizeof(test1)/sizeof(test1[0]) - 1;
    results[0] = driver_main(argc, test1);
    printf("Library call 1 result: %d\n", results[0]);
    
    /* Test 2: Default options (should trigger reset) */
    argc = sizeof(test2)/sizeof(test2[0]) - 1;
    results[1] = driver_main(argc, test2);
    printf("Library call 2 result: %d\n", results[1]);
    
    /* Test 3: Force failure */
    argc = sizeof(test3)/sizeof(test3[0]) - 1;
    results[2] = driver_main(argc, test3);
    printf("Library call 3 result: %d\n", results[2]);
    
    /* Test 4: Different sysroot and specs */
    argc = sizeof(test4)/sizeof(test4[0]) - 1;
    results[3] = driver_main(argc, test4);
    printf("Library call 4 result: %d\n", results[3]);
    
    /* Test 5: Back to defaults */
    argc = sizeof(test5)/sizeof(test5[0]) - 1;
    results[4] = driver_main(argc, test5);
    printf("Library call 5 result: %d\n", results[4]);
    
    /* Cleanup */
    dlclose(handle);
    unlink("libtest_input.c");
    for (int i = 1; i <= 4; i++) {
        char fname[50];
        sprintf(fname, "liboutput%d.o", i);
        unlink(fname);
    }
    
    printf("=== Library test complete ===\n");
}
#endif

/* Method C: Simulate multi-stage compilation pipeline */
void test_multi_stage_pipeline() {
    printf("\n=== Testing multi-stage compilation pipeline ===\n");
    
    /* Create a more complex source file */
    const char* complex_source = 
    "#define TEST_MACRO 42\n"
    "int add(int a, int b) { return a + b + TEST_MACRO; }\n"
    "int main() { return add(1, 2); }\n";
    
    FILE* fp = fopen("pipeline_input.c", "w");
    if (!fp) {
        perror("Failed to create pipeline_input.c");
        return;
    }
    fputs(complex_source, fp);
    fclose(fp);
    
    /* Simulate a full compilation pipeline with different flags at each stage */
    char* pipeline_stages[][15] = {
        /* Stage 1: Preprocessing with dump options */
        {"gcc", "-E", "-save-temps=obj", "-dumpdir", "pipeline_dumps",
         "-dumpbase", "stage1", "pipeline_input.c", 
         "-o", "pipeline_preprocessed.i", NULL},
        
        /* Stage 2: Compilation to assembly */
        {"gcc", "-S", "-dumpbase", "stage2", "pipeline_preprocessed.i",
         "-o", "pipeline_assembly.s", NULL},
        
        /* Stage 3: Assembly to object (reset dumpdir) */
        {"gcc", "-c", "pipeline_assembly.s", 
         "-o", "pipeline_object.o", NULL},
        
        /* Stage 4: Linking with sysroot */
        {"gcc", "--sysroot=/tmp/pipeline_sysroot", 
         "pipeline_object.o", "-o", "pipeline_executable", NULL},
        
        /* Stage 5: Clean compilation (full reset) */
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
            perror("execvp failed in pipeline");
            exit(127);
        } else if (pid > 0) {
            int status;
            waitpid(pid, &status, 0);
            printf("Stage %d exit: %d\n", i + 1, WEXITSTATUS(status));
        }
    }
    
    /* Cleanup pipeline files */
    unlink("pipeline_input.c");
    unlink("pipeline_preprocessed.i");
    unlink("pipeline_assembly.s");
    unlink("pipeline_object.o");
    unlink("pipeline_executable");
    unlink("pipeline_final");
    
    printf("\n=== Pipeline test complete ===\n");
}

int main(int argc, char** argv) {
    printf("GCC Driver Reinitialization Test Program\n");
    printf("=========================================\n");
    
    /* Test using process invocations (most reliable) */
    test_via_processes();
    
    /* Test multi-stage pipeline simulation */
    test_multi_stage_pipeline();
    
#ifdef DRIVER_LIBRARY_TEST
    /* Test direct library calls if enabled */
    test_via_library();
#endif
    
    printf("\nAll tests completed.\n");
    printf("Check coverage data to verify lines 11228-11250 in gcc.cc were executed.\n");
    
    return 0;
}
