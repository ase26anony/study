/* test_gcc_cleanup.c - Test program to cover driver cleanup lines in gcc.cc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

#define GCC_PATH "./xgcc"  /* Adjust based on your build directory */
#define TMP_DIR "/tmp/gcc_cleanup_test"

static void create_temp_source(const char *path) {
    FILE *f = fopen(path, "w");
    if (!f) {
        perror("fopen");
        exit(1);
    }
    fprintf(f, "int main(void) { return 0; }\n");
    fclose(f);
}

static void run_gcc(const char **argv, int argc) {
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        exit(1);
    }
    
    if (pid == 0) { /* Child */
        /* Set environment variables to affect driver state */
        setenv("GCC_EXEC_PREFIX", "/usr/local/lib/gcc/", 1);
        setenv("COMPILER_PATH", "/usr/bin:/usr/local/bin", 1);
        setenv("LIBRARY_PATH", "/usr/lib:/usr/local/lib", 1);
        
        /* Execute GCC driver */
        execv(GCC_PATH, (char *const *)argv);
        perror("execv");
        exit(1);
    } else { /* Parent */
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)) {
            printf("GCC exited with status %d\n", WEXITSTATUS(status));
        }
    }
}

static void test_case_1(void) {
    printf("=== Test Case 1: Full compilation with many state flags ===\n");
    
    /* Create temporary directory */
    mkdir(TMP_DIR, 0755);
    
    /* Create source file */
    char src_path[256];
    snprintf(src_path, sizeof(src_path), "%s/test1.c", TMP_DIR);
    create_temp_source(src_path);
    
    /* Construct complex GCC command line */
    const char *argv[] = {
        GCC_PATH,
        "-save-temps",              /* sets save_temps_flag */
        "-dumpdir", TMP_DIR,        /* allocates dumpdir */
        "-dumpbase", "test_dump",   /* allocates dumpbase */
        "-dumpbase-ext", ".ext",    /* allocates dumpbase_ext */
        "--sysroot=/opt/mysysroot", /* sets target_system_root, target_system_root_changed */
        "-fuse-ld=gold",            /* sets use_ld */
        "-ftime-report",            /* sets report_times_to_file */
        "-v",                       /* sets verbose_only_flag */
        "-mtune=native",            /* may affect spec_machine */
        "-march=x86-64",            /* may affect spec_machine */
        "-specs=/dev/null",         /* forces spec processing */
        "-B", "/usr/local/lib",     /* adds prefix for compiler executables */
        "-L", "/usr/local/lib",     /* adds library search path */
        "-I", "/usr/local/include", /* adds include search path */
        "-o", "/tmp/test_output.o", /* output file */
        src_path,                   /* source file */
        NULL
    };
    
    run_gcc(argv, sizeof(argv)/sizeof(argv[0]) - 1);
}

static void test_case_2(void) {
    printf("\n=== Test Case 2: Help and version flags ===\n");
    
    /* Test combination of help and version flags */
    const char *argv1[] = {
        GCC_PATH,
        "--help=common",            /* sets print_help_list */
        "--version",                /* sets print_version */
        "-v",                       /* sets verbose_only_flag */
        NULL
    };
    
    run_gcc(argv1, sizeof(argv1)/sizeof(argv1[0]) - 1);
    
    /* Test subprocess help */
    const char *argv2[] = {
        GCC_PATH,
        "-###",                     /* may set print_subprocess_help */
        "-E",                       /* preprocess only */
        "-dM",                      /* dump macros */
        "-xc",                      /* specify C language */
        "-",                        /* read from stdin */
        NULL
    };
    
    run_gcc(argv2, sizeof(argv2)/sizeof(argv2[0]) - 1);
}

static void test_case_3(void) {
    printf("\n=== Test Case 3: Cross-compilation scenario ===\n");
    
    char src_path[256];
    snprintf(src_path, sizeof(src_path), "%s/test3.c", TMP_DIR);
    create_temp_source(src_path);
    
    /* Simulate cross-compilation with different target */
    const char *argv[] = {
        GCC_PATH,
        "-save-temps=obj",          /* different save_temps value */
        "-dumpdir", "/var/tmp/dumps",
        "-dumpbase", "cross_compile",
        "-dumpbase-ext", ".dump",
        "--sysroot=/cross/root",
        "-target", "arm-linux-gnueabihf",  /* cross target */
        "-march=armv7-a",
        "-mtune=cortex-a8",
        "-mfpu=neon",
        "-mfloat-abi=hard",
        "-isysroot", "/cross/include/root", /* alternative sysroot */
        "-fuse-ld=bfd",
        "-ftime-report",
        "-v",
        "-o", "/tmp/cross.o",
        src_path,
        NULL
    };
    
    run_gcc(argv, sizeof(argv)/sizeof(argv[0]) - 1);
}

static void test_case_4(void) {
    printf("\n=== Test Case 4: Multiple outputs and dump options ===\n");
    
    char src_path[256];
    snprintf(src_path, sizeof(src_path), "%s/test4.c", TMP_DIR);
    create_temp_source(src_path);
    
    /* Test with multiple dump-related options */
    const char *argv[] = {
        GCC_PATH,
        "-save-temps=cwd",          /* another save_temps variant */
        "-dumpdir", ".",
        "-dumpbase", "multi",
        "-dumpbase-ext", ".multi",
        "-fdump-rtl-all",
        "-fdump-tree-all",
        "-fdump-ipa-all",
        "-fdump-statistics",
        "-ftime-report",
        "-fmem-report",
        "-fstack-usage",
        "-Wl,--verbose",            /* linker verbose */
        "-v",
        "-o", "/tmp/multi_output",
        src_path,
        NULL
    };
    
    run_gcc(argv, sizeof(argv)/sizeof(argv[0]) - 1);
}

static void cleanup(void) {
    /* Clean up temporary files */
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "rm -rf %s /tmp/test_output.o /tmp/cross.o /tmp/multi_output", TMP_DIR);
    system(cmd);
}

int main(void) {
    printf("Testing GCC driver cleanup lines coverage\n");
    printf("Using GCC at: %s\n", GCC_PATH);
    
    /* Verify GCC exists */
    if (access(GCC_PATH, X_OK) != 0) {
        fprintf(stderr, "GCC not found at %s\n", GCC_PATH);
        fprintf(stderr, "Please adjust GCC_PATH in the source code\n");
        return 1;
    }
    
    /* Run test cases */
    test_case_1();
    test_case_2();
    test_case_3();
    test_case_4();
    
    /* Cleanup */
    cleanup();
    
    printf("\nAll test cases completed. Check coverage with:\n");
    printf("  gcov gcc.cc\n");
    
    return 0;
}
