/* gcc_cleanup_test.c - Test program to cover driver cleanup logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>

#define GCC_PATH "./xgcc"  /* Adjust based on your build directory */
#define TEMP_DIR "/tmp/gcc_coverage_test"

/* Create a minimal test source file */
void create_test_source(const char *path) {
    FILE *f = fopen(path, "w");
    if (!f) {
        perror("fopen");
        exit(1);
    }
    fprintf(f, "int main(void) { return 0; }\n");
    fclose(f);
}

/* Remove temporary files */
void cleanup_temp_files(void) {
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "rm -rf %s", TEMP_DIR);
    system(cmd);
}

/* Run GCC with specific flags to set driver state */
int run_gcc_with_flags(const char **argv, int argc) {
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
        return WEXITSTATUS(status);
    } else {
        perror("fork");
        return -1;
    }
}

int main(void) {
    /* Create temporary directory */
    mkdir(TEMP_DIR, 0755);
    
    /* Create test source file */
    char src_path[256];
    snprintf(src_path, sizeof(src_path), "%s/test.c", TEMP_DIR);
    create_test_source(src_path);
    
    /* Set environment variables to affect driver state */
    setenv("GCC_EXEC_PREFIX", "/usr/local/lib/gcc/", 1);
    setenv("COMPILER_PATH", "/usr/local/bin:/usr/bin", 1);
    
    printf("=== Test 1: Full compilation with multiple state-altering flags ===\n");
    
    /* Test 1: Complex compilation with many flags that set driver state */
    const char *argv1[] = {
        GCC_PATH,
        "-save-temps",                    /* sets save_temps_flag */
        "-dumpdir", TEMP_DIR,             /* allocates dumpdir */
        "-dumpbase", "test_dump",         /* allocates dumpbase */
        "-dumpbase-ext", ".ext",          /* allocates dumpbase_ext */
        "--sysroot=/opt/sysroot",         /* sets target_system_root, target_system_root_changed */
        "-fuse-ld=gold",                  /* sets use_ld */
        "-ftime-report",                  /* sets report_times_to_file */
        "-v",                             /* sets verbose_only_flag in some contexts */
        "-march=native",                  /* may affect spec_machine */
        "-mtune=generic",
        "-o", TEMP_DIR "/test.o",
        src_path,
        NULL
    };
    
    int result1 = run_gcc_with_flags(argv1, 14);
    printf("Test 1 completed with exit code: %d\n\n", result1);
    
    /* Test 2: Help and version flags to set print_help_list and print_version */
    printf("=== Test 2: Help and version flags ===\n");
    
    const char *argv2[] = {
        GCC_PATH,
        "--help=common",                  /* sets print_help_list */
        "--version",                      /* sets print_version */
        "-v",                             /* verbose flag */
        NULL
    };
    
    int result2 = run_gcc_with_flags(argv2, 4);
    printf("Test 2 completed with exit code: %d\n\n", result2);
    
    /* Test 3: Subprocess help and verbose output */
    printf("=== Test 3: Subprocess help ===\n");
    
    const char *argv3[] = {
        GCC_PATH,
        "-###",                           /* may set print_subprocess_help */
        "-v",
        src_path,
        NULL
    };
    
    int result3 = run_gcc_with_flags(argv3, 4);
    printf("Test 3 completed with exit code: %d\n\n", result3);
    
    /* Test 4: Different sysroot and dump options */
    printf("=== Test 4: Alternative sysroot and dump options ===\n");
    
    const char *argv4[] = {
        GCC_PATH,
        "-isysroot", "/usr/local/sysroot", /* alternative sysroot */
        "-dumpdir", "/tmp/alt_dump",
        "-dumpbase", "alt_base",
        "-o", TEMP_DIR "/alt.o",
        src_path,
        NULL
    };
    
    int result4 = run_gcc_with_flags(argv4, 8);
    printf("Test 4 completed with exit code: %d\n\n", result4);
    
    /* Test 5: Cross-compilation simulation */
    printf("=== Test 5: Cross-compilation flags ===\n");
    
    const char *argv5[] = {
        GCC_PATH,
        "-target", "x86_64-linux-gnu",    /* attempt to set different target */
        "-m32",                           /* 32-bit mode */
        "--sysroot=/cross/sysroot",
        "-save-temps=obj",
        "-dumpdir", TEMP_DIR "/cross",
        src_path,
        "-o", TEMP_DIR "/cross.o",
        NULL
    };
    
    int result5 = run_gcc_with_flags(argv5, 11);
    printf("Test 5 completed with exit code: %d\n\n", result5);
    
    /* Cleanup */
    cleanup_temp_files();
    
    printf("All tests completed. Driver cleanup logic should have been triggered.\n");
    printf("Check coverage with: gcov -b gcc.cc\n");
    
    return 0;
}
