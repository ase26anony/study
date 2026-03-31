/* test_gcc_cleanup.c - Test program to cover driver cleanup lines in gcc.cc */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>

#define GCC_PATH "./xgcc"  /* Path to the coverage-instrumented GCC driver */
#define TMP_DIR "/tmp/gcc_test_cover"

/* Create a minimal C source file */
static void create_test_source(const char *filename) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        perror("fopen");
        exit(1);
    }
    fprintf(f, "int main(void) { return 0; }\n");
    fclose(f);
}

/* Run GCC with specific flags to set various global state variables */
static void run_gcc_with_flags(const char **argv, int argc) {
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(1);
    }
    
    if (pid == 0) {
        /* Child process - execute GCC */
        execv(GCC_PATH, (char * const *)argv);
        perror("execv");
        exit(1);
    } else {
        /* Parent process - wait for completion */
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)) {
            printf("GCC exited with status: %d\n", WEXITSTATUS(status));
        }
    }
}

int main(void) {
    /* Create temporary directory */
    mkdir(TMP_DIR, 0755);
    
    /* Create test source file */
    char src_file[256];
    snprintf(src_file, sizeof(src_file), "%s/test.c", TMP_DIR);
    create_test_source(src_file);
    
    /* Test 1: Compile with multiple flags that set various global variables */
    printf("=== Test 1: Full compilation with state-altering flags ===\n");
    const char *argv1[] = {
        GCC_PATH,
        "-save-temps",                    /* Sets save_temps_flag */
        "-dumpdir", TMP_DIR,              /* Allocates dumpdir */
        "-dumpbase", "test_dump",         /* Allocates dumpbase */
        "-dumpbase-ext", ".ext",          /* Allocates dumpbase_ext */
        "--sysroot=/opt/mock_sysroot",    /* Sets target_system_root, target_system_root_changed */
        "-fuse-ld=gold",                  /* Sets use_ld */
        "-ftime-report",                  /* Sets report_times_to_file */
        "-v",                             /* Sets verbose_only_flag */
        "-specs=/dev/null",               /* Affects spec processing */
        "-mtune=cortex-a72",              /* Affects spec_machine */
        "-march=armv8-a",                 /* Affects spec_machine */
        "-o", "/dev/null",                /* Output to /dev/null to avoid file creation */
        src_file,
        NULL
    };
    
    /* Set environment variables that affect driver state */
    setenv("GCC_EXEC_PREFIX", "/usr/lib/gcc/", 1);
    setenv("COMPILER_PATH", "/usr/bin:/usr/local/bin", 1);
    
    run_gcc_with_flags(argv1, sizeof(argv1)/sizeof(argv1[0]) - 1);
    
    /* Test 2: Invocation with help/version flags */
    printf("\n=== Test 2: Help and version flags ===\n");
    const char *argv2[] = {
        GCC_PATH,
        "--help=common",                  /* Sets print_help_list */
        "--version",                      /* Sets print_version */
        "-###",                           /* May set print_subprocess_help */
        NULL
    };
    
    run_gcc_with_flags(argv2, sizeof(argv2)/sizeof(argv2[0]) - 1);
    
    /* Test 3: Another compilation with different dump options */
    printf("\n=== Test 3: Different dump configuration ===\n");
    const char *argv3[] = {
        GCC_PATH,
        "-save-temps=obj",                /* Different save_temps_flag value */
        "-dumpdir", "/tmp/another_dump",
        "-dumpbase", "another",
        "-o", "/dev/null",
        src_file,
        NULL
    };
    
    run_gcc_with_flags(argv3, sizeof(argv3)/sizeof(argv3[0]) - 1);
    
    /* Test 4: Cross-compilation scenario */
    printf("\n=== Test 4: Cross-compilation flags ===\n");
    const char *argv4[] = {
        GCC_PATH,
        "-target", "x86_64-linux-gnu",    /* Explicit target specification */
        "--sysroot=/cross/sysroot",
        "-isysroot", "/cross/headers",    /* Alternative sysroot flag */
        "-B", "/cross/lib",
        "-o", "/dev/null",
        src_file,
        NULL
    };
    
    run_gcc_with_flags(argv4, sizeof(argv4)/sizeof(argv4[0]) - 1);
    
    /* Cleanup */
    unlink(src_file);
    rmdir(TMP_DIR);
    
    printf("\nAll tests completed. The GCC driver's cleanup code should have been exercised.\n");
    return 0;
}
