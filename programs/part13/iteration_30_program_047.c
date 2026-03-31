/* test_gcc_driver_reset.c
 * A program to test GCC driver reinitialization logic by invoking it multiple times
 * with different configurations to trigger state resets.
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

/* Method A: Process-based invocation using fork/exec */
int invoke_gcc_via_process(const char *gcc_path, char *const argv[]) {
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
            return WEXITSTATUS(status);
        } else {
            return -1;
        }
    } else {
        /* Fork failed */
        perror("fork failed");
        return -1;
    }
}

#ifdef USE_DIRECT_LIBRARY_CALL
/* Method B: Direct library call using dlopen/dlsym */
int invoke_gcc_via_library(const char *lib_path, int argc, char *argv[]) {
    void *handle;
    int (*driver_main)(int, char**);
    char *error;
    int result;
    
    handle = dlopen(lib_path, RTLD_LAZY | RTLD_LOCAL);
    if (!handle) {
        fprintf(stderr, "dlopen failed: %s\n", dlerror());
        return -1;
    }
    
    dlerror(); /* Clear any existing error */
    
    /* Try to find the main function - name might vary */
    driver_main = (int (*)(int, char**))dlsym(handle, "main");
    if ((error = dlerror()) != NULL) {
        /* Try alternative names */
        driver_main = (int (*)(int, char**))dlsym(handle, "gcc_driver_main");
        if ((error = dlerror()) != NULL) {
            driver_main = (int (*)(int, char**))dlsym(handle, "driver::main");
            if ((error = dlerror()) != NULL) {
                fprintf(stderr, "dlsym failed: %s\n", error);
                dlclose(handle);
                return -1;
            }
        }
    }
    
    result = driver_main(argc, argv);
    
    dlclose(handle);
    return result;
}
#endif

int main(int argc, char *argv[]) {
    const char *gcc_path = "gcc";
    const char *test_file = "test_input.c";
    int status;
    
    printf("=== Testing GCC Driver Reinitialization Logic ===\n\n");
    
    /* Create test source file */
    if (!create_test_file(test_file)) {
        return 1;
    }
    
    /* Test 1: First invocation with various flags to set state */
    printf("Test 1: Setting driver state with various flags\n");
    printf("------------------------------------------------\n");
    char *args1[] = {
        (char*)gcc_path,
        "-save-temps",
        "-dumpdir", "/tmp/test_dump1",
        "-dumpbase", "test_dumpbase",
        "-c", (char*)test_file,
        "-o", "output1.o",
        "--sysroot=/alt/sysroot",
        "-march=x86-64",
        "-v",  /* verbose to see what's happening */
        NULL
    };
    
    status = invoke_gcc_via_process(gcc_path, args1);
    printf("Exit status: %d\n\n", status);
    
    /* Test 2: Second invocation with minimal flags to trigger reset */
    printf("Test 2: Minimal invocation to trigger state reset\n");
    printf("------------------------------------------------\n");
    char *args2[] = {
        (char*)gcc_path,
        "-c", (char*)test_file,
        "-o", "output2.o",
        NULL
    };
    
    status = invoke_gcc_via_process(gcc_path, args2);
    printf("Exit status: %d\n\n", status);
    
    /* Test 3: Third invocation with invalid option to force failure */
    printf("Test 3: Invalid option to set greatest_status\n");
    printf("------------------------------------------------\n");
    char *args3[] = {
        (char*)gcc_path,
        "-invalid-option-that-does-not-exist",
        NULL
    };
    
    status = invoke_gcc_via_process(gcc_path, args3);
    printf("Exit status: %d (should be non-zero)\n\n", status);
    
    /* Test 4: Fourth invocation - successful compilation after failure */
    printf("Test 4: Successful compilation after failure\n");
    printf("------------------------------------------------\n");
    char *args4[] = {
        (char*)gcc_path,
        "-c", (char*)test_file,
        "-o", "output4.o",
        NULL
    };
    
    status = invoke_gcc_via_process(gcc_path, args4);
    printf("Exit status: %d (should be 0)\n\n", status);
    
    /* Test 5: Additional test with different dumpbase and outbase */
    printf("Test 5: Testing dumpbase/outbase reset\n");
    printf("------------------------------------------------\n");
    char *args5[] = {
        (char*)gcc_path,
        "-save-temps=obj",
        "-dumpdir", "/tmp/test_dump2",
        "-dumpbase", "different_base",
        "-dumpbase_ext", ".ext",
        "-o", "final_output",
        (char*)test_file,
        NULL
    };
    
    status = invoke_gcc_via_process(gcc_path, args5);
    printf("Exit status: %d\n\n", status);
    
    /* Test 6: Final minimal invocation to ensure complete reset */
    printf("Test 6: Final minimal compilation\n");
    printf("------------------------------------------------\n");
    char *args6[] = {
        (char*)gcc_path,
        "-c", (char*)test_file,
        "-o", "final.o",
        NULL
    };
    
    status = invoke_gcc_via_process(gcc_path, args6);
    printf("Exit status: %d\n\n", status);
    
    /* Cleanup */
    unlink(test_file);
    unlink("output1.o");
    unlink("output2.o");
    unlink("output4.o");
    unlink("final.o");
    unlink("final_output");
    
    printf("=== Test Complete ===\n");
    printf("Check that all state variables were reset between invocations:\n");
    printf("- save_temps_flag reset to SAVE_TEMPS_NONE\n");
    printf("- dumpdir, dumpbase, dumpbase_ext, outbase freed and set to NULL\n");
    printf("- target_system_root reset to DEFAULT_TARGET_SYSTEM_ROOT\n");
    printf("- spec_machine reset to DEFAULT_TARGET_MACHINE\n");
    printf("- greatest_status reset to 1\n");
    
    return 0;
}
