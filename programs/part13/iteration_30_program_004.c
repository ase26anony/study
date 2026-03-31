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
void create_test_source(const char *filename) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        perror("Failed to create test source file");
        exit(1);
    }
    fputs(test_source, f);
    fclose(f);
}

/* Method A: Process-based testing using fork/exec */
void test_via_processes() {
    printf("=== Testing via fork/exec processes ===\n");
    
    const char *input_file = "test_input.c";
    create_test_source(input_file);
    
    /* Array of test invocations with different flags */
    char *test_invocations[][20] = {
        /* First: Set various flags including sysroot and dump options */
        {"gcc", "-save-temps", "-dumpdir", "/tmp/test_dump1", 
         "-dumpbase", "test_dumpbase1", "-c", input_file, 
         "-o", "output1.o", "--sysroot=/tmp/fake_sysroot", 
         "-specs=/tmp/fake.specs", "-march=x86-64", NULL},
        
        /* Second: Minimal invocation to trigger reset to defaults */
        {"gcc", "-c", input_file, "-o", "output2.o", NULL},
        
        /* Third: Force failure with invalid option */
        {"gcc", "-invalid-option-that-does-not-exist", input_file, NULL},
        
        /* Fourth: Successful compilation again */
        {"gcc", "-c", input_file, "-o", "output3.o", NULL},
        
        /* Fifth: Another with different dump options */
        {"gcc", "-save-temps=obj", "-dumpdir", "/tmp/test_dump2",
         "-dumpbase", "test_dumpbase2", "-fdump-tree-original",
         "-c", input_file, "-o", "output4.o", NULL},
        
        /* Sixth: Back to minimal to trigger another reset */
        {"gcc", "-c", input_file, "-o", "output5.o", NULL}
    };
    
    int num_tests = sizeof(test_invocations) / sizeof(test_invocations[0]);
    int statuses[num_tests];
    
    for (int i = 0; i < num_tests; i++) {
        printf("\n--- Test %d: ", i + 1);
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
            exit(1);
        }
    }
    
    printf("\n=== Process-based test complete ===\n");
    printf("Expected: Test 3 should fail, others should succeed\n");
    
    /* Cleanup */
    unlink(input_file);
    for (int i = 1; i <= 5; i++) {
        char filename[32];
        snprintf(filename, sizeof(filename), "output%d.o", i);
        unlink(filename);
    }
}

/* Method B: Direct library call using dlopen (if available) */
#ifdef TEST_WITH_DLOPEN
void test_via_dlopen() {
    printf("\n=== Testing via dlopen (if driver is built as library) ===\n");
    
    void *handle = dlopen("./libgccdriver.so", RTLD_LAZY | RTLD_LOCAL);
    if (!handle) {
        /* Try common library names */
        handle = dlopen("libgccdriver.so", RTLD_LAZY | RTLD_LOCAL);
    }
    
    if (!handle) {
        printf("Driver library not found, skipping dlopen test\n");
        printf("dlerror: %s\n", dlerror());
        return;
    }
    
    /* Look for driver entry point - could be main() or driver::main() */
    int (*driver_main)(int, char**) = dlsym(handle, "main");
    if (!driver_main) {
        driver_main = dlsym(handle, "_Z6driveriPPc");  // mangled name for driver::main
    }
    
    if (!driver_main) {
        printf("Driver entry point not found\n");
        dlclose(handle);
        return;
    }
    
    const char *input_file = "test_input2.c";
    create_test_source(input_file);
    
    /* Test argument vectors for direct calls */
    char *test1_args[] = {
        "gcc", "-save-temps", "-dumpdir", "/tmp/dl_test1",
        "-dumpbase", "dl_dumpbase", "-c", input_file,
        "-o", "dl_output1.o", "--sysroot=/tmp/alt_sysroot",
        NULL
    };
    
    char *test2_args[] = {
        "gcc", "-c", input_file, "-o", "dl_output2.o", NULL
    };
    
    char *test3_args[] = {
        "gcc", "-invalid-option-force-failure", input_file, NULL
    };
    
    char *test4_args[] = {
        "gcc", "-c", input_file, "-o", "dl_output3.o", NULL
    };
    
    printf("\nCall 1: With special flags\n");
    int ret1 = driver_main(10, test1_args);
    printf("Return: %d\n", ret1);
    
    printf("\nCall 2: Minimal flags (should trigger reset)\n");
    int ret2 = driver_main(5, test2_args);
    printf("Return: %d\n", ret2);
    
    printf("\nCall 3: Invalid option (should fail)\n");
    int ret3 = driver_main(3, test3_args);
    printf("Return: %d\n", ret3);
    
    printf("\nCall 4: Valid again (testing greatest_status reset)\n");
    int ret4 = driver_main(5, test4_args);
    printf("Return: %d\n", ret4);
    
    dlclose(handle);
    unlink(input_file);
    unlink("dl_output1.o");
    unlink("dl_output2.o");
    unlink("dl_output3.o");
}
#endif

/* Method C: Direct compilation with driver code (requires special build) */
#ifdef COMPILE_WITH_DRIVER
/* This would require including gcc.cc and linking with driver objects */
extern int driver::main(int argc, char **argv);

void test_direct_driver_calls() {
    printf("\n=== Testing via direct driver::main calls ===\n");
    
    const char *input_file = "test_input3.c";
    create_test_source(input_file);
    
    /* Multiple calls to driver::main with different arguments */
    char *argv1[] = {
        "test_prog", "-save-temps", "-dumpdir", "/tmp/direct1",
        "-dumpbase", "direct_base", "-c", input_file,
        "-o", "direct1.o", "--sysroot=/tmp/direct_sysroot",
        "-mtune=generic", NULL
    };
    
    char *argv2[] = {
        "test_prog", "-c", input_file, "-o", "direct2.o", NULL
    };
    
    char *argv3[] = {
        "test_prog", "-nonexistent-flag-to-cause-error", NULL
    };
    
    char *argv4[] = {
        "test_prog", "-c", input_file, "-o", "direct3.o",
        "-fdump-rtl-all", "-fdump-tree-all", NULL
    };
    
    char *argv5[] = {
        "test_prog", "-c", input_file, "-o", "direct4.o", NULL
    };
    
    printf("Call 1: Full feature set\n");
    int ret1 = driver::main(11, argv1);
    printf("Return: %d\n", ret1);
    
    printf("\nCall 2: Reset to defaults\n");
    int ret2 = driver::main(5, argv2);
    printf("Return: %d\n", ret2);
    
    printf("\nCall 3: Force error\n");
    int ret3 = driver::main(2, argv3);
    printf("Return: %d\n", ret3);
    
    printf("\nCall 4: With dump flags\n");
    int ret4 = driver::main(7, argv4);
    printf("Return: %d\n", ret4);
    
    printf("\nCall 5: Minimal again\n");
    int ret5 = driver::main(5, argv5);
    printf("Return: %d\n", ret5);
    
    /* Cleanup */
    unlink(input_file);
    for (int i = 1; i <= 4; i++) {
        char filename[32];
        snprintf(filename, sizeof(filename), "direct%d.o", i);
        unlink(filename);
    }
}
#endif

/* Enhanced test with environment variable manipulation */
void test_with_env_vars() {
    printf("\n=== Testing with environment variables ===\n");
    
    const char *input_file = "test_env.c";
    create_test_source(input_file);
    
    /* Set environment variables that affect driver state */
    setenv("GCC_EXEC_PREFIX", "/tmp/gcc_prefix", 1);
    setenv("COMPILER_PATH", "/tmp/compiler_path", 1);
    setenv("LIBRARY_PATH", "/tmp/library_path", 1);
    
    /* Test sequence */
    char *tests[][15] = {
        {"gcc", "-save-temps", "-dumpdir", "/tmp/env_dump",
         "-dumpbase", "env_base", "-c", input_file,
         "-o", "env1.o", "--sysroot=/tmp/env_root", NULL},
        
        {"gcc", "-c", input_file, "-o", "env2.o", NULL},
        
        {"gcc", "-B/tmp/extra_path", "-c", input_file, 
         "-o", "env3.o", NULL},
        
        {"gcc", "-c", input_file, "-o", "env4.o", NULL}
    };
    
    for (int i = 0; i < 4; i++) {
        printf("\nEnv test %d: ", i + 1);
        for (int j = 0; tests[i][j]; j++) {
            printf("%s ", tests[i][j]);
        }
        printf("\n");
        
        pid_t pid = fork();
        if (pid == 0) {
            execvp("gcc", tests[i]);
            perror("execvp");
            exit(127);
        } else if (pid > 0) {
            int status;
            waitpid(pid, &status, 0);
            printf("Exit: %d\n", WEXITSTATUS(status));
        }
    }
    
    /* Cleanup */
    unlink(input_file);
    for (int i = 1; i <= 4; i++) {
        char filename[32];
        snprintf(filename, sizeof(filename), "env%d.o", i);
        unlink(filename);
    }
    
    /* Unset environment variables */
    unsetenv("GCC_EXEC_PREFIX");
    unsetenv("COMPILER_PATH");
    unsetenv("LIBRARY_PATH");
}

int main(int argc, char **argv) {
    printf("Driver Reinitialization Test Program\n");
    printf("====================================\n\n");
    
    /* Test using multiple processes (most reliable) */
    test_via_processes();
    
    /* Test with environment variable changes */
    test_with_env_vars();
    
#ifdef TEST_WITH_DLOPEN
    /* Test via dlopen if enabled */
    test_via_dlopen();
#endif
    
#ifdef COMPILE_WITH_DRIVER
    /* Test direct calls if compiled with driver */
    test_direct_driver_calls();
#endif
    
    printf("\n====================================\n");
    printf("All tests completed.\n");
    printf("Check coverage report for gcc.cc lines 11228-11250\n");
    
    return 0;
}
