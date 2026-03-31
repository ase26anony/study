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

/* Simple C source file content */
const char *input_c_content = 
"#include <stdio.h>\n"
"int main() {\n"
"    printf(\"Hello from test program\\n\");\n"
"    return 0;\n"
"}\n";

/* Create test input file */
static int create_input_file(const char *filename) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        perror("fopen");
        return 0;
    }
    fputs(input_c_content, f);
    fclose(f);
    return 1;
}

/* Method A: Process-based invocation using fork/exec */
static void test_process_based(void) {
    printf("=== Testing via Process-Based Invocation ===\n");
    
    const char *input_file = "test_input.c";
    if (!create_input_file(input_file)) {
        fprintf(stderr, "Failed to create input file\n");
        return;
    }
    
    /* Array of test invocations with different flags */
    char *invocations[][10] = {
        /* First: Set various flags including sysroot and dump options */
        {"gcc", "-save-temps", "-dumpdir", "/tmp/test_dump1", 
         "-dumpbase", "test_dumpbase1", "-c", input_file, 
         "-o", "output1.o", NULL},
        
        /* Second: Minimal compilation, should trigger reset to defaults */
        {"gcc", "-c", input_file, "-o", "output2.o", NULL},
        
        /* Third: Force failure with invalid option */
        {"gcc", "-invalid-option-that-does-not-exist", input_file, NULL},
        
        /* Fourth: Successful compilation again */
        {"gcc", "-c", input_file, "-o", "output3.o", NULL},
        
        /* Fifth: Another with different sysroot and machine spec */
        {"gcc", "--sysroot=/tmp/fake_sysroot", "-march=x86-64", 
         "-c", input_file, "-o", "output4.o", NULL},
        
        /* Sixth: Back to defaults */
        {"gcc", "-c", input_file, "-o", "output5.o", NULL}
    };
    
    int num_invocations = sizeof(invocations) / sizeof(invocations[0]);
    int statuses[num_invocations];
    
    for (int i = 0; i < num_invocations; i++) {
        pid_t pid = fork();
        
        if (pid == 0) {
            /* Child process */
            printf("Invocation %d: ", i + 1);
            for (int j = 0; invocations[i][j]; j++) {
                printf("%s ", invocations[i][j]);
            }
            printf("\n");
            
            execvp("gcc", invocations[i]);
            perror("execvp failed");
            exit(EXIT_FAILURE);
        } else if (pid > 0) {
            /* Parent process */
            waitpid(pid, &statuses[i], 0);
            printf("  Exit status: %d\n", WEXITSTATUS(statuses[i]));
        } else {
            perror("fork failed");
            statuses[i] = -1;
        }
    }
    
    /* Cleanup */
    unlink(input_file);
    for (int i = 1; i <= 5; i++) {
        char filename[20];
        snprintf(filename, sizeof(filename), "output%d.o", i);
        unlink(filename);
    }
    
    printf("Process-based test completed.\n\n");
}

/* Method B: Direct library call using dlopen (if available) */
#ifdef TEST_WITH_DLOPEN
static void test_library_based(void) {
    printf("=== Testing via Direct Library Loading ===\n");
    
    void *handle = dlopen("libgccdriver.so", RTLD_LAZY | RTLD_LOCAL);
    if (!handle) {
        /* Try alternative names */
        handle = dlopen("./libgccdriver.so", RTLD_LAZY | RTLD_LOCAL);
    }
    
    if (!handle) {
        printf("Could not load libgccdriver.so: %s\n", dlerror());
        printf("Skipping library-based test.\n\n");
        return;
    }
    
    /* Look for driver entry point - this depends on GCC's internal structure */
    typedef int (*driver_main_t)(int, char**);
    driver_main_t driver_main = (driver_main_t)dlsym(handle, "main");
    
    if (!driver_main) {
        /* Try alternative symbol names */
        driver_main = (driver_main_t)dlsym(handle, "driver::main");
    }
    
    if (!driver_main) {
        driver_main = (driver_main_t)dlsym(handle, "_Z6driveriPPc");  // mangled name
    }
    
    if (driver_main) {
        const char *input_file = "test_input2.c";
        if (!create_input_file(input_file)) {
            fprintf(stderr, "Failed to create input file\n");
            dlclose(handle);
            return;
        }
        
        /* Test invocations with different argument vectors */
        char *args1[] = {"gcc", "-save-temps", "-dumpdir", "/tmp/libtest1",
                        "-dumpbase", "libdump", "-c", input_file, 
                        "-o", "liboutput1.o", NULL};
        char *args2[] = {"gcc", "-c", input_file, "-o", "liboutput2.o", NULL};
        char *args3[] = {"gcc", "-invalid-option", NULL};
        char *args4[] = {"gcc", "-c", input_file, "-o", "liboutput3.o", NULL};
        
        printf("Call 1: With save-temps and dumpdir\n");
        int ret1 = driver_main(10, args1);
        printf("  Return: %d\n", ret1);
        
        printf("Call 2: Minimal (should trigger reset)\n");
        int ret2 = driver_main(4, args2);
        printf("  Return: %d\n", ret2);
        
        printf("Call 3: Invalid option (should fail)\n");
        int ret3 = driver_main(2, args3);
        printf("  Return: %d\n", ret3);
        
        printf("Call 4: Successful again\n");
        int ret4 = driver_main(4, args4);
        printf("  Return: %d\n", ret4);
        
        /* Cleanup */
        unlink(input_file);
        for (int i = 1; i <= 3; i++) {
            char filename[20];
            snprintf(filename, sizeof(filename), "liboutput%d.o", i);
            unlink(filename);
        }
    } else {
        printf("Could not find driver entry point: %s\n", dlerror());
    }
    
    dlclose(handle);
    printf("Library-based test completed.\n\n");
}
#endif

/* Method C: Direct compilation with driver internals */
#ifdef COMPILE_WITH_DRIVER
/* This would require linking against gcc driver object files */
extern int driver_main(int argc, char **argv);

static void test_direct_linking(void) {
    printf("=== Testing via Direct Linking ===\n");
    
    const char *input_file = "test_input3.c";
    if (!create_input_file(input_file)) {
        fprintf(stderr, "Failed to create input file\n");
        return;
    }
    
    /* Multiple invocations to trigger reinitialization */
    char *args1[] = {"testprog", "--sysroot=/tmp/mysysroot", "-specs=test.specs",
                    "-save-temps=obj", "-dumpbase", "direct_test",
                    "-c", input_file, "-o", "direct1.o", NULL};
    char *args2[] = {"testprog", "-c", input_file, "-o", "direct2.o", NULL};
    char *args3[] = {"testprog", "-nonexistent-flag", input_file, NULL};
    char *args4[] = {"testprog", "-mtune=generic", "-c", input_file, 
                    "-o", "direct3.o", NULL};
    char *args5[] = {"testprog", "-c", input_file, "-o", "direct4.o", NULL};
    
    printf("Invocation 1: With sysroot and specs\n");
    int ret1 = driver_main(10, args1);
    printf("  Status: %d\n", ret1);
    
    printf("Invocation 2: Back to defaults\n");
    int ret2 = driver_main(4, args2);
    printf("  Status: %d\n", ret2);
    
    printf("Invocation 3: Force error\n");
    int ret3 = driver_main(3, args3);
    printf("  Status: %d\n", ret3);
    
    printf("Invocation 4: With machine tuning\n");
    int ret4 = driver_main(5, args4);
    printf("  Status: %d\n", ret4);
    
    printf("Invocation 5: Defaults again\n");
    int ret5 = driver_main(4, args5);
    printf("  Status: %d\n", ret5);
    
    /* Cleanup */
    unlink(input_file);
    for (int i = 1; i <= 4; i++) {
        char filename[20];
        snprintf(filename, sizeof(filename), "direct%d.o", i);
        unlink(filename);
    }
    
    printf("Direct linking test completed.\n\n");
}
#endif

/* Multi-stage compilation simulation */
static void test_multi_stage(void) {
    printf("=== Testing Multi-Stage Compilation ===\n");
    
    const char *input_file = "multistage.c";
    if (!create_input_file(input_file)) {
        fprintf(stderr, "Failed to create input file\n");
        return;
    }
    
    /* Simulate full compilation pipeline with different flags at each stage */
    char *stages[][15] = {
        /* Stage 1: Preprocessing with dump options */
        {"gcc", "-E", "-save-temps", "-dumpdir", "/tmp/stage1",
         "-dumpbase", "stage1", input_file, "-o", "stage1.i", NULL},
        
        /* Stage 2: Compilation to assembly */
        {"gcc", "-S", "-dumpdir", "/tmp/stage2", 
         "-dumpbase", "stage2", "stage1.i", "-o", "stage2.s", NULL},
        
        /* Stage 3: Assembly to object (no dump options) */
        {"gcc", "-c", "stage2.s", "-o", "stage3.o", NULL},
        
        /* Stage 4: Linking with sysroot */
        {"gcc", "--sysroot=/tmp/fakeroot", "stage3.o", 
         "-o", "stage4.exe", NULL},
        
        /* Stage 5: Another compilation (should reset to defaults) */
        {"gcc", "-c", input_file, "-o", "stage5.o", NULL}
    };
    
    int num_stages = sizeof(stages) / sizeof(stages[0]);
    
    for (int i = 0; i < num_stages; i++) {
        pid_t pid = fork();
        
        if (pid == 0) {
            printf("Stage %d: ", i + 1);
            for (int j = 0; stages[i][j]; j++) {
                printf("%s ", stages[i][j]);
            }
            printf("\n");
            
            execvp("gcc", stages[i]);
            perror("execvp");
            exit(EXIT_FAILURE);
        } else if (pid > 0) {
            int status;
            waitpid(pid, &status, 0);
            printf("  Exit: %d\n", WEXITSTATUS(status));
        }
    }
    
    /* Cleanup intermediate files */
    unlink(input_file);
    unlink("stage1.i");
    unlink("stage2.s");
    unlink("stage3.o");
    unlink("stage4.exe");
    unlink("stage5.o");
    
    printf("Multi-stage test completed.\n\n");
}

int main(int argc, char **argv) {
    printf("GCC Driver Reinitialization Test Program\n");
    printf("=========================================\n\n");
    
    /* Test 1: Process-based approach (most reliable) */
    test_process_based();
    
    /* Test 2: Multi-stage compilation simulation */
    test_multi_stage();
    
#ifdef TEST_WITH_DLOPEN
    /* Test 3: Library loading approach */
    test_library_based();
#endif
    
#ifdef COMPILE_WITH_DRIVER
    /* Test 4: Direct linking approach */
    test_direct_linking();
#endif
    
    printf("All tests completed.\n");
    
    /* Final test: Verify we can still compile normally */
    printf("\n=== Final Verification ===\n");
    if (create_input_file("final_test.c")) {
        pid_t pid = fork();
        if (pid == 0) {
            char *args[] = {"gcc", "-c", "final_test.c", "-o", "final.o", NULL};
            execvp("gcc", args);
            exit(EXIT_FAILURE);
        } else if (pid > 0) {
            int status;
            waitpid(pid, &status, 0);
            printf("Final compilation exit status: %d\n", WEXITSTATUS(status));
        }
        unlink("final_test.c");
        unlink("final.o");
    }
    
    return 0;
}
