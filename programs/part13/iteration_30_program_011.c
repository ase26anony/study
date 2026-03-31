/* test_driver_reinit.c - Test program to trigger gcc.cc driver reinitialization logic */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <dlfcn.h>
#include <errno.h>

/* Define this if you want to test direct library loading */
/* #define USE_DIRECT_LIBRARY_CALL */

/* Simple test C source file content */
const char *test_source = 
"#include <stdio.h>\n"
"int main() {\n"
"    printf(\"Hello from test program\\n\");\n"
"    return 0;\n"
"}\n";

/* Create a temporary source file */
int create_test_file(const char *filename) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        perror("fopen");
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
        perror("execv");
        exit(127);
    } else if (pid > 0) {
        /* Parent process */
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)) {
            return WEXITSTATUS(status);
        }
        return -1;
    } else {
        perror("fork");
        return -1;
    }
}

#ifdef USE_DIRECT_LIBRARY_CALL
/* Method B: Direct library call using dlopen/dlsym */
int invoke_gcc_library(char *const argv[]) {
    static void *handle = NULL;
    static int (*gcc_main)(int, char**) = NULL;
    
    if (!handle) {
        handle = dlopen("libgccdriver.so", RTLD_LAZY | RTLD_LOCAL);
        if (!handle) {
            /* Try alternative names */
            handle = dlopen("./libgccdriver.so", RTLD_LAZY | RTLD_LOCAL);
            if (!handle) {
                fprintf(stderr, "dlopen failed: %s\n", dlerror());
                return -1;
            }
        }
        
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
    
    return gcc_main(argc, argv);
}
#endif

int main(int argc, char *argv[]) {
    const char *gcc_path = "gcc";
    const char *source_file = "test_input.c";
    int overall_status = 0;
    
    /* Create test source file */
    if (create_test_file(source_file) < 0) {
        fprintf(stderr, "Failed to create test source file\n");
        return 1;
    }
    
    printf("=== Testing GCC Driver Reinitialization Logic ===\n\n");
    
    /* Test 1: First invocation with various flags to set state */
    printf("Test 1: Setting driver state with various flags\n");
    printf("------------------------------------------------\n");
    char *test1_args[] = {
        (char*)gcc_path,
        "-save-temps",
        "-dumpdir", "/tmp/gcc_test_dump",
        "-dumpbase", "test_dumpbase",
        "-fdump-tree-original",
        "-fdump-rtl-all",
        "--sysroot=/alt/sysroot",
        "-specs=/usr/share/gcc/specs/test.spec",
        "-march=native",
        "-mtune=generic",
        "-c", (char*)source_file,
        "-o", "test_output1.o",
        "-v",  /* Verbose to trigger verbose_only_flag */
        NULL
    };
    
#ifdef USE_DIRECT_LIBRARY_CALL
    int status1 = invoke_gcc_library(test1_args);
#else
    int status1 = invoke_gcc_process(gcc_path, test1_args);
#endif
    printf("Status 1: %d\n\n", status1);
    
    /* Test 2: Second invocation with minimal flags to trigger reset */
    printf("Test 2: Minimal invocation to trigger state reset\n");
    printf("------------------------------------------------\n");
    char *test2_args[] = {
        (char*)gcc_path,
        "-c", (char*)source_file,
        "-o", "test_output2.o",
        NULL
    };
    
#ifdef USE_DIRECT_LIBRARY_CALL
    int status2 = invoke_gcc_library(test2_args);
#else
    int status2 = invoke_gcc_process(gcc_path, test2_args);
#endif
    printf("Status 2: %d\n\n", status2);
    
    /* Test 3: Third invocation that fails to set greatest_status */
    printf("Test 3: Failed invocation to set greatest_status\n");
    printf("------------------------------------------------\n");
    char *test3_args[] = {
        (char*)gcc_path,
        "-invalid-option-that-does-not-exist",
        "-c", (char*)source_file,
        "-o", "test_output3.o",
        NULL
    };
    
#ifdef USE_DIRECT_LIBRARY_CALL
    int status3 = invoke_gcc_library(test3_args);
#else
    int status3 = invoke_gcc_process(gcc_path, test3_args);
#endif
    printf("Status 3: %d (expected non-zero)\n\n", status3);
    
    /* Test 4: Fourth invocation that succeeds to test greatest_status reset */
    printf("Test 4: Successful invocation after failure\n");
    printf("------------------------------------------------\n");
    char *test4_args[] = {
        (char*)gcc_path,
        "-c", (char*)source_file,
        "-o", "test_output4.o",
        NULL
    };
    
#ifdef USE_DIRECT_LIBRARY_CALL
    int status4 = invoke_gcc_library(test4_args);
#else
    int status4 = invoke_gcc_process(gcc_path, test4_args);
#endif
    printf("Status 4: %d (expected 0)\n\n", status4);
    
    /* Test 5: Multi-stage compilation simulation */
    printf("Test 5: Simulating multi-stage compilation pipeline\n");
    printf("------------------------------------------------\n");
    
    /* Stage 1: Preprocessing with save-temps */
    printf("Stage 1: Preprocessing with -save-temps\n");
    char *stage1_args[] = {
        (char*)gcc_path,
        "-save-temps=obj",
        "-dumpdir", "stage1_dump",
        "-E",  /* Preprocess only */
        (char*)source_file,
        "-o", "test_preprocessed.i",
        NULL
    };
    
#ifdef USE_DIRECT_LIBRARY_CALL
    int stage1_status = invoke_gcc_library(stage1_args);
#else
    int stage1_status = invoke_gcc_process(gcc_path, stage1_args);
#endif
    printf("Stage 1 status: %d\n", stage1_status);
    
    /* Stage 2: Compilation with different dumpbase */
    printf("Stage 2: Compilation with new dumpbase\n");
    char *stage2_args[] = {
        (char*)gcc_path,
        "-save-temps=cwd",
        "-dumpbase", "stage2_compilation",
        "-dumpdir", "stage2_dump",
        "-S",  /* Compile to assembly */
        (char*)source_file,
        "-o", "test_assembly.s",
        NULL
    };
    
#ifdef USE_DIRECT_LIBRARY_CALL
    int stage2_status = invoke_gcc_library(stage2_args);
#else
    int stage2_status = invoke_gcc_process(gcc_path, stage2_args);
#endif
    printf("Stage 2 status: %d\n", stage2_status);
    
    /* Stage 3: Assembly with no special flags (trigger reset) */
    printf("Stage 3: Assembly with minimal flags\n");
    char *stage3_args[] = {
        (char*)gcc_path,
        "-c",  /* Compile and assemble */
        "test_assembly.s",
        "-o", "test_object.o",
        NULL
    };
    
#ifdef USE_DIRECT_LIBRARY_CALL
    int stage3_status = invoke_gcc_library(stage3_args);
#else
    int stage3_status = invoke_gcc_process(gcc_path, stage3_args);
#endif
    printf("Stage 3 status: %d\n\n", stage3_status);
    
    /* Test 6: Testing target system root changes */
    printf("Test 6: Target system root manipulation\n");
    printf("------------------------------------------------\n");
    
    /* First with custom sysroot */
    printf("With custom sysroot:\n");
    char *sysroot1_args[] = {
        (char*)gcc_path,
        "--sysroot=/custom/sysroot",
        "-print-sysroot",
        NULL
    };
    
#ifdef USE_DIRECT_LIBRARY_CALL
    int sysroot1_status = invoke_gcc_library(sysroot1_args);
#else
    int sysroot1_status = invoke_gcc_process(gcc_path, sysroot1_args);
#endif
    
    /* Then without sysroot (should reset to default) */
    printf("Without sysroot (should reset to default):\n");
    char *sysroot2_args[] = {
        (char*)gcc_path,
        "-print-sysroot",
        NULL
    };
    
#ifdef USE_DIRECT_LIBRARY_CALL
    int sysroot2_status = invoke_gcc_library(sysroot2_args);
#else
    int sysroot2_status = invoke_gcc_process(gcc_path, sysroot2_args);
#endif
    
    printf("\n=== Test Summary ===\n");
    printf("Test 1 (state setting): %d\n", status1);
    printf("Test 2 (state reset): %d\n", status2);
    printf("Test 3 (failure): %d\n", status3);
    printf("Test 4 (recovery): %d\n", status4);
    printf("Multi-stage pipeline: %d,%d,%d\n", stage1_status, stage2_status, stage3_status);
    printf("Sysroot test: %d,%d\n", sysroot1_status, sysroot2_status);
    
    /* Cleanup */
    unlink(source_file);
    unlink("test_output1.o");
    unlink("test_output2.o");
    unlink("test_output4.o");
    unlink("test_preprocessed.i");
    unlink("test_assembly.s");
    unlink("test_object.o");
    
    return overall_status;
}
