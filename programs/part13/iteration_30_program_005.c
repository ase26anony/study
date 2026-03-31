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
void invoke_via_process(const char *driver_path, char *const argv[]) {
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
#ifdef USE_DIRECT_CALL
void invoke_via_dlopen(const char *lib_path, int argc, char *argv[]) {
    void *handle = dlopen(lib_path, RTLD_LAZY | RTLD_LOCAL);
    if (!handle) {
        fprintf(stderr, "dlopen failed: %s\n", dlerror());
        return;
    }
    
    /* Look for driver entry point - could be main() or driver::main() */
    int (*driver_main)(int, char**) = dlsym(handle, "main");
    if (!driver_main) {
        /* Try alternative names */
        driver_main = dlsym(handle, "_Z6driver4mainiPPc");  // mangled name for driver::main
    }
    
    if (driver_main) {
        printf("Calling driver directly (argc=%d)\n", argc);
        int result = driver_main(argc, argv);
        printf("Direct call returned: %d\n", result);
    } else {
        fprintf(stderr, "Could not find driver entry point: %s\n", dlerror());
    }
    
    dlclose(handle);
}
#endif

int main(int argc, char *argv[]) {
    const char *driver_path = "gcc";  /* Use system gcc by default */
    const char *source_file = "test_input.c";
    const char *temp_dir = "/tmp/gcc_test";
    
    /* Create test directory */
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "mkdir -p %s", temp_dir);
    system(cmd);
    
    /* Create minimal source file */
    create_test_source(source_file);
    
    printf("=== Testing GCC Driver Reinitialization ===\n\n");
    
    /* Stage 1: Invoke with various flags to set global state */
    printf("Stage 1: Setting driver state with special flags\n");
    char *args1[] = {
        "gcc",
        "-save-temps",          /* Sets save_temps_flag */
        "-dumpdir", temp_dir,   /* Sets dumpdir */
        "-dumpbase", "stage1",  /* Sets dumpbase */
        "-dumpbase-ext", ".c",  /* Sets dumpbase_ext */
        "--sysroot=/alt/sysroot", /* Alters target_system_root */
        "-specs=/dev/null",     /* May affect spec_machine */
        "-march=native",        /* Affects machine spec */
        "-c", source_file,
        "-o", "output1.o",
        NULL
    };
    invoke_via_process(driver_path, args1);
    
    /* Stage 2: Invoke without special flags to trigger reset to defaults */
    printf("\nStage 2: Resetting to defaults (no special flags)\n");
    char *args2[] = {
        "gcc",
        "-c", source_file,
        "-o", "output2.o",
        NULL
    };
    invoke_via_process(driver_path, args2);
    
    /* Stage 3: Force failure to set greatest_status */
    printf("\nStage 3: Forcing failure with invalid option\n");
    char *args3[] = {
        "gcc",
        "-invalid-option-that-does-not-exist",
        "-c", source_file,
        NULL
    };
    invoke_via_process(driver_path, args3);
    
    /* Stage 4: Successful compilation to test greatest_status reset */
    printf("\nStage 4: Successful compilation after failure\n");
    char *args4[] = {
        "gcc",
        "-c", source_file,
        "-o", "output4.o",
        NULL
    };
    invoke_via_process(driver_path, args4);
    
    /* Stage 5: Test save_temps overrides */
    printf("\nStage 5: Testing save_temps overrides\n");
    char *args5[] = {
        "gcc",
        "-save-temps=obj",
        "-dumpdir", "/tmp/override",
        "-c", source_file,
        "-o", "output5.o",
        NULL
    };
    invoke_via_process(driver_path, args5);
    
    /* Stage 6: Another reset to trigger free() calls */
    printf("\nStage 6: Final reset to trigger memory cleanup\n");
    char *args6[] = {
        "gcc",
        "-c", source_file,
        "-o", "output6.o",
        NULL
    };
    invoke_via_process(driver_path, args6);
    
#ifdef USE_DIRECT_CALL
    /* Alternative: Direct library calls if driver is built as shared library */
    printf("\n=== Testing via Direct Library Calls ===\n");
    
    /* First call with flags */
    char *dl_args1[] = {"gcc", "-save-temps", "-c", source_file, "-o", "dl1.o", NULL};
    invoke_via_dlopen("./libgccdriver.so", 6, dl_args1);
    
    /* Second call without flags */
    char *dl_args2[] = {"gcc", "-c", source_file, "-o", "dl2.o", NULL};
    invoke_via_dlopen("./libgccdriver.so", 5, dl_args2);
#endif
    
    /* Cleanup */
    unlink(source_file);
    unlink("output1.o");
    unlink("output2.o");
    unlink("output4.o");
    unlink("output5.o");
    unlink("output6.o");
    
    /* Clean up temp files from -save-temps */
    system("rm -f *.i *.s *.o");
    
    printf("\n=== Test Complete ===\n");
    return 0;
}
