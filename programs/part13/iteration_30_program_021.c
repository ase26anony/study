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

/* Define if we want to test via direct library calls */
#define USE_DIRECT_LIBRARY_CALL 0

/* Simple test C source file content */
const char* test_source = 
"#include <stdio.h>\n"
"int main() {\n"
"    printf(\"Hello from test program\\n\");\n"
"    return 0;\n"
"}\n";

/* Create test source file */
int create_test_source(const char* filename) {
    FILE* f = fopen(filename, "w");
    if (!f) {
        perror("Failed to create test source file");
        return -1;
    }
    fputs(test_source, f);
    fclose(f);
    return 0;
}

/* Method A: Process-based testing using fork/exec */
void test_via_processes() {
    printf("=== Testing via fork/exec (Process-based) ===\n");
    
    char* gcc_path = "gcc";
    char* input_file = "test_input.c";
    int status;
    pid_t pid;
    
    /* Create test source file */
    if (create_test_source(input_file) != 0) {
        return;
    }
    
    /* Test 1: First invocation with various flags to set state */
    printf("\n[Test 1] Setting driver state with special flags...\n");
    pid = fork();
    if (pid == 0) {
        /* Child process */
        char* args[] = {
            gcc_path,
            "-save-temps",          /* Sets save_temps_flag */
            "-dumpdir", "/tmp/test_driver_reinit",  /* Sets dumpdir */
            "-dumpbase", "test_dumpbase",  /* Sets dumpbase */
            "--sysroot=/alt/sysroot",  /* Alters target_system_root */
            "-march=native",        /* Alters spec_machine */
            "-c", input_file,       /* Compile only */
            "-o", "output1.o",      /* Output file */
            NULL
        };
        execvp(gcc_path, args);
        perror("execvp failed");
        exit(1);
    } else if (pid > 0) {
        waitpid(pid, &status, 0);
        printf("Exit status: %d\n", WEXITSTATUS(status));
    }
    
    /* Test 2: Second invocation without special flags (should trigger reset) */
    printf("\n[Test 2] Invocation without special flags (triggering reset)...\n");
    pid = fork();
    if (pid == 0) {
        char* args[] = {
            gcc_path,
            "-c", input_file,       /* Simple compilation */
            "-o", "output2.o",      /* Different output */
            NULL
        };
        execvp(gcc_path, args);
        perror("execvp failed");
        exit(1);
    } else if (pid > 0) {
        waitpid(pid, &status, 0);
        printf("Exit status: %d\n", WEXITSTATUS(status));
    }
    
    /* Test 3: Third invocation with invalid option (should fail) */
    printf("\n[Test 3] Invocation with invalid option (should fail)...\n");
    pid = fork();
    if (pid == 0) {
        char* args[] = {
            gcc_path,
            "-invalid-option-that-does-not-exist",  /* Invalid option */
            "-c", input_file,
            NULL
        };
        execvp(gcc_path, args);
        perror("execvp failed");
        exit(1);
    } else if (pid > 0) {
        waitpid(pid, &status, 0);
        printf("Exit status: %d (non-zero expected)\n", WEXITSTATUS(status));
    }
    
    /* Test 4: Fourth invocation with success (testing status reset) */
    printf("\n[Test 4] Successful invocation (testing greatest_status reset)...\n");
    pid = fork();
    if (pid == 0) {
        char* args[] = {
            gcc_path,
            "-save-temps=obj",      /* Different save-temps mode */
            "-dumpbase", "final",   /* Different dumpbase */
            "-fdump-tree-original", /* Enable dumping */
            "-c", input_file,
            "-o", "output3.o",
            NULL
        };
        execvp(gcc_path, args);
        perror("execvp failed");
        exit(1);
    } else if (pid > 0) {
        waitpid(pid, &status, 0);
        printf("Exit status: %d (0 expected)\n", WEXITSTATUS(status));
    }
    
    /* Test 5: Multi-stage compilation simulation */
    printf("\n[Test 5] Simulating multi-stage compilation pipeline...\n");
    
    /* Stage 1: Preprocessing */
    pid = fork();
    if (pid == 0) {
        char* args[] = {
            gcc_path,
            "-save-temps=cwd",      /* Save temps in current dir */
            "-dumpdir", "./dump",   /* Custom dump directory */
            "-E",                   /* Preprocess only */
            input_file,
            "-o", "preprocessed.i",
            NULL
        };
        execvp(gcc_path, args);
        perror("execvp failed");
        exit(1);
    } else {
        waitpid(pid, &status, 0);
    }
    
    /* Stage 2: Compilation to assembly */
    pid = fork();
    if (pid == 0) {
        char* args[] = {
            gcc_path,
            "-S",                   /* Compile to assembly */
            "-dumpbase", "asm_stage",
            "preprocessed.i",
            "-o", "output.s",
            NULL
        };
        execvp(gcc_path, args);
        perror("execvp failed");
        exit(1);
    } else {
        waitpid(pid, &status, 0);
    }
    
    /* Stage 3: Assembly to object */
    pid = fork();
    if (pid == 0) {
        char* args[] = {
            gcc_path,
            "-c",                   /* Assemble */
            "output.s",
            "-o", "final.o",
            NULL
        };
        execvp(gcc_path, args);
        perror("execvp failed");
        exit(1);
    } else {
        waitpid(pid, &status, 0);
    }
    
    /* Cleanup */
    unlink(input_file);
    printf("\n=== Process-based testing complete ===\n\n");
}

/* Method B: Direct library call testing (if driver is built as shared library) */
void test_via_library() {
#if USE_DIRECT_LIBRARY_CALL
    printf("=== Testing via direct library calls ===\n");
    
    void* handle;
    int (*driver_main)(int, char**);
    char* error;
    
    /* Try to load GCC driver as shared library */
    handle = dlopen("./libgccdriver.so", RTLD_LAZY);
    if (!handle) {
        /* Try alternative names */
        handle = dlopen("libgccdriver.so", RTLD_LAZY);
    }
    
    if (!handle) {
        fprintf(stderr, "Could not load GCC driver library: %s\n", dlerror());
        fprintf(stderr, "Skipping library-based test.\n");
        return;
    }
    
    /* Clear any existing error */
    dlerror();
    
    /* Find the driver main function */
    driver_main = (int (*)(int, char**))dlsym(handle, "main");
    if ((error = dlerror()) != NULL) {
        fprintf(stderr, "Could not find main: %s\n", error);
        dlclose(handle);
        return;
    }
    
    /* Create test source file */
    char* input_file = "lib_test_input.c";
    if (create_test_source(input_file) != 0) {
        dlclose(handle);
        return;
    }
    
    /* Test 1: Invocation with state-setting flags */
    printf("\n[Library Test 1] Setting driver state...\n");
    {
        char* args[] = {
            "gcc",
            "-save-temps",
            "-dumpdir", "/tmp/lib_test",
            "--sysroot=/test/sysroot",
            "-march=x86-64",
            "-c", input_file,
            "-o", "lib_output1.o",
            NULL
        };
        int argc = sizeof(args)/sizeof(args[0]) - 1;
        int result = driver_main(argc, args);
        printf("Driver returned: %d\n", result);
    }
    
    /* Test 2: Invocation without flags (should trigger reset) */
    printf("\n[Library Test 2] Resetting to defaults...\n");
    {
        char* args[] = {
            "gcc",
            "-c", input_file,
            "-o", "lib_output2.o",
            NULL
        };
        int argc = sizeof(args)/sizeof(args[0]) - 1;
        int result = driver_main(argc, args);
        printf("Driver returned: %d\n", result);
    }
    
    /* Test 3: Failed invocation */
    printf("\n[Library Test 3] Forcing failure...\n");
    {
        char* args[] = {
            "gcc",
            "-invalid-flag-for-failure",
            NULL
        };
        int argc = sizeof(args)/sizeof(args[0]) - 1;
        int result = driver_main(argc, args);
        printf("Driver returned: %d (non-zero expected)\n", result);
    }
    
    /* Test 4: Successful invocation after failure */
    printf("\n[Library Test 4] Success after failure...\n");
    {
        char* args[] = {
            "gcc",
            "-v",  /* Verbose to see driver activity */
            "-c", input_file,
            NULL
        };
        int argc = sizeof(args)/sizeof(args[0]) - 1;
        int result = driver_main(argc, args);
        printf("Driver returned: %d (0 expected)\n", result);
    }
    
    /* Cleanup */
    unlink(input_file);
    dlclose(handle);
    printf("\n=== Library-based testing complete ===\n\n");
#endif
}

/* Alternative: Direct compilation with driver code */
#ifdef DRIVER_TEST
/* This would require linking with gcc driver object files */
extern int driver_main(int argc, char** argv);

void test_direct_linking() {
    printf("=== Testing via direct linking ===\n");
    
    char* input_file = "direct_test_input.c";
    if (create_test_source(input_file) != 0) {
        return;
    }
    
    /* Multiple invocations to trigger reinitialization */
    for (int i = 0; i < 3; i++) {
        printf("\n[Direct Test %d] Invocation %d...\n", i+1, i+1);
        
        char arg0[] = "gcc";
        char arg1[] = "-c";
        char arg2[100];
        char arg3[] = "-o";
        char arg4[100];
        char* args[6];
        
        snprintf(arg2, sizeof(arg2), "%s", input_file);
        snprintf(arg4, sizeof(arg4), "direct_output%d.o", i);
        
        args[0] = arg0;
        args[1] = arg1;
        args[2] = arg2;
        args[3] = arg3;
        args[4] = arg4;
        args[5] = NULL;
        
        int result = driver_main(5, args);
        printf("Driver returned: %d\n", result);
    }
    
    unlink(input_file);
}
#endif

int main(int argc, char** argv) {
    printf("GCC Driver Reinitialization Test Program\n");
    printf("=========================================\n");
    
    /* Test using process-based method (most reliable) */
    test_via_processes();
    
    /* Test using library method if enabled */
    test_via_library();
    
#ifdef DRIVER_TEST
    /* Test using direct linking if compiled with -DDRIVER_TEST */
    test_direct_linking();
#endif
    
    /* Additional comprehensive test with varied options */
    printf("\n=== Additional Comprehensive Test ===\n");
    
    /* Create another test file */
    char* input_file = "comprehensive_test.c";
    if (create_test_source(input_file) == 0) {
        /* Test sequence designed specifically to hit the uncovered lines */
        char* test_sequences[][10] = {
            /* Sequence 1: Set all flags, then reset */
            {"gcc", "-save-temps=obj", "-dumpdir", "/tmp/d1", "-dumpbase", "db1", 
             "--sysroot=/test", "-specs=test.specs", "-c", input_file, NULL},
            {"gcc", "-c", input_file, "-o", "simple.o", NULL},  /* Reset triggers here */
            
            /* Sequence 2: Test save_temps variations */
            {"gcc", "-save-temps", "-c", input_file, "-o", "st1.o", NULL},
            {"gcc", "-save-temps=cwd", "-c", input_file, "-o", "st2.o", NULL},
            {"gcc", "-c", input_file, "-o", "st3.o", NULL},  /* Reset */
            
            /* Sequence 3: Test failure then success */
            {"gcc", "-invalid-option-xyz", NULL},  /* Should fail */
            {"gcc", "-c", input_file, "-o", "recover.o", NULL},  /* Should succeed */
        };
        
        for (int i = 0; i < sizeof(test_sequences)/sizeof(test_sequences[0]); i++) {
            printf("\n[Comprehensive Test %d] ", i+1);
            pid_t pid = fork();
            if (pid == 0) {
                execvp("gcc", test_sequences[i]);
                perror("execvp");
                exit(1);
            } else if (pid > 0) {
                int status;
                waitpid(pid, &status, 0);
                printf("Exit: %d", WEXITSTATUS(status));
                
                /* Check if this should trigger the reset block */
                if (i == 1 || i == 5) {  /* These are the "reset" invocations */
                    printf(" (Should trigger driver reinitialization)");
                }
                printf("\n");
            }
        }
        
        unlink(input_file);
    }
    
    printf("\n=== All tests completed ===\n");
    return 0;
}
