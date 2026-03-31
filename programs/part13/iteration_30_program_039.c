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
const char* test_source = 
"#include <stdio.h>\n"
"int main() {\n"
"    printf(\"Hello from test program\\n\");\n"
"    return 0;\n"
"}\n";

/* Method A: Process-based invocation using fork/exec */
void test_via_process() {
    printf("=== Testing via fork/exec process method ===\n");
    
    /* Create test source file */
    FILE *fp = fopen("test_input.c", "w");
    if (!fp) {
        perror("Failed to create test_input.c");
        return;
    }
    fputs(test_source, fp);
    fclose(fp);
    
    /* Array of test invocations with different flags */
    char *test_invocations[][10] = {
        /* First: Set various flags including sysroot and dump options */
        {"gcc", "-save-temps", "-dumpdir", "/tmp/test_dump1", 
         "-dumpbase", "test_dumpbase1", "-c", "test_input.c", 
         "-o", "output1.o", NULL},
        
        /* Second: Reset to defaults (no special flags) */
        {"gcc", "-c", "test_input.c", "-o", "output2.o", NULL},
        
        /* Third: Force failure with invalid option */
        {"gcc", "-invalid-option-that-doesnt-exist", NULL},
        
        /* Fourth: Successful compilation again */
        {"gcc", "-c", "test_input.c", "-o", "output3.o", NULL},
        
        /* Fifth: Test with sysroot and machine spec changes */
        {"gcc", "--sysroot=/tmp/fake_sysroot", "-march=x86-64", 
         "-c", "test_input.c", "-o", "output4.o", NULL},
        
        /* Sixth: Reset again (no sysroot, default machine) */
        {"gcc", "-c", "test_input.c", "-o", "output5.o", NULL}
    };
    
    int num_tests = sizeof(test_invocations) / sizeof(test_invocations[0]);
    int status;
    pid_t pid;
    
    for (int i = 0; i < num_tests; i++) {
        printf("\nTest %d: ", i + 1);
        for (int j = 0; test_invocations[i][j] != NULL; j++) {
            printf("%s ", test_invocations[i][j]);
        }
        printf("\n");
        
        pid = fork();
        if (pid == 0) {
            /* Child process */
            execvp("gcc", test_invocations[i]);
            perror("execvp failed");
            exit(127);
        } else if (pid > 0) {
            /* Parent process */
            waitpid(pid, &status, 0);
            if (WIFEXITED(status)) {
                printf("Exit status: %d\n", WEXITSTATUS(status));
            } else {
                printf("Process terminated abnormally\n");
            }
        } else {
            perror("fork failed");
        }
        
        /* Small delay to ensure clean separation between invocations */
        usleep(10000);
    }
    
    /* Cleanup */
    unlink("test_input.c");
    unlink("output1.o");
    unlink("output2.o");
    unlink("output3.o");
    unlink("output4.o");
    unlink("output5.o");
}

/* Method B: Direct library call using dlopen/dlsym */
#ifdef DRIVER_TEST
void test_via_dlopen() {
    printf("\n=== Testing via dlopen/dlsym method ===\n");
    
    /* Try to load GCC driver as a shared library */
    void *handle = dlopen("libgccdriver.so", RTLD_LAZY | RTLD_LOCAL);
    if (!handle) {
        /* Try alternative names */
        handle = dlopen("./libgccdriver.so", RTLD_LAZY | RTLD_LOCAL);
    }
    
    if (!handle) {
        printf("Could not load libgccdriver.so: %s\n", dlerror());
        printf("Skipping dlopen test (driver may not be built as shared library)\n");
        return;
    }
    
    /* Look for driver entry point - could be main() or driver::main() */
    int (*driver_main)(int, char**) = dlsym(handle, "main");
    if (!driver_main) {
        driver_main = dlsym(handle, "_Z6driveriPPc");  // mangled name for driver::main
    }
    
    if (!driver_main) {
        printf("Could not find driver entry point: %s\n", dlerror());
        dlclose(handle);
        return;
    }
    
    /* Create test source file */
    FILE *fp = fopen("test_input2.c", "w");
    if (!fp) {
        perror("Failed to create test_input2.c");
        dlclose(handle);
        return;
    }
    fputs(test_source, fp);
    fclose(fp);
    
    /* Test invocations with different argument vectors */
    char *argv1[] = {"gcc", "-save-temps", "-dumpdir", "/tmp/dl_test", 
                     "-dumpbase", "dl_base", "-c", "test_input2.c", 
                     "-o", "dl_output1.o", NULL};
    char *argv2[] = {"gcc", "-c", "test_input2.c", "-o", "dl_output2.o", NULL};
    char *argv3[] = {"gcc", "-invalid-option-test", NULL};
    char *argv4[] = {"gcc", "--sysroot=/tmp/test_sysroot", 
                     "-c", "test_input2.c", "-o", "dl_output3.o", NULL};
    char *argv5[] = {"gcc", "-c", "test_input2.c", "-o", "dl_output4.o", NULL};
    
    int argc1 = sizeof(argv1)/sizeof(argv1[0]) - 1;
    int argc2 = sizeof(argv2)/sizeof(argv2[0]) - 1;
    int argc3 = sizeof(argv3)/sizeof(argv3[0]) - 1;
    int argc4 = sizeof(argv4)/sizeof(argv4[0]) - 1;
    int argc5 = sizeof(argv5)/sizeof(argv5[0]) - 1;
    
    printf("\nTest 1 (with dump flags): ");
    int result1 = driver_main(argc1, argv1);
    printf("Result: %d\n", result1);
    
    printf("Test 2 (reset to defaults): ");
    int result2 = driver_main(argc2, argv2);
    printf("Result: %d\n", result2);
    
    printf("Test 3 (force failure): ");
    int result3 = driver_main(argc3, argv3);
    printf("Result: %d\n", result3);
    
    printf("Test 4 (with sysroot): ");
    int result4 = driver_main(argc4, argv4);
    printf("Result: %d\n", result4);
    
    printf("Test 5 (reset again): ");
    int result5 = driver_main(argc5, argv5);
    printf("Result: %d\n", result5);
    
    /* Cleanup */
    unlink("test_input2.c");
    unlink("dl_output1.o");
    unlink("dl_output2.o");
    unlink("dl_output3.o");
    unlink("dl_output4.o");
    
    dlclose(handle);
}
#endif

/* Method C: Direct linking with driver object files */
#ifdef LINK_WITH_DRIVER
/* This would require including gcc.cc and linking with all driver objects */
extern int driver::main(int argc, char **argv);

void test_via_direct_link() {
    printf("\n=== Testing via direct linking ===\n");
    
    /* Similar test sequence as above */
    char *argv1[] = {"gcc", "-save-temps=obj", "-dumpdir", "./dumps",
                     "-dumpbase", "linked_test", "-c", "test.c", NULL};
    char *argv2[] = {"gcc", "-c", "test.c", NULL};
    
    int argc1 = sizeof(argv1)/sizeof(argv1[0]) - 1;
    int argc2 = sizeof(argv2)/sizeof(argv2[0]) - 1;
    
    printf("First call with dump flags...\n");
    int r1 = driver::main(argc1, argv1);
    
    printf("Second call without flags (should trigger reset)...\n");
    int r2 = driver::main(argc2, argv2);
    
    printf("Results: %d, %d\n", r1, r2);
}
#endif

int main(int argc, char **argv) {
    printf("GCC Driver Reinitialization Test Program\n");
    printf("========================================\n");
    
    /* Always run process-based test */
    test_via_process();
    
    /* Conditionally run dlopen test */
#ifdef DRIVER_TEST
    test_via_dlopen();
#endif
    
#ifdef LINK_WITH_DRIVER
    test_via_direct_link();
#endif
    
    printf("\n=== Test Complete ===\n");
    
    /* Create additional test to specifically trigger save_temps flag reset */
    printf("\n=== Additional save-temps specific test ===\n");
    
    /* Create another test file */
    FILE *fp = fopen("extra_test.c", "w");
    if (fp) {
        fputs("int foo() { return 42; }\n", fp);
        fclose(fp);
        
        /* Test sequence that should trigger save_temps_flag reset */
        pid_t pid = fork();
        if (pid == 0) {
            char *args1[] = {"gcc", "-save-temps=obj", "-dumpdir", "/tmp/extra",
                           "-dumpbase", "extra_base", "-c", "extra_test.c", 
                           "-o", "extra.o", NULL};
            execvp("gcc", args1);
            exit(1);
        }
        waitpid(pid, NULL, 0);
        
        pid = fork();
        if (pid == 0) {
            /* This should trigger the reset block */
            char *args2[] = {"gcc", "-c", "extra_test.c", "-o", "extra2.o", NULL};
            execvp("gcc", args2);
            exit(1);
        }
        waitpid(pid, NULL, 0);
        
        /* Cleanup */
        unlink("extra_test.c");
        unlink("extra.o");
        unlink("extra2.o");
    }
    
    return 0;
}
