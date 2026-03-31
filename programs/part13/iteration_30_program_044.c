/* test_driver_reinit.c - Test GCC driver reinitialization logic */
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

/* Create a minimal input file for compilation */
void create_test_input(void) {
    FILE *f = fopen("input.c", "w");
    if (!f) {
        perror("Failed to create input.c");
        exit(1);
    }
    fprintf(f, "int main() { return 0; }\n");
    fclose(f);
}

/* Method A: Process-based testing using fork/exec */
void test_via_processes(void) {
    printf("=== Testing via fork/exec (Method A) ===\n");
    
    char *gcc_path = "gcc";
    char *input_file = "input.c";
    
    /* Test case 1: First invocation with special flags */
    char *argv1[] = {
        gcc_path,
        "-save-temps",
        "-dumpdir", "/tmp/test_driver_reinit",
        "-dumpbase", "test_dumpbase",
        "-c", input_file,
        "-o", "output1.o",
        "--sysroot=/tmp/fake_sysroot",
        "-specs=/dev/null",  /* Try to use a spec file */
        "-march=native",     /* Change machine spec */
        NULL
    };
    
    /* Test case 2: Second invocation with minimal flags (triggers reset) */
    char *argv2[] = {
        gcc_path,
        "-c", input_file,
        "-o", "output2.o",
        NULL
    };
    
    /* Test case 3: Third invocation that fails */
    char *argv4[] = {
        gcc_path,
        "-invalid-option-that-does-not-exist",
        NULL
    };
    
    /* Test case 4: Fourth invocation that succeeds */
    char *argv5[] = {
        gcc_path,
        "-c", input_file,
        "-o", "output3.o",
        NULL
    };
    
    char **test_cases[] = {argv1, argv2, argv4, argv5};
    int num_cases = 4;
    
    for (int i = 0; i < num_cases; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            /* Child process */
            execvp(gcc_path, test_cases[i]);
            perror("execvp failed");
            exit(127);
        } else if (pid > 0) {
            /* Parent process */
            int status;
            waitpid(pid, &status, 0);
            printf("Test %d exit status: %d\n", i+1, WEXITSTATUS(status));
        } else {
            perror("fork failed");
        }
    }
}

/* Method B: Direct library call if available */
#ifdef DRIVER_TEST
/* This would require linking with gcc driver object files */
extern int main(int argc, char **argv);

void test_via_direct_call(void) {
    printf("\n=== Testing via direct function call (Method B) ===\n");
    
    /* First call with special flags */
    char *argv1[] = {
        "gcc",
        "-save-temps",
        "-dumpdir", "/tmp/test_direct",
        "-dumpbase", "direct_test",
        "-c", "input.c",
        "-o", "direct1.o",
        "--sysroot=/tmp/alt_sysroot",
        NULL
    };
    
    int argc1 = sizeof(argv1)/sizeof(argv1[0]) - 1;
    int status1 = main(argc1, argv1);
    printf("Direct call 1 returned: %d\n", status1);
    
    /* Second call with minimal flags - triggers reset */
    char *argv2[] = {
        "gcc",
        "-c", "input.c",
        "-o", "direct2.o",
        NULL
    };
    
    int argc2 = sizeof(argv2)/sizeof(argv2[0]) - 1;
    int status2 = main(argc2, argv2);
    printf("Direct call 2 returned: %d\n", status2);
    
    /* Third call that fails */
    char *argv3[] = {
        "gcc",
        "-nonexistent-flag",
        NULL
    };
    
    int argc3 = sizeof(argv3)/sizeof(argv3[0]) - 1;
    int status3 = main(argc3, argv3);
    printf("Direct call 3 (should fail) returned: %d\n", status3);
    
    /* Fourth call that succeeds */
    char *argv4[] = {
        "gcc",
        "-c", "input.c",
        "-o", "direct3.o",
        NULL
    };
    
    int argc4 = sizeof(argv4)/sizeof(argv4[0]) - 1;
    int status4 = main(argc4, argv4);
    printf("Direct call 4 returned: %d\n", status4);
}
#endif

/* Method C: dlopen/dlsym approach */
void test_via_dlopen(void) {
    printf("\n=== Testing via dlopen (Method C) ===\n");
    
    /* Try to load gcc as a shared library */
    void *handle = dlopen("libgccdriver.so", RTLD_LAZY);
    if (!handle) {
        /* Try alternative names */
        handle = dlopen("gcc", RTLD_LAZY);
    }
    
    if (!handle) {
        printf("Could not load gcc as shared library: %s\n", dlerror());
        printf("This method requires gcc built as a shared library.\n");
        return;
    }
    
    /* Look for main function */
    int (*gcc_main)(int, char**) = dlsym(handle, "main");
    if (!gcc_main) {
        printf("Could not find main function: %s\n", dlerror());
        dlclose(handle);
        return;
    }
    
    /* Create fake sysroot directory for testing */
    system("mkdir -p /tmp/test_sysroot 2>/dev/null");
    
    /* First invocation with flags that set global state */
    char *dl_argv1[] = {
        "gcc",
        "-save-temps=obj",
        "-dumpdir", "/tmp/dl_test",
        "-dumpbase", "dl_base",
        "-dumpbase-ext", ".c",
        "-c", "input.c",
        "-o", "dl_output1.o",
        "--sysroot=/tmp/test_sysroot",
        "-mtune=generic",
        NULL
    };
    
    int dl_argc1 = sizeof(dl_argv1)/sizeof(dl_argv1[0]) - 1;
    printf("DL call 1 (with sysroot and dump flags):\n");
    int result1 = gcc_main(dl_argc1, dl_argv1);
    printf("Returned: %d\n", result1);
    
    /* Second invocation - minimal, should trigger reset */
    char *dl_argv2[] = {
        "gcc",
        "-c", "input.c",
        "-o", "dl_output2.o",
        NULL
    };
    
    int dl_argc2 = sizeof(dl_argv2)/sizeof(dl_argv2[0]) - 1;
    printf("\nDL call 2 (minimal, triggers reset):\n");
    int result2 = gcc_main(dl_argc2, dl_argv2);
    printf("Returned: %d\n", result2);
    
    /* Third invocation - force failure */
    char *dl_argv3[] = {
        "gcc",
        "-invalid-compiler-option",
        NULL
    };
    
    int dl_argc3 = sizeof(dl_argv3)/sizeof(dl_argv3[0]) - 1;
    printf("\nDL call 3 (should fail):\n");
    int result3 = gcc_main(dl_argc3, dl_argv3);
    printf("Returned: %d\n", result3);
    
    /* Fourth invocation - success after failure */
    char *dl_argv4[] = {
        "gcc",
        "-v",  /* Add verbose to see more driver activity */
        "-c", "input.c",
        "-o", "dl_output3.o",
        NULL
    };
    
    int dl_argc4 = sizeof(dl_argv4)/sizeof(dl_argv4[0]) - 1;
    printf("\nDL call 4 (success after failure):\n");
    int result4 = gcc_main(dl_argc4, dl_argv4);
    printf("Returned: %d\n", result4);
    
    dlclose(handle);
}

/* Additional test: Simulate multi-stage compilation pipeline */
void test_multi_stage_pipeline(void) {
    printf("\n=== Testing multi-stage compilation pipeline ===\n");
    
    char *stages[] = {
        "gcc -E -save-temps -dumpdir /tmp/pipeline input.c -o input.i",
        "gcc -S -dumpbase pipeline input.i -o input.s",
        "gcc -c input.s -o pipeline.o",
        "gcc pipeline.o -o pipeline_exec"
    };
    
    for (int i = 0; i < 4; i++) {
        printf("Stage %d: %s\n", i+1, stages[i]);
        int status = system(stages[i]);
        printf("Exit code: %d\n", WEXITSTATUS(status));
        
        /* Small delay to ensure cleanup */
        usleep(10000);
    }
}

int main(int argc, char **argv) {
    printf("GCC Driver Reinitialization Test Program\n");
    printf("========================================\n\n");
    
    /* Create test input file */
    create_test_input();
    
    /* Clean up any previous test files */
    system("rm -f output*.o direct*.o dl_output*.o *.i *.s pipeline* 2>/dev/null");
    
    /* Test using different methods */
    test_via_processes();
    
    #ifdef DRIVER_TEST
    test_via_direct_call();
    #endif
    
    test_via_dlopen();
    
    test_multi_stage_pipeline();
    
    /* Cleanup */
    printf("\n=== Cleaning up ===\n");
    system("rm -f input.c output*.o direct*.o dl_output*.o *.i *.s pipeline_exec 2>/dev/null");
    
    return 0;
}
