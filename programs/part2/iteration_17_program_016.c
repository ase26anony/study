/* test_gcc_cleanup.c - Test program to cover driver cleanup lines in gcc.cc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

/* Helper function to create a temporary source file */
static int create_temp_source(const char *filename) {
    FILE *f = fopen(filename, "w");
    if (!f) return -1;
    fprintf(f, "int main(void) { return 0; }\n");
    fclose(f);
    return 0;
}

/* Helper function to spawn GCC with given arguments */
static int spawn_gcc(const char *gcc_path, char *const argv[]) {
    pid_t pid = fork();
    if (pid == 0) {
        /* Child process */
        execv(gcc_path, argv);
        perror("execv failed");
        exit(1);
    } else if (pid > 0) {
        /* Parent process */
        int status;
        waitpid(pid, &status, 0);
        return WEXITSTATUS(status);
    } else {
        perror("fork failed");
        return -1;
    }
}

/* Test case 1: Comprehensive compilation with many state-altering flags */
static int test_comprehensive_compilation(const char *gcc_path) {
    printf("Running comprehensive compilation test...\n");
    
    /* Create temporary files */
    const char *src_file = "/tmp/test_gcc_cover.c";
    const char *obj_file = "/tmp/test_gcc_cover.o";
    
    if (create_temp_source(src_file) < 0) {
        perror("Failed to create source file");
        return -1;
    }
    
    /* Set environment variables to affect driver state */
    setenv("GCC_EXEC_PREFIX", "/tmp/gcc_exec_prefix_test", 1);
    setenv("COMPILER_PATH", "/tmp/compiler_path_test:/usr/local/bin", 1);
    
    /* Construct complex command line to set many global variables */
    char *argv[] = {
        (char *)gcc_path,
        "-save-temps",                    /* sets save_temps_flag */
        "-dumpdir", "/tmp/my_dump_dir",   /* allocates dumpdir */
        "-dumpbase", "my_dump_base",      /* allocates dumpbase */
        "-dumpbase-ext", ".my_ext",       /* allocates dumpbase_ext */
        "--sysroot=/opt/custom_sysroot",  /* sets target_system_root, target_system_root_changed */
        "-isysroot", "/opt/headers_sysroot", /* may affect target_sysroot_hdrs_suffix */
        "-fuse-ld=gold",                  /* sets use_ld */
        "-ftime-report",                  /* sets report_times_to_file */
        "-v",                             /* sets verbose_only_flag in some contexts */
        "-specs=/dev/null",               /* affects spec processing */
        "-mtune=generic",                 /* may affect spec_machine */
        "-march=x86-64",                  /* may affect spec_machine */
        "-B/tmp/dummy_bindir",            /* adds to compiler execution prefixes */
        "-L/tmp/dummy_libdir",            /* adds to library search path */
        "-I/tmp/dummy_includedir",        /* adds to include search path */
        "-o", obj_file,                   /* output file */
        src_file,                         /* source file */
        NULL
    };
    
    int result = spawn_gcc(gcc_path, argv);
    
    /* Cleanup temporary files */
    unlink(src_file);
    unlink(obj_file);
    unlink("/tmp/my_dump_dir.my_dump_base.i");
    unlink("/tmp/my_dump_dir.my_dump_base.s");
    unlink("/tmp/my_dump_dir.my_dump_base.o");
    
    return result;
}

/* Test case 2: Help and version flags to set print_help_list and print_version */
static int test_help_and_version(const char *gcc_path) {
    printf("Running help and version test...\n");
    
    /* Test 2a: Combined help and version with other flags */
    char *argv1[] = {
        (char *)gcc_path,
        "--help=common",                  /* sets print_help_list */
        "--version",                      /* sets print_version */
        "-v",                             /* verbose flag */
        NULL
    };
    
    /* Test 2b: Subprocess help */
    char *argv2[] = {
        (char *)gcc_path,
        "--help=subprocess",              /* may set print_subprocess_help */
        "-###",                           /* also may set print_subprocess_help */
        NULL
    };
    
    /* Test 2c: Combined with dump options */
    char *argv3[] = {
        (char *)gcc_path,
        "--help",
        "-dumpversion",
        "-dumpmachine",
        NULL
    };
    
    spawn_gcc(gcc_path, argv1);
    spawn_gcc(gcc_path, argv2);
    spawn_gcc(gcc_path, argv3);
    
    return 0;
}

/* Test case 3: Cross-compilation scenario */
static int test_cross_compilation(const char *gcc_path) {
    printf("Running cross-compilation test...\n");
    
    const char *src_file = "/tmp/test_gcc_cross.c";
    if (create_temp_source(src_file) < 0) {
        perror("Failed to create source file");
        return -1;
    }
    
    /* Try to simulate cross-compilation by targeting different architectures */
    char *architectures[] = {
        "-m32",                    /* 32-bit x86 */
        "-march=armv7-a",         /* ARM */
        "-march=powerpc",         /* PowerPC */
        "-march=riscv64",         /* RISC-V */
        NULL
    };
    
    for (int i = 0; architectures[i]; i++) {
        char *argv[] = {
            (char *)gcc_path,
            architectures[i],
            "--sysroot=/opt/cross_sysroot",
            "-target", "x86_64-linux-gnu",  /* Explicit target triple */
            "-save-temps",
            "-dumpdir", "/tmp/cross_dump",
            "-o", "/tmp/test_cross.o",
            src_file,
            NULL
        };
        
        spawn_gcc(gcc_path, argv);
        
        /* Cleanup */
        unlink("/tmp/test_cross.o");
        unlink("/tmp/cross_dump.test_gcc_cross.i");
        unlink("/tmp/cross_dump.test_gcc_cross.s");
        unlink("/tmp/cross_dump.test_gcc_cross.o");
    }
    
    unlink(src_file);
    return 0;
}

/* Test case 4: Time reporting and profiling */
static int test_timing_and_profiling(const char *gcc_path) {
    printf("Running timing and profiling test...\n");
    
    const char *src_file = "/tmp/test_gcc_time.c";
    if (create_temp_source(src_file) < 0) {
        perror("Failed to create source file");
        return -1;
    }
    
    /* Test with time report to file */
    char *argv1[] = {
        (char *)gcc_path,
        "-ftime-report",                  /* sets report_times_to_file */
        "-fprofile-arcs",                 /* enables profiling */
        "-ftest-coverage",                /* enables coverage */
        "-save-temps",
        "-dumpdir", "/tmp/time_dump",
        "-o", "/tmp/test_time.o",
        src_file,
        NULL
    };
    
    /* Test with time report to stderr */
    char *argv2[] = {
        (char *)gcc_path,
        "-ftime-report",
        "-fprofile-generate",             /* different profiling mode */
        "-v",
        "-o", "/tmp/test_time2.o",
        src_file,
        NULL
    };
    
    spawn_gcc(gcc_path, argv1);
    spawn_gcc(gcc_path, argv2);
    
    /* Cleanup */
    unlink(src_file);
    unlink("/tmp/test_time.o");
    unlink("/tmp/test_time2.o");
    unlink("/tmp/time_dump.test_gcc_time.i");
    unlink("/tmp/time_dump.test_gcc_time.s");
    unlink("/tmp/time_dump.test_gcc_time.o");
    unlink("/tmp/test_gcc_time.gcno");
    
    return 0;
}

/* Test case 5: Multiple output formats and dump options */
static int test_multiple_outputs(const char *gcc_path) {
    printf("Running multiple outputs test...\n");
    
    const char *src_file = "/tmp/test_gcc_multi.c";
    if (create_temp_source(src_file) < 0) {
        perror("Failed to create source file");
        return -1;
    }
    
    /* Test with various output combinations */
    char *argv1[] = {
        (char *)gcc_path,
        "-S",                             /* Generate assembly */
        "-save-temps",
        "-dumpdir", "/tmp/multi1",
        "-dumpbase", "output1",
        "-dumpbase-ext", ".ext1",
        "-o", "/tmp/output1.s",
        src_file,
        NULL
    };
    
    char *argv2[] = {
        (char *)gcc_path,
        "-c",                             /* Compile only */
        "-E",                             /* Preprocess only */
        "-save-temps=obj",
        "-dumpdir", "/tmp/multi2/",       /* Note trailing slash */
        "-dumpbase", "output2",
        "-o", "/tmp/output2.i",
        src_file,
        NULL
    };
    
    char *argv3[] = {
        (char *)gcc_path,
        "-c",
        "-save-temps=cwd",                /* Different save_temps mode */
        "-fdump-rtl-all",                 /* Enable RTL dumps */
        "-fdump-tree-all",                /* Enable tree dumps */
        "-o", "/tmp/output3.o",
        src_file,
        NULL
    };
    
    spawn_gcc(gcc_path, argv1);
    spawn_gcc(gcc_path, argv2);
    spawn_gcc(gcc_path, argv3);
    
    /* Cleanup */
    unlink(src_file);
    unlink("/tmp/output1.s");
    unlink("/tmp/output2.i");
    unlink("/tmp/output3.o");
    unlink("/tmp/multi1.output1.i");
    unlink("/tmp/multi1.output1.s");
    unlink("/tmp/multi2/output2.i");
    
    return 0;
}

int main(int argc, char *argv[]) {
    const char *gcc_path = "./gcc/xgcc";  /* Default path to instrumented GCC */
    
    /* Allow override of GCC path via command line */
    if (argc > 1) {
        gcc_path = argv[1];
    }
    
    printf("Testing GCC driver cleanup coverage with: %s\n", gcc_path);
    
    /* Run all test cases */
    test_comprehensive_compilation(gcc_path);
    test_help_and_version(gcc_path);
    test_cross_compilation(gcc_path);
    test_timing_and_profiling(gcc_path);
    test_multiple_outputs(gcc_path);
    
    /* Final test: Run GCC with minimal flags to ensure cleanup still happens */
    printf("Running final minimal compilation test...\n");
    const char *src_file = "/tmp/test_gcc_final.c";
    create_temp_source(src_file);
    
    char *final_argv[] = {
        (char *)gcc_path,
        "-O0",
        "-o", "/tmp/final_output",
        src_file,
        NULL
    };
    
    spawn_gcc(gcc_path, final_argv);
    
    /* Cleanup */
    unlink(src_file);
    unlink("/tmp/final_output");
    
    printf("All tests completed. Check coverage data for gcc.cc lines 11228-11250.\n");
    return 0;
}
