/* test_driver_reinit.c - Test GCC driver reinitialization logic */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <dlfcn.h>
#include <errno.h>

/* Define if we want to test via direct library calls */
#define USE_DIRECT_LIBRARY_CALL 0

/* Create a minimal C source file for compilation */
void create_test_source(const char *filename) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        perror("fopen");
        exit(1);
    }
    fprintf(f, "/* Test source for driver reinitialization */\n");
    fprintf(f, "#include <stdio.h>\n\n");
    fprintf(f, "int main() {\n");
    fprintf(f, "    printf(\"Hello from test program\\n\");\n");
    fprintf(f, "    return 0;\n");
    fprintf(f, "}\n");
    fclose(f);
}

/* Method A: Process-based testing using fork/exec */
void test_via_processes() {
    printf("=== Testing via fork/exec (Process-based) ===\n");
    
    char *input_file = "test_input.c";
    create_test_source(input_file);
    
    /* Array of test invocations with different flags */
    char *test_cases[][10] = {
        /* First: Set various flags including sysroot and dump options */
        {"gcc", "-save-temps", "-dumpdir", "/tmp/gcc_test1", 
         "-dumpbase", "test_dump", "-c", input_file, 
         "-o", "output1.o", "--sysroot=/alt/sysroot", NULL},
        
        /* Second: Minimal compilation, should reset to defaults */
        {"gcc", "-c", input_file, "-o", "output2.o", NULL},
        
        /* Third: Force failure with invalid option */
        {"gcc", "-invalid-option-that-does-not-exist", input_file, NULL},
        
        /* Fourth: Successful compilation again */
        {"gcc", "-c", input_file, "-o", "output3.o", NULL},
        
        /* Fifth: Test with different machine specs */
        {"gcc", "-march=native", "-mtune=generic", "-c", input_file, 
         "-o", "output4.o", NULL},
        
        /* Sixth: Back to defaults */
        {"gcc", "-c", input_file, "-o", "output5.o", NULL},
    };
    
    int num_cases = sizeof(test_cases) / sizeof(test_cases[0]);
    int statuses[num_cases];
    
    for (int i = 0; i < num_cases; i++) {
        printf("\n--- Invocation %d ---\n", i + 1);
        printf("Args: ");
        for (int j = 0; test_cases[i][j] != NULL; j++) {
            printf("%s ", test_cases[i][j]);
        }
        printf("\n");
        
        pid_t pid = fork();
        if (pid == 0) {
            /* Child process */
            execvp("gcc", test_cases[i]);
            /* If execvp fails */
            perror("execvp");
            exit(127);
        } else if (pid > 0) {
            /* Parent process */
            waitpid(pid, &statuses[i], 0);
            printf("Exit status: %d\n", WEXITSTATUS(statuses[i]));
        } else {
            perror("fork");
            exit(1);
        }
    }
    
    /* Cleanup */
    unlink(input_file);
    for (int i = 1; i <= 5; i++) {
        char fname[20];
        snprintf(fname, sizeof(fname), "output%d.o", i);
        unlink(fname);
    }
    
    printf("\n=== Process-based test complete ===\n");
}

#if USE_DIRECT_LIBRARY_CALL
/* Method B: Direct library call testing (if driver is built as shared lib) */
void test_via_library() {
    printf("\n=== Testing via direct library calls ===\n");
    
    /* Try to load GCC driver as a shared library */
    void *handle = dlopen("./libgccdriver.so", RTLD_LAZY | RTLD_LOCAL);
    if (!handle) {
        /* Try common locations */
        handle = dlopen("/usr/lib/gcc/libgccdriver.so", RTLD_LAZY | RTLD_LOCAL);
    }
    
    if (!handle) {
        printf("Could not load GCC driver library: %s\n", dlerror());
        printf("Skipping library-based test.\n");
        return;
    }
    
    /* Look for main or driver entry point */
    int (*driver_main)(int, char**) = dlsym(handle, "main");
    if (!driver_main) {
        driver_main = dlsym(handle, "gcc_driver_main");
    }
    
    if (!driver_main) {
        printf("Could not find driver entry point: %s\n", dlerror());
        dlclose(handle);
        return;
    }
    
    /* Create test source */
    char *input_file = "libtest_input.c";
    create_test_source(input_file);
    
    /* Test case 1: With various flags */
    char *args1[] = {
        "gcc", "-save-temps", "-dumpdir", "/tmp/lib_test",
        "-dumpbase", "libdump", "-c", input_file,
        "-o", "lib_output1.o", "--sysroot=/test/sysroot",
        NULL
    };
    
    /* Test case 2: Minimal (should trigger reset) */
    char *args2[] = {
        "gcc", "-c", input_file, "-o", "lib_output2.o",
        NULL
    };
    
    /* Test case 3: Force failure */
    char *args3[] = {
        "gcc", "-nonexistent-flag-xyz", input_file,
        NULL
    };
    
    /* Test case 4: Success again */
    char *args4[] = {
        "gcc", "-c", input_file, "-o", "lib_output3.o",
        NULL
    };
    
    printf("Call 1: With save-temps and sysroot\n");
    int ret1 = driver_main(10, args1);
    printf("Return: %d\n", ret1);
    
    printf("\nCall 2: Minimal (should reset state)\n");
    int ret2 = driver_main(5, args2);
    printf("Return: %d\n", ret2);
    
    printf("\nCall 3: Invalid option (should fail)\n");
    int ret3 = driver_main(3, args3);
    printf("Return: %d\n", ret3);
    
    printf("\nCall 4: Valid again\n");
    int ret4 = driver_main(5, args4);
    printf("Return: %d\n", ret4);
    
    /* Cleanup */
    unlink(input_file);
    for (int i = 1; i <= 3; i++) {
        char fname[30];
        snprintf(fname, sizeof(fname), "lib_output%d.o", i);
        unlink(fname);
    }
    
    dlclose(handle);
    printf("\n=== Library-based test complete ===\n");
}
#endif

/* Alternative: Direct compilation with driver code */
#ifdef DRIVER_TEST
/* This would require linking with gcc driver object files */
extern int driver_main(int argc, char **argv);

void test_direct_linking() {
    printf("\n=== Testing via direct linking ===\n");
    
    char *input_file = "direct_input.c";
    create_test_source(input_file);
    
    /* Multiple invocations with different state */
    char *argv1[] = {"gcc", "-save-temps=obj", "-dumpbase", "direct", 
                     "-c", input_file, "-o", "direct1.o", NULL};
    char *argv2[] = {"gcc", "-c", input_file, "-o", "direct2.o", NULL};
    char *argv3[] = {"gcc", "-invalid-flag-test", NULL};
    char *argv4[] = {"gcc", "-c", input_file, "-o", "direct3.o", NULL};
    
    printf("Invocation 1 (with save-temps):\n");
    int ret1 = driver_main(7, argv1);
    printf("Status: %d\n", ret1);
    
    printf("\nInvocation 2 (minimal, should reset):\n");
    int ret2 = driver_main(5, argv2);
    printf("Status: %d\n", ret2);
    
    printf("\nInvocation 3 (should fail):\n");
    int ret3 = driver_main(2, argv3);
    printf("Status: %d\n", ret3);
    
    printf("\nInvocation 4 (should succeed):\n");
    int ret4 = driver_main(5, argv4);
    printf("Status: %d\n", ret4);
    
    /* Cleanup */
    unlink(input_file);
    for (int i = 1; i <= 3; i++) {
        char fname[20];
        snprintf(fname, sizeof(fname), "direct%d.o", i);
        unlink(fname);
    }
}
#endif

int main(int argc, char **argv) {
    printf("GCC Driver Reinitialization Test\n");
    printf("=================================\n");
    
    /* Create a dummy sysroot directory for testing */
    mkdir("/tmp/gcc_test1", 0755);
    mkdir("/alt/sysroot", 0755);
    
    /* Test using processes (most reliable) */
    test_via_processes();
    
#if USE_DIRECT_LIBRARY_CALL
    /* Test using direct library calls if enabled */
    test_via_library();
#endif
    
#ifdef DRIVER_TEST
    /* Test via direct linking if compiled with driver */
    test_direct_linking();
#endif
    
    /* Cleanup test directories */
    rmdir("/tmp/gcc_test1");
    rmdir("/alt/sysroot");
    
    printf("\nAll tests completed.\n");
    return 0;
}
