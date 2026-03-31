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

/* Define this if testing with direct library calls */
/* #define USE_DIRECT_LIBRARY_CALL */

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

/* Method A: Process-based invocation using fork/exec */
void test_via_processes() {
    printf("=== Testing via fork/exec processes ===\n");
    
    char *gcc_path = "gcc";
    char *input_file = "test_input.c";
    
    /* Create test source file */
    if (create_test_file(input_file) != 0) {
        fprintf(stderr, "Failed to create test file\n");
        return;
    }
    
    /* Array of test invocations with different flags */
    char *test_invocations[][20] = {
        /* First: Set various flags including sysroot and dump options */
        {gcc_path, "-save-temps", "-dumpdir", "/tmp/test_dump1", 
         "-dumpbase", "test_dumpbase1", "-dumpbase-ext", ".c",
         "-c", input_file, "-o", "output1.o", 
         "--sysroot=/tmp/fake_sysroot", "-march=x86-64", NULL},
        
        /* Second: Minimal invocation to trigger reset to defaults */
        {gcc_path, "-c", input_file, "-o", "output2.o", NULL},
        
        /* Third: Force failure with invalid option */
        {gcc_path, "-invalid-option-that-does-not-exist", input_file, NULL},
        
        /* Fourth: Successful compilation again */
        {gcc_path, "-c", input_file, "-o", "output3.o", 
         "-mtune=generic", NULL},
        
        /* Fifth: Another with different dump options */
        {gcc_path, "-save-temps=obj", "-dumpdir", "/tmp/test_dump2",
         "-fdump-tree-all", "-c", input_file, "-o", "output4.o", NULL},
        
        /* Sixth: Back to minimal to trigger another reset */
        {gcc_path, "-c", input_file, "-o", "output5.o", NULL}
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
            execvp(gcc_path, test_invocations[i]);
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
        char filename[20];
        snprintf(filename, sizeof(filename), "output%d.o", i);
        unlink(filename);
        
        /* Also clean up any .i, .s files from -save-temps */
        snprintf(filename, sizeof(filename), "output%d.i", i);
        unlink(filename);
        snprintf(filename, sizeof(filename), "output%d.s", i);
        unlink(filename);
    }
}

#ifdef USE_DIRECT_LIBRARY_CALL
/* Method B: Direct library calls using dlopen/dlsym */
void test_via_library() {
    printf("\n=== Testing via direct library calls ===\n");
    
    void *handle = dlopen("./libgccdriver.so", RTLD_LAZY | RTLD_LOCAL);
    if (!handle) {
        fprintf(stderr, "dlopen failed: %s\n", dlerror());
        fprintf(stderr, "Trying with libgccjit.so...\n");
        handle = dlopen("libgccjit.so", RTLD_LAZY | RTLD_LOCAL);
        if (!handle) {
            fprintf(stderr, "Cannot load GCC driver library\n");
            return;
        }
    }
    
    /* Look for driver main function - actual symbol name may vary */
    int (*driver_main)(int, char**) = dlsym(handle, "main");
    if (!driver_main) {
        driver_main = dlsym(handle, "gcc_driver_main");
    }
    if (!driver_main) {
        driver_main = dlsym(handle, "driver::main");
    }
    
    if (!driver_main) {
        fprintf(stderr, "Could not find driver main function: %s\n", dlerror());
        dlclose(handle);
        return;
    }
    
    /* Create test source file */
    char *input_file = "test_input_lib.c";
    if (create_test_file(input_file) != 0) {
        fprintf(stderr, "Failed to create test file\n");
        dlclose(handle);
        return;
    }
    
    /* Test 1: With various flags */
    char *args1[] = {
        "gcc", "-save-temps", "-dumpdir", "/tmp/lib_test1",
        "-dumpbase", "libtest", "-c", input_file, 
        "-o", "lib_output1.o", "--sysroot=/tmp/alt_sysroot",
        NULL
    };
    printf("Test 1: With flags\n");
    int result1 = driver_main(10, args1);
    printf("Result: %d\n", result1);
    
    /* Test 2: Minimal to trigger reset */
    char *args2[] = {
        "gcc", "-c", input_file, "-o", "lib_output2.o", NULL
    };
    printf("\nTest 2: Minimal (should trigger reset)\n");
    int result2 = driver_main(4, args2);
    printf("Result: %d\n", result2);
    
    /* Test 3: Force failure */
    char *args3[] = {
        "gcc", "-invalid-flag-for-failure", input_file, NULL
    };
    printf("\nTest 3: Invalid flag (should fail)\n");
    int result3 = driver_main(3, args3);
    printf("Result: %d\n", result3);
    
    /* Test 4: Success again */
    char *args4[] = {
        "gcc", "-c", input_file, "-o", "lib_output3.o",
        "-march=native", NULL
    };
    printf("\nTest 4: Success again\n");
    int result4 = driver_main(5, args4);
    printf("Result: %d\n", result4);
    
    /* Cleanup */
    unlink(input_file);
    for (int i = 1; i <= 3; i++) {
        char filename[30];
        snprintf(filename, sizeof(filename), "lib_output%d.o", i);
        unlink(filename);
    }
    
    dlclose(handle);
}
#endif

/* Alternative: Direct compilation with driver code */
#ifdef DRIVER_TEST
/* This would require linking with gcc driver object files */
extern int driver_main(int argc, char **argv);

void test_direct_linking() {
    printf("\n=== Testing via direct linking ===\n");
    
    char *input_file = "test_input_direct.c";
    if (create_test_file(input_file) != 0) {
        fprintf(stderr, "Failed to create test file\n");
        return;
    }
    
    /* Multiple invocations to trigger reinitialization */
    for (int i = 0; i < 3; i++) {
        char output_file[30];
        snprintf(output_file, sizeof(output_file), "direct_output%d.o", i);
        
        char *args[] = {
            "gcc",
            (i == 0) ? "-save-temps" : "-c",
            "-dumpdir", "/tmp/direct_test",
            "-dumpbase", "direct_base",
            "-c", input_file,
            "-o", output_file,
            (i == 0) ? "--sysroot=/tmp/direct_sysroot" : NULL,
            NULL
        };
        
        int argc = (i == 0) ? 10 : 8;
        printf("Invocation %d\n", i);
        int result = driver_main(argc, args);
        printf("Result: %d\n", result);
    }
    
    unlink(input_file);
}
#endif

int main(int argc, char **argv) {
    printf("GCC Driver Reinitialization Test Program\n");
    printf("========================================\n");
    
    /* Test using process-based method (most reliable) */
    test_via_processes();
    
#ifdef USE_DIRECT_LIBRARY_CALL
    /* Test using direct library calls if enabled */
    test_via_library();
#endif
    
#ifdef DRIVER_TEST
    /* Test via direct linking if enabled */
    test_direct_linking();
#endif
    
    printf("\n=== Test Complete ===\n");
    
    /* Clean up any remaining temp files */
    system("rm -f test_input*.c output*.o output*.i output*.s "
           "lib_output*.o direct_output*.o "
           "/tmp/test_dump1* /tmp/test_dump2* 2>/dev/null");
    
    return 0;
}
