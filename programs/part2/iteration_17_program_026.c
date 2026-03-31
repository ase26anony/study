/* test_gcc_cleanup.c - Test program to cover driver cleanup block in gcc.cc */
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
static int run_gcc_with_flags(const char **argv, int argc) {
    pid_t pid = fork();
    if (pid < 0) {
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

/* Test 1: Comprehensive compilation with many state-altering flags */
static void test_comprehensive_compilation(void) {
    printf("=== Test 1: Comprehensive compilation ===\n");
    
    /* Create temporary directory */
    mkdir(TMP_DIR, 0755);
    
    const char *source_file = TMP_DIR "/test.c";
    const char *output_file = TMP_DIR "/test.o";
    const char *dump_dir = TMP_DIR "/dump";
    
    create_test_source(source_file);
    mkdir(dump_dir, 0755);
    
    /* Construct command line with flags that set the target variables */
    const char *argv[] = {
        GCC_PATH,
        "-save-temps",              /* sets save_temps_flag */
        "-dumpdir", dump_dir,       /* allocates dumpdir */
        "-dumpbase", "test_dump",   /* allocates dumpbase */
        "-dumpbase-ext", ".ext",    /* allocates dumpbase_ext */
        "--sysroot=/opt/mysysroot", /* sets target_system_root, target_system_root_changed */
        "-fuse-ld=gold",            /* sets use_ld */
        "-ftime-report",            /* sets report_times_to_file */
        "-v",                       /* sets verbose_only_flag */
        "-mtune=cortex-a72",        /* affects spec_machine */
        "-march=armv8-a",           /* affects spec_machine */
        "-specs=" TMP_DIR "/specs", /* triggers spec processing */
        "-o", output_file,
        source_file,
        NULL
    };
    
    int argc = sizeof(argv)/sizeof(argv[0]) - 1;
    
    /* Set environment variables that affect driver state */
    setenv("GCC_EXEC_PREFIX", TMP_DIR "/lib/gcc", 1);
    setenv("COMPILER_PATH", TMP_DIR "/bin:" TMP_DIR "/lib", 1);
    setenv("LIBRARY_PATH", TMP_DIR "/lib:" TMP_DIR "/usr/lib", 1);
    
    int ret = run_gcc_with_flags(argv, argc);
    printf("GCC returned: %d\n", ret);
    
    /* Cleanup */
    unlink(source_file);
    unlink(output_file);
    unlink(TMP_DIR "/test.i");
    unlink(TMP_DIR "/test.s");
    unlink(TMP_DIR "/test.o");
}

/* Test 2: Help and version flags */
static void test_help_and_version(void) {
    printf("\n=== Test 2: Help and version flags ===\n");
    
    /* Test with --help=common (sets print_help_list) */
    const char *argv1[] = {
        GCC_PATH,
        "--help=common",            /* sets print_help_list */
        "--version",                /* sets print_version */
        NULL
    };
    
    printf("Running with --help=common --version\n");
    run_gcc_with_flags(argv1, 2);
    
    /* Test with --help=subprocess (may set print_subprocess_help) */
    const char *argv2[] = {
        GCC_PATH,
        "--help=subprocess",        /* may set print_subprocess_help */
        "-###",                     /* alternative way to trigger subprocess help */
        NULL
    };
    
    printf("\nRunning with --help=subprocess -###\n");
    run_gcc_with_flags(argv2, 2);
}

/* Test 3: Different save-temps modes */
static void test_save_temps_variants(void) {
    printf("\n=== Test 3: Save-temps variants ===\n");
    
    const char *source_file = TMP_DIR "/test2.c";
    create_test_source(source_file);
    
    /* Test with save-temps=cwd */
    const char *argv1[] = {
        GCC_PATH,
        "-save-temps=cwd",          /* different save_temps_flag value */
        "-dumpdir", ".",            /* current directory dumpdir */
        "-dumpbase", "variant",
        "-o", TMP_DIR "/test2.o",
        source_file,
        NULL
    };
    
    printf("Testing -save-temps=cwd\n");
    run_gcc_with_flags(argv1, 6);
    
    /* Test with outbase */
    const char *argv2[] = {
        GCC_PATH,
        "-save-temps",
        "-dumpbase", "myprog",
        "-dumpbase-ext", ".myext",
        "-o", TMP_DIR "/myoutput.o",
        source_file,
        NULL
    };
    
    printf("\nTesting with custom dumpbase and output\n");
    run_gcc_with_flags(argv2, 7);
    
    unlink(source_file);
    unlink(TMP_DIR "/test2.o");
    unlink(TMP_DIR "/myoutput.o");
}

/* Test 4: Cross-compilation scenario */
static void test_cross_compilation(void) {
    printf("\n=== Test 4: Cross-compilation scenario ===\n");
    
    const char *source_file = TMP_DIR "/cross.c";
    create_test_source(source_file);
    
    /* Simulate cross-compilation with target-specific flags */
    const char *argv[] = {
        GCC_PATH,
        "--target=arm-linux-gnueabihf",  /* Cross-target specification */
        "--sysroot=" TMP_DIR "/sysroot", /* Custom sysroot */
        "-mcpu=cortex-a53",
        "-mfpu=neon-fp-armv8",
        "-mfloat-abi=hard",
        "-save-temps",
        "-dumpdir", TMP_DIR "/crossdump",
        "-v",
        "-o", TMP_DIR "/cross.o",
        source_file,
        NULL
    };
    
    /* Create dummy sysroot structure */
    mkdir(TMP_DIR "/sysroot", 0755);
    mkdir(TMP_DIR "/sysroot/usr", 0755);
    mkdir(TMP_DIR "/sysroot/usr/include", 0755);
    mkdir(TMP_DIR "/crossdump", 0755);
    
    printf("Testing cross-compilation flags\n");
    int ret = run_gcc_with_flags(argv, 13);
    printf("Cross-compilation test returned: %d\n", ret);
    
    /* Cleanup */
    unlink(source_file);
    unlink(TMP_DIR "/cross.o");
}

/* Test 5: Multiple invocations to trigger repeated cleanup */
static void test_multiple_invocations(void) {
    printf("\n=== Test 5: Multiple rapid invocations ===\n");
    
    const char *source_file = TMP_DIR "/multi.c";
    create_test_source(source_file);
    
    for (int i = 0; i < 3; i++) {
        char output[256], dumpbase[256];
        snprintf(output, sizeof(output), TMP_DIR "/multi%d.o", i);
        snprintf(dumpbase, sizeof(dumpbase), "multi%d", i);
        
        const char *argv[] = {
            GCC_PATH,
            "-save-temps",
            "-dumpdir", TMP_DIR,
            "-dumpbase", dumpbase,
            "-v",
            "-o", output,
            source_file,
            NULL
        };
        
        printf("Invocation %d\n", i + 1);
        run_gcc_with_flags(argv, 9);
    }
    
    unlink(source_file);
}

int main(void) {
    printf("=== GCC Driver Cleanup Coverage Test ===\n");
    
    /* Create main temporary directory */
    mkdir(TMP_DIR, 0755);
    
    /* Run all test scenarios */
    test_comprehensive_compilation();
    test_help_and_version();
    test_save_temps_variants();
    test_cross_compilation();
    test_multiple_invocations();
    
    /* Final cleanup of temporary directory */
    system("rm -rf " TMP_DIR);
    
    printf("\n=== All tests completed ===\n");
    printf("The GCC driver's cleanup block should have been exercised multiple times.\n");
    printf("Check coverage data with: gcov -b gcc.cc\n");
    
    return 0;
}
