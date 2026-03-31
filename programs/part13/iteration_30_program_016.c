/* test_driver_reinit.c - Program to test GCC driver reinitialization logic */
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
const char* test_source = 
"#include <stdio.h>\n"
"int main() {\n"
"    printf(\"Hello from test program\\n\");\n"
"    return 0;\n"
"}\n";

/* Create test source file */
int create_test_file(const char* filename) {
    FILE* f = fopen(filename, "w");
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
    
    const char* input_file = "test_input.c";
    if (!create_test_file(input_file)) {
        return;
    }
    
    /* Array of test invocations with different flags */
    char* invocations[][10] = {
        /* First: Set various flags including sysroot */
        {"gcc", "-save-temps", "-dumpdir", "/tmp/test_dump1", 
         "-dumpbase", "test_dumpbase", "-c", input_file, 
         "-o", "output1.o", NULL},
        
        /* Second: Reset to defaults (no special flags) */
        {"gcc", "-c", input_file, "-o", "output2.o", NULL},
        
        /* Third: Force failure with invalid option */
        {"gcc", "-invalid-option-that-does-not-exist", input_file, NULL},
        
        /* Fourth: Successful compilation again */
        {"gcc", "-c", input_file, "-o", "output3.o", NULL},
        
        /* Fifth: Test with sysroot and machine specs */
        {"gcc", "--sysroot=/tmp/fake_sysroot", "-march=x86-64", 
         "-mtune=generic", "-c", input_file, "-o", "output4.o", NULL},
        
        /* Sixth: Back to defaults */
        {"gcc", "-c", input_file, "-o", "output5.o", NULL}
    };
    
    int num_invocations = sizeof(invocations) / sizeof(invocations[0]);
    
    for (int i = 0; i < num_invocations; i++) {
        pid_t pid = fork();
        
        if (pid == 0) {
            /* Child process */
            printf("Invocation %d: ", i+1);
            for (int j = 0; invocations[i][j] != NULL; j++) {
                printf("%s ", invocations[i][j]);
            }
            printf("\n");
            
            execvp("gcc", invocations[i]);
            
            /* If execvp fails */
            perror("execvp failed");
            exit(EXIT_FAILURE);
        } 
        else if (pid > 0) {
            /* Parent process */
            int status;
            waitpid(pid, &status, 0);
            
            if (WIFEXITED(status)) {
                printf("  Exit status: %d\n", WEXITSTATUS(status));
            } else if (WIFSIGNALED(status)) {
                printf("  Terminated by signal: %d\n", WTERMSIG(status));
            }
            
            /* Small delay to ensure cleanup */
            usleep(10000);
        } 
        else {
            perror("fork failed");
        }
    }
    
    /* Cleanup */
    unlink(input_file);
    for (int i = 1; i <= 5; i++) {
        char filename[20];
        snprintf(filename, sizeof(filename), "output%d.o", i);
        unlink(filename);
    }
}

/* Method B: Direct library call using dlopen/dlsym */
#ifdef DRIVER_TEST
void test_via_direct_call() {
    printf("\n=== Testing via Direct Library Call ===\n");
    
    /* Try to load GCC driver as a shared library */
    void* handle = dlopen("./libgccdriver.so", RTLD_LAZY | RTLD_LOCAL);
    if (!handle) {
        /* Try common locations */
        handle = dlopen("/usr/lib/gcc/libgccdriver.so", RTLD_LAZY | RTLD_LOCAL);
    }
    
    if (!handle) {
        printf("Could not load GCC driver library: %s\n", dlerror());
        printf("Skipping direct library test.\n");
        return;
    }
    
    /* Look for main function */
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
    const char* input_file = "test_input2.c";
    if (!create_test_file(input_file)) {
        dlclose(handle);
        return;
    }
    
    /* Test 1: With save-temps and dumpdir */
    char* args1[] = {
        "gcc", "-save-temps", "-dumpdir", "/tmp/test_direct",
        "-dumpbase", "direct_test", "-c", "test_input2.c",
        "-o", "direct_output1.o", NULL
    };
    
    printf("Direct call 1: With save-temps and dumpdir\n");
    int result1 = driver_main(10, args1);
    printf("  Result: %d\n", result1);
    
    /* Test 2: Reset to defaults */
    char* args2[] = {
        "gcc", "-c", "test_input2.c", "-o", "direct_output2.o", NULL
    };
    
    printf("Direct call 2: Default compilation\n");
    int result2 = driver_main(4, args2);
    printf("  Result: %d\n", result2);
    
    /* Test 3: Force failure */
    char* args3[] = {
        "gcc", "-invalid-option-force-failure", "test_input2.c", NULL
    };
    
    printf("Direct call 3: Invalid option (should fail)\n");
    int result3 = driver_main(3, args3);
    printf("  Result: %d\n", result3);
    
    /* Test 4: Successful again */
    char* args4[] = {
        "gcc", "-c", "test_input2.c", "-o", "direct_output3.o", NULL
    };
    
    printf("Direct call 4: Successful compilation\n");
    int result4 = driver_main(4, args4);
    printf("  Result: %d\n", result4);
    
    /* Cleanup */
    unlink(input_file);
    for (int i = 1; i <= 3; i++) {
        char filename[30];
        snprintf(filename, sizeof(filename), "direct_output%d.o", i);
        unlink(filename);
    }
    
    dlclose(handle);
}
#endif

/* Method C: Simulate multi-stage compilation pipeline */
void test_multi_stage_pipeline() {
    printf("\n=== Testing Multi-Stage Compilation Pipeline ===\n");
    
    const char* input_file = "pipeline_input.c";
    if (!create_test_file(input_file)) {
        return;
    }
    
    /* Stage 1: Preprocessing with save-temps */
    printf("Stage 1: Preprocessing with -save-temps\n");
    pid_t pid1 = fork();
    if (pid1 == 0) {
        char* args[] = {
            "gcc", "-E", "-save-temps", "-dumpdir", "/tmp/pipeline",
            "-dumpbase", "stage1", input_file, "-o", "stage1.i", NULL
        };
        execvp("gcc", args);
        exit(EXIT_FAILURE);
    }
    waitpid(pid1, NULL, 0);
    
    /* Stage 2: Compilation with different dumpbase */
    printf("Stage 2: Compilation with new dumpbase\n");
    pid_t pid2 = fork();
    if (pid2 == 0) {
        char* args[] = {
            "gcc", "-S", "-save-temps=obj", "-dumpbase", "stage2",
            "-dumpdir", "/tmp/pipeline2", "stage1.i", "-o", "stage2.s", NULL
        };
        execvp("gcc", args);
        exit(EXIT_FAILURE);
    }
    waitpid(pid2, NULL, 0);
    
    /* Stage 3: Assembly with sysroot */
    printf("Stage 3: Assembly with --sysroot\n");
    pid_t pid3 = fork();
    if (pid3 == 0) {
        char* args[] = {
            "gcc", "-c", "--sysroot=/tmp/fake_root",
            "-specs=/dev/null",  /* Try to trigger spec machine reset */
            "stage2.s", "-o", "stage3.o", NULL
        };
        execvp("gcc", args);
        exit(EXIT_FAILURE);
    }
    waitpid(pid3, NULL, 0);
    
    /* Stage 4: Linking - back to defaults */
    printf("Stage 4: Linking (defaults)\n");
    pid_t pid4 = fork();
    if (pid4 == 0) {
        char* args[] = {
            "gcc", "stage3.o", "-o", "pipeline_output", NULL
        };
        execvp("gcc", args);
        exit(EXIT_FAILURE);
    }
    waitpid(pid4, NULL, 0);
    
    /* Stage 5: Clean compilation to test reset */
    printf("Stage 5: Clean compilation to verify reset\n");
    pid_t pid5 = fork();
    if (pid5 == 0) {
        char* args[] = {
            "gcc", "-c", input_file, "-o", "final.o", NULL
        };
        execvp("gcc", args);
        exit(EXIT_FAILURE);
    }
    waitpid(pid5, NULL, 0);
    
    /* Cleanup */
    unlink(input_file);
    unlink("stage1.i");
    unlink("stage2.s");
    unlink("stage3.o");
    unlink("pipeline_output");
    unlink("final.o");
}

int main(int argc, char** argv) {
    printf("GCC Driver Reinitialization Test Program\n");
    printf("=========================================\n\n");
    
    /* Test using process invocation (most reliable) */
    test_via_processes();
    
    /* Test multi-stage pipeline */
    test_multi_stage_pipeline();
    
#ifdef DRIVER_TEST
    /* Test direct library call if enabled */
    test_via_direct_call();
#endif
    
    printf("\n=== Test Complete ===\n");
    
    /* Create a dummy spec file for testing if needed */
    FILE* spec = fopen("/tmp/test.specs", "w");
    if (spec) {
        fprintf(spec, "*link:\n%s\n", "-lm");
        fclose(spec);
    }
    
    /* Additional test: Invoke with spec file then without */
    printf("\n=== Additional Test: Spec File Reset ===\n");
    const char* spec_test_file = "spec_test.c";
    create_test_file(spec_test_file);
    
    /* With spec file */
    if (fork() == 0) {
        char* args[] = {
            "gcc", "-specs=/tmp/test.specs", "-c", "spec_test.c", 
            "-o", "spec_test.o", NULL
        };
        execvp("gcc", args);
        exit(EXIT_FAILURE);
    }
    wait(NULL);
    
    /* Without spec file (should reset spec_machine) */
    if (fork() == 0) {
        char* args[] = {
            "gcc", "-c", "spec_test.c", "-o", "spec_test2.o", NULL
        };
        execvp("gcc", args);
        exit(EXIT_FAILURE);
    }
    wait(NULL);
    
    /* Cleanup */
    unlink(spec_test_file);
    unlink("spec_test.o");
    unlink("spec_test2.o");
    unlink("/tmp/test.specs");
    
    return 0;
}
