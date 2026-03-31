/* test_gcc_driver_reinit.c
 * A program to test GCC driver reinitialization logic by invoking
 * the driver multiple times with different configurations.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <dlfcn.h>
#include <errno.h>

/* Define this to use direct library loading instead of fork/exec */
/* #define USE_DIRECT_LIBRARY_CALL */

/* Simple test C source file content */
const char* test_source = 
"#include <stdio.h>\n"
"int main() {\n"
"    printf(\"Hello from test program\\n\");\n"
"    return 0;\n"
"}\n";

/* Create a temporary source file for compilation */
int create_test_source(const char* filename) {
    FILE* fp = fopen(filename, "w");
    if (!fp) {
        perror("Failed to create test source file");
        return -1;
    }
    fputs(test_source, fp);
    fclose(fp);
    return 0;
}

/* Clean up temporary files */
void cleanup_files(const char* base_name) {
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "rm -f %s.c %s*.o %s*.i %s*.s %s*.ii %s*.s %s",
             base_name, base_name, base_name, base_name, base_name, base_name, base_name);
    system(cmd);
}

#ifdef USE_DIRECT_LIBRARY_CALL
/* Direct library call approach - requires gcc driver built as shared library */
void test_direct_library_call() {
    void* handle = dlopen("./libgccdriver.so", RTLD_LAZY | RTLD_LOCAL);
    if (!handle) {
        fprintf(stderr, "Failed to load libgccdriver.so: %s\n", dlerror());
        return;
    }
    
    /* Find the driver main function */
    int (*driver_main)(int, char**) = dlsym(handle, "main");
    if (!driver_main) {
        fprintf(stderr, "Failed to find driver main: %s\n", dlerror());
        dlclose(handle);
        return;
    }
    
    printf("Testing driver reinitialization via direct library calls...\n");
    
    /* First invocation: Set various flags including sysroot */
    char* argv1[] = {
        "gcc",
        "-save-temps",
        "-dumpdir", "/tmp/test_dump1",
        "-dumpbase", "test_driver",
        "-c", "test_input.c",
        "-o", "output1.o",
        "--sysroot=/alternative/sysroot",
        "-v",  /* Enable verbose to trigger verbose_only_flag */
        NULL
    };
    
    printf("Invocation 1: With sysroot and save-temps\n");
    int status1 = driver_main(10, argv1);
    printf("Driver returned: %d\n", status1);
    
    /* Second invocation: Reset to defaults (no special flags) */
    char* argv2[] = {
        "gcc",
        "-c", "test_input.c",
        "-o", "output2.o",
        NULL
    };
    
    printf("\nInvocation 2: Without special flags (should trigger reset)\n");
    int status2 = driver_main(4, argv2);
    printf("Driver returned: %d\n", status2);
    
    /* Third invocation: Force failure with invalid option */
    char* argv3[] = {
        "gcc",
        "-invalid-option-that-does-not-exist",
        NULL
    };
    
    printf("\nInvocation 3: With invalid option (should fail)\n");
    int status3 = driver_main(2, argv3);
    printf("Driver returned: %d\n", status3);
    
    /* Fourth invocation: Successful compilation again */
    char* argv4[] = {
        "gcc",
        "-c", "test_input.c",
        "-o", "output3.o",
        NULL
    };
    
    printf("\nInvocation 4: Valid compilation (should succeed)\n");
    int status4 = driver_main(4, argv4);
    printf("Driver returned: %d\n", status4);
    
    dlclose(handle);
}
#endif

/* Fork/exec approach - uses actual gcc binary */
void test_fork_exec() {
    printf("Testing driver reinitialization via fork/exec...\n");
    
    /* Create test source file */
    if (create_test_source("test_input.c") != 0) {
        return;
    }
    
    int overall_status = 0;
    
    /* First invocation: Set various flags to modify global state */
    printf("\n=== Invocation 1: Setting sysroot, save-temps, dumpdir ===\n");
    char* argv1[] = {
        "gcc",
        "-save-temps",           /* Sets save_temps_flag */
        "-dumpdir", "/tmp/gcc_test_dump",
        "-dumpbase", "multi_stage",
        "-c", "test_input.c",
        "-o", "stage1.o",
        "--sysroot=/tmp/fake_sysroot",  /* Changes target_system_root */
        "-specs=/dev/null",      /* Could affect spec_machine */
        "-march=x86-64",         /* Affects machine spec */
        "-v",                    /* Triggers verbose_only_flag */
        "-ftime-report",         /* Could affect report_times_to_file */
        NULL
    };
    
    pid_t pid1 = fork();
    if (pid1 == 0) {
        /* Child process */
        execvp("gcc", argv1);
        perror("execvp failed");
        exit(1);
    } else if (pid1 > 0) {
        /* Parent process */
        int status1;
        waitpid(pid1, &status1, 0);
        printf("Invocation 1 exit status: %d\n", WEXITSTATUS(status1));
    }
    
    /* Second invocation: Minimal flags to trigger reset to defaults */
    printf("\n=== Invocation 2: Minimal flags (should trigger reset) ===\n");
    char* argv2[] = {
        "gcc",
        "-c", "test_input.c",
        "-o", "stage2.o",
        NULL
    };
    
    pid_t pid2 = fork();
    if (pid2 == 0) {
        execvp("gcc", argv2);
        perror("execvp failed");
        exit(1);
    } else if (pid2 > 0) {
        int status2;
        waitpid(pid2, &status2, 0);
        printf("Invocation 2 exit status: %d\n", WEXITSTATUS(status2));
    }
    
    /* Third invocation: Force failure to test greatest_status */
    printf("\n=== Invocation 3: Invalid option (should fail) ===\n");
    char* argv3[] = {
        "gcc",
        "-this-option-does-not-exist-and-will-cause-error",
        NULL
    };
    
    pid_t pid3 = fork();
    if (pid3 == 0) {
        execvp("gcc", argv3);
        perror("execvp failed");
        exit(1);
    } else if (pid3 > 0) {
        int status3;
        waitpid(pid3, &status3, 0);
        printf("Invocation 3 exit status: %d\n", WEXITSTATUS(status3));
        if (WEXITSTATUS(status3) != 0) {
            overall_status = 1;
        }
    }
    
    /* Fourth invocation: Successful compilation to test status reset */
    printf("\n=== Invocation 4: Valid compilation (should succeed) ===\n");
    char* argv4[] = {
        "gcc",
        "-c", "test_input.c",
        "-o", "stage4.o",
        NULL
    };
    
    pid_t pid4 = fork();
    if (pid4 == 0) {
        execvp("gcc", argv4);
        perror("execvp failed");
        exit(1);
    } else if (pid4 > 0) {
        int status4;
        waitpid(pid4, &status4, 0);
        printf("Invocation 4 exit status: %d\n", WEXITSTATUS(status4));
    }
    
    /* Fifth invocation: Test multi-stage with different dumpbase */
    printf("\n=== Invocation 5: Different dumpbase and outbase ===\n");
    char* argv5[] = {
        "gcc",
        "-save-temps=obj",
        "-dumpdir", ".",
        "-dumpbase", "newbase",
        "-dumpbase-ext", ".ext",
        "-c", "test_input.c",
        "-o", "final.o",
        NULL
    };
    
    pid_t pid5 = fork();
    if (pid5 == 0) {
        execvp("gcc", argv5);
        perror("execvp failed");
        exit(1);
    } else if (pid5 > 0) {
        int status5;
        waitpid(pid5, &status5, 0);
        printf("Invocation 5 exit status: %d\n", WEXITSTATUS(status5));
    }
    
    /* Clean up */
    cleanup_files("test_input");
    cleanup_files("stage");
    cleanup_files("final");
    
    printf("\nOverall test status: %d\n", overall_status);
}

/* Alternative: Simulate multi-stage compilation in single process */
void test_multi_stage_simulation() {
    printf("\n=== Testing multi-stage compilation simulation ===\n");
    
    /* Create a simple pipeline that mimics what build systems do */
    char* stages[][10] = {
        /* Stage 1: Preprocess only */
        {"gcc", "-E", "-dumpdir", "/tmp/stage1", "-dumpbase", "pipeline", 
         "-o", "test.i", "test_input.c", NULL},
        
        /* Stage 2: Compile to assembly */
        {"gcc", "-S", "-dumpdir", "/tmp/stage2", "-dumpbase", "pipeline",
         "-o", "test.s", "test.i", NULL},
        
        /* Stage 3: Assemble */
        {"gcc", "-c", "-dumpdir", "/tmp/stage3", "-dumpbase", "pipeline",
         "-o", "test.o", "test.s", NULL},
        
        /* Stage 4: Link */
        {"gcc", "-dumpdir", "/tmp/stage4", "-dumpbase", "pipeline",
         "-o", "test_prog", "test.o", NULL}
    };
    
    int num_stages = 4;
    
    for (int i = 0; i < num_stages; i++) {
        printf("\nStage %d: ", i + 1);
        for (int j = 0; stages[i][j] != NULL; j++) {
            printf("%s ", stages[i][j]);
        }
        printf("\n");
        
        pid_t pid = fork();
        if (pid == 0) {
            execvp("gcc", stages[i]);
            perror("execvp failed");
            exit(1);
        } else if (pid > 0) {
            int status;
            waitpid(pid, &status, 0);
            printf("  Exit status: %d\n", WEXITSTATUS(status));
        }
    }
    
    /* Clean up intermediate files */
    system("rm -f test.i test.s test.o test_prog");
}

int main(int argc, char** argv) {
    printf("GCC Driver Reinitialization Test Program\n");
    printf("=========================================\n");
    
    /* Create test input file */
    if (create_test_source("test_input.c") != 0) {
        return 1;
    }
    
    /* Test using fork/exec (most reliable) */
    test_fork_exec();
    
    /* Test multi-stage simulation */
    test_multi_stage_simulation();
    
#ifdef USE_DIRECT_LIBRARY_CALL
    /* Test direct library call if enabled */
    test_direct_library_call();
#endif
    
    /* Final cleanup */
    cleanup_files("test_input");
    
    printf("\nTest completed.\n");
    return 0;
}
