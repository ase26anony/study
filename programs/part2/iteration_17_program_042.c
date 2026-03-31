/* test_gcc_cleanup.c - Test program to cover driver cleanup lines in gcc.cc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

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

/* Run GCC with specific flags to set driver state variables */
static void run_gcc_with_flags(const char **argv, int argc) {
    pid_t pid = fork();
    
    if (pid == 0) {
        /* Child process */
        execv(GCC_PATH, (char * const *)argv);
        perror("execv");
        exit(1);
    } else if (pid > 0) {
        /* Parent process */
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)) {
            printf("GCC exited with status %d\n", WEXITSTATUS(status));
        }
    } else {
        perror("fork");
        exit(1);
    }
}

int main(void) {
    /* Create temporary directory */
    mkdir(TMP_DIR, 0755);
    
    /* Create test source file */
    char src_file[256];
    snprintf(src_file, sizeof(src_file), "%s/test.c", TMP_DIR);
    create_test_source(src_file);
    
    /* Set environment variables to affect driver state */
    setenv("GCC_EXEC_PREFIX", "/usr/lib/gcc/", 1);
    setenv("COMPILER_PATH", "/usr/bin:/usr/local/bin", 1);
    
    printf("=== Test 1: Compilation with multiple state-setting flags ===\n");
    
    /* First invocation: Comprehensive flags to set many state variables */
    const char *argv1[] = {
        GCC_PATH,
        "-save-temps",                    /* Sets save_temps_flag */
        "-dumpdir", TMP_DIR,              /* Allocates dumpdir */
        "-dumpbase", "test_dump",         /* Allocates dumpbase */
        "-dumpbase-ext", ".ext",          /* Allocates dumpbase_ext */
        "--sysroot=/opt/mysysroot",       /* Sets target_system_root, target_system_root_changed */
        "-fuse-ld=gold",                  /* Sets use_ld */
        "-ftime-report",                  /* Sets report_times_to_file */
        "-v",                             /* Sets verbose_only_flag in some contexts */
        "-specs=/dev/null",               /* Affects spec processing */
        "-mtune=cortex-a72",              /* Affects spec_machine */
        "-march=armv8-a",                 /* Further affects spec_machine */
        "-B", "/usr/lib/gcc",             /* Adds prefix to compiler search path */
        "-c",                             /* Compile only */
        "-o", "/tmp/test.o",              /* Output file */
        src_file,                         /* Source file */
        NULL
    };
    
    run_gcc_with_flags(argv1, sizeof(argv1)/sizeof(argv1[0]) - 1);
    
    printf("\n=== Test 2: Help and version flags ===\n");
    
    /* Second invocation: Help and version flags */
    const char *argv2[] = {
        GCC_PATH,
        "--help=common",                  /* Sets print_help_list */
        "--version",                      /* Sets print_version */
        "-###",                           /* May set print_subprocess_help */
        src_file,
        NULL
    };
    
    run_gcc_with_flags(argv2, sizeof(argv2)/sizeof(argv2[0]) - 1);
    
    printf("\n=== Test 3: More state variables ===\n");
    
    /* Third invocation: Different combination */
    const char *argv3[] = {
        GCC_PATH,
        "-save-temps=obj",                /* Different save_temps_flag value */
        "-dumpdir", "/tmp/another/dump/", /* Different dumpdir with trailing slash */
        "-dumpbase", "another",
        "-isysroot", "/opt/headers",      /* Alternative sysroot flag */
        "-fuse-ld=bfd",
        "-ftime-report",
        "-wrapper", "/usr/bin/wrapper",   /* Additional flag */
        "-c",
        "-o", "/tmp/another.o",
        src_file,
        NULL
    };
    
    run_gcc_with_flags(argv3, sizeof(argv3)/sizeof(argv3[0]) - 1);
    
    printf("\n=== Test 4: Error case to ensure cleanup still happens ===\n");
    
    /* Fourth invocation: Invalid input to trigger error but still cleanup */
    const char *argv4[] = {
        GCC_PATH,
        "-save-temps",
        "-dumpdir", TMP_DIR,
        "-dumpbase", "error_test",
        "--sysroot=/invalid/path",
        "-c",
        "-o", "/tmp/error.o",
        "/nonexistent/file.c",           /* This will cause an error */
        NULL
    };
    
    run_gcc_with_flags(argv4, sizeof(argv4)/sizeof(argv4[0]) - 1);
    
    /* Cleanup temporary files */
    unlink(src_file);
    rmdir(TMP_DIR);
    
    printf("\n=== All tests completed ===\n");
    printf("The GCC driver should have executed cleanup code for each invocation.\n");
    
    return 0;
}
