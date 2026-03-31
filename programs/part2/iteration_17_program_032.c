/* test_gcc_cleanup.c - Test program to cover GCC driver cleanup code */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

#define GCC_PATH "./xgcc"  /* Adjust based on your build directory */
#define TMP_DIR "/tmp/gcc_test_cover"

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
    
    if (pid == 0) {
        /* Child process */
        execv(argv[0], (char * const *)argv);
        perror("execv");
        exit(1);
    } else {
        /* Parent process */
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)) {
            printf("GCC exited with status %d\n", WEXITSTATUS(status));
        }
    }
}

static void cleanup_temp_files(void) {
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "rm -rf %s/* 2>/dev/null", TMP_DIR);
    system(cmd);
}

int main(void) {
    /* Create temporary directory */
    mkdir(TMP_DIR, 0755);
    
    /* Create test source file */
    char src_path[256];
    snprintf(src_path, sizeof(src_path), "%s/test.c", TMP_DIR);
    create_test_source(src_path);
    
    /* Set environment variables to affect driver state */
    setenv("GCC_EXEC_PREFIX", "/usr/lib/gcc/", 1);
    setenv("COMPILER_PATH", "/usr/bin:/usr/local/bin", 1);
    setenv("LIBRARY_PATH", "/usr/lib:/usr/local/lib", 1);
    
    printf("=== Test 1: Full compilation with multiple state-altering flags ===\n");
    {
        const char *argv[] = {
            GCC_PATH,
            "-save-temps",                    /* sets save_temps_flag */
            "-dumpdir", TMP_DIR,              /* allocates dumpdir */
            "-dumpbase", "test_dump",         /* allocates dumpbase */
            "-dumpbase-ext", ".ext",          /* allocates dumpbase_ext */
            "--sysroot=/opt/mysysroot",       /* sets target_system_root, target_system_root_changed */
            "-fuse-ld=gold",                  /* sets use_ld */
            "-ftime-report",                  /* sets report_times_to_file */
            "-v",                             /* sets verbose_only_flag */
            "-mtune=native",                  /* affects spec_machine */
            "-march=x86-64",                  /* affects spec_machine */
            "-specs=/dev/null",               /* triggers spec processing */
            "-o", "/dev/null",                /* output to /dev/null to avoid file creation */
            src_path,
            NULL
        };
        run_gcc_with_flags(argv, 14);
    }
    
    printf("\n=== Test 2: Help and version flags ===\n");
    {
        const char *argv[] = {
            GCC_PATH,
            "--help=common",                  /* sets print_help_list */
            "--version",                      /* sets print_version */
            "-v",                             /* verbose flag */
            NULL
        };
        run_gcc_with_flags(argv, 4);
    }
    
    printf("\n=== Test 3: Subprocess help and verbose ===\n");
    {
        const char *argv[] = {
            GCC_PATH,
            "-###",                           /* may set print_subprocess_help */
            "-v",
            src_path,
            NULL
        };
        run_gcc_with_flags(argv, 4);
    }
    
    printf("\n=== Test 4: Cross-compilation-like flags ===\n");
    {
        const char *argv[] = {
            GCC_PATH,
            "-target", "x86_64-linux-gnu",    /* explicit target */
            "-isysroot", "/opt/sysroot",      /* alternative sysroot flag */
            "-B", "/usr/x86_64-linux-gnu/bin", /* add search path */
            "-save-temps=obj",                /* different save_temps value */
            "-dumpdir", "/tmp/another_dump",
            "-dumpbase", "cross_test",
            "-o", "/dev/null",
            src_path,
            NULL
        };
        run_gcc_with_flags(argv, 12);
    }
    
    printf("\n=== Test 5: Minimal compilation to trigger normal cleanup ===\n");
    {
        const char *argv[] = {
            GCC_PATH,
            "-c",                             /* compile only */
            "-o", "/dev/null",
            src_path,
            NULL
        };
        run_gcc_with_flags(argv, 4);
    }
    
    /* Cleanup */
    cleanup_temp_files();
    rmdir(TMP_DIR);
    
    printf("\nAll tests completed. Check coverage with:\n");
    printf("  gcov -b gcc.cc\n");
    
    return 0;
}
