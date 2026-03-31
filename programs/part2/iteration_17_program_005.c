/* test_gcc_cleanup.c - Test program to cover driver cleanup lines in gcc.cc */
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

/* Run GCC with specific flags to set state variables */
int run_gcc_with_flags(const char *gcc_path, const char *source_file, 
                       const char *output_file, int test_num) {
    char dumpdir[256];
    char sysroot[256];
    
    /* Create unique dump directory for each test */
    snprintf(dumpdir, sizeof(dumpdir), "/tmp/gcc_dump_%d_%d", getpid(), test_num);
    snprintf(sysroot, sizeof(sysroot), "/tmp/sysroot_%d_%d", getpid(), test_num);
    
    /* Create dump directory */
    mkdir(dumpdir, 0755);
    
    /* Build command line with flags that set the target state variables */
    const char *argv[] = {
        gcc_path,
        /* Flags that set variables in the uncovered block */
        "-save-temps",                    /* sets save_temps_flag */
        "-dumpdir", dumpdir,              /* allocates dumpdir */
        "-dumpbase", "test_dumpbase",     /* allocates dumpbase */
        "-dumpbase-ext", ".ext",          /* allocates dumpbase_ext */
        "--sysroot=", sysroot,            /* sets target_system_root, target_system_root_changed */
        "-fuse-ld=gold",                  /* sets use_ld */
        "-ftime-report",                  /* sets report_times_to_file */
        "-v",                             /* sets verbose_only_flag in some contexts */
        "-specs=/dev/null",               /* affects spec processing */
        "-o", output_file,
        source_file,
        NULL
    };
    
    printf("Test %d: Running GCC with cleanup-triggering flags\n", test_num);
    printf("Command:");
    for (int i = 0; argv[i]; i++) {
        printf(" %s", argv[i]);
    }
    printf("\n");
    
    pid_t pid = fork();
    if (pid == 0) {
        /* Child process */
        /* Set environment variables that affect driver state */
        setenv("GCC_EXEC_PREFIX", "/tmp/gcc_exec_prefix", 1);
        setenv("COMPILER_PATH", "/tmp/compiler_path:/usr/bin", 1);
        
        execv(gcc_path, (char *const *)argv);
        /* If execv fails */
        perror("execv");
        exit(1);
    } else if (pid > 0) {
        /* Parent process */
        int status;
        waitpid(pid, &status, 0);
        
        /* Clean up temporary directories */
        char cmd[512];
        snprintf(cmd, sizeof(cmd), "rm -rf %s %s", dumpdir, sysroot);
        system(cmd);
        
        return WEXITSTATUS(status);
    } else {
        perror("fork");
        return -1;
    }
}

/* Test with help/version flags that set print_help_list and print_version */
int run_gcc_help_version(const char *gcc_path, int test_num) {
    printf("Test %d: Running GCC with help/version flags\n", test_num);
    
    /* Test 1: --help with categories */
    const char *argv1[] = {
        gcc_path,
        "--help=common",                  /* sets print_help_list */
        "--version",                      /* sets print_version */
        "-v",                             /* verbose flag */
        NULL
    };
    
    pid_t pid = fork();
    if (pid == 0) {
        execv(gcc_path, (char *const *)argv1);
        perror("execv");
        exit(1);
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
        return WEXITSTATUS(status);
    }
    return -1;
}

/* Test with subprocess help flag */
int run_gcc_subprocess_help(const char *gcc_path, int test_num) {
    printf("Test %d: Running GCC with subprocess help flag\n", test_num);
    
    const char *argv[] = {
        gcc_path,
        "-###",                           /* may set print_subprocess_help */
        "-E",                             /* preprocess only */
        "-dM",                            /* dump macros */
        "-",                              /* read from stdin */
        NULL
    };
    
    pid_t pid = fork();
    if (pid == 0) {
        /* Provide some input on stdin */
        const char *input = "#define TEST 1\n";
        write(STDIN_FILENO, input, strlen(input));
        close(STDIN_FILENO);
        
        execv(gcc_path, (char *const *)argv);
        perror("execv");
        exit(1);
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
        return WEXITSTATUS(status);
    }
    return -1;
}

/* Test with target-specific options to affect spec_machine */
int run_gcc_with_target_options(const char *gcc_path, const char *source_file, 
                                int test_num) {
    printf("Test %d: Running GCC with target-specific options\n", test_num);
    
    const char *output_file = "/tmp/test_target.o";
    const char *argv[] = {
        gcc_path,
        "-march=armv7-a",                 /* affects target machine specs */
        "-mtune=cortex-a8",               /* more target-specific */
        "--sysroot=/opt/arm-sysroot",     /* target system root */
        "-save-temps",
        "-dumpdir", "/tmp/arm_dump",
        "-dumpbase", "arm_test",
        "-o", output_file,
        source_file,
        NULL
    };
    
    pid_t pid = fork();
    if (pid == 0) {
        execv(gcc_path, (char *const *)argv);
        perror("execv");
        exit(1);
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
        
        /* Clean up output file */
        unlink(output_file);
        unlink("/tmp/arm_test.i");  /* -save-temps intermediate files */
        unlink("/tmp/arm_test.s");
        
        return WEXITSTATUS(status);
    }
    return -1;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <path-to-gcc>\n", argv[0]);
        fprintf(stderr, "Example: %s ./gcc/xgcc\n", argv[0]);
        return 1;
    }
    
    const char *gcc_path = argv[1];
    
    /* Create a test source file */
    char source_file[] = "/tmp/test_gcc_cleanup_XXXXXX.c";
    int fd = mkstemps(source_file, 2);  /* Create temp file with .c extension */
    if (fd < 0) {
        perror("mkstemps");
        return 1;
    }
    close(fd);
    
    create_test_source(source_file);
    
    int test_count = 0;
    int passed = 0;
    
    /* Test 1: Compilation with multiple state-setting flags */
    char output_file1[] = "/tmp/test_output1_XXXXXX.o";
    int fd1 = mkstemps(output_file1, 2);
    if (fd1 >= 0) close(fd1);
    
    if (run_gcc_with_flags(gcc_path, source_file, output_file1, ++test_count) == 0) {
        printf("Test %d PASSED\n", test_count);
        passed++;
    } else {
        printf("Test %d FAILED\n", test_count);
    }
    
    /* Clean up output files from test 1 */
    unlink(output_file1);
    unlink("/tmp/test_dumpbase.i");
    unlink("/tmp/test_dumpbase.s");
    unlink("/tmp/test_dumpbase.o");
    
    /* Test 2: Help and version flags */
    if (run_gcc_help_version(gcc_path, ++test_count) == 0) {
        printf("Test %d PASSED\n", test_count);
        passed++;
    } else {
        printf("Test %d FAILED\n", test_count);
    }
    
    /* Test 3: Subprocess help flag */
    if (run_gcc_subprocess_help(gcc_path, ++test_count) == 0) {
        printf("Test %d PASSED\n", test_count);
        passed++;
    } else {
        printf("Test %d FAILED\n", test_count);
    }
    
    /* Test 4: Target-specific options */
    if (run_gcc_with_target_options(gcc_path, source_file, ++test_count) == 0) {
        printf("Test %d PASSED\n", test_count);
        passed++;
    } else {
        printf("Test %d FAILED\n", test_count);
    }
    
    /* Clean up source file */
    unlink(source_file);
    
    printf("\nSummary: %d/%d tests passed\n", passed, test_count);
    
    return (passed == test_count) ? 0 : 1;
}
