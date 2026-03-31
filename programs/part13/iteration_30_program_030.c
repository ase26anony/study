/* test_driver_reinit.c - Program to test GCC driver reinitialization logic */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <dlfcn.h>
#include <errno.h>

/* Define this if you want to test via direct library calls */
/* #define USE_DIRECT_LIBRARY_CALL */

/* Simple test C source file content */
const char *test_source = 
"#include <stdio.h>\n"
"int main() {\n"
"    printf(\"Hello from test program\\n\");\n"
"    return 0;\n"
"}\n";

/* Create a test source file */
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
    
    /* Create test source file */
    if (!create_test_file("test_input.c")) {
        return;
    }
    
    /* Array of test invocations with different flags */
    char *test_invocations[][20] = {
        /* First: Set various flags including sysroot and dump options */
        {"gcc", "-save-temps", "-dumpdir", "/tmp/test_dump1", 
         "-dumpbase", "test_dumpbase", "-c", "test_input.c", 
         "-o", "test_output1.o", "--sysroot=/usr/alt", 
         "-specs=/usr/share/gcc/specs/default.spec", NULL},
        
        /* Second: Minimal invocation to trigger reset to defaults */
        {"gcc", "-c", "test_input.c", "-o", "test_output2.o", NULL},
        
        /* Third: Force failure with invalid option */
        {"gcc", "-invalid-option-that-does-not-exist", "test_input.c", NULL},
        
        /* Fourth: Successful compilation again */
        {"gcc", "-c", "test_input.c", "-o", "test_output3.o", NULL},
        
        /* Fifth: Another with different dump options */
        {"gcc", "-save-temps=obj", "-dumpdir", "/tmp/test_dump2",
         "-dumpbase", "another_base", "-c", "test_input.c",
         "-o", "test_output4.o", NULL},
        
        /* Sixth: Back to minimal to trigger another reset */
        {"gcc", "-c", "test_input.c", "-o", "test_output5.o", NULL}
    };
    
    int num_tests = sizeof(test_invocations) / sizeof(test_invocations[0]);
    int statuses[num_tests];
    
    for (int i = 0; i < num_tests; i++) {
        printf("\n--- Test %d: ", i + 1);
        for (int j = 0; test_invocations[i][j] != NULL; j++) {
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
            
            /* Small delay to ensure clean separation between invocations */
            usleep(10000);
        } else {
            perror("fork failed");
            statuses[i] = -1;
        }
    }
    
    printf("\n=== Summary of exit statuses ===\n");
    for (int i = 0; i < num_tests; i++) {
        printf("Test %d: %d\n", i + 1, statuses[i]);
    }
    
    /* Cleanup */
    unlink("test_input.c");
    for (int i = 1; i <= 5; i++) {
        char filename[50];
        snprintf(filename, sizeof(filename), "test_output%d.o", i);
        unlink(filename);
    }
}

#ifdef USE_DIRECT_LIBRARY_CALL
/* Method B: Direct library call testing using dlopen/dlsym */
void test_via_library_calls() {
    printf("\n=== Testing via direct library calls ===\n");
    
    /* Try to load the GCC driver as a shared library */
    void *handle = dlopen("libgccdriver.so", RTLD_LAZY | RTLD_LOCAL);
    if (!handle) {
        /* Try alternative names */
        handle = dlopen("./libgccdriver.so", RTLD_LAZY | RTLD_LOCAL);
    }
    
    if (!handle) {
        printf("Could not load libgccdriver.so: %s\n", dlerror());
        printf("Falling back to process-based testing only.\n");
        return;
    }
    
    /* Find the main function */
    int (*driver_main)(int, char**) = dlsym(handle, "main");
    if (!driver_main) {
        /* Try alternative entry point */
        driver_main = dlsym(handle, "driver::main");
    }
    
    if (!driver_main) {
        printf("Could not find driver main function: %s\n", dlerror());
        dlclose(handle);
        return;
    }
    
    /* Create test source file */
    if (!create_test_file("test_lib_input.c")) {
        dlclose(handle);
        return;
    }
    
    /* Test argument vectors for direct calls */
    char *test1_args[] = {
        "gcc", "-save-temps", "-dumpdir", "/tmp/lib_test1",
        "-dumpbase", "lib_test", "-c", "test_lib_input.c",
        "-o", "lib_output1.o", "--sysroot=/usr/alt", NULL
    };
    
    char *test2_args[] = {
        "gcc", "-c", "test_lib_input.c", "-o", "lib_output2.o", NULL
    };
    
    char *test3_args[] = {
        "gcc", "-invalid-option-force-failure", NULL
    };
    
    char *test4_args[] = {
        "gcc", "-c", "test_lib_input.c", "-o", "lib_output3.o", NULL
    };
    
    /* Execute the tests */
    printf("\nTest 1 (with flags): ");
    int result1 = driver_main(11, test1_args);
    printf("Result: %d\n", result1);
    
    printf("Test 2 (minimal): ");
    int result2 = driver_main(5, test2_args);
    printf("Result: %d\n", result2);
    
    printf("Test 3 (force failure): ");
    int result3 = driver_main(2, test3_args);
    printf("Result: %d\n", result3);
    
    printf("Test 4 (success again): ");
    int result4 = driver_main(5, test4_args);
    printf("Result: %d\n", result4);
    
    /* Cleanup */
    dlclose(handle);
    unlink("test_lib_input.c");
    unlink("lib_output1.o");
    unlink("lib_output2.o");
    unlink("lib_output3.o");
}
#endif

/* Alternative: Compile and link directly with driver code */
#ifdef DRIVER_TEST
/* This would require linking with gcc driver object files */
extern int driver_main(int argc, char **argv);

void test_via_direct_link() {
    printf("\n=== Testing via direct linking ===\n");
    
    if (!create_test_file("test_direct_input.c")) {
        return;
    }
    
    /* Multiple invocations to trigger reinitialization */
    char *args1[] = {"gcc", "-save-temps", "-dumpdir", "/tmp/direct1",
                     "-dumpbase", "direct_test", "-c", "test_direct_input.c",
                     "-o", "direct1.o", "--sysroot=/opt/special", NULL};
    
    char *args2[] = {"gcc", "-c", "test_direct_input.c", "-o", "direct2.o", NULL};
    
    char *args3[] = {"gcc", "-nonexistent-flag-to-fail", NULL};
    
    char *args4[] = {"gcc", "-c", "test_direct_input.c", "-o", "direct3.o", NULL};
    
    printf("Invocation 1 (with special flags):\n");
    int r1 = driver_main(11, args1);
    printf("Return: %d\n\n", r1);
    
    printf("Invocation 2 (minimal, should reset):\n");
    int r2 = driver_main(5, args2);
    printf("Return: %d\n\n", r2);
    
    printf("Invocation 3 (force error):\n");
    int r3 = driver_main(2, args3);
    printf("Return: %d\n\n", r3);
    
    printf("Invocation 4 (success again):\n");
    int r4 = driver_main(5, args4);
    printf("Return: %d\n", r4);
    
    /* Cleanup */
    unlink("test_direct_input.c");
    unlink("direct1.o");
    unlink("direct2.o");
    unlink("direct3.o");
}
#endif

int main(int argc, char **argv) {
    printf("GCC Driver Reinitialization Test Program\n");
    printf("=========================================\n\n");
    
    /* Create necessary directories */
    mkdir("/tmp/test_dump1", 0755);
    mkdir("/tmp/test_dump2", 0755);
    mkdir("/tmp/lib_test1", 0755);
    
    /* Test using processes (most reliable) */
    test_via_processes();
    
#ifdef USE_DIRECT_LIBRARY_CALL
    /* Test using direct library calls if enabled */
    test_via_library_calls();
#endif
    
#ifdef DRIVER_TEST
    /* Test via direct linking if enabled */
    test_via_direct_link();
#endif
    
    printf("\n=== Test completed ===\n");
    
    /* Cleanup temporary directories */
    rmdir("/tmp/test_dump1");
    rmdir("/tmp/test_dump2");
    rmdir("/tmp/lib_test1");
    
    return 0;
}
