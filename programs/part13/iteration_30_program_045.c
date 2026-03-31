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

/* Create test source file */
int create_test_file(const char *filename) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        perror("fopen");
        return -1;
    }
    fputs(test_source, f);
    fclose(f);
    return 0;
}

/* Method A: Process-based testing using fork/exec */
void test_with_processes() {
    printf("=== Testing with fork/exec ===\n");
    
    const char *input_file = "test_input.c";
    if (create_test_file(input_file) != 0) {
        fprintf(stderr, "Failed to create test file\n");
        return;
    }
    
    /* Array of test invocations with different flags */
    char *test_invocations[][10] = {
        /* First: Set various flags including sysroot and dump options */
        {"gcc", "-save-temps", "-dumpdir", "/tmp/test_dump1", 
         "-dumpbase", "test_dumpbase1", "-c", input_file, 
         "-o", "output1.o", NULL},
        
        /* Second: Minimal compilation to trigger reset to defaults */
        {"gcc", "-c", input_file, "-o", "output2.o", NULL},
        
        /* Third: Force failure with invalid option */
        {"gcc", "-invalid-option-that-does-not-exist", input_file, NULL},
        
        /* Fourth: Successful compilation after failure */
        {"gcc", "-c", input_file, "-o", "output3.o", NULL},
        
        /* Fifth: Another with different sysroot and machine spec */
        {"gcc", "--sysroot=/tmp/fake_sysroot", "-march=x86-64", 
         "-mtune=generic", "-c", input_file, "-o", "output4.o", NULL},
        
        /* Sixth: Back to defaults again */
        {"gcc", "-c", input_file, "-o", "output5.o", NULL}
    };
    
    int num_tests = sizeof(test_invocations) / sizeof(test_invocations[0]);
    
    for (int i = 0; i < num_tests; i++) {
        printf("\n--- Test %d ---\n", i + 1);
        printf("Args: ");
        for (int j = 0; test_invocations[i][j] != NULL; j++) {
            printf("%s ", test_invocations[i][j]);
        }
        printf("\n");
        
        pid_t pid = fork();
        if (pid == 0) {
            /* Child process */
            execvp("gcc", test_invocations[i]);
            /* If execvp fails */
            perror("execvp");
            exit(127);
        } else if (pid > 0) {
            /* Parent process */
            int status;
            waitpid(pid, &status, 0);
            
            if (WIFEXITED(status)) {
                printf("Exit status: %d\n", WEXITSTATUS(status));
            } else if (WIFSIGNALED(status)) {
                printf("Terminated by signal: %d\n", WTERMSIG(status));
            }
        } else {
            perror("fork");
        }
    }
    
    /* Cleanup */
    unlink(input_file);
    for (int i = 1; i <= 5; i++) {
        char filename[32];
        snprintf(filename, sizeof(filename), "output%d.o", i);
        unlink(filename);
    }
}

/* Method B: Direct library call if GCC driver is built as shared library */
#ifdef TEST_WITH_DLOPEN
void test_with_dlopen() {
    printf("\n=== Testing with dlopen ===\n");
    
    void *handle = dlopen("./libgccdriver.so", RTLD_LAZY | RTLD_LOCAL);
    if (!handle) {
        /* Try common library names */
        handle = dlopen("libgccdriver.so", RTLD_LAZY | RTLD_LOCAL);
        if (!handle) {
            fprintf(stderr, "dlopen failed: %s\n", dlerror());
            fprintf(stderr, "Skipping dlopen test - driver not built as shared library\n");
            return;
        }
    }
    
    /* Look for main or driver entry point */
    int (*driver_main)(int, char**) = dlsym(handle, "main");
    if (!driver_main) {
        driver_main = dlsym(handle, "gcc_driver_main");
    }
    
    if (!driver_main) {
        fprintf(stderr, "dlsym failed: %s\n", dlerror());
        dlclose(handle);
        return;
    }
    
    /* Create test source file */
    const char *input_file = "test_input2.c";
    if (create_test_file(input_file) != 0) {
        dlclose(handle);
        return;
    }
    
    /* Test 1: With save-temps and dumpdir */
    char *args1[] = {
        "gcc", "-save-temps", "-dumpdir", "/tmp/dl_test",
        "-dumpbase", "dl_dumpbase", "-c", "test_input2.c",
        "-o", "dl_output1.o", NULL
    };
    
    printf("Test 1 via dlopen: ");
    int result1 = driver_main(10, args1);
    printf("Result: %d\n", result1);
    
    /* Test 2: Minimal to trigger reset */
    char *args2[] = {
        "gcc", "-c", "test_input2.c", "-o", "dl_output2.o", NULL
    };
    
    printf("Test 2 via dlopen: ");
    int result2 = driver_main(4, args2);
    printf("Result: %d\n", result2);
    
    /* Test 3: Force failure */
    char *args3[] = {
        "gcc", "-invalid-option-force-failure", "test_input2.c", NULL
    };
    
    printf("Test 3 via dlopen (should fail): ");
    int result3 = driver_main(3, args3);
    printf("Result: %d\n", result3);
    
    /* Test 4: Success after failure */
    char *args4[] = {
        "gcc", "-c", "test_input2.c", "-o", "dl_output3.o", NULL
    };
    
    printf("Test 4 via dlopen: ");
    int result4 = driver_main(4, args4);
    printf("Result: %d\n", result4);
    
    /* Cleanup */
    unlink(input_file);
    unlink("dl_output1.o");
    unlink("dl_output2.o");
    unlink("dl_output3.o");
    
    dlclose(handle);
}
#endif

/* Method C: Direct compilation with driver code (requires special build) */
#ifdef COMPILE_WITH_DRIVER
/* This would require including gcc.cc and linking with driver objects */
extern int driver_main(int argc, char **argv);

void test_direct_call() {
    printf("\n=== Testing with direct driver call ===\n");
    
    const char *input_file = "test_input3.c";
    if (create_test_file(input_file) != 0) {
        return;
    }
    
    /* Multiple invocations to trigger reinitialization */
    for (int i = 0; i < 3; i++) {
        char output_name[32];
        char dumpdir_name[32];
        snprintf(output_name, sizeof(output_name), "direct%d.o", i);
        snprintf(dumpdir_name, sizeof(dumpdir_name), "/tmp/direct%d", i);
        
        char *args[] = {
            "gcc",
            "-save-temps",
            "-dumpdir", dumpdir_name,
            "-dumpbase", "direct_base",
            "-c", "test_input3.c",
            "-o", output_name,
            NULL
        };
        
        printf("Direct call %d: ", i + 1);
        int result = driver_main(9, args);
        printf("Result: %d\n", result);
        
        /* Alternate with minimal call to trigger reset */
        if (i < 2) {
            char *simple_args[] = {
                "gcc", "-c", "test_input3.c", 
                "-o", "simple.o", NULL
            };
            printf("Reset call %d: ", i + 1);
            int simple_result = driver_main(5, simple_args);
            printf("Result: %d\n", simple_result);
            unlink("simple.o");
        }
        
        unlink(output_name);
    }
    
    unlink(input_file);
}
#endif

/* Advanced test: Simulate multi-stage compilation pipeline */
void test_multi_stage_pipeline() {
    printf("\n=== Testing multi-stage compilation pipeline ===\n");
    
    const char *input_file = "pipeline_test.c";
    if (create_test_file(input_file) != 0) {
        return;
    }
    
    /* Stage 1: Preprocessing with save-temps */
    printf("Stage 1: Preprocessing\n");
    char *stage1[] = {
        "gcc", "-E", "-save-temps", "-dumpdir", "/tmp/pipeline",
        "-dumpbase", "pipeline", input_file,
        "-o", "pipeline.i", NULL
    };
    
    pid_t pid1 = fork();
    if (pid1 == 0) {
        execvp("gcc", stage1);
        perror("execvp stage1");
        exit(127);
    } else {
        waitpid(pid1, NULL, 0);
    }
    
    /* Stage 2: Compilation to assembly */
    printf("Stage 2: Compilation\n");
    char *stage2[] = {
        "gcc", "-S", "pipeline.i",
        "-o", "pipeline.s", NULL
    };
    
    pid_t pid2 = fork();
    if (pid2 == 0) {
        execvp("gcc", stage2);
        perror("execvp stage2");
        exit(127);
    } else {
        waitpid(pid2, NULL, 0);
    }
    
    /* Stage 3: Assembly to object */
    printf("Stage 3: Assembly\n");
    char *stage3[] = {
        "gcc", "-c", "pipeline.s",
        "-o", "pipeline.o", NULL
    };
    
    pid_t pid3 = fork();
    if (pid3 == 0) {
        execvp("gcc", stage3);
        perror("execvp stage3");
        exit(127);
    } else {
        waitpid(pid3, NULL, 0);
    }
    
    /* Stage 4: Linking with different sysroot */
    printf("Stage 4: Linking with sysroot\n");
    char *stage4[] = {
        "gcc", "--sysroot=/tmp/fake_root",
        "pipeline.o", "-o", "pipeline.exe", NULL
    };
    
    pid_t pid4 = fork();
    if (pid4 == 0) {
        execvp("gcc", stage4);
        perror("execvp stage4");
        exit(127);
    } else {
        waitpid(pid4, NULL, 0);
    }
    
    /* Stage 5: Final clean compilation (triggers reset) */
    printf("Stage 5: Clean compilation\n");
    char *stage5[] = {
        "gcc", "-c", input_file,
        "-o", "final.o", NULL
    };
    
    pid_t pid5 = fork();
    if (pid5 == 0) {
        execvp("gcc", stage5);
        perror("execvp stage5");
        exit(127);
    } else {
        waitpid(pid5, NULL, 0);
    }
    
    /* Cleanup */
    unlink(input_file);
    unlink("pipeline.i");
    unlink("pipeline.s");
    unlink("pipeline.o");
    unlink("pipeline.exe");
    unlink("final.o");
}

int main(int argc, char **argv) {
    printf("GCC Driver Reinitialization Test\n");
    printf("================================\n");
    
    /* Test with processes (most reliable) */
    test_with_processes();
    
    /* Test multi-stage pipeline */
    test_multi_stage_pipeline();
    
#ifdef TEST_WITH_DLOPEN
    test_with_dlopen();
#endif
    
#ifdef COMPILE_WITH_DRIVER
    test_direct_call();
#endif
    
    printf("\n=== Test Complete ===\n");
    printf("Check coverage data to verify uncovered lines were hit.\n");
    printf("Look for execution of the reset block in gcc.cc lines 11228-11250.\n");
    
    return 0;
}
