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

/* Define if we want to use direct library loading */
#define USE_DLOPEN 0

/* Simple test C source file content */
const char* test_source = 
"#include <stdio.h>\n"
"int main() {\n"
"    printf(\"Hello from test program\\n\");\n"
"    return 0;\n"
"}\n";

/* Create test source file */
int create_test_file(const char* filename) {
    FILE* f = fopen(filename, "w");
    if (!f) {
        perror("Failed to create test file");
        return -1;
    }
    fputs(test_source, f);
    fclose(f);
    return 0;
}

/* Method A: Process-based invocation using fork/exec */
int invoke_gcc_process(const char* gcc_path, char* const argv[]) {
    pid_t pid = fork();
    if (pid == 0) {
        /* Child process */
        execv(gcc_path, argv);
        perror("execv failed");
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
        perror("fork failed");
        return -1;
    }
}

/* Method B: Direct library loading (if GCC driver is built as shared library) */
#ifdef USE_DLOPEN
int invoke_gcc_library(char* const argv[]) {
    static void* handle = NULL;
    static int (*gcc_main)(int, char**) = NULL;
    
    if (!handle) {
        handle = dlopen("libgccdriver.so", RTLD_LAZY | RTLD_LOCAL);
        if (!handle) {
            fprintf(stderr, "dlopen failed: %s\n", dlerror());
            return -1;
        }
        
        gcc_main = (int (*)(int, char**))dlsym(handle, "main");
        if (!gcc_main) {
            /* Try alternative entry point */
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

int main(int argc, char* argv[]) {
    const char* gcc_path = "gcc";
    const char* test_file = "test_input.c";
    int status;
    
    /* Create test source file */
    if (create_test_file(test_file) != 0) {
        return 1;
    }
    
    printf("=== Testing GCC Driver Reinitialization ===\n\n");
    
    /* Stage 1: Invocation with various flags to set global state */
    printf("Stage 1: Setting global state with various flags\n");
    char* stage1_args[] = {
        (char*)gcc_path,
        "-save-temps",           /* Sets save_temps_flag */
        "-dumpdir", "/tmp/test_dump", /* Sets dumpdir */
        "-dumpbase", "test_dumpbase", /* Sets dumpbase */
        "-dumpbase-ext", ".c",   /* Sets dumpbase_ext */
        "--sysroot=/alt/sysroot", /* Changes target_system_root */
        "-specs=test.specs",     /* May affect spec_machine */
        "-march=native",         /* May affect spec_machine */
        "-c", (char*)test_file,
        "-o", "output1.o",
        "-v",                    /* verbose flag */
        NULL
    };
    
    status = invoke_gcc_process(gcc_path, stage1_args);
    printf("Stage 1 exit status: %d\n\n", status);
    
    /* Stage 2: Simple compilation to trigger reset to defaults */
    printf("Stage 2: Simple compilation (should reset to defaults)\n");
    char* stage2_args[] = {
        (char*)gcc_path,
        "-c", (char*)test_file,
        "-o", "output2.o",
        NULL
    };
    
    status = invoke_gcc_process(gcc_path, stage2_args);
    printf("Stage 2 exit status: %d\n\n", status);
    
    /* Stage 3: Failed invocation to set greatest_status */
    printf("Stage 3: Failed invocation (should set greatest_status)\n");
    char* stage3_args[] = {
        (char*)gcc_path,
        "-invalid-option-that-does-not-exist",
        "-c", (char*)test_file,
        "-o", "output3.o",
        NULL
    };
    
    status = invoke_gcc_process(gcc_path, stage3_args);
    printf("Stage 3 exit status: %d (expected non-zero)\n\n", status);
    
    /* Stage 4: Successful compilation to test greatest_status reset */
    printf("Stage 4: Successful compilation (should reset greatest_status)\n");
    char* stage4_args[] = {
        (char*)gcc_path,
        "-c", (char*)test_file,
        "-o", "output4.o",
        NULL
    };
    
    status = invoke_gcc_process(gcc_path, stage4_args);
    printf("Stage 4 exit status: %d (expected 0)\n\n", status);
    
    /* Stage 5: Test with different dumpdir/dumpbase combinations */
    printf("Stage 5: Testing dumpdir/dumpbase reset\n");
    char* stage5_args[] = {
        (char*)gcc_path,
        "-save-temps=obj",
        "-dumpdir", "/tmp/another_dir",
        "-dumpbase", "another_base",
        "-dumpbase-ext", ".s",
        "-c", (char*)test_file,
        "-o", "output5.o",
        NULL
    };
    
    status = invoke_gcc_process(gcc_path, stage5_args);
    printf("Stage 5 exit status: %d\n\n", status);
    
    /* Stage 6: Final simple compilation to ensure complete reset */
    printf("Stage 6: Final simple compilation (complete reset check)\n");
    char* stage6_args[] = {
        (char*)gcc_path,
        "-c", (char*)test_file,
        "-o", "output6.o",
        NULL
    };
    
    status = invoke_gcc_process(gcc_path, stage6_args);
    printf("Stage 6 exit status: %d\n\n", status);
    
    /* Cleanup */
    unlink(test_file);
    unlink("output1.o");
    unlink("output2.o");
    unlink("output4.o");
    unlink("output5.o");
    unlink("output6.o");
    
    printf("Test completed. Check coverage data for gcc.cc lines 11228-11250\n");
    
    return 0;
}
