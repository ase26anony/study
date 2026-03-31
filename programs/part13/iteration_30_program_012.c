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

/* Create a minimal C source file for compilation */
const char* test_source = 
"#include <stdio.h>\n"
"int main() {\n"
"    printf(\"Hello from test program\\n\");\n"
"    return 0;\n"
"}\n";

/* Method A: Process-based invocation using fork/exec */
void test_via_processes() {
    printf("=== Testing via fork/exec processes ===\n");
    
    /* Write test source file */
    FILE* f = fopen("test_input.c", "w");
    if (!f) {
        perror("Failed to create test_input.c");
        return;
    }
    fputs(test_source, f);
    fclose(f);
    
    /* Array of test invocations with different flags */
    char* test_invocations[][10] = {
        /* First: Set various flags including sysroot and dump options */
        {"gcc", "-save-temps", "-dumpdir", "/tmp/test_dump1", 
         "-dumpbase", "test_dumpbase", "-c", "test_input.c", 
         "-o", "test_output1.o", NULL},
        
        /* Second: Reset to defaults (no special flags) */
        {"gcc", "-c", "test_input.c", "-o", "test_output2.o", NULL},
        
        /* Third: Force failure with invalid option */
        {"gcc", "-invalid-option-that-does-not-exist", "test_input.c", NULL},
        
        /* Fourth: Successful compilation again */
        {"gcc", "-c", "test_input.c", "-o", "test_output3.o", NULL},
        
        /* Fifth: Test with sysroot and machine specs */
        {"gcc", "--sysroot=/tmp/fake_sysroot", "-march=x86-64", 
         "-mtune=generic", "-c", "test_input.c", "-o", "test_output4.o", NULL},
        
        /* Sixth: Back to defaults */
        {"gcc", "-c", "test_input.c", "-o", "test_output5.o", NULL}
    };
    
    int num_tests = sizeof(test_invocations) / sizeof(test_invocations[0]);
    
    for (int i = 0; i < num_tests; i++) {
        pid_t pid = fork();
        
        if (pid == 0) {
            /* Child process */
            printf("Test %d: ", i+1);
            for (int j = 0; test_invocations[i][j]; j++) {
                printf("%s ", test_invocations[i][j]);
            }
            printf("\n");
            
            execvp("gcc", test_invocations[i]);
            
            /* If execvp fails */
            perror("execvp failed");
            exit(EXIT_FAILURE);
        } 
        else if (pid > 0) {
            /* Parent process */
            int status;
            waitpid(pid, &status, 0);
            
            if (WIFEXITED(status)) {
                printf("  Exit status: %d\n", WEXITSTATUS(status));
            } else if (WIFSIGNALED(status)) {
                printf("  Terminated by signal: %d\n", WTERMSIG(status));
            }
        } 
        else {
            perror("fork failed");
        }
    }
    
    /* Cleanup */
    unlink("test_input.c");
    unlink("test_output1.o");
    unlink("test_output2.o");
    unlink("test_output3.o");
    unlink("test_output4.o");
    unlink("test_output5.o");
}

/* Method B: Direct library call using dlopen (if available) */
#ifdef USE_DLOPEN
void test_via_dlopen() {
    printf("\n=== Testing via dlopen (if driver is built as shared library) ===\n");
    
    void* handle = dlopen("./libgccdriver.so", RTLD_LAZY | RTLD_LOCAL);
    if (!handle) {
        printf("Could not load libgccdriver.so: %s\n", dlerror());
        printf("Trying with absolute path...\n");
        
        char libpath[1024];
        snprintf(libpath, sizeof(libpath), "%s/libgccdriver.so", 
                 getenv("GCC_BUILD_DIR") ?: ".");
        handle = dlopen(libpath, RTLD_LAZY | RTLD_LOCAL);
    }
    
    if (!handle) {
        printf("Skipping dlopen test: %s\n", dlerror());
        return;
    }
    
    /* Look for driver entry point - could be main() or driver::main() */
    int (*driver_main)(int, char**) = dlsym(handle, "main");
    if (!driver_main) {
        driver_main = dlsym(handle, "_Z6driver4mainiPPc");  // mangled name for driver::main
    }
    
    if (!driver_main) {
        printf("Could not find driver entry point: %s\n", dlerror());
        dlclose(handle);
        return;
    }
    
    /* Create test source file */
    FILE* f = fopen("dl_test.c", "w");
    if (f) {
        fputs(test_source, f);
        fclose(f);
    }
    
    /* Test 1: With save-temps and dumpdir */
    char* args1[] = {
        "gcc", "-save-temps", "-dumpdir", "/tmp/dl_test",
        "-dumpbase", "dl_base", "-c", "dl_test.c",
        "-o", "dl_output1.o", NULL
    };
    
    /* Test 2: Reset to defaults */
    char* args2[] = {
        "gcc", "-c", "dl_test.c", "-o", "dl_output2.o", NULL
    };
    
    /* Test 3: Force failure */
    char* args3[] = {
        "gcc", "-invalid-flag-for-failure", NULL
    };
    
    /* Test 4: Success again */
    char* args4[] = {
        "gcc", "-c", "dl_test.c", "-o", "dl_output3.o", NULL
    };
    
    printf("Call 1: With save-temps and dumpdir\n");
    int ret1 = driver_main(10, args1);
    printf("  Return: %d\n", ret1);
    
    printf("Call 2: Reset to defaults\n");
    int ret2 = driver_main(5, args2);
    printf("  Return: %d\n", ret2);
    
    printf("Call 3: Invalid option (should fail)\n");
    int ret3 = driver_main(2, args3);
    printf("  Return: %d\n", ret3);
    
    printf("Call 4: Valid again\n");
    int ret4 = driver_main(5, args4);
    printf("  Return: %d\n", ret4);
    
    /* Cleanup */
    unlink("dl_test.c");
    unlink("dl_output1.o");
    unlink("dl_output2.o");
    unlink("dl_output3.o");
    
    dlclose(handle);
}
#endif

/* Method C: Direct compilation with driver code (requires special build) */
#ifdef COMPILE_WITH_DRIVER
/* This would require including gcc.cc and linking with driver objects */
extern int driver_main(int argc, char** argv);

void test_direct_call() {
    printf("\n=== Testing via direct function call ===\n");
    
    /* Write test source */
    FILE* f = fopen("direct_test.c", "w");
    if (f) {
        fputs(test_source, f);
        fclose(f);
    }
    
    /* Multiple invocations with different states */
    char* args_list[][8] = {
        {"gcc", "--sysroot=/tmp/alt_root", "-save-temps", 
         "-dumpbase", "direct", "-c", "direct_test.c", NULL},
        {"gcc", "-c", "direct_test.c", NULL},
        {"gcc", "-nonexistent-option", NULL},
        {"gcc", "-c", "direct_test.c", "-o", "direct_out.o", NULL}
    };
    
    for (int i = 0; i < 4; i++) {
        printf("Direct call %d\n", i+1);
        int ret = driver_main(
            sizeof(args_list[i])/sizeof(args_list[i][0]) - 1,
            args_list[i]
        );
        printf("  Return: %d\n", ret);
    }
    
    unlink("direct_test.c");
    unlink("direct_out.o");
}
#endif

/* Helper to create a fake sysroot for testing */
void create_fake_sysroot() {
    system("mkdir -p /tmp/fake_sysroot/usr/include 2>/dev/null");
    system("mkdir -p /tmp/fake_sysroot/usr/lib 2>/dev/null");
    
    /* Create minimal headers */
    FILE* f = fopen("/tmp/fake_sysroot/usr/include/stdio.h", "w");
    if (f) {
        fputs("#ifndef _STDIO_H\n#define _STDIO_H\ntypedef struct FILE FILE;\nextern int printf(const char*, ...);\n#endif\n", f);
        fclose(f);
    }
}

int main(int argc, char** argv) {
    printf("Driver Reinitialization Test Program\n");
    printf("Targeting uncovered lines in gcc.cc (11228-11250)\n\n");
    
    /* Create fake sysroot for sysroot tests */
    create_fake_sysroot();
    
    /* Test using processes (most reliable) */
    test_via_processes();
    
    /* Try dlopen method if requested */
    if (argc > 1 && strcmp(argv[1], "--dlopen") == 0) {
#ifdef USE_DLOPEN
        test_via_dlopen();
#else
        printf("\nDLOPEN method not enabled in this build.\n");
        printf("Compile with: gcc -DUSE_DLOPEN -ldl -o test test_driver_reinit.c\n");
#endif
    }
    
    /* Try direct call if requested */
    if (argc > 1 && strcmp(argv[1], "--direct") == 0) {
#ifdef COMPILE_WITH_DRIVER
        test_direct_call();
#else
        printf("\nDirect call method not enabled.\n");
        printf("This requires linking with gcc driver objects.\n");
#endif
    }
    
    /* Additional test: Simulate multi-stage compilation pipeline */
    printf("\n=== Testing multi-stage compilation pipeline ===\n");
    
    /* Write a more complex test file */
    const char* complex_source = 
    "#define TEST_MACRO 42\n"
    "int add(int a, int b) { return a + b + TEST_MACRO; }\n"
    "int main() { return add(1, 2); }\n";
    
    FILE* cf = fopen("complex.c", "w");
    if (cf) {
        fputs(complex_source, cf);
        fclose(cf);
    }
    
    /* Simulate compilation pipeline with different flags per stage */
    char* pipeline[][8] = {
        /* Preprocess with dump options */
        {"gcc", "-E", "-save-temps", "-dumpdir", "/tmp/pipe", 
         "complex.c", "-o", "complex.i", NULL},
        
        /* Compile to assembly (reset dumpdir) */
        {"gcc", "-S", "complex.i", "-o", "complex.s", NULL},
        
        /* Assemble with different output base */
        {"gcc", "-c", "complex.s", "-o", "complex.o", 
         "-dumpbase", "asm_base", NULL},
        
        /* Link (full reset) */
        {"gcc", "complex.o", "-o", "complex.exe", NULL}
    };
    
    for (int i = 0; i < 4; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            printf("Pipeline stage %d: ", i+1);
            for (int j = 0; pipeline[i][j]; j++) {
                printf("%s ", pipeline[i][j]);
            }
            printf("\n");
            
            execvp("gcc", pipeline[i]);
            perror("execvp");
            exit(1);
        } else if (pid > 0) {
            int status;
            waitpid(pid, &status, 0);
            printf("  Stage %d exit: %d\n", i+1, WEXITSTATUS(status));
        }
    }
    
    /* Cleanup pipeline files */
    unlink("complex.c");
    unlink("complex.i");
    unlink("complex.s");
    unlink("complex.o");
    unlink("complex.exe");
    
    printf("\n=== Test Summary ===\n");
    printf("The following driver state variables should have been reset:\n");
    printf("  - save_temps_flag (SAVE_TEMPS_NONE)\n");
    printf("  - dumpdir, dumpbase, dumpbase_ext, outbase (NULL)\n");
    printf("  - target_system_root (DEFAULT_TARGET_SYSTEM_ROOT)\n");
    printf("  - spec_machine (DEFAULT_TARGET_MACHINE)\n");
    printf("  - greatest_status (1)\n");
    printf("\nCheck coverage report to verify lines 11228-11250 were executed.\n");
    
    return 0;
}
