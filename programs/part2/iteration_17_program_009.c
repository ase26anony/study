/* gcc_cleanup_test.c - Test program to cover driver cleanup lines in gcc.cc */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#define GCC_PATH "./gcc/xgcc"  /* Adjust this path to your gcc driver */
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

static void setup_environment(void) {
    /* Set environment variables that affect driver state */
    setenv("GCC_EXEC_PREFIX", "/usr/local/lib/gcc/", 1);
    setenv("COMPILER_PATH", "/usr/bin:/usr/local/bin", 1);
    setenv("LIBRARY_PATH", "/usr/lib:/usr/local/lib", 1);
}

int main(void) {
    /* Create temporary directory */
    mkdir(TMP_DIR, 0755);
    
    /* Create test source file */
    char src_path[256];
    snprintf(src_path, sizeof(src_path), "%s/test.c", TMP_DIR);
    create_test_source(src_path);
    
    /* Setup environment variables */
    setup_environment();
    
    /* Test 1: Comprehensive compilation with many state-altering flags */
    printf("=== Test 1: Comprehensive compilation ===\n");
    const char *argv1[] = {
        GCC_PATH,
        "-save-temps",                    /* sets save_temps_flag */
        "-dumpdir", TMP_DIR,              /* allocates dumpdir */
        "-dumpbase", "test_dump",         /* allocates dumpbase */
        "-dumpbase-ext", ".ext",          /* allocates dumpbase_ext */
        "--sysroot=/opt/mysysroot",       /* sets target_system_root, target_system_root_changed */
        "-fuse-ld=gold",                  /* sets use_ld */
        "-ftime-report",                  /* sets report_times_to_file */
        "-v",                             /* sets verbose_only_flag */
        "-specs=/dev/null",               /* triggers spec processing */
        "-mtune=native",                  /* may affect spec_machine */
        "-march=x86-64",                  /* may affect spec_machine */
        "-o", "/tmp/test_output.o",
        src_path,
        NULL
    };
    run_gcc_with_flags(argv1, 14);
    
    /* Test 2: Help and version flags */
    printf("\n=== Test 2: Help and version flags ===\n");
    const char *argv2[] = {
        GCC_PATH,
        "--help=common",                  /* sets print_help_list */
        "--version",                      /* sets print_version */
        "-###",                           /* may set print_subprocess_help */
        NULL
    };
    run_gcc_with_flags(argv2, 4);
    
    /* Test 3: Subprocess help with dump options */
    printf("\n=== Test 3: Subprocess help with dump options ===\n");
    const char *argv3[] = {
        GCC_PATH,
        "--help=subprocess",              /* sets print_subprocess_help */
        "-dumpdir", "/another/dir",
        "-dumpbase", "another",
        src_path,
        NULL
    };
    run_gcc_with_flags(argv3, 5);
    
    /* Test 4: Cross-compilation scenario */
    printf("\n=== Test 4: Cross-compilation flags ===\n");
    const char *argv4[] = {
        GCC_PATH,
        "-target", "x86_64-linux-gnu",
        "--sysroot=/cross/sysroot",
        "-isysroot", "/cross/headers",
        "-save-temps=obj",
        "-dumpdir", "",
        "-o", "/tmp/cross.o",
        src_path,
        NULL
    };
    run_gcc_with_flags(argv4, 11);
    
    /* Test 5: Minimal compilation to ensure normal cleanup path */
    printf("\n=== Test 5: Minimal compilation ===\n");
    const char *argv5[] = {
        GCC_PATH,
        "-c",
        "-o", "/tmp/minimal.o",
        src_path,
        NULL
    };
    run_gcc_with_flags(argv5, 4);
    
    /* Cleanup */
    unlink(src_path);
    rmdir(TMP_DIR);
    
    printf("\nAll tests completed. Check coverage data.\n");
    return 0;
}
