/* test_gcc_driver_reset.c
 * 
 * This program tests the gcc driver's internal state reset logic
 * by invoking it multiple times with different configurations.
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <dlfcn.h>
#include <errno.h>

/* Define this if you want to use direct library calls instead of fork/exec */
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
        return -1;
    }
    fputs(test_source, f);
    fclose(f);
    return 0;
}

/* Method A: Process-based invocation using fork/exec */
int invoke_gcc_process(const char *gcc_path, char *const argv[]) {
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
        perror("fork failed");
        return -1;
    }
}

/* Method B: Direct library call (if gcc driver is built as shared library) */
#ifdef USE_DIRECT_LIBRARY_CALL
int invoke_gcc_library(char *const argv[]) {
    static void *handle = NULL;
    static int (*gcc_main)(int, char**) = NULL;
    
    if (!handle) {
        handle = dlopen("./libgccdriver.so", RTLD_LAZY | RTLD_LOCAL);
        if (!handle) {
            fprintf(stderr, "dlopen failed: %s\n", dlerror());
            return -1;
        }
        
        /* Clear any existing error */
        dlerror();
        
        /* Try to find the main entry point */
        gcc_main = (int (*)(int, char**))dlsym(handle, "main");
        if (!gcc_main) {
            gcc_main = (int (*)(int, char**))dlsym(handle, "driver::main");
        }
        
        if (!gcc_main) {
            fprintf(stderr, "dlsym failed: %s\n", dlerror());
            dlclose(handle);
            return -1;
        }
    }
    
    /* Count arguments */
    int argc = 0;
    while (argv[argc] != NULL) argc++;
    
    /* Call the driver's main function */
    return gcc_main(argc, argv);
}
#endif

/* Build argument vector for different test scenarios */
char **build_argv(const char *gcc_path, const char *first_arg, ...) {
    va_list args;
    const char *arg;
    int count = 2; /* gcc_path + NULL terminator */
    
    /* Count arguments */
    va_start(args, first_arg);
    arg = first_arg;
    while (arg) {
        count++;
        arg = va_arg(args, const char *);
    }
    va_end(args);
    
    /* Allocate argv array */
    char **argv = malloc(count * sizeof(char *));
    if (!argv) return NULL;
    
    /* Fill arguments */
    argv[0] = (char *)gcc_path;
    argv[1] = (char *)first_arg;
    
    int i = 2;
    va_start(args, first_arg);
    arg = first_arg;
    while (arg) {
        argv[i++] = (char *)arg;
        arg = va_arg(args, const char *);
    }
    va_end(args);
    
    argv[count-1] = NULL;
    return argv;
}

int main(int argc, char *argv[]) {
    const char *gcc_path = "gcc";
    const char *test_file = "test_input.c";
    int status;
    
    /* Create test source file */
    if (create_test_file(test_file) < 0) {
        return 1;
    }
    
    printf("=== Testing GCC Driver State Reset Logic ===\n\n");
    
    /* Test 1: First invocation with various flags to set state */
    printf("Test 1: Setting driver state with various flags\n");
    printf("------------------------------------------------\n");
    
    char **test1_argv = build_argv(gcc_path,
        "-save-temps",
        "-dumpdir", "/tmp/gcc_test_dump",
        "-dumpbase", "test_dumpbase",
        "-c", test_file,
        "-o", "output1.o",
        "--sysroot=/alt/sysroot",
        "-march=native",
        "-v",  /* verbose flag */
        NULL);
    
    if (!test1_argv) {
        fprintf(stderr, "Failed to build argv for test 1\n");
        return 1;
    }
    
    #ifdef USE_DIRECT_LIBRARY_CALL
    status = invoke_gcc_library(test1_argv);
    #else
    status = invoke_gcc_process(gcc_path, test1_argv);
    #endif
    
    printf("Test 1 exit status: %d\n\n", status);
    free(test1_argv);
    
    /* Test 2: Second invocation with minimal flags to trigger reset */
    printf("Test 2: Minimal invocation to trigger state reset\n");
    printf("---------------------------------------------------\n");
    
    char **test2_argv = build_argv(gcc_path,
        "-c", test_file,
        "-o", "output2.o",
        NULL);
    
    if (!test2_argv) {
        fprintf(stderr, "Failed to build argv for test 2\n");
        return 1;
    }
    
    #ifdef USE_DIRECT_LIBRARY_CALL
    status = invoke_gcc_library(test2_argv);
    #else
    status = invoke_gcc_process(gcc_path, test2_argv);
    #endif
    
    printf("Test 2 exit status: %d\n\n", status);
    free(test2_argv);
    
    /* Test 3: Third invocation with invalid option to force failure */
    printf("Test 3: Invalid option to set greatest_status\n");
    printf("---------------------------------------------\n");
    
    char **test3_argv = build_argv(gcc_path,
        "-invalid-option-that-does-not-exist",
        "-c", test_file,
        NULL);
    
    if (!test3_argv) {
        fprintf(stderr, "Failed to build argv for test 3\n");
        return 1;
    }
    
    #ifdef USE_DIRECT_LIBRARY_CALL
    status = invoke_gcc_library(test3_argv);
    #else
    status = invoke_gcc_process(gcc_path, test3_argv);
    #endif
    
    printf("Test 3 exit status: %d\n\n", status);
    free(test3_argv);
    
    /* Test 4: Fourth invocation - successful compilation after failure */
    printf("Test 4: Successful compilation after failure\n");
    printf("--------------------------------------------\n");
    
    char **test4_argv = build_argv(gcc_path,
        "-c", test_file,
        "-o", "output4.o",
        "-O2",
        NULL);
    
    if (!test4_argv) {
        fprintf(stderr, "Failed to build argv for test 4\n");
        return 1;
    }
    
    #ifdef USE_DIRECT_LIBRARY_CALL
    status = invoke_gcc_library(test4_argv);
    #else
    status = invoke_gcc_process(gcc_path, test4_argv);
    #endif
    
    printf("Test 4 exit status: %d\n\n", status);
    free(test4_argv);
    
    /* Test 5: Multi-stage compilation simulation */
    printf("Test 5: Multi-stage compilation pipeline\n");
    printf("----------------------------------------\n");
    
    /* Stage 1: Preprocessing with save-temps */
    char **stage1_argv = build_argv(gcc_path,
        "-save-temps=obj",
        "-dumpdir", "stage1_dump",
        "-E", test_file,
        "-o", "test_input.i",
        NULL);
    
    if (!stage1_argv) {
        fprintf(stderr, "Failed to build argv for stage 1\n");
        return 1;
    }
    
    #ifdef USE_DIRECT_LIBRARY_CALL
    int stage1_status = invoke_gcc_library(stage1_argv);
    #else
    int stage1_status = invoke_gcc_process(gcc_path, stage1_argv);
    #endif
    
    printf("Stage 1 (preprocess) exit status: %d\n", stage1_status);
    free(stage1_argv);
    
    /* Stage 2: Compilation with different dumpbase */
    char **stage2_argv = build_argv(gcc_path,
        "-save-temps=cwd",
        "-dumpbase", "stage2_compile",
        "-S", test_file,
        "-o", "test_input.s",
        NULL);
    
    if (!stage2_argv) {
        fprintf(stderr, "Failed to build argv for stage 2\n");
        return 1;
    }
    
    #ifdef USE_DIRECT_LIBRARY_CALL
    int stage2_status = invoke_gcc_library(stage2_argv);
    #else
    int stage2_status = invoke_gcc_process(gcc_path, stage2_argv);
    #endif
    
    printf("Stage 2 (compile) exit status: %d\n", stage2_status);
    free(stage2_argv);
    
    /* Stage 3: Assembly with no special flags (trigger reset) */
    char **stage3_argv = build_argv(gcc_path,
        "-c", "test_input.s",
        "-o", "test_input.o",
        NULL);
    
    if (!stage3_argv) {
        fprintf(stderr, "Failed to build argv for stage 3\n");
        return 1;
    }
    
    #ifdef USE_DIRECT_LIBRARY_CALL
    int stage3_status = invoke_gcc_library(stage3_argv);
    #else
    int stage3_status = invoke_gcc_process(gcc_path, stage3_argv);
    #endif
    
    printf("Stage 3 (assemble) exit status: %d\n\n", stage3_status);
    free(stage3_argv);
    
    /* Cleanup */
    unlink(test_file);
    unlink("output1.o");
    unlink("output2.o");
    unlink("output4.o");
    unlink("test_input.i");
    unlink("test_input.s");
    unlink("test_input.o");
    
    printf("=== Test Complete ===\n");
    printf("The following driver state variables should have been reset:\n");
    printf("  - save_temps_flag (from SAVE_TEMPS_OBJ/CWD to SAVE_TEMPS_NONE)\n");
    printf("  - dumpdir, dumpbase, dumpbase_ext, outbase (freed and set to NULL)\n");
    printf("  - target_system_root (reset to DEFAULT_TARGET_SYSTEM_ROOT)\n");
    printf("  - spec_machine (reset to DEFAULT_TARGET_MACHINE)\n");
    printf("  - greatest_status (reset to 1 after failure)\n");
    
    return 0;
}
