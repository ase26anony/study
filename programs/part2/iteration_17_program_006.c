/* test_gcc_cleanup.c - Test program to cover GCC driver cleanup logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

#define GCC_PATH "./gcc/xgcc"  /* Adjust based on your build directory */
#define TMP_DIR "/tmp/gcc_cover_test"

static void create_test_source(const char *path) {
    FILE *f = fopen(path, "w");
    if (!f) {
        perror("fopen");
        exit(1);
    }
    fprintf(f, "int main(void) { return 0; }\n");
    fclose(f);
}

static void run_gcc_with_flags(const char **argv, int argc) {
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        exit(1);
    }
    
    if (pid == 0) { /* Child process */
        /* Set environment variables to affect driver state */
        setenv("GCC_EXEC_PREFIX", "/usr/local/lib/gcc/", 1);
        setenv("COMPILER_PATH", "/usr/bin:/usr/local/bin", 1);
        setenv("LIBRARY_PATH", "/usr/lib:/usr/local/lib", 1);
        
        /* Execute GCC with all the state-altering flags */
        execv(GCC_PATH, (char * const *)argv);
        perror("execv");
        exit(1);
    } else { /* Parent process */
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)) {
            printf("GCC exited with status %d\n", WEXITSTATUS(status));
        } else {
            printf("GCC terminated abnormally\n");
        }
    }
}

static void test_combination_1(void) {
    printf("=== Test Combination 1: Full compilation with dump options ===\n");
    
    /* Create test directory */
    mkdir(TMP_DIR, 0755);
    
    const char *source_path = TMP_DIR "/test.c";
    const char *output_path = TMP_DIR "/test.o";
    create_test_source(source_path);
    
    /* Complex command line to set multiple state variables */
    const char *argv[] = {
        GCC_PATH,
        "-save-temps",              /* sets save_temps_flag */
        "-dumpdir", TMP_DIR "/dump",/* allocates dumpdir */
        "-dumpbase", "test_dump",   /* allocates dumpbase */
        "-dumpbase-ext", ".ext",    /* allocates dumpbase_ext */
        "--sysroot=/opt/mysysroot", /* sets target_system_root, target_system_root_changed */
        "-fuse-ld=gold",            /* sets use_ld */
        "-ftime-report",            /* sets report_times_to_file */
        "-v",                       /* sets verbose_only_flag */
        "-march=armv7-a",           /* affects spec_machine */
        "-mtune=cortex-a8",
        "-isystem", "/usr/include/extra",
        "-idirafter", "/usr/include/after",
        "-B", "/usr/local/lib/gcc",
        "-specs=/usr/share/gcc/specs",
        source_path,
        "-o", output_path,
        NULL
    };
    
    run_gcc_with_flags(argv, sizeof(argv)/sizeof(argv[0]) - 1);
    
    /* Cleanup test files */
    unlink(source_path);
    unlink(output_path);
    /* Remove any dump files created */
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "rm -rf %s/dump*", TMP_DIR);
    system(cmd);
}

static void test_combination_2(void) {
    printf("\n=== Test Combination 2: Help and version flags ===\n");
    
    /* This combination sets print_help_list and print_version */
    const char *argv[] = {
        GCC_PATH,
        "--help=common",            /* sets print_help_list */
        "--version",                /* sets print_version */
        "-###",                     /* may set print_subprocess_help */
        "-v",
        NULL
    };
    
    run_gcc_with_flags(argv, sizeof(argv)/sizeof(argv[0]) - 1);
}

static void test_combination_3(void) {
    printf("\n=== Test Combination 3: Subprocess help and verbose ===\n");
    
    const char *source_path = TMP_DIR "/test2.c";
    create_test_source(source_path);
    
    /* Test with outbase option */
    const char *argv[] = {
        GCC_PATH,
        "--help=subprocess",        /* sets print_subprocess_help */
        "-save-temps=obj",
        "-dumpdir", ".",
        "-dumpbase", "myprog",
        "-dumpbase-ext", ".c",
        "-o", TMP_DIR "/output",
        "-V", "4.9.0",              /* Version specific flag */
        "-frandom-seed=12345",
        "-g", "-O2",
        "-ftree-vectorize",
        "-fdump-tree-all",
        "-fdump-rtl-all",
        "-wrapper", "/bin/true",
        source_path,
        NULL
    };
    
    run_gcc_with_flags(argv, sizeof(argv)/sizeof(argv[0]) - 1);
    
    unlink(source_path);
    unlink(TMP_DIR "/output");
}

static void test_combination_4(void) {
    printf("\n=== Test Combination 4: Cross-compilation scenario ===\n");
    
    const char *source_path = TMP_DIR "/cross.c";
    create_test_source(source_path);
    
    /* Simulate cross-compilation with extensive sysroot options */
    const char *argv[] = {
        GCC_PATH,
        "-target", "arm-linux-gnueabihf",  /* Force cross-compilation mode */
        "--sysroot=/opt/arm-sysroot",
        "-isysroot", "/opt/arm-headers",
        "-I/opt/arm-sysroot/usr/include",
        "-L/opt/arm-sysroot/usr/lib",
        "-B/opt/arm-sysroot/usr/bin",
        "-save-temps=cwd",
        "-dumpdir", "",
        "-dumpbase", "cross_compile",
        "-dumpbase-ext", ".dump",
        "-specs=/opt/arm-sysroot/specs",
        "-mcpu=cortex-a53",
        "-mfloat-abi=hard",
        "-mfpu=neon-vfpv4",
        "-v",
        "-ftime-report",
        "-fuse-ld=bfd",
        source_path,
        "-o", TMP_DIR "/cross.o",
        NULL
    };
    
    run_gcc_with_flags(argv, sizeof(argv)/sizeof(argv[0]) - 1);
    
    unlink(source_path);
    unlink(TMP_DIR "/cross.o");
}

static void test_combination_5(void) {
    printf("\n=== Test Combination 5: Error case to trigger cleanup ===\n");
    
    /* Test with invalid combination to ensure cleanup still happens */
    const char *argv[] = {
        GCC_PATH,
        "-save-temps",
        "-dumpdir", "/nonexistent/path/dump",
        "-dumpbase", "error_test",
        "-x", "c",  /* Specify language */
        "-",        /* Read from stdin */
        "-o", "/dev/null",
        "-E",       /* Preprocessor only */
        NULL
    };
    
    /* Provide some input via pipe */
    pid_t pid = fork();
    if (pid == 0) {
        setenv("GCC_EXEC_PREFIX", "/test/prefix", 1);
        
        /* Create a pipe for stdin */
        int pipefd[2];
        if (pipe(pipefd) == -1) {
            perror("pipe");
            exit(1);
        }
        
        if (fork() == 0) {
            close(pipefd[0]);
            write(pipefd[1], "int x = 0;\n", 11);
            close(pipefd[1]);
            exit(0);
        }
        
        close(pipefd[1]);
        dup2(pipefd[0], STDIN_FILENO);
        close(pipefd[0]);
        
        execv(GCC_PATH, (char * const *)argv);
        perror("execv");
        exit(1);
    } else {
        int status;
        waitpid(pid, &status, 0);
    }
}

int main(void) {
    printf("Starting GCC driver cleanup coverage test\n");
    
    /* Create temporary directory */
    mkdir(TMP_DIR, 0755);
    
    /* Run multiple test combinations to maximize coverage */
    test_combination_1();
    test_combination_2();
    test_combination_3();
    test_combination_4();
    test_combination_5();
    
    /* Cleanup temporary directory */
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "rm -rf %s", TMP_DIR);
    system(cmd);
    
    printf("\nAll tests completed. Check coverage data for gcc.cc\n");
    return 0;
}
