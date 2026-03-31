/* test_driver_reset.c - Program to test GCC driver reinitialization logic */
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
#define TEST_DIRECT_LIBRARY_CALL 0

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

/* Method A: Process-based testing using fork/exec */
void test_via_processes() {
    printf("=== Testing via fork/exec processes ===\n");
    
    const char *input_file = "test_input.c";
    if (!create_test_file(input_file)) {
        return;
    }
    
    /* Array of test invocations with different flags */
    char *test_invocations[][10] = {
        /* First: Set various flags including sysroot and dump options */
        {"gcc", "-save-temps", "-dumpdir", "/tmp/test_dump1", 
         "-dumpbase", "test_dumpbase", "-c", input_file, 
         "-o", "output1.o", NULL},
        
        /* Second: Minimal invocation to trigger reset to defaults */
        {"gcc", "-c", input_file, "-o", "output2.o", NULL},
        
        /* Third: Force failure with invalid option */
        {"gcc", "-invalid-option-that-does-not-exist", input_file, NULL},
        
        /* Fourth: Successful compilation again */
        {"gcc", "-c", input_file, "-o", "output3.o", NULL},
        
        /* Fifth: Test with different machine specs */
        {"gcc", "-march=native", "-mtune=generic", "-c", input_file, 
         "-o", "output4.o", NULL},
        
        /* Sixth: Back to minimal to trigger spec machine reset */
        {"gcc", "-c", input_file, "-o", "output5.o", NULL}
    };
    
    int num_tests = sizeof(test_invocations) / sizeof(test_invocations[0]);
    int status;
    pid_t pid;
    
    for (int i = 0; i < num_tests; i++) {
        printf("\nTest %d: ", i + 1);
        for (int j = 0; test_invocations[i][j] != NULL; j++) {
            printf("%s ", test_invocations[i][j]);
        }
        printf("\n");
        
        pid = fork();
        if (pid == 0) {
            /* Child process */
            execvp("gcc", test_invocations[i]);
            /* If execvp fails */
            perror("execvp failed");
            exit(127);
        } else if (pid > 0) {
            /* Parent process */
            waitpid(pid, &status, 0);
            if (WIFEXITED(status)) {
                printf("Exit status: %d\n", WEXITSTATUS(status));
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
        char filename[32];
        snprintf(filename, sizeof(filename), "output%d.o", i);
        unlink(filename);
    }
}

#if TEST_DIRECT_LIBRARY_CALL
/* Method B: Direct library call testing using dlopen/dlsym */
void test_via_library() {
    printf("\n=== Testing via direct library calls ===\n");
    
    /* Try to load GCC driver as a shared library */
    void *handle = dlopen("libgccdriver.so", RTLD_LAZY | RTLD_LOCAL);
    if (!handle) {
        /* Try alternative names */
        handle = dlopen("/usr/lib/gcc/x86_64-linux-gnu/*/libgccdriver.so", RTLD_LAZY);
    }
    
    if (!handle) {
        printf("Could not load GCC driver library: %s\n", dlerror());
        printf("This test requires GCC built with --enable-shared and appropriate linking\n");
        return;
    }
    
    /* Look for the main driver function */
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
    const char *input_file = "test_lib_input.c";
    if (!create_test_file(input_file)) {
        dlclose(handle);
        return;
    }
    
    /* Test 1: With various flags set */
    char *argv1[] = {
        "gcc", "-save-temps=obj", "-dumpdir", "./libtest_dump",
        "-dumpbase", "libtest", "-c", input_file,
        "-o", "lib_output1.o", NULL
    };
    int argc1 = sizeof(argv1)/sizeof(argv1[0]) - 1;
    
    printf("\nLibrary Test 1: With save-temps and dump options\n");
    int result1 = driver_main(argc1, argv1);
    printf("Result: %d\n", result1);
    
    /* Test 2: Minimal to trigger reset */
    char *argv2[] = {
        "gcc", "-c", input_file, "-o", "lib_output2.o", NULL
    };
    int argc2 = sizeof(argv2)/sizeof(argv2[0]) - 1;
    
    printf("\nLibrary Test 2: Minimal invocation (should reset state)\n");
    int result2 = driver_main(argc2, argv2);
    printf("Result: %d\n", result2);
    
    /* Test 3: Force failure */
    char *argv3[] = {
        "gcc", "-this-option-does-not-exist-12345", NULL
    };
    int argc3 = sizeof(argv3)/sizeof(argv3[0]) - 1;
    
    printf("\nLibrary Test 3: Invalid option (should fail)\n");
    int result3 = driver_main(argc3, argv3);
    printf("Result: %d\n", result3);
    
    /* Test 4: Successful compilation again */
    char *argv4[] = {
        "gcc", "-c", input_file, "-o", "lib_output3.o", NULL
    };
    int argc4 = sizeof(argv4)/sizeof(argv4[0]) - 1;
    
    printf("\nLibrary Test 4: Successful compilation (testing status reset)\n");
    int result4 = driver_main(argc4, argv4);
    printf("Result: %d\n", result4);
    
    /* Cleanup */
    unlink(input_file);
    for (int i = 1; i <= 3; i++) {
        char filename[32];
        snprintf(filename, sizeof(filename), "lib_output%d.o", i);
        unlink(filename);
    }
    
    dlclose(handle);
}
#endif

/* Alternative: Compile with driver object files directly */
#ifdef DRIVER_TEST
/* This would require linking with gcc driver object files */
extern int driver_main(int argc, char **argv);

void test_direct_linking() {
    printf("\n=== Testing via direct function calls ===\n");
    
    const char *input_file = "test_direct_input.c";
    if (!create_test_file(input_file)) {
        return;
    }
    
    /* Series of invocations to trigger the uncovered reset block */
    
    /* 1. First with various flags */
    char *argv1[] = {
        "gcc", "-save-temps", "-dumpdir", "/tmp/direct_test",
        "-dumpbase", "direct_base", "--sysroot=/opt/custom_sysroot",
        "-specs=custom.specs", "-c", input_file, "-o", "direct1.o", NULL
    };
    
    /* 2. Then minimal to trigger reset */
    char *argv2[] = {
        "gcc", "-c", input_file, "-o", "direct2.o", NULL
    };
    
    /* 3. Force failure */
    char *argv3[] = {
        "gcc", "-invalid-flag-xyz", NULL
    };
    
    /* 4. Successful again */
    char *argv4[] = {
        "gcc", "-c", input_file, "-o", "direct3.o", NULL
    };
    
    printf("Test 1: Complex flags\n");
    driver_main(11, argv1);
    
    printf("\nTest 2: Minimal (triggering reset)\n");
    driver_main(4, argv2);
    
    printf("\nTest 3: Invalid (testing greatest_status)\n");
    driver_main(2, argv3);
    
    printf("\nTest 4: Successful (testing status reset)\n");
    driver_main(4, argv4);
    
    unlink(input_file);
}
#endif

int main(int argc, char **argv) {
    printf("GCC Driver Reinitialization Test Program\n");
    printf("=========================================\n\n");
    
    /* Test using process-based approach (most reliable) */
    test_via_processes();
    
#if TEST_DIRECT_LIBRARY_CALL
    /* Test using direct library calls if enabled */
    test_via_library();
#endif
    
#ifdef DRIVER_TEST
    /* Test via direct linking if compiled with driver objects */
    test_direct_linking();
#endif
    
    printf("\n=== Test Complete ===\n");
    printf("Check coverage data to verify lines 11228-11250 in gcc.cc were executed.\n");
    
    return 0;
}
