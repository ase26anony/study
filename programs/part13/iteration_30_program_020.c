/* test_gcc_driver_reset.c
 * A program to test GCC driver reinitialization logic
 * Specifically targets lines 11228-11250 in gcc.cc
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
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

/* Create a temporary test source file */
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
void test_via_processes(const char *gcc_path, const char *input_file) {
    int status;
    pid_t pid;
    
    printf("=== Testing GCC Driver Reinitialization via Processes ===\n\n");
    
    /* First invocation: Set various flags including sysroot and dump options */
    printf("1. First invocation with special flags...\n");
    pid = fork();
    if (pid == 0) {
        /* Child process */
        char *args[] = {
            (char *)gcc_path,
            "-save-temps",
            "-dumpdir", "/tmp/gcc_test_dump",
            "-dumpbase", "test_dumpbase",
            "-c", (char *)input_file,
            "-o", "output1.o",
            "--sysroot=/tmp/fake_sysroot",
            "-v",  /* Verbose to see what's happening */
            NULL
        };
        execv(gcc_path, args);
        perror("execv failed");
        exit(1);
    } else if (pid > 0) {
        waitpid(pid, &status, 0);
        printf("Exit status: %d\n\n", WEXITSTATUS(status));
    }
    
    /* Second invocation: Minimal flags to trigger reset to defaults */
    printf("2. Second invocation with minimal flags (trigger reset)...\n");
    pid = fork();
    if (pid == 0) {
        char *args[] = {
            (char *)gcc_path,
            "-c", (char *)input_file,
            "-o", "output2.o",
            NULL
        };
        execv(gcc_path, args);
        perror("execv failed");
        exit(1);
    } else if (pid > 0) {
        waitpid(pid, &status, 0);
        printf("Exit status: %d\n\n", WEXITSTATUS(status));
    }
    
    /* Third invocation: Force failure with invalid option */
    printf("3. Third invocation with invalid option (force failure)...\n");
    pid = fork();
    if (pid == 0) {
        char *args[] = {
            (char *)gcc_path,
            "-invalid-option-that-does-not-exist",
            "-c", (char *)input_file,
            NULL
        };
        execv(gcc_path, args);
        perror("execv failed");
        exit(1);
    } else if (pid > 0) {
        waitpid(pid, &status, 0);
        printf("Exit status: %d (should be non-zero)\n\n", WEXITSTATUS(status));
    }
    
    /* Fourth invocation: Successful compilation after failure */
    printf("4. Fourth invocation after failure (test status reset)...\n");
    pid = fork();
    if (pid == 0) {
        char *args[] = {
            (char *)gcc_path,
            "-c", (char *)input_file,
            "-o", "output3.o",
            NULL
        };
        execv(gcc_path, args);
        perror("execv failed");
        exit(1);
    } else if (pid > 0) {
        waitpid(pid, &status, 0);
        printf("Exit status: %d (should be zero)\n\n", WEXITSTATUS(status));
    }
    
    /* Fifth invocation: Test spec machine reset with architecture flags */
    printf("5. Fifth invocation with architecture flags then reset...\n");
    pid = fork();
    if (pid == 0) {
        char *args[] = {
            (char *)gcc_path,
            "-march=native",
            "-mtune=generic",
            "-c", (char *)input_file,
            "-o", "output4.o",
            NULL
        };
        execv(gcc_path, args);
        perror("execv failed");
        exit(1);
    } else if (pid > 0) {
        waitpid(pid, &status, 0);
        printf("Exit status: %d\n", WEXITSTATUS(status));
    }
}

#ifdef USE_DIRECT_LIBRARY_CALL
/* Method B: Direct library call testing using dlopen/dlsym */
void test_via_library(const char *libgcc_path) {
    printf("=== Testing GCC Driver Reinitialization via Library Calls ===\n\n");
    
    void *handle = dlopen(libgcc_path, RTLD_LAZY | RTLD_LOCAL);
    if (!handle) {
        fprintf(stderr, "Failed to load library: %s\n", dlerror());
        return;
    }
    
    /* Look for the driver's main function */
    int (*driver_main)(int, char**) = dlsym(handle, "main");
    if (!driver_main) {
        /* Try alternative names */
        driver_main = dlsym(handle, "gcc_main");
        if (!driver_main) {
            fprintf(stderr, "Failed to find driver main function: %s\n", dlerror());
            dlclose(handle);
            return;
        }
    }
    
    /* First call with special flags */
    printf("1. First library call with special flags...\n");
    char *args1[] = {
        "gcc",
        "-save-temps",
        "-dumpdir", "/tmp/lib_test",
        "-dumpbase", "lib_test",
        "--sysroot=/tmp/alt_sysroot",
        "-c", "test_input.c",
        "-o", "lib_output1.o",
        NULL
    };
    int argc1 = sizeof(args1)/sizeof(args1[0]) - 1;
    int status1 = driver_main(argc1, args1);
    printf("Return status: %d\n\n", status1);
    
    /* Second call with minimal flags */
    printf("2. Second library call with minimal flags...\n");
    char *args2[] = {
        "gcc",
        "-c", "test_input.c",
        "-o", "lib_output2.o",
        NULL
    };
    int argc2 = sizeof(args2)/sizeof(args2[0]) - 1;
    int status2 = driver_main(argc2, args2);
    printf("Return status: %d\n\n", status2);
    
    /* Third call with invalid option */
    printf("3. Third library call with invalid option...\n");
    char *args3[] = {
        "gcc",
        "-invalid-option-force-failure",
        NULL
    };
    int argc3 = sizeof(args3)/sizeof(args3[0]) - 1;
    int status3 = driver_main(argc3, args3);
    printf("Return status: %d (should be non-zero)\n\n", status3);
    
    /* Fourth call after failure */
    printf("4. Fourth library call after failure...\n");
    char *args4[] = {
        "gcc",
        "-c", "test_input.c",
        "-o", "lib_output3.o",
        NULL
    };
    int argc4 = sizeof(args4)/sizeof(args4[0]) - 1;
    int status4 = driver_main(argc4, args4);
    printf("Return status: %d (should be zero)\n", status4);
    
    dlclose(handle);
}
#endif

/* Alternative: Direct compilation with driver code */
#ifdef COMPILE_WITH_DRIVER
/* This would require linking with gcc driver object files */
extern int driver_main(int argc, char **argv);

void test_direct_linking() {
    printf("=== Testing via Direct Driver Linking ===\n\n");
    
    /* Similar test sequence as above */
    char *args1[] = {"gcc", "-save-temps", "-dumpdir", "/tmp/direct", 
                     "-c", "test.c", "-o", "direct1.o", NULL};
    driver_main(7, args1);
    
    char *args2[] = {"gcc", "-c", "test.c", "-o", "direct2.o", NULL};
    driver_main(4, args2);
}
#endif

int main(int argc, char **argv) {
    const char *input_file = "test_input.c";
    const char *gcc_path = "gcc";
    
    /* Override gcc path if provided */
    if (argc > 1) {
        gcc_path = argv[1];
    }
    
    /* Create test input file */
    if (!create_test_file(input_file)) {
        return 1;
    }
    
    printf("Testing GCC driver reinitialization logic\n");
    printf("Targeting lines 11228-11250 in gcc.cc\n");
    printf("========================================\n\n");
    
    /* Test using process-based method (most reliable) */
    test_via_processes(gcc_path, input_file);
    
#ifdef USE_DIRECT_LIBRARY_CALL
    /* Test using library calls if enabled */
    if (argc > 2) {
        test_via_library(argv[2]);
    }
#endif
    
    /* Cleanup temporary files */
    printf("\n=== Cleaning up temporary files ===\n");
    unlink(input_file);
    unlink("output1.o");
    unlink("output2.o");
    unlink("output3.o");
    unlink("output4.o");
    
    /* Also clean up any save-temps files */
    system("rm -f test_dumpbase.* /tmp/gcc_test_dump/* 2>/dev/null");
    
    printf("Test completed.\n");
    return 0;
}
