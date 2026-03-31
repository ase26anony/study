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

/* Create a minimal C source file for compilation */
void create_test_source(const char *filename) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        perror("fopen");
        exit(1);
    }
    fprintf(f, "/* Test source for driver reinitialization */\n");
    fprintf(f, "int main(void) {\n");
    fprintf(f, "    return 0;\n");
    fprintf(f, "}\n");
    fclose(f);
}

/* Method A: Process-based invocation using fork/exec */
void test_via_processes(void) {
    printf("=== Testing via fork/exec (Process-based) ===\n");
    
    char *gcc_path = "gcc";
    char *input_file = "test_input.c";
    int status;
    
    /* Create test source file */
    create_test_source(input_file);
    
    /* Test 1: First invocation with various flags to set state */
    printf("\n[Test 1] Setting driver state with flags...\n");
    char *argv1[] = {
        gcc_path,
        "-save-temps",
        "-dumpdir", "/tmp/driver_test1",
        "-dumpbase", "test_dump",
        "-c",
        input_file,
        "-o", "output1.o",
        "--sysroot=/alt/sysroot",
        "-march=x86-64",
        NULL
    };
    
    pid_t pid1 = fork();
    if (pid1 == 0) {
        execvp(gcc_path, argv1);
        perror("execvp");
        exit(1);
    } else if (pid1 > 0) {
        waitpid(pid1, &status, 0);
        printf("Exit status 1: %d\n", WEXITSTATUS(status));
    }
    
    /* Test 2: Second invocation without flags - should trigger reset */
    printf("\n[Test 2] Invoking without flags (triggering reset)...\n");
    char *argv2[] = {
        gcc_path,
        "-c",
        input_file,
        "-o", "output2.o",
        NULL
    };
    
    pid_t pid2 = fork();
    if (pid2 == 0) {
        execvp(gcc_path, argv2);
        perror("execvp");
        exit(1);
    } else if (pid2 > 0) {
        waitpid(pid2, &status, 0);
        printf("Exit status 2: %d\n", WEXITSTATUS(status));
    }
    
    /* Test 3: Third invocation with invalid option to force failure */
    printf("\n[Test 3] Forcing failure with invalid option...\n");
    char *argv3[] = {
        gcc_path,
        "-invalid-option-that-does-not-exist",
        input_file,
        NULL
    };
    
    pid_t pid3 = fork();
    if (pid3 == 0) {
        execvp(gcc_path, argv3);
        perror("execvp");
        exit(1);
    } else if (pid3 > 0) {
        waitpid(pid3, &status, 0);
        printf("Exit status 3: %d\n", WEXITSTATUS(status));
    }
    
    /* Test 4: Fourth invocation should succeed - testing status reset */
    printf("\n[Test 4] Successful compilation after failure...\n");
    char *argv4[] = {
        gcc_path,
        "-c",
        input_file,
        "-o", "output4.o",
        NULL
    };
    
    pid_t pid4 = fork();
    if (pid4 == 0) {
        execvp(gcc_path, argv4);
        perror("execvp");
        exit(1);
    } else if (pid4 > 0) {
        waitpid(pid4, &status, 0);
        printf("Exit status 4: %d\n", WEXITSTATUS(status));
    }
    
    /* Test 5: Multi-stage compilation simulation */
    printf("\n[Test 5] Multi-stage compilation pipeline...\n");
    
    /* Stage 1: Preprocessing with save-temps */
    char *argv5a[] = {
        gcc_path,
        "-E",
        "-save-temps=obj",
        "-dumpdir", "/tmp/stage1",
        input_file,
        "-o", "preprocessed.i",
        NULL
    };
    
    pid_t pid5a = fork();
    if (pid5a == 0) {
        execvp(gcc_path, argv5a);
        perror("execvp");
        exit(1);
    } else if (pid5a > 0) {
        waitpid(pid5a, &status, 0);
        printf("Preprocessing exit: %d\n", WEXITSTATUS(status));
    }
    
    /* Stage 2: Compilation with different dumpbase */
    char *argv5b[] = {
        gcc_path,
        "-S",
        "-dumpbase", "stage2_asm",
        "-dumpdir", "/tmp/stage2",
        "preprocessed.i",
        "-o", "assembly.s",
        NULL
    };
    
    pid_t pid5b = fork();
    if (pid5b == 0) {
        execvp(gcc_path, argv5b);
        perror("execvp");
        exit(1);
    } else if (pid5b > 0) {
        waitpid(pid5b, &status, 0);
        printf("Compilation exit: %d\n", WEXITSTATUS(status));
    }
    
    /* Stage 3: Assembly with no dump flags */
    char *argv5c[] = {
        gcc_path,
        "-c",
        "assembly.s",
        "-o", "final.o",
        NULL
    };
    
    pid_t pid5c = fork();
    if (pid5c == 0) {
        execvp(gcc_path, argv5c);
        perror("execvp");
        exit(1);
    } else if (pid5c > 0) {
        waitpid(pid5c, &status, 0);
        printf("Assembly exit: %d\n", WEXITSTATUS(status));
    }
    
    /* Cleanup */
    unlink(input_file);
    unlink("output1.o");
    unlink("output2.o");
    unlink("output4.o");
    unlink("preprocessed.i");
    unlink("assembly.s");
    unlink("final.o");
}

/* Method B: Direct library call using dlopen/dlsym */
#ifdef DRIVER_TEST
void test_via_library(void) {
    printf("\n=== Testing via dlopen (Library-based) ===\n");
    
    void *handle;
    int (*driver_main)(int, char**);
    char *error;
    
    /* Try to load GCC driver as shared library */
    handle = dlopen("./libgccdriver.so", RTLD_LAZY);
    if (!handle) {
        /* Try alternative names */
        handle = dlopen("libgccjit.so", RTLD_LAZY);
    }
    
    if (!handle) {
        printf("Could not load GCC driver library: %s\n", dlerror());
        printf("Skipping library-based test. Ensure GCC is built with -shared -fPIC\n");
        return;
    }
    
    /* Clear any existing error */
    dlerror();
    
    /* Find the driver main function */
    driver_main = (int (*)(int, char**))dlsym(handle, "main");
    if ((error = dlerror()) != NULL) {
        printf("Could not find main: %s\n", error);
        dlclose(handle);
        return;
    }
    
    /* Create test source */
    create_test_source("libtest.c");
    
    /* Test 1: With sysroot and dump flags */
    printf("\n[Lib Test 1] With sysroot and dump flags\n");
    char *argv1[] = {
        "gcc",
        "-save-temps",
        "-dumpdir", "/tmp/libtest",
        "-dumpbase", "libdump",
        "-c",
        "libtest.c",
        "-o", "libout1.o",
        "--sysroot=/test/sysroot",
        NULL
    };
    
    int argc1 = sizeof(argv1)/sizeof(argv1[0]) - 1;
    int result1 = driver_main(argc1, argv1);
    printf("Result 1: %d\n", result1);
    
    /* Test 2: Without flags - should trigger reset */
    printf("\n[Lib Test 2] Without flags (trigger reset)\n");
    char *argv2[] = {
        "gcc",
        "-c",
        "libtest.c",
        "-o", "libout2.o",
        NULL
    };
    
    int argc2 = sizeof(argv2)/sizeof(argv2[0]) - 1;
    int result2 = driver_main(argc2, argv2);
    printf("Result 2: %d\n", result2);
    
    /* Test 3: Force failure */
    printf("\n[Lib Test 3] Force failure\n");
    char *argv3[] = {
        "gcc",
        "-invalid-flag-for-failure",
        NULL
    };
    
    int argc3 = sizeof(argv3)/sizeof(argv3[0]) - 1;
    int result3 = driver_main(argc3, argv3);
    printf("Result 3: %d\n", result3);
    
    /* Test 4: Success after failure */
    printf("\n[Lib Test 4] Success after failure\n");
    char *argv4[] = {
        "gcc",
        "--version",
        NULL
    };
    
    int argc4 = sizeof(argv4)/sizeof(argv4[0]) - 1;
    int result4 = driver_main(argc4, argv4);
    printf("Result 4: %d\n", result4);
    
    /* Cleanup */
    dlclose(handle);
    unlink("libtest.c");
    unlink("libout1.o");
    unlink("libout2.o");
}
#endif

/* Alternative: Direct compilation with driver code */
#ifdef COMPILE_WITH_DRIVER
/* This would require linking with gcc.o and other driver objects */
extern int main(int argc, char **argv);

void test_direct_call(void) {
    printf("\n=== Testing via direct function call ===\n");
    
    create_test_source("direct_test.c");
    
    /* Multiple calls to driver main to trigger reinitialization */
    for (int i = 0; i < 3; i++) {
        char *argv[5];
        char arg0[] = "gcc";
        char arg1[] = "-c";
        char arg2[] = "direct_test.c";
        char output[20];
        
        sprintf(output, "direct_out%d.o", i);
        
        argv[0] = arg0;
        argv[1] = arg1;
        argv[2] = arg2;
        argv[3] = output;
        argv[4] = NULL;
        
        printf("Call %d: ", i+1);
        int result = main(4, argv);
        printf("Result: %d\n", result);
    }
    
    unlink("direct_test.c");
}
#endif

int main(int argc, char **argv) {
    printf("GCC Driver Reinitialization Test\n");
    printf("================================\n");
    
    /* Test using process-based method (most reliable) */
    test_via_processes();
    
#ifdef DRIVER_TEST
    /* Test using library method if enabled */
    test_via_library();
#endif
    
#ifdef COMPILE_WITH_DRIVER
    /* Test direct call if compiled with driver */
    test_direct_call();
#endif
    
    printf("\n=== Test Complete ===\n");
    printf("Check coverage data to verify lines 11228-11250 in gcc.cc were executed.\n");
    
    return 0;
}
