/* test_driver_reinit.c
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

/* Method selection - choose one approach */
#define USE_PROCESS_METHOD 1    /* Fork/exec approach */
#define USE_LIBRARY_METHOD 0    /* dlopen/dlsym approach */

/* Create a minimal C source file for compilation */
void create_test_source(const char *filename) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        perror("Failed to create test source");
        exit(1);
    }
    
    fprintf(f, "/* Test source for driver reinitialization */\n");
    fprintf(f, "#include <stdio.h>\n\n");
    fprintf(f, "int main() {\n");
    fprintf(f, "    printf(\"Hello from test program\\n\");\n");
    fprintf(f, "    return 0;\n");
    fprintf(f, "}\n");
    
    fclose(f);
    printf("Created test source: %s\n", filename);
}

/* Method A: Process-based invocation using fork/exec */
void invoke_via_process(const char *gcc_path, char *const argv[]) {
    pid_t pid = fork();
    
    if (pid == 0) {
        /* Child process */
        execv(gcc_path, argv);
        /* If execv fails */
        perror("execv failed");
        exit(127);
    } else if (pid > 0) {
        /* Parent process */
        int status;
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status)) {
            printf("Driver exited with status: %d\n", WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            printf("Driver terminated by signal: %d\n", WTERMSIG(status));
        }
    } else {
        perror("fork failed");
    }
}

/* Method B: Direct library call using dlopen/dlsym */
#ifdef USE_LIBRARY_METHOD
void invoke_via_library(const char *lib_path, int argc, char *argv[]) {
    void *handle;
    int (*driver_main)(int, char**);
    char *error;
    
    /* Open the driver library */
    handle = dlopen(lib_path, RTLD_LAZY | RTLD_LOCAL);
    if (!handle) {
        fprintf(stderr, "dlopen failed: %s\n", dlerror());
        return;
    }
    
    /* Clear any existing error */
    dlerror();
    
    /* Find the main function */
    driver_main = (int (*)(int, char**))dlsym(handle, "main");
    error = dlerror();
    if (error != NULL) {
        fprintf(stderr, "dlsym failed: %s\n", error);
        dlclose(handle);
        return;
    }
    
    /* Call the driver main function */
    printf("Calling driver main with %d arguments\n", argc);
    int result = driver_main(argc, argv);
    printf("Driver returned: %d\n", result);
    
    dlclose(handle);
}
#endif

int main(int argc, char *argv[]) {
    const char *test_source = "test_input.c";
    const char *gcc_path = "gcc";  /* Adjust if needed */
    
    /* Create a test source file */
    create_test_source(test_source);
    
    printf("=== Testing GCC Driver Reinitialization ===\n");
    printf("Targeting lines 11228-11250 in gcc.cc\n\n");
    
#if USE_PROCESS_METHOD
    /* Method A: Process-based testing */
    
    /* Stage 1: Invoke with various flags to set global state */
    printf("Stage 1: Setting up global state with special flags\n");
    char *stage1_args[] = {
        "gcc",
        "-save-temps",          /* Sets save_temps_flag */
        "-dumpdir", "/tmp/test_dump",  /* Sets dumpdir */
        "-dumpbase", "test_dumpbase",  /* Sets dumpbase */
        "-dumpbase-ext", ".c",  /* Sets dumpbase_ext */
        "--sysroot=/tmp/fake_sysroot", /* Alters target_system_root */
        "-specs=/dev/null",     /* May affect spec_machine */
        "-march=native",        /* May affect spec_machine */
        "-c", test_source,
        "-o", "output1.o",
        NULL
    };
    invoke_via_process(gcc_path, stage1_args);
    
    /* Stage 2: Invoke without special flags to trigger reset to defaults */
    printf("\nStage 2: Resetting to defaults (no special flags)\n");
    char *stage2_args[] = {
        "gcc",
        "-c", test_source,
        "-o", "output2.o",
        NULL
    };
    invoke_via_process(gcc_path, stage2_args);
    
    /* Stage 3: Force failure to set greatest_status */
    printf("\nStage 3: Forcing failure to set greatest_status\n");
    char *stage3_args[] = {
        "gcc",
        "-invalid-option-that-does-not-exist",
        "-c", test_source,
        NULL
    };
    invoke_via_process(gcc_path, stage3_args);
    
    /* Stage 4: Successful compilation to test status reset */
    printf("\nStage 4: Successful compilation to test status reset\n");
    char *stage4_args[] = {
        "gcc",
        "-c", test_source,
        "-o", "output4.o",
        NULL
    };
    invoke_via_process(gcc_path, stage4_args);
    
    /* Stage 5: Test with different dumpdir/dumpbase to ensure cleanup */
    printf("\nStage 5: Testing dumpdir/dumpbase cleanup\n");
    char *stage5_args[] = {
        "gcc",
        "-save-temps=obj",
        "-dumpdir", "/tmp/another_dump",
        "-dumpbase", "another_base",
        "-fdump-tree-original",  /* Force dump generation */
        "-c", test_source,
        "-o", "output5.o",
        NULL
    };
    invoke_via_process(gcc_path, stage5_args);
    
    /* Stage 6: Final invocation with minimal flags */
    printf("\nStage 6: Final minimal invocation\n");
    char *stage6_args[] = {
        "gcc",
        "-c", test_source,
        "-o", "output6.o",
        NULL
    };
    invoke_via_process(gcc_path, stage6_args);
    
#elif USE_LIBRARY_METHOD
    /* Method B: Library-based testing */
    #ifdef USE_LIBRARY_METHOD
    const char *driver_lib = "./libgccdriver.so";
    
    /* Similar argument arrays as above, but called via library */
    char *stage1_lib_args[] = {"gcc", "-save-temps", "-dumpdir", "/tmp/test", 
                               "-c", test_source, "-o", "output1.o", NULL};
    invoke_via_library(driver_lib, 8, stage1_lib_args);
    
    char *stage2_lib_args[] = {"gcc", "-c", test_source, "-o", "output2.o", NULL};
    invoke_via_library(driver_lib, 5, stage2_lib_args);
    #endif
#endif
    
    /* Cleanup */
    printf("\n=== Test Complete ===\n");
    printf("Created files:\n");
    system("ls -la output*.o test_input.c 2>/dev/null || true");
    
    /* Optional: Remove test files */
    /*
    unlink("test_input.c");
    unlink("output1.o");
    unlink("output2.o");
    unlink("output4.o");
    unlink("output5.o");
    unlink("output6.o");
    */
    
    return 0;
}
