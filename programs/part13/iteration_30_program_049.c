/* test_driver_reinit.c - Test GCC driver reinitialization logic */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <errno.h>

/* Create a minimal input file for compilation */
void create_test_source(const char *filename) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        perror("Failed to create test source");
        exit(1);
    }
    
    fprintf(f, "/* Test source for driver reinitialization */\n");
    fprintf(f, "#include <stdio.h>\n\n");
    fprintf(f, "int main() {\n");
    fprintf(f, "    printf(\"Hello from driver test\\n\");\n");
    fprintf(f, "    return 0;\n");
    fprintf(f, "}\n");
    
    fclose(f);
}

/* Method A: Process-based testing using fork/exec */
void test_via_processes() {
    printf("=== Testing via fork/exec ===\n");
    
    const char *input_file = "driver_test_input.c";
    create_test_source(input_file);
    
    /* Define test invocations with different flags */
    char *invocations[][20] = {
        /* First: Set various flags including sysroot and dump options */
        {"gcc", "-save-temps", "-dumpdir", "/tmp/driver_test1",
         "-dumpbase", "test_dump", "-c", input_file,
         "-o", "output1.o", "--sysroot=/tmp/fake_sysroot",
         "-specs=/tmp/test.specs", NULL},
        
        /* Second: Minimal invocation to trigger reset to defaults */
        {"gcc", "-c", input_file, "-o", "output2.o", NULL},
        
        /* Third: Force failure with invalid option */
        {"gcc", "-invalid-option-that-does-not-exist", input_file, NULL},
        
        /* Fourth: Successful compilation again */
        {"gcc", "-c", input_file, "-o", "output4.o", NULL},
        
        /* Fifth: Another with different dump options */
        {"gcc", "-save-temps=obj", "-dumpdir", "/tmp/driver_test2",
         "-dumpbase", "test2", "-dumpbase-ext", ".ext",
         "-c", input_file, "-o", "output5.o", NULL},
        
        /* Sixth: Back to minimal to trigger another reset */
        {"gcc", "-c", input_file, "-o", "output6.o", NULL}
    };
    
    int num_invocations = sizeof(invocations) / sizeof(invocations[0]);
    int statuses[num_invocations];
    
    for (int i = 0; i < num_invocations; i++) {
        printf("\nInvocation %d: ", i + 1);
        for (int j = 0; invocations[i][j]; j++) {
            printf("%s ", invocations[i][j]);
        }
        printf("\n");
        
        pid_t pid = fork();
        if (pid == 0) {
            /* Child process */
            execvp("gcc", invocations[i]);
            perror("execvp failed");
            exit(127);
        } else if (pid > 0) {
            /* Parent process */
            waitpid(pid, &statuses[i], 0);
            printf("Exit status: %d\n", WEXITSTATUS(statuses[i]));
        } else {
            perror("fork failed");
            exit(1);
        }
        
        /* Small delay to ensure cleanup */
        usleep(10000);
    }
    
    /* Clean up test files */
    unlink(input_file);
    for (int i = 1; i <= 6; i++) {
        char filename[32];
        snprintf(filename, sizeof(filename), "output%d.o", i);
        unlink(filename);
    }
    
    printf("\n=== Process-based test complete ===\n\n");
}

/* Method B: Direct library call if driver is built as shared library */
#ifdef TEST_DIRECT_CALL
void test_via_direct_call() {
    printf("=== Testing via direct library call ===\n");
    
    /* Try to load GCC driver as shared library */
    void *handle = dlopen("./libgccdriver.so", RTLD_LAZY | RTLD_LOCAL);
    if (!handle) {
        /* Try alternative names */
        handle = dlopen("libgccdriver.so", RTLD_LAZY | RTLD_LOCAL);
    }
    
    if (!handle) {
        printf("Direct library call not available: %s\n", dlerror());
        printf("To enable, build GCC driver as shared library:\n");
        printf("  g++ -shared -fPIC -o libgccdriver.so gcc.o ...\n");
        return;
    }
    
    /* Look for driver entry point */
    typedef int (*driver_main_t)(int, char**);
    driver_main_t driver_main = (driver_main_t)dlsym(handle, "main");
    if (!driver_main) {
        driver_main = (driver_main_t)dlsym(handle, "driver::main");
    }
    
    if (!driver_main) {
        printf("Could not find driver entry point: %s\n", dlerror());
        dlclose(handle);
        return;
    }
    
    /* Create test source */
    const char *input_file = "direct_test_input.c";
    create_test_source(input_file);
    
    /* Test invocations */
    char *test_args[][10] = {
        {"gcc", "-save-temps", "-dumpdir", "/tmp/direct1", 
         "-c", input_file, "-o", "direct1.o", NULL},
        {"gcc", "-c", input_file, "-o", "direct2.o", NULL},
        {"gcc", "-invalid-option", NULL},
        {"gcc", "-c", input_file, "-o", "direct3.o", NULL}
    };
    
    for (int i = 0; i < 4; i++) {
        printf("\nDirect call %d: ", i + 1);
        for (int j = 0; test_args[i][j]; j++) {
            printf("%s ", test_args[i][j]);
        }
        printf("\n");
        
        int argc = 0;
        while (test_args[i][argc]) argc++;
        
        int result = driver_main(argc, test_args[i]);
        printf("Driver returned: %d\n", result);
        
        /* Force cleanup between calls by unloading/reloading */
        if (i < 3) {
            /* Simulate process exit by resetting via dlclose/dlopen */
            dlclose(handle);
            handle = dlopen("libgccdriver.so", RTLD_LAZY | RTLD_LOCAL);
            if (handle) {
                driver_main = (driver_main_t)dlsym(handle, "main");
            }
        }
    }
    
    dlclose(handle);
    unlink(input_file);
    printf("\n=== Direct call test complete ===\n\n");
}
#endif

/* Method C: Simulate multi-stage compilation pipeline */
void test_multi_stage_pipeline() {
    printf("=== Testing multi-stage compilation pipeline ===\n");
    
    const char *input_file = "pipeline_input.c";
    create_test_source(input_file);
    
    /* Simulate: preprocess -> compile -> assemble -> link */
    char *stages[][15] = {
        /* Stage 1: Preprocess with dump options */
        {"gcc", "-E", "-save-temps", "-dumpdir", "/tmp/stage1",
         "-dumpbase", "pipeline", input_file, 
         "-o", "pipeline.i", NULL},
        
        /* Stage 2: Compile to assembly */
        {"gcc", "-S", "-save-temps=obj", "-dumpdir", "/tmp/stage2",
         input_file, "-o", "pipeline.s", NULL},
        
        /* Stage 3: Assemble */
        {"gcc", "-c", "pipeline.s", "-o", "pipeline.o", NULL},
        
        /* Stage 4: Link with different sysroot */
        {"gcc", "pipeline.o", "-o", "pipeline.out",
         "--sysroot=/tmp/alt_root", NULL},
        
        /* Stage 5: Another compile to trigger reset */
        {"gcc", "-c", input_file, "-o", "final.o", NULL}
    };
    
    for (int i = 0; i < 5; i++) {
        printf("\nStage %d: ", i + 1);
        for (int j = 0; stages[i][j]; j++) {
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
            printf("Exit: %d\n", WEXITSTATUS(status));
        }
        
        usleep(5000);
    }
    
    /* Cleanup */
    unlink(input_file);
    unlink("pipeline.i");
    unlink("pipeline.s");
    unlink("pipeline.o");
    unlink("pipeline.out");
    unlink("final.o");
    
    printf("\n=== Pipeline test complete ===\n\n");
}

/* Helper to create dummy spec file */
void create_dummy_specs() {
    FILE *f = fopen("/tmp/test.specs", "w");
    if (f) {
        fprintf(f, "*link:\n  --dummy-spec\n");
        fclose(f);
    }
    
    /* Create dummy sysroot directories */
    mkdir("/tmp/fake_sysroot", 0755);
    mkdir("/tmp/alt_root", 0755);
}

int main(int argc, char **argv) {
    printf("GCC Driver Reinitialization Test\n");
    printf("================================\n\n");
    
    /* Create necessary dummy files/directories */
    create_dummy_specs();
    
    /* Run all test methods */
    test_via_processes();
    
    test_multi_stage_pipeline();
    
#ifdef TEST_DIRECT_CALL
    test_via_direct_call();
#endif
    
    /* Additional targeted test for specific flag combinations */
    printf("=== Targeted flag combination test ===\n");
    
    /* Test save_temps flag variations */
    char *save_temp_tests[][10] = {
        {"gcc", "-save-temps", "-c", "driver_test_input.c", "-o", "st1.o", NULL},
        {"gcc", "-save-temps=cwd", "-c", "driver_test_input.c", "-o", "st2.o", NULL},
        {"gcc", "-save-temps=obj", "-c", "driver_test_input.c", "-o", "st3.o", NULL},
        {"gcc", "-c", "driver_test_input.c", "-o", "st4.o", NULL}  /* Should reset flag */
    };
    
    create_test_source("driver_test_input.c");
    
    for (int i = 0; i < 4; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            execvp("gcc", save_temp_tests[i]);
            exit(127);
        } else if (pid > 0) {
            int status;
            waitpid(pid, &status, 0);
            printf("Save-temps test %d: %d\n", i + 1, WEXITSTATUS(status));
        }
    }
    
    unlink("driver_test_input.c");
    
    printf("\n=== All tests completed ===\n");
    return 0;
}
