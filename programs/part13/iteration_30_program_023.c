/* test_gcc_driver_reset.c
 * A program to test GCC driver reinitialization logic by invoking it multiple times
 * with different flags to trigger state resets between invocations.
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <errno.h>

/* Create a minimal C source file for compilation tests */
void create_test_source(const char *filename) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        perror("Failed to create test source");
        exit(1);
    }
    
    fprintf(f, "/* Test source for GCC driver reset testing */\n");
    fprintf(f, "#include <stdio.h>\n\n");
    fprintf(f, "int main() {\n");
    fprintf(f, "    printf(\"Hello from test program\\n\");\n");
    fprintf(f, "    return 0;\n");
    fprintf(f, "}\n");
    
    fclose(f);
}

/* Method A: Process-based invocation using fork/exec */
void invoke_gcc_process(const char **argv, int *status) {
    pid_t pid = fork();
    
    if (pid == 0) {
        /* Child process */
        execvp("gcc", (char *const *)argv);
        perror("execvp failed");
        exit(127);
    } else if (pid > 0) {
        /* Parent process */
        waitpid(pid, status, 0);
        if (WIFEXITED(*status)) {
            printf("GCC exited with status: %d\n", WEXITSTATUS(*status));
        } else if (WIFSIGNALED(*status)) {
            printf("GCC terminated by signal: %d\n", WTERMSIG(*status));
        }
    } else {
        perror("fork failed");
        exit(1);
    }
}

/* Method B: Direct library call using dlopen (if available) */
#ifdef USE_DIRECT_CALL
void invoke_gcc_direct(int argc, char **argv) {
    static void *gcc_lib = NULL;
    static int (*gcc_main)(int, char **) = NULL;
    
    if (!gcc_lib) {
        /* Try to load GCC driver as a shared library */
        gcc_lib = dlopen("./libgccdriver.so", RTLD_LAZY);
        if (!gcc_lib) {
            gcc_lib = dlopen("libgccjit.so", RTLD_LAZY);
        }
        if (!gcc_lib) {
            fprintf(stderr, "Failed to load GCC library: %s\n", dlerror());
            return;
        }
        
        /* Find the main entry point */
        gcc_main = (int (*)(int, char **))dlsym(gcc_lib, "main");
        if (!gcc_main) {
            gcc_main = (int (*)(int, char **))dlsym(gcc_lib, "gcc_main");
        }
        if (!gcc_main) {
            fprintf(stderr, "Failed to find GCC main: %s\n", dlerror());
            dlclose(gcc_lib);
            return;
        }
    }
    
    if (gcc_main) {
        printf("Calling GCC driver directly (argc=%d)\n", argc);
        int result = gcc_main(argc, argv);
        printf("Direct call returned: %d\n", result);
    }
}
#endif

/* Test sequence to trigger all uncovered lines */
void run_comprehensive_test(void) {
    printf("=== Starting GCC Driver Reinitialization Test ===\n\n");
    
    /* Create test source file */
    create_test_source("test_reset.c");
    
    /* Test 1: First invocation with various flags to set state */
    printf("Test 1: Setting driver state with various flags\n");
    printf("-------------------------------------------------\n");
    const char *test1_args[] = {
        "gcc",
        "-save-temps",          /* Sets save_temps_flag */
        "-dumpdir", "/tmp/gcc_test_dump",  /* Sets dumpdir */
        "-dumpbase", "test_dumpbase",      /* Sets dumpbase */
        "-dumpbase-ext", ".c",             /* Sets dumpbase_ext */
        "--sysroot=/tmp/fake_sysroot",     /* Sets target_system_root */
        "-specs=/tmp/test.specs",          /* May affect spec_machine */
        "-march=x86-64",                   /* Affects machine spec */
        "-mtune=generic",
        "-c", "test_reset.c",
        "-o", "test1.o",
        "-v",                              /* Sets verbose flag */
        "--help",                          /* May set print_help_list */
        NULL
    };
    
    int status1;
    invoke_gcc_process(test1_args, &status1);
    printf("\n");
    
    /* Test 2: Second invocation with minimal flags to trigger reset */
    printf("Test 2: Minimal invocation to trigger state reset\n");
    printf("-------------------------------------------------\n");
    const char *test2_args[] = {
        "gcc",
        "-c", "test_reset.c",
        "-o", "test2.o",
        NULL
    };
    
    int status2;
    invoke_gcc_process(test2_args, &status2);
    printf("\n");
    
    /* Test 3: Third invocation with invalid option to set greatest_status */
    printf("Test 3: Invalid option to test greatest_status\n");
    printf("-------------------------------------------------\n");
    const char *test3_args[] = {
        "gcc",
        "-invalid-option-that-does-not-exist",
        "-another-bad-flag",
        NULL
    };
    
    int status3;
    invoke_gcc_process(test3_args, &status3);
    printf("\n");
    
    /* Test 4: Fourth invocation to test status reset after failure */
    printf("Test 4: Valid compilation after failure\n");
    printf("-------------------------------------------------\n");
    const char *test4_args[] = {
        "gcc",
        "-c", "test_reset.c",
        "-o", "test4.o",
        NULL
    };
    
    int status4;
    invoke_gcc_process(test4_args, &status4);
    printf("\n");
    
    /* Test 5: Multi-stage compilation simulation */
    printf("Test 5: Simulating multi-stage compilation\n");
    printf("-------------------------------------------------\n");
    
    /* Stage 1: Preprocessing with save-temps */
    const char *stage1_args[] = {
        "gcc",
        "-save-temps=obj",
        "-dumpdir", "stage1",
        "-E",  /* Preprocess only */
        "test_reset.c",
        "-o", "test_reset.i",
        NULL
    };
    
    int stage1_status;
    invoke_gcc_process(stage1_args, &stage1_status);
    
    /* Stage 2: Compilation with different dumpbase */
    const char *stage2_args[] = {
        "gcc",
        "-save-temps=cwd",
        "-dumpbase", "stage2_compile",
        "-S",  /* Compile to assembly */
        "test_reset.i",
        "-o", "test_reset.s",
        NULL
    };
    
    int stage2_status;
    invoke_gcc_process(stage2_args, &stage2_status);
    
    /* Stage 3: Assembly with no special flags */
    const char *stage3_args[] = {
        "gcc",
        "-c",  /* Assemble */
        "test_reset.s",
        "-o", "test_reset.o",
        NULL
    };
    
    int stage3_status;
    invoke_gcc_process(stage3_args, &stage3_status);
    
    /* Test 6: Test with outbase and different output formats */
    printf("\nTest 6: Testing outbase and output variations\n");
    printf("-------------------------------------------------\n");
    
    const char *test6_args[] = {
        "gcc",
        "-save-temps",
        "-dumpdir", "/tmp/final",
        "-dumpbase", "final_output",
        "-dumpbase-ext", ".final",
        "-x", "c",  /* Explicitly specify language */
        "test_reset.c",
        "-o", "test_reset",
        NULL
    };
    
    int status6;
    invoke_gcc_process(test6_args, &status6);
    
    /* Summary */
    printf("\n=== Test Summary ===\n");
    printf("Test 1 (state setting): %s\n", WIFEXITED(status1) ? "exited" : "signaled");
    printf("Test 2 (minimal reset): %s\n", WIFEXITED(status2) ? "exited" : "signaled");
    printf("Test 3 (invalid args):  %s\n", WIFEXITED(status3) ? "exited" : "signaled");
    printf("Test 4 (recovery):      %s\n", WIFEXITED(status4) ? "exited" : "signaled");
    printf("Test 5 stage1:          %s\n", WIFEXITED(stage1_status) ? "exited" : "signaled");
    printf("Test 5 stage2:          %s\n", WIFEXITED(stage2_status) ? "exited" : "signaled");
    printf("Test 5 stage3:          %s\n", WIFEXITED(stage3_status) ? "exited" : "signaled");
    printf("Test 6 (final):         %s\n", WIFEXITED(status6) ? "exited" : "signaled");
    
    /* Cleanup */
    unlink("test_reset.c");
    unlink("test_reset.i");
    unlink("test_reset.s");
    unlink("test_reset.o");
    unlink("test1.o");
    unlink("test2.o");
    unlink("test4.o");
    unlink("test_reset");
}

/* Alternative: Direct linking test if GCC driver is available as object files */
#ifdef LINK_WITH_DRIVER
/* This would require linking with gcc.o and other driver objects */
extern int main(int argc, char **argv);

void test_direct_linking(void) {
    printf("Testing via direct driver linking\n");
    
    /* First call with flags */
    char *args1[] = {
        "gcc",
        "-save-temps",
        "-dumpdir", "/tmp/direct",
        "-c", "test.c",
        "-o", "direct.o",
        NULL
    };
    
    printf("First direct call:\n");
    main(6, args1);
    
    /* Second call without flags */
    char *args2[] = {
        "gcc",
        "-c", "test.c",
        "-o", "direct2.o",
        NULL
    };
    
    printf("\nSecond direct call:\n");
    main(4, args2);
}
#endif

int main(int argc, char **argv) {
    printf("GCC Driver Reinitialization Test Program\n");
    printf("========================================\n\n");
    
    /* Check if we should use direct library calls */
    int use_direct = 0;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--direct") == 0) {
            use_direct = 1;
        }
    }
    
    if (use_direct) {
#ifdef USE_DIRECT_CALL
        printf("Using direct library call method\n");
        /* This would require GCC built as shared library */
        invoke_gcc_direct(argc, argv);
#else
        printf("Direct call not enabled in this build\n");
#endif
    } else {
        printf("Using process-based invocation method\n");
        run_comprehensive_test();
    }
    
    return 0;
}
