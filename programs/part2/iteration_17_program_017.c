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
#define TMP_DIR "/tmp/gcc_test_cover"

/* Create a minimal C source file */
static void create_test_source(const char *path) {
    FILE *f = fopen(path, "w");
    if (!f) {
        perror("fopen");
        exit(1);
    }
    fprintf(f, "int main(void) { return 0; }\n");
    fclose(f);
}

/* Run GCC with specific flags to set driver state variables */
static int run_gcc_with_flags(const char **argv, int argc) {
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return -1;
    }
    
    if (pid == 0) { /* Child process */
        /* Set environment variables that affect driver state */
        setenv("GCC_EXEC_PREFIX", "/tmp/dummy_gcc_prefix", 1);
        setenv("COMPILER_PATH", "/usr/bin:/usr/local/bin", 1);
        setenv("LIBRARY_PATH", "/usr/lib:/usr/local/lib", 1);
        
        execv(GCC_PATH, (char * const *)argv);
        perror("execv");
        exit(1);
    } else { /* Parent process */
        int status;
        waitpid(pid, &status, 0);
        return WEXITSTATUS(status);
    }
}

int main(void) {
    /* Create temporary directory for test files */
    mkdir(TMP_DIR, 0755);
    
    /* Test 1: Complex compilation with many state-altering flags */
    printf("=== Test 1: Complex compilation with state flags ===\n");
    const char *source1 = TMP_DIR "/test1.c";
    const char *output1 = TMP_DIR "/test1.o";
    create_test_source(source1);
    
    const char *argv1[] = {
        GCC_PATH,
        "-save-temps",                    /* Sets save_temps_flag */
        "-dumpdir", TMP_DIR "/dumpdir",   /* Allocates dumpdir */
        "-dumpbase", "testdump",          /* Allocates dumpbase */
        "-dumpbase-ext", ".ext",          /* Allocates dumpbase_ext */
        "--sysroot=" TMP_DIR "/sysroot",  /* Sets target_system_root */
        "-fuse-ld=gold",                  /* Sets use_ld */
        "-ftime-report",                  /* Sets report_times_to_file */
        "-v",                             /* Sets verbose_only_flag */
        "-march=native",                  /* May affect spec_machine */
        "-mtune=generic",
        "-specs=" TMP_DIR "/specs",
        "-o", output1,
        source1,
        NULL
    };
    
    /* Create dummy sysroot directory */
    mkdir(TMP_DIR "/sysroot", 0755);
    mkdir(TMP_DIR "/sysroot/usr", 0755);
    mkdir(TMP_DIR "/sysroot/usr/include", 0755);
    
    int ret1 = run_gcc_with_flags(argv1, sizeof(argv1)/sizeof(argv1[0]) - 1);
    printf("Test 1 exit status: %d\n\n", ret1);
    
    /* Test 2: Help and version flags to set print_help_list and print_version */
    printf("=== Test 2: Help and version flags ===\n");
    const char *argv2[] = {
        GCC_PATH,
        "--help=common",                  /* Sets print_help_list */
        "--version",                      /* Sets print_version */
        "-###",                           /* May set print_subprocess_help */
        source1,
        NULL
    };
    
    int ret2 = run_gcc_with_flags(argv2, sizeof(argv2)/sizeof(argv2[0]) - 1);
    printf("Test 2 exit status: %d\n\n", ret2);
    
    /* Test 3: Different target configuration */
    printf("=== Test 3: Cross-compilation-like flags ===\n");
    const char *source3 = TMP_DIR "/test3.c";
    const char *output3 = TMP_DIR "/test3";
    create_test_source(source3);
    
    const char *argv3[] = {
        GCC_PATH,
        "-save-temps=obj",
        "-dumpdir", TMP_DIR "/dump2",
        "-dumpbase", "crossdump",
        "--sysroot=/opt/cross/sysroot",
        "-isysroot", "/opt/cross/include",
        "-target", "x86_64-linux-gnu",    /* Force target specification */
        "-B", "/opt/cross/bin",
        "-fuse-ld=bfd",
        "-ftime-report",
        "-v",
        "-print-prog-name=cc1",           /* Triggers subprocess help logic */
        "-o", output3,
        source3,
        NULL
    };
    
    int ret3 = run_gcc_with_flags(argv3, sizeof(argv3)/sizeof(argv3[0]) - 1);
    printf("Test 3 exit status: %d\n\n", ret3);
    
    /* Test 4: Multiple dump options with outbase */
    printf("=== Test 4: Comprehensive dump options ===\n");
    const char *source4 = TMP_DIR "/test4.c";
    const char *output4 = TMP_DIR "/test4.exe";
    create_test_source(source4);
    
    const char *argv4[] = {
        GCC_PATH,
        "-save-temps=cwd",
        "-dumpdir", ".",
        "-dumpbase", "mydump",
        "-dumpbase-ext", ".myext",
        "-o", output4,
        "-specs=" TMP_DIR "/myspecs",
        "-ftime-report",
        "-fverbose-asm",
        "-wrapper", "/bin/true",
        source4,
        NULL
    };
    
    int ret4 = run_gcc_with_flags(argv4, sizeof(argv4)/sizeof(argv4[0]) - 1);
    printf("Test 4 exit status: %d\n\n", ret4);
    
    /* Test 5: Error case that still triggers cleanup */
    printf("=== Test 5: Error case with cleanup ===\n");
    const char *argv5[] = {
        GCC_PATH,
        "-save-temps",
        "-dumpdir", TMP_DIR "/errdump",
        "-dumpbase", "errdump",
        "--sysroot=/nonexistent/sysroot",
        "-fuse-ld=nonexistent",
        "-o", TMP_DIR "/error.out",
        "/nonexistent/file.c",  /* Non-existent source file */
        NULL
    };
    
    int ret5 = run_gcc_with_flags(argv5, sizeof(argv5)/sizeof(argv5[0]) - 1);
    printf("Test 5 exit status: %d\n\n", ret5);
    
    /* Cleanup temporary files */
    unlink(source1);
    unlink(source3);
    unlink(source4);
    rmdir(TMP_DIR "/sysroot/usr/include");
    rmdir(TMP_DIR "/sysroot/usr");
    rmdir(TMP_DIR "/sysroot");
    rmdir(TMP_DIR);
    
    printf("All tests completed. Check coverage data for gcc.cc lines 11228-11250\n");
    
    return 0;
}
