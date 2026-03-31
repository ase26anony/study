/* test_driver_reinit.c - Test program to trigger GCC driver reinitialization logic */
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
#define USE_DLOPEN 0

/* Simple test C source file content */
const char *test_source = 
"#include <stdio.h>\n"
"int main() {\n"
"    printf(\"Hello from test program\\n\");\n"
"    return 0;\n"
"}\n";

/* Create test input file */
static int create_test_file(const char *filename) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        perror("fopen");
        return 0;
    }
    fputs(test_source, f);
    fclose(f);
    return 1;
}

/* Method A: Process-based invocation using fork/exec */
static int invoke_gcc_process(const char *gcc_path, char *const argv[]) {
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

/* Method B: Direct library call using dlopen/dlsym */
#if USE_DLOPEN
static int invoke_gcc_library(char *const argv[]) {
    static void *gcc_lib = NULL;
    static int (*gcc_main)(int, char **) = NULL;
    
    if (!gcc_lib) {
        gcc_lib = dlopen("libgccdriver.so", RTLD_LAZY | RTLD_LOCAL);
        if (!gcc_lib) {
            fprintf(stderr, "dlopen failed: %s\n", dlerror());
            return -1;
        }
        
        gcc_main = (int (*)(int, char **))dlsym(gcc_lib, "main");
        if (!gcc_main) {
            fprintf(stderr, "dlsym failed: %s\n", dlerror());
            dlclose(gcc_lib);
            return -1;
        }
    }
    
    /* Count arguments */
    int argc = 0;
    while (argv[argc]) argc++;
    
    return gcc_main(argc, argv);
}
#endif

int main(int argc, char *argv[]) {
    const char *gcc_path = "gcc";
    const char *input_file = "test_input.c";
    int overall_status = 0;
    
    /* Create test input file */
    if (!create_test_file(input_file)) {
        fprintf(stderr, "Failed to create test file\n");
        return 1;
    }
    
    printf("=== Testing GCC Driver Reinitialization ===\n\n");
    
    /* Test 1: First invocation with various flags to set state */
    printf("Test 1: Setting driver state with various flags\n");
    printf("------------------------------------------------\n");
    char *args1[] = {
        (char *)gcc_path,
        "-save-temps",           /* Sets save_temps_flag */
        "-dumpdir", "/tmp/test_dump", /* Sets dumpdir */
        "-dumpbase", "test_dumpbase", /* Sets dumpbase */
        "-dumpbase-ext", ".c",   /* Sets dumpbase_ext */
        "--sysroot=/alt/sysroot", /* Changes target_system_root */
        "-march=native",         /* May affect spec_machine */
        "-v",                    /* Verbose flag */
        "-c", (char *)input_file,
        "-o", "output1.o",
        NULL
    };
    
    int status1 = invoke_gcc_process(gcc_path, args1);
    printf("Status 1: %d\n\n", status1);
    
    /* Test 2: Second invocation without special flags - should trigger reset */
    printf("Test 2: Invocation without special flags (triggering reset)\n");
    printf("-----------------------------------------------------------\n");
    char *args2[] = {
        (char *)gcc_path,
        "-c", (char *)input_file,
        "-o", "output2.o",
        NULL
    };
    
    int status2 = invoke_gcc_process(gcc_path, args2);
    printf("Status 2: %d\n\n", status2);
    
    /* Test 3: Third invocation with invalid option to force failure */
    printf("Test 3: Invalid option to set greatest_status\n");
    printf("---------------------------------------------\n");
    char *args3[] = {
        (char *)gcc_path,
        "-invalid-option-that-does-not-exist",
        "-c", (char *)input_file,
        "-o", "output3.o",
        NULL
    };
    
    int status3 = invoke_gcc_process(gcc_path, args3);
    printf("Status 3: %d (should be non-zero)\n\n", status3);
    
    /* Test 4: Fourth invocation - successful compilation after failure */
    printf("Test 4: Successful compilation after failure\n");
    printf("-------------------------------------------\n");
    char *args4[] = {
        (char *)gcc_path,
        "-c", (char *)input_file,
        "-o", "output4.o",
        NULL
    };
    
    int status4 = invoke_gcc_process(gcc_path, args4);
    printf("Status 4: %d (should be 0)\n\n", status4);
    
    /* Test 5: Multi-stage compilation simulation */
    printf("Test 5: Multi-stage compilation pipeline\n");
    printf("---------------------------------------\n");
    
    /* Stage 1: Preprocessing with save-temps */
    char *args5a[] = {
        (char *)gcc_path,
        "-save-temps=obj",
        "-dumpdir", "pipeline_dump",
        "-E", (char *)input_file,
        "-o", "test_input.i",
        NULL
    };
    
    printf("Stage 1: Preprocessing\n");
    int status5a = invoke_gcc_process(gcc_path, args5a);
    printf("Status 5a: %d\n", status5a);
    
    /* Stage 2: Compilation with different dumpbase */
    char *args5b[] = {
        (char *)gcc_path,
        "-save-temps=cwd",
        "-dumpbase", "pipeline_compile",
        "-S", "test_input.i",
        "-o", "test_input.s",
        NULL
    };
    
    printf("\nStage 2: Compilation\n");
    int status5b = invoke_gcc_process(gcc_path, args5b);
    printf("Status 5b: %d\n", status5b);
    
    /* Stage 3: Assembly with no special flags */
    char *args5c[] = {
        (char *)gcc_path,
        "-c", "test_input.s",
        "-o", "test_input.o",
        NULL
    };
    
    printf("\nStage 3: Assembly\n");
    int status5c = invoke_gcc_process(gcc_path, args5c);
    printf("Status 5c: %d\n", status5c);
    
    /* Test 6: Using specs file to manipulate spec_machine */
    printf("\nTest 6: Testing spec_machine reset\n");
    printf("----------------------------------\n");
    
    /* Create a dummy specs file */
    FILE *specs = fopen("test.specs", "w");
    if (specs) {
        fprintf(specs, "*cpp:\n");
        fprintf(specs, "-DTEST_SPEC\n");
        fclose(specs);
        
        char *args6a[] = {
            (char *)gcc_path,
            "-specs=test.specs",
            "-c", (char *)input_file,
            "-o", "spec_test.o",
            NULL
        };
        
        printf("With specs file:\n");
        int status6a = invoke_gcc_process(gcc_path, args6a);
        printf("Status 6a: %d\n", status6a);
        
        /* Follow with invocation without specs */
        char *args6b[] = {
            (char *)gcc_path,
            "-c", (char *)input_file,
            "-o", "no_spec_test.o",
            NULL
        };
        
        printf("\nWithout specs file (should reset):\n");
        int status6b = invoke_gcc_process(gcc_path, args6b);
        printf("Status 6b: %d\n", status6b);
        
        unlink("test.specs");
    }
    
    /* Cleanup */
    unlink(input_file);
    unlink("test_input.i");
    unlink("test_input.s");
    unlink("test_input.o");
    unlink("output1.o");
    unlink("output2.o");
    unlink("output4.o");
    unlink("spec_test.o");
    unlink("no_spec_test.o");
    
    printf("\n=== Test Summary ===\n");
    printf("Test 1 (set state): %d\n", status1);
    printf("Test 2 (reset): %d\n", status2);
    printf("Test 3 (failure): %d\n", status3);
    printf("Test 4 (recovery): %d\n", status4);
    printf("Multi-stage: %d,%d,%d\n", status5a, status5b, status5c);
    
    return overall_status;
}
