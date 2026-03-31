/* test_gcc_cleanup.c - Test program to cover driver cleanup block in gcc.cc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

#define GCC_PATH "./xgcc"  /* Adjust based on your build directory */
#define TEMP_DIR "/tmp/gcc_test_cover"

/* Create a minimal C source file for compilation */
static int create_test_source(const char *filename) {
    FILE *f = fopen(filename, "w");
    if (!f) return -1;
    fprintf(f, "int main(void) { return 0; }\n");
    fclose(f);
    return 0;
}

/* Run GCC with specific flags to set various global state variables */
static int run_gcc_with_flags(const char **argv, int argc) {
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return -1;
    }
    
    if (pid == 0) { /* Child process */
        /* Set environment variables that affect driver state */
        setenv("GCC_EXEC_PREFIX", "/usr/local/lib/gcc/", 1);
        setenv("COMPILER_PATH", "/usr/bin:/usr/local/bin", 1);
        setenv("LIBRARY_PATH", "/usr/lib:/usr/local/lib", 1);
        
        /* Execute GCC driver */
        execv(GCC_PATH, (char *const *)argv);
        
        /* If execv fails */
        perror("execv");
        exit(EXIT_FAILURE);
    } else { /* Parent process */
        int status;
        waitpid(pid, &status, 0);
        return WEXITSTATUS(status);
    }
}

int main(void) {
    int ret = 0;
    
    /* Create temporary directory for test files */
    mkdir(TEMP_DIR, 0755);
    
    /* Test 1: Compile with multiple state-altering flags */
    printf("=== Test 1: Full compilation with state flags ===\n");
    {
        const char *source_file = TEMP_DIR "/test1.c";
        const char *output_file = TEMP_DIR "/test1.o";
        
        if (create_test_source(source_file) < 0) {
            fprintf(stderr, "Failed to create test source\n");
            return 1;
        }
        
        /* Construct command line with flags that set the uncovered variables */
        const char *argv[] = {
            GCC_PATH,
            "-save-temps",              /* sets save_temps_flag */
            "-dumpdir", TEMP_DIR "/dump", /* allocates dumpdir */
            "-dumpbase", "testdump",    /* allocates dumpbase */
            "-dumpbase-ext", ".ext",    /* allocates dumpbase_ext */
            "--sysroot=" TEMP_DIR "/sysroot", /* sets target_system_root */
            "-fuse-ld=gold",            /* sets use_ld */
            "-ftime-report",            /* sets report_times_to_file */
            "-v",                       /* sets verbose_only_flag */
            "-mtune=native",            /* affects spec_machine */
            "-march=x86-64",            /* affects spec_machine */
            "-o", output_file,
            source_file,
            NULL
        };
        
        int argc = sizeof(argv)/sizeof(argv[0]) - 1;
        ret = run_gcc_with_flags(argv, argc);
        printf("Test 1 exit code: %d\n", ret);
    }
    
    /* Test 2: Help and version flags (different state variables) */
    printf("\n=== Test 2: Help and version flags ===\n");
    {
        const char *argv[] = {
            GCC_PATH,
            "--help=common",            /* sets print_help_list */
            "--version",                /* sets print_version */
            "-###",                     /* may set print_subprocess_help */
            NULL
        };
        
        int argc = sizeof(argv)/sizeof(argv[0]) - 1;
        ret = run_gcc_with_flags(argv, argc);
        printf("Test 2 exit code: %d\n", ret);
    }
    
    /* Test 3: Cross-compilation scenario */
    printf("\n=== Test 3: Cross-compilation flags ===\n");
    {
        const char *source_file = TEMP_DIR "/test3.c";
        const char *output_file = TEMP_DIR "/test3.elf";
        
        if (create_test_source(source_file) < 0) {
            fprintf(stderr, "Failed to create test source\n");
            return 1;
        }
        
        const char *argv[] = {
            GCC_PATH,
            "-target", "arm-linux-gnueabihf",  /* Cross-compilation target */
            "--sysroot=" TEMP_DIR "/arm-sysroot",
            "-isysroot", TEMP_DIR "/arm-headers", /* target_sysroot_hdrs_suffix */
            "-save-temps=obj",
            "-dumpdir", TEMP_DIR "/arm_dump",
            "-dumpbase", "arm_test",
            "-o", output_file,
            source_file,
            NULL
        };
        
        int argc = sizeof(argv)/sizeof(argv[0]) - 1;
        ret = run_gcc_with_flags(argv, argc);
        printf("Test 3 exit code: %d\n", ret);
    }
    
    /* Test 4: Multiple dumpdir variations */
    printf("\n=== Test 4: Dump directory variations ===\n");
    {
        const char *source_file = TEMP_DIR "/test4.c";
        
        if (create_test_source(source_file) < 0) {
            fprintf(stderr, "Failed to create test source\n");
            return 1;
        }
        
        /* Test dumpdir with trailing dash (dumpdir_trailing_dash_added) */
        const char *argv[] = {
            GCC_PATH,
            "-dumpdir", TEMP_DIR "/dump-",  /* Should trigger trailing dash logic */
            "-dumpbase", "variation",
            "-save-temps",
            "-c",
            source_file,
            NULL
        };
        
        int argc = sizeof(argv)/sizeof(argv[0]) - 1;
        ret = run_gcc_with_flags(argv, argc);
        printf("Test 4 exit code: %d\n", ret);
    }
    
    /* Cleanup temporary files */
    char cleanup_cmd[256];
    snprintf(cleanup_cmd, sizeof(cleanup_cmd), "rm -rf %s", TEMP_DIR);
    system(cleanup_cmd);
    
    printf("\nAll tests completed. Check coverage with:\n");
    printf("  gcov gcc.cc\n");
    
    return 0;
}
