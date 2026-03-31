/* test_gcc_driver_reset.c
 * A program to test GCC driver reinitialization logic
 * Specifically targets lines 11228-11250 in gcc.cc
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

/* Create a minimal C source file for compilation */
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

/* Method A: Process-based testing using fork/exec */
void test_via_processes() {
    printf("=== Testing via fork/exec processes ===\n");
    
    const char *gcc_path = "gcc";
    const char *input_file = "test_input.c";
    int status;
    
    /* Create test source file */
    create_test_source(input_file);
    
    /* Test 1: First invocation with various flags to set state */
    printf("\n--- Test 1: Setting driver state ---\n");
    char *argv1[] = {
        (char *)gcc_path,
        "-save-temps",
        "-dumpdir", "/tmp/gcc_test_dump1",
        "-dumpbase", "test_dump",
        "-c",
        (char *)input_file,
        "-o", "output1.o",
        "--sysroot=/tmp/fake_sysroot",
        "-march=x86-64",
        "-v",  /* verbose to trigger verbose_only_flag */
        NULL
    };
    
    pid_t pid1 = fork();
    if (pid1 == 0) {
        /* Child process */
        execvp(gcc_path, argv1);
        perror("execvp failed");
        exit(1);
    } else {
        waitpid(pid1, &status, 0);
        printf("Test 1 exit status: %d\n", WEXITSTATUS(status));
    }
    
    /* Test 2: Second invocation with minimal flags to trigger reset */
    printf("\n--- Test 2: Triggering state reset ---\n");
    char *argv2[] = {
        (char *)gcc_path,
        "-c",
        (char *)input_file,
        "-o", "output2.o",
        NULL
    };
    
    pid_t pid2 = fork();
    if (pid2 == 0) {
        execvp(gcc_path, argv2);
        perror("execvp failed");
        exit(1);
    } else {
        waitpid(pid2, &status, 0);
        printf("Test 2 exit status: %d\n", WEXITSTATUS(status));
    }
    
    /* Test 3: Third invocation with invalid option to set greatest_status */
    printf("\n--- Test 3: Forcing failure to set greatest_status ---\n");
    char *argv3[] = {
        (char *)gcc_path,
        "-invalid-option-that-does-not-exist",
        (char *)input_file,
        NULL
    };
    
    pid_t pid3 = fork();
    if (pid3 == 0) {
        execvp(gcc_path, argv3);
        perror("execvp failed");
        exit(1);
    } else {
        waitpid(pid3, &status, 0);
        printf("Test 3 exit status: %d\n", WEXITSTATUS(status));
    }
    
    /* Test 4: Fourth invocation to test greatest_status reset */
    printf("\n--- Test 4: Testing greatest_status reset ---\n");
    char *argv4[] = {
        (char *)gcc_path,
        "-c",
        (char *)input_file,
        "-o", "output4.o",
        NULL
    };
    
    pid_t pid4 = fork();
    if (pid4 == 0) {
        execvp(gcc_path, argv4);
        perror("execvp failed");
        exit(1);
    } else {
        waitpid(pid4, &status, 0);
        printf("Test 4 exit status: %d\n", WEXITSTATUS(status));
    }
    
    /* Cleanup */
    unlink(input_file);
    unlink("output1.o");
    unlink("output2.o");
    unlink("output4.o");
}

/* Method B: Direct library call testing (if available) */
#ifdef TEST_WITH_DLOPEN
void test_via_dlopen() {
    printf("\n=== Testing via dlopen (if driver is shared library) ===\n");
    
    void *handle = dlopen("./libgccdriver.so", RTLD_LAZY | RTLD_LOCAL);
    if (!handle) {
        printf("dlopen failed: %s\n", dlerror());
        printf("Skipping dlopen test (driver not built as shared library)\n");
        return;
    }
    
    /* Look for driver entry point - this varies by GCC version */
    int (*driver_main)(int, char**) = dlsym(handle, "main");
    if (!driver_main) {
        driver_main = dlsym(handle, "gcc_main");
    }
    if (!driver_main) {
        driver_main = dlsym(handle, "driver::main");
    }
    
    if (!driver_main) {
        printf("Could not find driver entry point: %s\n", dlerror());
        dlclose(handle);
        return;
    }
    
    /* Create test source */
    const char *input_file = "dlopen_test.c";
    create_test_source(input_file);
    
    /* Test 1: Set various flags */
    printf("\n--- DLOPEN Test 1: Setting state ---\n");
    char *argv1[] = {
        "gcc",
        "-save-temps=obj",
        "-dumpdir", "/tmp/dltest",
        "-dumpbase", "dl_base",
        "-specs=/dev/null",  /* Try to change specs */
        "-c",
        (char *)input_file,
        "-o", "dl_output1.o",
        NULL
    };
    
    int argc1 = sizeof(argv1)/sizeof(argv1[0]) - 1;
    int result1 = driver_main(argc1, argv1);
    printf("DLOPEN Test 1 result: %d\n", result1);
    
    /* Test 2: Minimal invocation to trigger reset */
    printf("\n--- DLOPEN Test 2: Triggering reset ---\n");
    char *argv2[] = {
        "gcc",
        "-c",
        (char *)input_file,
        "-o", "dl_output2.o",
        NULL
    };
    
    int argc2 = sizeof(argv2)/sizeof(argv2[0]) - 1;
    int result2 = driver_main(argc2, argv2);
    printf("DLOPEN Test 2 result: %d\n", result2);
    
    /* Cleanup */
    dlclose(handle);
    unlink(input_file);
    unlink("dl_output1.o");
    unlink("dl_output2.o");
}
#endif

/* Method C: Simulate multi-stage compilation pipeline */
void test_multi_stage_pipeline() {
    printf("\n=== Testing multi-stage compilation pipeline ===\n");
    
    const char *gcc_path = "gcc";
    const char *input_file = "pipeline_test.c";
    int status;
    
    /* Create test source */
    create_test_source(input_file);
    
    /* Stage 1: Preprocessing with save-temps */
    printf("\n--- Stage 1: Preprocessing with save-temps ---\n");
    char *stage1_args[] = {
        (char *)gcc_path,
        "-save-temps",
        "-dumpdir", "/tmp/pipeline",
        "-dumpbase", "stage1",
        "-E",  /* Preprocess only */
        (char *)input_file,
        "-o", "stage1.i",
        NULL
    };
    
    pid_t pid1 = fork();
    if (pid1 == 0) {
        execvp(gcc_path, stage1_args);
        perror("execvp failed");
        exit(1);
    } else {
        waitpid(pid1, &status, 0);
        printf("Stage 1 exit: %d\n", WEXITSTATUS(status));
    }
    
    /* Stage 2: Compilation with different dumpbase */
    printf("\n--- Stage 2: Compilation with new dumpbase ---\n");
    char *stage2_args[] = {
        (char *)gcc_path,
        "-save-temps=cwd",
        "-dumpbase", "stage2",
        "-dumpbase_ext", ".ext",
        "-c",
        (char *)input_file,
        "-o", "stage2.o",
        NULL
    };
    
    pid_t pid2 = fork();
    if (pid2 == 0) {
        execvp(gcc_path, stage2_args);
        perror("execvp failed");
        exit(1);
    } else {
        waitpid(pid2, &status, 0);
        printf("Stage 2 exit: %d\n", WEXITSTATUS(status));
    }
    
    /* Stage 3: Linking with outbase */
    printf("\n--- Stage 3: Linking with outbase ---\n");
    char *stage3_args[] = {
        (char *)gcc_path,
        "-o", "pipeline_out",
        "stage2.o",
        NULL
    };
    
    pid_t pid3 = fork();
    if (pid3 == 0) {
        execvp(gcc_path, stage3_args);
        perror("execvp failed");
        exit(1);
    } else {
        waitpid(pid3, &status, 0);
        printf("Stage 3 exit: %d\n", WEXITSTATUS(status));
    }
    
    /* Stage 4: Clean compilation to trigger full reset */
    printf("\n--- Stage 4: Clean compilation (trigger reset) ---\n");
    char *stage4_args[] = {
        (char *)gcc_path,
        "-c",
        (char *)input_file,
        NULL
    };
    
    pid_t pid4 = fork();
    if (pid4 == 0) {
        execvp(gcc_path, stage4_args);
        perror("execvp failed");
        exit(1);
    } else {
        waitpid(pid4, &status, 0);
        printf("Stage 4 exit: %d\n", WEXITSTATUS(status));
    }
    
    /* Cleanup */
    unlink(input_file);
    unlink("stage1.i");
    unlink("stage2.o");
    unlink("pipeline_out");
    unlink("a.out");
}

int main(int argc, char **argv) {
    printf("GCC Driver Reinitialization Test Program\n");
    printf("Targeting uncovered lines 11228-11250 in gcc.cc\n\n");
    
    /* Create a fake sysroot directory for testing */
    mkdir("/tmp/fake_sysroot", 0755);
    mkdir("/tmp/gcc_test_dump1", 0755);
    mkdir("/tmp/pipeline", 0755);
    
    /* Run process-based tests */
    test_via_processes();
    
    /* Run multi-stage pipeline test */
    test_multi_stage_pipeline();
    
#ifdef TEST_WITH_DLOPEN
    /* Run dlopen-based tests if enabled */
    test_via_dlopen();
#endif
    
    printf("\n=== All tests completed ===\n");
    
    /* Cleanup test directories */
    rmdir("/tmp/fake_sysroot");
    rmdir("/tmp/gcc_test_dump1");
    rmdir("/tmp/pipeline");
    
    return 0;
}
