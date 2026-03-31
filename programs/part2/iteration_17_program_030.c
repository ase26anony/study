/* test_gcc_cleanup.c - Test program to cover driver cleanup logic */
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

static void create_temp_source(const char *path) {
    FILE *f = fopen(path, "w");
    if (!f) {
        perror("fopen");
        exit(1);
    }
    fprintf(f, "int main() { return 0; }\n");
    fclose(f);
}

static void run_gcc_with_flags(const char **argv, int argc) {
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
        execv(GCC_PATH, (char * const *)argv);
        perror("execv");
        exit(1);
    } else { /* Parent */
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)) {
            printf("GCC exited with status %d\n", WEXITSTATUS(status));
        } else {
            printf("GCC terminated abnormally\n");
        }
    }
}

static void test_case_1(void) {
    printf("=== Test Case 1: Full compilation with many flags ===\n");
    
    /* Create temporary directory */
    mkdir(TMP_DIR, 0755);
    
    const char *source = TMP_DIR "/test.c";
    const char *output = TMP_DIR "/test.o";
    create_temp_source(source);
    
    /* Construct complex command line to set multiple global variables */
    const char *argv[] = {
        GCC_PATH,
        "-save-temps",              /* sets save_temps_flag */
        "-dumpdir", TMP_DIR "/dump",/* allocates dumpdir */
        "-dumpbase", "testdump",    /* allocates dumpbase */
        "-dumpbase-ext", ".ext",    /* allocates dumpbase_ext */
        "--sysroot=/opt/mysysroot", /* sets target_system_root, target_system_root_changed */
        "-fuse-ld=gold",            /* sets use_ld */
        "-ftime-report",            /* sets report_times_to_file */
        "-v",                       /* sets verbose_only_flag */
        "-mtune=native",            /* affects spec_machine */
        "-march=x86-64",            /* affects spec_machine */
        "-specs=/dev/null",         /* forces spec processing */
        "-B", "/usr/local/lib",     /* adds prefix */
        "-idirafter", "/usr/include",
        "-I", TMP_DIR "/include",
        "-L", TMP_DIR "/lib",
        "-Wl,-rpath," TMP_DIR "/lib",
        "-o", output,
        source,
        NULL
    };
    
    int argc = sizeof(argv)/sizeof(argv[0]) - 1;
    run_gcc_with_flags(argv, argc);
    
    /* Cleanup */
    unlink(source);
    unlink(output);
}

static void test_case_2(void) {
    printf("=== Test Case 2: Help and version flags ===\n");
    
    /* Test combinations that set print_help_list, print_version */
    const char *argv1[] = {
        GCC_PATH,
        "--help=common",            /* sets print_help_list */
        "--version",                /* sets print_version */
        "-v",                       /* verbose */
        NULL
    };
    
    const char *argv2[] = {
        GCC_PATH,
        "-###",                     /* may set print_subprocess_help */
        "-save-temps=obj",
        "-dumpdir", ".",
        "-dumpbase", "helpdump",
        NULL
    };
    
    run_gcc_with_flags(argv1, sizeof(argv1)/sizeof(argv1[0]) - 1);
    run_gcc_with_flags(argv2, sizeof(argv2)/sizeof(argv2[0]) - 1);
}

static void test_case_3(void) {
    printf("=== Test Case 3: Cross-compilation scenario ===\n");
    
    const char *source = TMP_DIR "/cross.c";
    create_temp_source(source);
    
    /* Simulate cross-compilation with different target */
    const char *argv[] = {
        GCC_PATH,
        "--target=arm-linux-gnueabihf",  /* Cross target */
        "--sysroot=/opt/arm-sysroot",    /* Different sysroot */
        "-mcpu=cortex-a9",
        "-save-temps=cwd",
        "-dumpdir", ".",
        "-dumpbase", "armtest",
        "-dumpbase-ext", ".arm",
        "-ftime-report",
        "-fuse-ld=bfd",
        "-v",
        "-o", TMP_DIR "/cross.o",
        source,
        NULL
    };
    
    run_gcc_with_flags(argv, sizeof(argv)/sizeof(argv[0]) - 1);
    
    unlink(source);
    unlink(TMP_DIR "/cross.o");
}

static void test_case_4(void) {
    printf("=== Test Case 4: Error case with cleanup ===\n");
    
    /* Trigger error but still run cleanup */
    const char *argv[] = {
        GCC_PATH,
        "-save-temps",
        "-dumpdir", TMP_DIR,
        "-dumpbase", "errortest",
        "-o", "/nonexistent/output.o",  /* Will fail but cleanup still runs */
        "/nonexistent/source.c",        /* Non-existent source */
        NULL
    };
    
    run_gcc_with_flags(argv, sizeof(argv)/sizeof(argv[0]) - 1);
}

int main(void) {
    printf("Testing GCC driver cleanup block coverage\n");
    
    /* Create test directory */
    mkdir(TMP_DIR, 0755);
    
    /* Run multiple test cases to cover different paths */
    test_case_1();  /* Full compilation with many flags */
    test_case_2();  /* Help/version flags */
    test_case_3();  /* Cross-compilation */
    test_case_4();  /* Error case */
    
    /* Cleanup test directory */
    rmdir(TMP_DIR);
    
    printf("\nAll test cases completed. Check coverage with:\n");
    printf("  gcov gcc.cc\n");
    printf("  or\n");
    printf("  lcov --capture --directory . --output-file coverage.info\n");
    
    return 0;
}
