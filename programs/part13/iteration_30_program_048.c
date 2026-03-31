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
void invoke_driver_process(const char *driver_path, char *const argv[]) {
    pid_t pid = fork();
    if (pid == 0) {
        /* Child process */
        execv(driver_path, argv);
        perror("execv failed");
        exit(127);
    } else if (pid > 0) {
        /* Parent process */
        int status;
        waitpid(pid, &status, 0);
        printf("Driver exit status: %d\n", WEXITSTATUS(status));
    } else {
        perror("fork failed");
    }
}

/* Method B: Direct library call using dlopen/dlsym */
#ifdef USE_DIRECT_CALL
void invoke_driver_library(const char *lib_path, int argc, char *argv[]) {
    void *handle = dlopen(lib_path, RTLD_LAZY | RTLD_LOCAL);
    if (!handle) {
        fprintf(stderr, "dlopen failed: %s\n", dlerror());
        return;
    }
    
    /* Look for main or driver entry point */
    int (*driver_main)(int, char**) = dlsym(handle, "main");
    if (!driver_main) {
        /* Try alternative entry point names */
        driver_main = dlsym(handle, "driver::main");
        if (!driver_main) {
            driver_main = dlsym(handle, "_Z6driver4main");
        }
    }
    
    if (driver_main) {
        printf("Calling driver main directly...\n");
        int result = driver_main(argc, argv);
        printf("Driver returned: %d\n", result);
    } else {
        fprintf(stderr, "Could not find driver entry point: %s\n", dlerror());
    }
    
    dlclose(handle);
}
#endif

int main(int argc, char *argv[]) {
    const char *driver_path = "gcc";  /* Adjust if needed */
    const char *source_file = "test_input.c";
    
    /* Create test source file */
    create_test_source(source_file);
    
    printf("=== Testing GCC Driver Reinitialization ===\n\n");
    
    /* Test 1: First invocation with various flags to set state */
    printf("Test 1: Setting driver state with flags\n");
    char *args1[] = {
        "gcc",
        "-save-temps",          /* Sets save_temps_flag */
        "-dumpdir", "/tmp/test_dump",  /* Sets dumpdir */
        "-dumpbase", "test_dumpbase",  /* Sets dumpbase */
        "-dumpbase-ext", ".c",  /* Sets dumpbase_ext */
        "--sysroot=/alt/sysroot",  /* Changes target_system_root */
        "-specs=test.specs",    /* Could change spec_machine */
        "-march=native",        /* Could affect spec_machine */
        "-c", source_file,
        "-o", "output1.o",
        NULL
    };
    invoke_driver_process(driver_path, args1);
    
    /* Test 2: Second invocation with minimal flags to trigger reset */
    printf("\nTest 2: Minimal invocation to trigger reset to defaults\n");
    char *args2[] = {
        "gcc",
        "-c", source_file,
        "-o", "output2.o",
        NULL
    };
    invoke_driver_process(driver_path, args2);
    
    /* Test 3: Third invocation that fails to set greatest_status */
    printf("\nTest 3: Failing invocation to set greatest_status\n");
    char *args3[] = {
        "gcc",
        "-invalid-option-that-does-not-exist",
        "-c", source_file,
        NULL
    };
    invoke_driver_process(driver_path, args3);
    
    /* Test 4: Fourth invocation that succeeds, testing status reset */
    printf("\nTest 4: Successful invocation after failure\n");
    char *args4[] = {
        "gcc",
        "-c", source_file,
        "-o", "output4.o",
        NULL
    };
    invoke_driver_process(driver_path, args4);
    
    /* Test 5: Multi-stage compilation simulation */
    printf("\nTest 5: Multi-stage compilation pipeline\n");
    
    /* Stage 1: Preprocessing with save-temps */
    printf("Stage 1: Preprocessing\n");
    char *args5a[] = {
        "gcc",
        "-save-temps=obj",
        "-dumpdir", "stage1",
        "-E", source_file,
        "-o", "output.i",
        NULL
    };
    invoke_driver_process(driver_path, args5a);
    
    /* Stage 2: Compilation with different dumpbase */
    printf("\nStage 2: Compilation\n");
    char *args5b[] = {
        "gcc",
        "-dumpbase", "stage2_compile",
        "-dumpbase-ext", ".s",
        "-S", "output.i",
        "-o", "output.s",
        NULL
    };
    invoke_driver_process(driver_path, args5b);
    
    /* Stage 3: Assembly with sysroot */
    printf("\nStage 3: Assembly with sysroot\n");
    char *args5c[] = {
        "gcc",
        "--sysroot=/usr/local/custom",
        "-c", "output.s",
        "-o", "output5.o",
        NULL
    };
    invoke_driver_process(driver_path, args5c);
    
    /* Stage 4: Linking with no special flags (triggers reset) */
    printf("\nStage 4: Linking (reset to defaults)\n");
    char *args5d[] = {
        "gcc",
        "output5.o",
        "-o", "final_output",
        NULL
    };
    invoke_driver_process(driver_path, args5d);
    
#ifdef USE_DIRECT_CALL
    /* Alternative: Direct library calls if supported */
    printf("\n=== Testing Direct Library Calls ===\n");
    
    /* Create argument arrays for direct calls */
    char *lib_args1[] = {"gcc", "-c", source_file, "-o", "lib_output1.o", NULL};
    char *lib_args2[] = {"gcc", "--sysroot=/tmp", "-c", source_file, "-o", "lib_output2.o", NULL};
    char *lib_args3[] = {"gcc", "-c", source_file, "-o", "lib_output3.o", NULL};
    
    invoke_driver_library("libgccdriver.so", 5, lib_args1);
    invoke_driver_library("libgccdriver.so", 5, lib_args2);
    invoke_driver_library("libgccdriver.so", 5, lib_args3);
#endif
    
    /* Cleanup */
    unlink(source_file);
    unlink("output1.o");
    unlink("output2.o");
    unlink("output4.o");
    unlink("output.i");
    unlink("output.s");
    unlink("output5.o");
    unlink("final_output");
    
    printf("\n=== Test Complete ===\n");
    return 0;
}
