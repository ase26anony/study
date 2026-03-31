/* test_driver_reset.c - Program to test GCC driver reinitialization logic */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <dlfcn.h>
#include <errno.h>

/* Define this if testing with direct library calls */
/* #define USE_DIRECT_LIBRARY_CALL */

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

#ifdef USE_DIRECT_LIBRARY_CALL
/* Method B: Direct library call using dlopen/dlsym */
void invoke_driver_library(const char *lib_path, int argc, char *argv[]) {
    void *handle = dlopen(lib_path, RTLD_LAZY | RTLD_LOCAL);
    if (!handle) {
        fprintf(stderr, "dlopen failed: %s\n", dlerror());
        return;
    }
    
    /* Look for main function or driver entry point */
    int (*driver_main)(int, char**) = dlsym(handle, "main");
    if (!driver_main) {
        /* Try alternative entry points */
        driver_main = dlsym(handle, "driver::main");
        if (!driver_main) {
            driver_main = dlsym(handle, "_Z6driver4main");
        }
    }
    
    if (driver_main) {
        printf("Calling driver main directly\n");
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
    const char *temp_dir = "/tmp/gcc_test";
    
    /* Create test directory */
    mkdir(temp_dir, 0755);
    
    /* Create minimal source file */
    create_test_source(source_file);
    
    printf("=== Testing GCC Driver Reinitialization ===\n\n");
    
    /* Stage 1: First invocation with various flags to set state */
    printf("Stage 1: Setting driver state with various flags\n");
    char *args1[] = {
        (char*)driver_path,
        "-save-temps",           /* Sets save_temps_flag */
        "-dumpdir", (char*)temp_dir, /* Sets dumpdir */
        "-dumpbase", "stage1",   /* Sets dumpbase */
        "--sysroot=/alt/sysroot", /* Alters target_system_root */
        "-march=x86-64",         /* Changes spec_machine */
        "-c", (char*)source_file,
        "-o", "output1.o",
        "-v",                   /* verbose flag */
        NULL
    };
    invoke_driver_process(driver_path, args1);
    
    /* Stage 2: Second invocation without special flags - should trigger reset */
    printf("\nStage 2: Invocation without special flags (should trigger reset)\n");
    char *args2[] = {
        (char*)driver_path,
        "-c", (char*)source_file,
        "-o", "output2.o",
        NULL
    };
    invoke_driver_process(driver_path, args2);
    
    /* Stage 3: Third invocation with invalid option to force failure */
    printf("\nStage 3: Invalid option to set greatest_status\n");
    char *args3[] = {
        (char*)driver_path,
        "-invalid-option-that-does-not-exist",
        "-c", (char*)source_file,
        "-o", "output3.o",
        NULL
    };
    invoke_driver_process(driver_path, args3);
    
    /* Stage 4: Fourth invocation - successful compilation */
    printf("\nStage 4: Successful compilation (testing status reset)\n");
    char *args4[] = {
        (char*)driver_path,
        "-c", (char*)source_file,
        "-o", "output4.o",
        NULL
    };
    invoke_driver_process(driver_path, args4);
    
    /* Stage 5: Test with different dumpbase and outbase */
    printf("\nStage 5: Testing dumpbase/outbase reset\n");
    char *args5[] = {
        (char*)driver_path,
        "-save-temps=obj",
        "-dumpbase", "custom_base",
        "-dumpbase_ext", ".ext",
        "-o", "final_output",
        (char*)source_file,
        NULL
    };
    invoke_driver_process(driver_path, args5);
    
    /* Stage 6: Final invocation with minimal args to ensure full reset */
    printf("\nStage 6: Minimal invocation to verify complete reset\n");
    char *args6[] = {
        (char*)driver_path,
        "--version",  /* Triggers print_version logic */
        NULL
    };
    invoke_driver_process(driver_path, args6);
    
#ifdef USE_DIRECT_LIBRARY_CALL
    /* Alternative: Direct library calls if driver is built as shared library */
    printf("\n=== Testing with direct library calls ===\n");
    
    /* You would need to build the driver as a shared library first */
    const char *lib_path = "./libgccdriver.so";
    
    /* Create argument arrays for library calls */
    char *lib_args1[] = { "gcc", "-save-temps", "-c", source_file, "-o", "lib1.o", NULL };
    char *lib_args2[] = { "gcc", "-c", source_file, "-o", "lib2.o", NULL };
    
    invoke_driver_library(lib_path, 6, lib_args1);
    invoke_driver_library(lib_path, 5, lib_args2);
#endif
    
    /* Cleanup */
    unlink(source_file);
    printf("\nTest completed. Check coverage data for gcc.cc lines 11228-11250\n");
    
    return 0;
}
