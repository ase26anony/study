/* gcc_cleanup_coverage.c - Test program to cover driver cleanup code */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

#define TEMP_SOURCE "/tmp/test_cover.c"
#define TEMP_OUTPUT "/tmp/test_cover.o"
#define DUMP_DIR "/tmp/gcc_dump"
#define SYSROOT_PATH "/tmp/test_sysroot"

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
}

/* Run GCC with specific flags to set driver state variables */
static int run_gcc_with_flags(const char *gcc_path, const char **args, int argc) {
    pid_t pid = fork();
    
    if (pid == 0) {
        /* Child process */
        execv(gcc_path, (char *const *)args);
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

int main(int argc, char *argv[]) {
    const char *gcc_path = "./xgcc";  /* Adjust based on your build directory */
    
    /* Check if GCC path was provided as argument */
    if (argc > 1) {
        gcc_path = argv[1];
    }
    
    printf("Testing GCC driver cleanup with: %s\n", gcc_path);
    
    /* Create test files */
    create_test_source();
    create_dummy_sysroot();
    
    /* Test 1: Complex compilation with many state-altering flags */
    printf("\n=== Test 1: Complex compilation with state flags ===\n");
    {
        const char *args1[] = {
            gcc_path,
            "-save-temps",              /* Sets save_temps_flag */
            "-dumpdir", DUMP_DIR,       /* Allocates dumpdir */
            "-dumpbase", "testdump",    /* Allocates dumpbase */
            "-dumpbase-ext", ".ext",    /* Allocates dumpbase_ext */
            "--sysroot=" SYSROOT_PATH,  /* Sets target_system_root, target_system_root_changed */
            "-fuse-ld=gold",            /* Sets use_ld */
            "-ftime-report",            /* Sets report_times_to_file */
            "-v",                       /* Sets verbose_only_flag */
            "-march=armv7-a",           /* May affect spec_machine */
            "-mtune=cortex-a8",         /* May affect spec_machine */
            TEMP_SOURCE,
            "-o", TEMP_OUTPUT,
            NULL
        };
        
        /* Set environment variables that affect driver state */
        setenv("GCC_EXEC_PREFIX", "/tmp/gcc_exec_prefix", 1);
        setenv("COMPILER_PATH", "/tmp/compiler_path:/usr/bin", 1);
        
        int result = run_gcc_with_flags(gcc_path, args1, 
                                       sizeof(args1)/sizeof(args1[0]) - 1);
        printf("Test 1 completed with exit code: %d\n", result);
        
        /* Clean environment for next test */
        unsetenv("GCC_EXEC_PREFIX");
        unsetenv("COMPILER_PATH");
    }
    
    /* Test 2: Help and version flags (different state variables) */
    printf("\n=== Test 2: Help and version flags ===\n");
    {
        const char *args2[] = {
            gcc_path,
            "--help=common",            /* Sets print_help_list */
            "--version",                /* Sets print_version */
            NULL
        };
        
        run_gcc_with_flags(gcc_path, args2, 
                          sizeof(args2)/sizeof(args2[0]) - 1);
        printf("Test 2 completed\n");
    }
    
    /* Test 3: Subprocess help flag */
    printf("\n=== Test 3: Subprocess help ===\n");
    {
        const char *args3[] = {
            gcc_path,
            "-###",                     /* May set print_subprocess_help */
            TEMP_SOURCE,
            NULL
        };
        
        run_gcc_with_flags(gcc_path, args3, 
                          sizeof(args3)/sizeof(args3[0]) - 1);
        printf("Test 3 completed\n");
    }
    
    /* Test 4: Different target configuration */
    printf("\n=== Test 4: Different target with isysroot ===\n");
    {
        const char *args4[] = {
            gcc_path,
            "-isysroot", SYSROOT_PATH,  /* Alternative sysroot setting */
            "-dumpdir", "/tmp/other_dump",
            "-dumpbase", "otherbase",
            "-save-temps=obj",
            TEMP_SOURCE,
            "-o", "/tmp/other_output.o",
            NULL
        };
        
        run_gcc_with_flags(gcc_path, args4, 
                          sizeof(args4)/sizeof(args4[0]) - 1);
        printf("Test 4 completed\n");
    }
    
    /* Test 5: Force error case to test cleanup after failure */
    printf("\n=== Test 5: Error case cleanup ===\n");
    {
        const char *args5[] = {
            gcc_path,
            "-save-temps",
            "-dumpdir", "/tmp/error_dump",
            "-x", "c",                  /* Specify language */
            "-",                        /* Read from stdin */
            "-o", "/tmp/error_output.o",
            NULL
        };
        
        /* Run with empty stdin */
        pid_t pid = fork();
        if (pid == 0) {
            /* Close stdin to force error */
            close(0);
            execv(gcc_path, (char *const *)args5);
            perror("execv failed");
            exit(1);
        } else if (pid > 0) {
            int status;
            waitpid(pid, &status, 0);
            printf("Test 5 (error case) completed with exit code: %d\n", 
                   WEXITSTATUS(status));
        }
    }
    
    /* Cleanup temporary files */
    unlink(TEMP_SOURCE);
    unlink(TEMP_OUTPUT);
    unlink("/tmp/other_output.o");
    
    /* Clean up dump files that might have been created */
    system("rm -f /tmp/testdump* /tmp/otherbase*");
    
    printf("\nAll tests completed. Driver cleanup code should have been exercised.\n");
    
    return 0;
}
