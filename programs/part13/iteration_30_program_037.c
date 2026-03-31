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

/* Simple C source file content for compilation */
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

/* Method A: Process-based testing using fork/exec */
void test_with_processes() {
    printf("=== Testing with fork/exec (Process-based) ===\n");
    
    const char* input_file = "test_input.c";
    if (!create_test_file(input_file)) {
        return;
    }
    
    /* Array of test invocations with different flags */
    char* test_invocations[][10] = {
        /* First: Set various flags including sysroot and dump options */
        {"gcc", "-save-temps", "-dumpdir", "/tmp/test_dump1", 
         "-dumpbase", "test_dumpbase", "-c", input_file, 
         "-o", "output1.o", NULL},
        
        /* Second: Reset to defaults (no special flags) */
        {"gcc", "-c", input_file, "-o", "output2.o", NULL},
        
        /* Third: Force failure with invalid option */
        {"gcc", "-invalid-option-that-does-not-exist", input_file, NULL},
        
        /* Fourth: Successful compilation again */
        {"gcc", "-c", input_file, "-o", "output3.o", NULL},
        
        /* Fifth: Test with --sysroot to modify target_system_root */
        {"gcc", "--sysroot=/tmp/fake_sysroot", "-c", input_file, 
         "-o", "output4.o", NULL},
        
        /* Sixth: Back to defaults (should trigger reset) */
        {"gcc", "-c", input_file, "-o", "output5.o", NULL},
        
        /* Seventh: Test with specs file if available */
        {"gcc", "-specs=/dev/null", "-c", input_file, 
         "-o", "output6.o", NULL},
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
            int status;
            waitpid(pid, &status, 0);
            statuses[i] = WEXITSTATUS(status);
            printf("Exit status: %d\n", statuses[i]);
        } else {
            perror("fork failed");
            statuses[i] = -1;
        }
    }
    
    /* Cleanup */
    unlink(input_file);
    for (int i = 1; i <= 6; i++) {
        char filename[20];
        snprintf(filename, sizeof(filename), "output%d.o", i);
        unlink(filename);
    }
    
    printf("\n=== Process-based test complete ===\n");
    printf("Expected pattern: success, success, failure, success, success, success, success\n");
}

/* Method B: Direct library call testing (if driver is built as shared library) */
#ifdef DRIVER_TEST
void test_with_dlopen() {
    printf("\n=== Testing with dlopen (Library-based) ===\n");
    
    /* Try to load GCC driver as shared library */
    void* handle = dlopen("libgccdriver.so", RTLD_LAZY);
    if (!handle) {
        /* Try alternative names */
        handle = dlopen("./libgccdriver.so", RTLD_LAZY);
    }
    
    if (!handle) {
        printf("Could not load libgccdriver.so: %s\n", dlerror());
        printf("Skipping dlopen test. Build GCC with -shared -fPIC to enable.\n");
        return;
    }
    
    /* Look for driver entry point - could be main() or driver::main() */
    int (*driver_main)(int, char**) = dlsym(handle, "main");
    if (!driver_main) {
        driver_main = dlsym(handle, "_Z6driveriPPc");  // mangled name for driver::main
    }
    
    if (!driver_main) {
        printf("Could not find driver entry point: %s\n", dlerror());
        dlclose(handle);
        return;
    }
    
    /* Create test source file */
    const char* input_file = "test_input_lib.c";
    if (!create_test_file(input_file)) {
        dlclose(handle);
        return;
    }
    
    /* Test 1: With save-temps and dumpdir */
    char* test1[] = {
        "gcc", "-save-temps", "-dumpdir", "/tmp/lib_test",
        "-dumpbase", "lib_test", "-c", "test_input_lib.c",
        "-o", "lib_output1.o", NULL
    };
    
    /* Test 2: Reset to defaults */
    char* test2[] = {
        "gcc", "-c", "test_input_lib.c", "-o", "lib_output2.o", NULL
    };
    
    /* Test 3: Invalid option to trigger failure */
    char* test3[] = {
        "gcc", "-this-option-does-not-exist-12345", NULL
    };
    
    /* Test 4: Successful compilation again */
    char* test4[] = {
        "gcc", "-c", "test_input_lib.c", "-o", "lib_output3.o", NULL
    };
    
    printf("Test 1 (with flags): ");
    int result1 = driver_main(10, test1);
    printf("Result: %d\n", result1);
    
    printf("Test 2 (defaults): ");
    int result2 = driver_main(4, test2);
    printf("Result: %d\n", result2);
    
    printf("Test 3 (invalid): ");
    int result3 = driver_main(2, test3);
    printf("Result: %d\n", result3);
    
    printf("Test 4 (success): ");
    int result4 = driver_main(4, test4);
    printf("Result: %d\n", result4);
    
    /* Cleanup */
    unlink(input_file);
    unlink("lib_output1.o");
    unlink("lib_output2.o");
    unlink("lib_output3.o");
    
    dlclose(handle);
    printf("=== Library-based test complete ===\n");
}
#endif

/* Method C: Direct compilation with driver internals */
#ifdef LINK_WITH_DRIVER
/* This would require linking with gcc.o and other driver objects */
extern int driver::main(int argc, char** argv);

void test_direct_linking() {
    printf("\n=== Testing with direct driver linking ===\n");
    
    /* Similar test sequences as above */
    char* test_args[] = {
        "gcc", "-save-temps", "-dumpdir", "/tmp/direct",
        "-dumpbase", "direct_test", "-c", "test.c",
        "-o", "direct.o", NULL
    };
    
    int result = driver::main(10, test_args);
    printf("First invocation result: %d\n", result);
    
    /* Second invocation with defaults */
    char* test_args2[] = {
        "gcc", "-c", "test.c", "-o", "direct2.o", NULL
    };
    
    result = driver::main(4, test_args2);
    printf("Second invocation result: %d\n", result);
}
#endif

int main(int argc, char** argv) {
    printf("GCC Driver Reinitialization Test Program\n");
    printf("=========================================\n");
    
    /* Always run process-based test */
    test_with_processes();
    
    /* Conditionally run library-based test */
#ifdef DRIVER_TEST
    test_with_dlopen();
#endif
    
#ifdef LINK_WITH_DRIVER
    test_direct_linking();
#endif
    
    printf("\nAll tests completed.\n");
    printf("Check coverage data to verify lines 11228-11250 in gcc.cc were executed.\n");
    
    return 0;
}
