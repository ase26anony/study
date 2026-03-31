/* test_gcc_cleanup.c - Test program to cover GCC driver cleanup logic */
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

/* Create a minimal test source file */
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
    
    /* Test 1: Compile with multiple state-altering flags */
    printf("=== Test 1: Full compilation with state flags ===\n");
    const char *source1 = TMP_DIR "/test1.c";
    const char *output1 = TMP_DIR "/test1.o";
    create_test_source(source1);
    
    const char *argv1[] = {
        GCC_PATH,
        "-save-temps",                    /* sets save_temps_flag */
        "-dumpdir", TMP_DIR "/dumpdir",   /* allocates dumpdir */
        "-dumpbase", "testdump",          /* allocates dumpbase */
        "-dumpbase-ext", ".ext",          /* allocates dumpbase_ext */
        "--sysroot=/opt/mysysroot",       /* sets target_system_root, target_system_root_changed */
        "-fuse-ld=gold",                  /* sets use_ld */
        "-ftime-report",                  /* sets report_times_to_file */
        "-v",                             /* sets verbose_only_flag */
        "-specs=/dev/null",               /* affects spec processing */
        "-c",
        source1,
        "-o", output1,
        NULL
    };
    
    /* Set environment variables that affect driver state */
    setenv("GCC_EXEC_PREFIX", "/usr/lib/gcc", 1);
    setenv("COMPILER_PATH", "/usr/bin:/usr/local/bin", 1);
    
    run_gcc_with_flags(argv1, sizeof(argv1)/sizeof(argv1[0]) - 1);
    
    /* Test 2: Help and version flags */
    printf("\n=== Test 2: Help and version flags ===\n");
    const char *argv2[] = {
        GCC_PATH,
        "--help=common",                  /* sets print_help_list */
        "--version",                      /* sets print_version */
        "-###",                           /* may set print_subprocess_help */
        NULL
    };
    
    run_gcc_with_flags(argv2, sizeof(argv2)/sizeof(argv2[0]) - 1);
    
    /* Test 3: Different target configuration */
    printf("\n=== Test 3: Cross-compilation-like flags ===\n");
    const char *source3 = TMP_DIR "/test3.c";
    const char *output3 = TMP_DIR "/test3.o";
    create_test_source(source3);
    
    const char *argv3[] = {
        GCC_PATH,
        "-march=armv7-a",                 /* changes spec_machine */
        "-mtune=cortex-a8",
        "-isysroot", "/opt/arm-sysroot",  /* alternative sysroot setting */
        "-save-temps=obj",
        "-dumpdir", TMP_DIR "/dump2",
        "-dumpbase", "armtest",
        "-fuse-ld=bfd",
        "-ftime-report",
        "-c",
        source3,
        "-o", output3,
        NULL
    };
    
    run_gcc_with_flags(argv3, sizeof(argv3)/sizeof(argv3[0]) - 1);
    
    /* Test 4: Test with outbase (affects outbase variable) */
    printf("\n=== Test 4: Testing outbase ===\n");
    const char *source4 = TMP_DIR "/test4.c";
    create_test_source(source4);
    
    const char *argv4[] = {
        GCC_PATH,
        "-save-temps",
        "-dumpdir", TMP_DIR "/dump3",
        "-dumpbase", "outbasetest",
        "-o", TMP_DIR "/custom_out.o",    /* affects outbase */
        "-c",
        source4,
        NULL
    };
    
    run_gcc_with_flags(argv4, sizeof(argv4)/sizeof(argv4[0]) - 1);
    
    /* Test 5: Error case to ensure cleanup still happens */
    printf("\n=== Test 5: Error case with flags set ===\n");
    const char *argv5[] = {
        GCC_PATH,
        "-save-temps",
        "-dumpdir", TMP_DIR "/dump4",
        "--sysroot=/nonexistent",
        "-fuse-ld=nonexistent",
        "nonexistent.c",                  /* Will cause error but cleanup should run */
        NULL
    };
    
    run_gcc_with_flags(argv5, sizeof(argv5)/sizeof(argv5[0]) - 1);
    
    /* Cleanup temporary files */
    unlink(source1);
    unlink(output1);
    unlink(TMP_DIR "/test1.i");
    unlink(TMP_DIR "/test1.s");
    unlink(source3);
    unlink(output3);
    unlink(TMP_DIR "/test3.i");
    unlink(TMP_DIR "/test3.s");
    unlink(source4);
    unlink(TMP_DIR "/custom_out.o");
    unlink(TMP_DIR "/test4.i");
    unlink(TMP_DIR "/test4.s");
    
    /* Remove dump directories */
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "rm -rf %s", TMP_DIR);
    system(cmd);
    
    printf("\n=== All tests completed ===\n");
    return 0;
}
