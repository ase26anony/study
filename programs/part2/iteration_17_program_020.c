/* test_gcc_cleanup.c - Test program to cover driver cleanup block in gcc.cc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

/* Create a minimal C source file for compilation */
void create_test_source(const char *filename) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        perror("fopen");
        exit(1);
    }
    fprintf(f, "int main(void) { return 0; }\n");
    fclose(f);
}

/* Execute GCC with specific flags to set driver state variables */
int run_gcc_with_flags(const char *gcc_path, const char *source_file, 
                       const char *output_file, int test_num) {
    pid_t pid;
    int status;
    
    /* Different flag combinations for different test cases */
    const char *flags[][20] = {
        /* Test 1: Comprehensive flags setting most variables */
        {
            gcc_path,
            "-save-temps",              /* sets save_temps_flag */
            "-dumpdir", "/tmp/gcc_dump",/* allocates dumpdir */
            "-dumpbase", "testdump",    /* allocates dumpbase */
            "-dumpbase-ext", ".ext",    /* allocates dumpbase_ext */
            "--sysroot=/opt/mysysroot", /* sets target_system_root, target_system_root_changed */
            "-fuse-ld=gold",            /* sets use_ld */
            "-ftime-report",            /* sets report_times_to_file */
            "-v",                       /* sets verbose_only_flag */
            "-specs=/dev/null",         /* affects spec processing */
            "-o", output_file,
            source_file,
            NULL
        },
        /* Test 2: Help and version flags */
        {
            gcc_path,
            "--help=common",            /* sets print_help_list */
            "--version",                /* sets print_version */
            "-v",                       /* verbose flag */
            NULL
        },
        /* Test 3: Subprocess help and machine specs */
        {
            gcc_path,
            "-###",                     /* may set print_subprocess_help */
            "-save-temps=obj",
            "-dumpdir", ".",
            "-dumpbase", "mydump",
            "-mtune=native",            /* affects spec_machine */
            "-march=x86-64",
            source_file,
            NULL
        },
        /* Test 4: Cross-compilation scenario */
        {
            gcc_path,
            "-save-temps",
            "--sysroot=/cross/root",
            "-target", "x86_64-linux-gnu",
            "-dumpdir", "/tmp/cross",
            "-dumpbase", "cross",
            "-v",
            "-o", "/tmp/cross.o",
            source_file,
            NULL
        }
    };
    
    if (test_num < 0 || test_num >= (int)(sizeof(flags)/sizeof(flags[0]))) {
        fprintf(stderr, "Invalid test number\n");
        return -1;
    }
    
    printf("Running test %d with flags:\n", test_num + 1);
    for (int i = 0; flags[test_num][i]; i++) {
        printf("%s ", flags[test_num][i]);
    }
    printf("\n\n");
    
    pid = fork();
    if (pid == 0) {
        /* Child process */
        
        /* Set environment variables that affect driver state */
        setenv("GCC_EXEC_PREFIX", "/usr/lib/gcc/", 1);
        setenv("COMPILER_PATH", "/usr/bin:/usr/local/bin", 1);
        setenv("LIBRARY_PATH", "/usr/lib:/usr/local/lib", 1);
        
        /* Execute GCC */
        execv(gcc_path, (char * const *)flags[test_num]);
        
        /* If execv fails */
        perror("execv");
        exit(1);
    } else if (pid > 0) {
        /* Parent process */
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status)) {
            printf("GCC exited with status %d\n\n", WEXITSTATUS(status));
            return WEXITSTATUS(status);
        } else {
            printf("GCC terminated abnormally\n\n");
            return -1;
        }
    } else {
        perror("fork");
        return -1;
    }
}

/* Test specific cleanup scenarios */
void test_cleanup_scenarios(const char *gcc_path) {
    char source_file[] = "/tmp/test_gcc_cover_XXXXXX.c";
    char output_file[] = "/tmp/test_gcc_out_XXXXXX.o";
    int fd;
    
    /* Create unique temporary filenames */
    strcpy(source_file, "/tmp/test_gcc_cover_XXXXXX.c");
    strcpy(output_file, "/tmp/test_gcc_out_XXXXXX.o");
    
    /* Create source file */
    create_test_source(source_file);
    
    printf("=== Testing GCC Driver Cleanup Block Coverage ===\n\n");
    
    /* Run multiple test cases to cover different state variable combinations */
    for (int i = 0; i < 4; i++) {
        printf("--- Test Case %d ---\n", i + 1);
        run_gcc_with_flags(gcc_path, source_file, output_file, i);
        
        /* Small delay between tests */
        sleep(1);
    }
    
    /* Additional test: Compile this wrapper with the instrumented GCC */
    printf("--- Test Case 5: Compile wrapper with instrumented GCC ---\n");
    {
        const char *compile_cmd[] = {
            gcc_path,
            "-O0",
            "-fprofile-arcs",
            "-ftest-coverage",
            "-save-temps",
            "-dumpdir", "/tmp/coverage_dump",
            "-dumpbase", "wrapper_coverage",
            "--sysroot=/",
            "-v",
            "-o", "/tmp/wrapper_test",
            __FILE__,
            NULL
        };
        
        pid_t pid = fork();
        if (pid == 0) {
            setenv("GCC_EXEC_PREFIX", "/test/prefix", 1);
            execv(gcc_path, (char * const *)compile_cmd);
            perror("execv");
            exit(1);
        } else if (pid > 0) {
            int status;
            waitpid(pid, &status, 0);
            printf("Compilation exit status: %d\n\n", 
                   WIFEXITED(status) ? WEXITSTATUS(status) : -1);
        }
    }
    
    /* Cleanup temporary files */
    unlink(source_file);
    unlink(output_file);
    unlink("/tmp/wrapper_test");
    
    /* Cleanup potential dump files */
    system("rm -f /tmp/gcc_dump* /tmp/dump* /tmp/cross* /tmp/coverage_dump* 2>/dev/null");
}

int main(int argc, char *argv[]) {
    const char *gcc_path;
    
    /* Determine GCC path to test */
    if (argc > 1) {
        gcc_path = argv[1];
    } else {
        /* Default to the GCC in the current directory */
        gcc_path = "./xgcc";
        
        /* Fallback to system GCC if not found */
        if (access(gcc_path, X_OK) != 0) {
            gcc_path = "gcc";
        }
    }
    
    printf("Testing GCC driver at: %s\n", gcc_path);
    
    /* Run the cleanup coverage tests */
    test_cleanup_scenarios(gcc_path);
    
    printf("=== All tests completed ===\n");
    printf("The driver's cleanup block should have been exercised multiple times.\n");
    printf("Check coverage data with: gcov -b gcc.cc\n");
    
    return 0;
}
