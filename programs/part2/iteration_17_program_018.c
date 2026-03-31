/* test_gcc_cleanup.c - Test program to cover driver cleanup block */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

#define TEMP_SOURCE "/tmp/test_cover_gcc.c"
#define TEMP_OUTPUT "/tmp/test_cover_gcc.o"
#define DUMP_DIR "/tmp/gcc_dump_test"
#define SYSROOT_PATH "/tmp/fake_sysroot"

/* Create a minimal C source file */
static void create_test_source(void) {
    FILE *f = fopen(TEMP_SOURCE, "w");
    if (!f) {
        perror("Failed to create test source");
        exit(1);
    }
    fprintf(f, "int main(void) { return 0; }\n");
    fclose(f);
}

/* Create a dummy sysroot directory structure */
static void create_dummy_sysroot(void) {
    mkdir(SYSROOT_PATH, 0755);
    mkdir(SYSROOT_PATH "/usr", 0755);
    mkdir(SYSROOT_PATH "/usr/include", 0755);
    mkdir(SYSROOT_PATH "/usr/lib", 0755);
    
    /* Create dummy header to avoid warnings */
    FILE *h = fopen(SYSROOT_PATH "/usr/include/stdio.h", "w");
    if (h) {
        fprintf(h, "#ifndef _STDIO_H\n#define _STDIO_H\ntypedef struct FILE FILE;\n#endif\n");
        fclose(h);
    }
}

/* Run GCC with specific flags to set driver state */
static int run_gcc_with_flags(const char *gcc_path, const char **argv, int argc) {
    pid_t pid = fork();
    if (pid == 0) {
        /* Child process */
        execv(gcc_path, (char *const *)argv);
        perror("execv failed");
        exit(1);
    } else if (pid > 0) {
        /* Parent process */
        int status;
        waitpid(pid, &status, 0);
        return WEXITSTATUS(status);
    } else {
        perror("fork failed");
        return -1;
    }
}

int main(int argc, char **argv) {
    const char *gcc_path = "./xgcc";  /* Path to the GCC driver under test */
    
    /* Check if GCC exists */
    if (access(gcc_path, X_OK) != 0) {
        /* Try alternative paths */
        gcc_path = "../gcc/xgcc";
        if (access(gcc_path, X_OK) != 0) {
            gcc_path = "xgcc";
            if (access(gcc_path, X_OK) != 0) {
                fprintf(stderr, "Could not find GCC driver at ./xgcc\n");
                return 1;
            }
        }
    }
    
    printf("Testing GCC driver cleanup with: %s\n", gcc_path);
    
    /* Create test files */
    create_test_source();
    create_dummy_sysroot();
    
    /* Set environment variables to affect driver state */
    setenv("GCC_EXEC_PREFIX", "/tmp/gcc_exec_prefix", 1);
    setenv("COMPILER_PATH", "/tmp/compiler_path:/usr/bin", 1);
    setenv("COLLECT_GCC_OPTIONS", "-v -save-temps", 1);
    
    /* Test 1: Comprehensive compilation with many state-altering flags */
    printf("\n=== Test 1: Comprehensive compilation ===\n");
    {
        const char *gcc_argv[] = {
            gcc_path,
            "-save-temps",                    /* Sets save_temps_flag */
            "-dumpdir", DUMP_DIR,             /* Allocates dumpdir */
            "-dumpbase", "test_dumpbase",     /* Allocates dumpbase */
            "-dumpbase-ext", ".ext",          /* Allocates dumpbase_ext */
            "--sysroot=" SYSROOT_PATH,        /* Sets target_system_root */
            "-fuse-ld=gold",                  /* Sets use_ld */
            "-ftime-report",                  /* Sets report_times_to_file */
            "-v",                             /* Sets verbose_only_flag */
            "-specs=/dev/null",               /* Affects spec processing */
            "-mtune=cortex-a72",              /* Affects spec_machine */
            "-march=armv8-a",                 /* Further affects spec_machine */
            "-o", TEMP_OUTPUT,
            TEMP_SOURCE,
            NULL
        };
        
        int ret = run_gcc_with_flags(gcc_path, gcc_argv, 
                                    sizeof(gcc_argv)/sizeof(gcc_argv[0]) - 1);
        printf("Test 1 completed with exit code: %d\n", ret);
    }
    
    /* Test 2: Help and version flags */
    printf("\n=== Test 2: Help and version flags ===\n");
    {
        const char *gcc_argv[] = {
            gcc_path,
            "--help=common",                  /* Sets print_help_list */
            "--version",                      /* Sets print_version */
            "-v",                             /* Sets verbose_only_flag */
            NULL
        };
        
        int ret = run_gcc_with_flags(gcc_path, gcc_argv, 
                                    sizeof(gcc_argv)/sizeof(gcc_argv[0]) - 1);
        printf("Test 2 completed with exit code: %d\n", ret);
    }
    
    /* Test 3: Subprocess help and verbose output */
    printf("\n=== Test 3: Subprocess help ===\n");
    {
        const char *gcc_argv[] = {
            gcc_path,
            "-###",                           /* May set print_subprocess_help */
            "-v",
            "-save-temps=obj",
            "-dumpdir", "/tmp/another_dump",
            TEMP_SOURCE,
            NULL
        };
        
        int ret = run_gcc_with_flags(gcc_path, gcc_argv, 
                                    sizeof(gcc_argv)/sizeof(gcc_argv[0]) - 1);
        printf("Test 3 completed with exit code: %d\n", ret);
    }
    
    /* Test 4: Different target configuration */
    printf("\n=== Test 4: Cross-compilation-like flags ===\n");
    {
        const char *gcc_argv[] = {
            gcc_path,
            "-target", "x86_64-linux-gnu",    /* Affects target configuration */
            "--sysroot=" SYSROOT_PATH,
            "-isysroot", SYSROOT_PATH,        /* Alternative sysroot flag */
            "-save-temps=cwd",
            "-dumpbase", "cross_compile",
            "-o", "/tmp/cross.o",
            TEMP_SOURCE,
            NULL
        };
        
        int ret = run_gcc_with_flags(gcc_path, gcc_argv, 
                                    sizeof(gcc_argv)/sizeof(gcc_argv[0]) - 1);
        printf("Test 4 completed with exit code: %d\n", ret);
    }
    
    /* Test 5: Error case to ensure cleanup still happens */
    printf("\n=== Test 5: Error case ===\n");
    {
        const char *gcc_argv[] = {
            gcc_path,
            "-save-temps",
            "-dumpdir", DUMP_DIR "/subdir",
            "-dumpbase", "error_test",
            "-invalid-flag",                  /* Should cause error */
            TEMP_SOURCE,
            NULL
        };
        
        int ret = run_gcc_with_flags(gcc_path, gcc_argv, 
                                    sizeof(gcc_argv)/sizeof(gcc_argv[0]) - 1);
        printf("Test 5 (error case) completed with exit code: %d\n", ret);
    }
    
    /* Cleanup temporary files */
    unlink(TEMP_SOURCE);
    unlink(TEMP_OUTPUT);
    unlink("/tmp/cross.o");
    
    /* Cleanup dump directory contents */
    system("rm -rf " DUMP_DIR " /tmp/another_dump /tmp/gcc_dump_test");
    
    printf("\nAll tests completed. Driver cleanup block should be covered.\n");
    return 0;
}
