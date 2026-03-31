/* test_gcc_cleanup.c - Test program to cover GCC driver cleanup code */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

#define GCC_PATH "./gcc/xgcc"  /* Adjust based on your build directory */
#define TMP_DIR "/tmp/gcc_test"

/* Create a minimal C source file */
void create_test_source(const char *filename) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        perror("fopen");
        exit(1);
    }
    fprintf(f, "int main(void) { return 0; }\n");
    fclose(f);
}

/* Run GCC with specific flags to set driver state variables */
int run_gcc_with_flags(const char **argv, int argc) {
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        return -1;
    }
    
    if (pid == 0) {
        /* Child process */
        execv(GCC_PATH, (char * const *)argv);
        perror("execv");
        exit(1);
    } else {
        /* Parent process */
        int status;
        waitpid(pid, &status, 0);
        return WEXITSTATUS(status);
    }
}

int main(void) {
    /* Create temporary directory */
    mkdir(TMP_DIR, 0755);
    
    /* Test 1: Complex compilation with many state-altering flags */
    printf("=== Test 1: Complex compilation with state-altering flags ===\n");
    const char *source1 = TMP_DIR "/test1.c";
    const char *output1 = TMP_DIR "/test1.o";
    create_test_source(source1);
    
    const char *argv1[] = {
        GCC_PATH,
        "-save-temps",                    /* sets save_temps_flag */
        "-dumpdir", TMP_DIR "/dump",      /* allocates dumpdir */
        "-dumpbase", "test_dump",         /* allocates dumpbase */
        "-dumpbase-ext", ".ext",          /* allocates dumpbase_ext */
        "--sysroot=" TMP_DIR "/sysroot",  /* sets target_system_root */
        "-fuse-ld=gold",                  /* sets use_ld */
        "-ftime-report",                  /* sets report_times_to_file */
        "-v",                             /* sets verbose_only_flag */
        "-specs=" TMP_DIR "/specs",       /* affects spec processing */
        "-o", output1,
        source1,
        NULL
    };
    
    /* Set environment variables that affect driver state */
    setenv("GCC_EXEC_PREFIX", TMP_DIR "/lib/gcc", 1);
    setenv("COMPILER_PATH", TMP_DIR "/bin:" TMP_DIR "/lib", 1);
    
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
    
    /* Test 3: Different target configuration */
    printf("\n=== Test 3: Different target configuration ===\n");
    const char *source3 = TMP_DIR "/test3.c";
    const char *output3 = TMP_DIR "/test3";
    create_test_source(source3);
    
    const char *argv3[] = {
        GCC_PATH,
        "-save-temps=obj",                /* different save_temps value */
        "-dumpdir", TMP_DIR "/dump2/",    /* with trailing slash */
        "-dumpbase", "test2",
        "-o", output3,
        "-march=x86-64",                  /* affects spec_machine */
        "-mtune=generic",
        "-isysroot", TMP_DIR "/isysroot", /* alternative sysroot flag */
        "-fuse-ld=bfd",
        source3,
        NULL
    };
    run_gcc_with_flags(argv3, 12);
    
    /* Test 4: Error case that still triggers cleanup */
    printf("\n=== Test 4: Error case ===\n");
    const char *argv4[] = {
        GCC_PATH,
        "-save-temps",
        "-dumpdir", TMP_DIR "/dump3",
        "-o", TMP_DIR "/test4.o",
        TMP_DIR "/nonexistent.c",         /* Non-existent source file */
        NULL
    };
    run_gcc_with_flags(argv4, 6);
    
    /* Test 5: Multiple output base options */
    printf("\n=== Test 5: Multiple output options ===\n");
    const char *source5 = TMP_DIR "/test5.c";
    create_test_source(source5);
    
    const char *argv5[] = {
        GCC_PATH,
        "-save-temps=cwd",
        "-dumpdir", ".",
        "-dumpbase", "multi_test",
        "-dumpbase-ext", ".multi",
        "-o", TMP_DIR "/test5_output",
        "-specs=" TMP_DIR "/my.specs",
        source5,
        NULL
    };
    run_gcc_with_flags(argv5, 10);
    
    /* Cleanup temporary files */
    printf("\n=== Cleaning up ===\n");
    unlink(source1);
    unlink(output1);
    unlink(source3);
    unlink(output3);
    unlink(source5);
    
    /* Remove dump files that might have been created */
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "rm -rf %s/dump* %s/*.i %s/*.s %s/*.o", 
             TMP_DIR, TMP_DIR, TMP_DIR, TMP_DIR);
    system(cmd);
    
    rmdir(TMP_DIR);
    
    printf("Test completed. Check coverage with:\n");
    printf("  gcov gcc.cc\n");
    
    return 0;
}
