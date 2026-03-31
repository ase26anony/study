/* gcc_cleanup_coverage.c - Test program to cover driver cleanup code */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

/* Helper function to create a temporary source file */
static char *create_temp_source(const char *content) {
    char template[] = "/tmp/gcc_test_XXXXXX.c";
    int fd = mkstemps(template, 2);  /* .c suffix is 2 chars */
    if (fd < 0) {
        perror("mkstemps");
        return NULL;
    }
    
    write(fd, content, strlen(content));
    close(fd);
    
    return strdup(template);
}

/* Helper to run GCC with specific arguments */
static int run_gcc(const char *gcc_path, char *const argv[]) {
    pid_t pid = fork();
    
    if (pid == 0) {
        /* Child process */
        execv(gcc_path, argv);
        perror("execv");
        exit(EXIT_FAILURE);
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

/* Test case 1: Compile with many state-altering flags */
static void test_complex_compilation(const char *gcc_path) {
    printf("=== Test 1: Complex compilation with state-altering flags ===\n");
    
    /* Create a minimal C source file */
    char *source_file = create_temp_source(
        "int main(void) {\n"
        "    return 0;\n"
        "}\n"
    );
    
    if (!source_file) {
        fprintf(stderr, "Failed to create source file\n");
        return;
    }
    
    /* Output file */
    char output_file[] = "/tmp/test_output.o";
    
    /* Construct complex command line to set many global variables */
    char *argv[] = {
        (char *)gcc_path,
        "-save-temps",              /* sets save_temps_flag */
        "-dumpdir", "/tmp/mydump",  /* allocates dumpdir */
        "-dumpbase", "myprogram",   /* allocates dumpbase */
        "-dumpbase-ext", ".ext",    /* allocates dumpbase_ext */
        "--sysroot=/opt/mysysroot", /* sets target_system_root, target_system_root_changed */
        "-fuse-ld=gold",            /* sets use_ld */
        "-ftime-report",            /* sets report_times_to_file */
        "-v",                       /* sets verbose_only_flag */
        "-c",                       /* compile only */
        source_file,
        "-o", output_file,
        NULL
    };
    
    /* Set environment variables that affect driver state */
    setenv("GCC_EXEC_PREFIX", "/usr/lib/gcc/", 1);
    setenv("COMPILER_PATH", "/usr/bin:/usr/local/bin", 1);
    
    printf("Running: ");
    for (int i = 0; argv[i] != NULL; i++) {
        printf("%s ", argv[i]);
    }
    printf("\n");
    
    int result = run_gcc(gcc_path, argv);
    printf("Exit code: %d\n\n", result);
    
    /* Cleanup temporary files */
    unlink(source_file);
    unlink(output_file);
    
    /* Also clean up any dump files created */
    char dump_files[][100] = {
        "/tmp/mydumpmyprogram.ext.i",
        "/tmp/mydumpmyprogram.ext.s",
        "/tmp/mydumpmyprogram.ext.o"
    };
    
    for (size_t i = 0; i < sizeof(dump_files)/sizeof(dump_files[0]); i++) {
        unlink(dump_files[i]);
    }
    
    free(source_file);
}

/* Test case 2: Help and version flags */
static void test_help_and_version(const char *gcc_path) {
    printf("=== Test 2: Help and version flags ===\n");
    
    /* Test with --help= to set print_help_list */
    char *argv1[] = {
        (char *)gcc_path,
        "--help=common",            /* sets print_help_list */
        "--version",                /* sets print_version */
        NULL
    };
    
    printf("Running with --help=common --version\n");
    int result1 = run_gcc(gcc_path, argv1);
    printf("Exit code: %d\n\n", result1);
    
    /* Test with -### to potentially set print_subprocess_help */
    char *argv2[] = {
        (char *)gcc_path,
        "-###",                     /* may set print_subprocess_help */
        "-E",                       /* preprocess only */
        "-",
        NULL
    };
    
    printf("Running with -### -E -\n");
    int result2 = run_gcc(gcc_path, argv2);
    printf("Exit code: %d\n\n", result2);
}

/* Test case 3: Different target configurations */
static void test_target_configs(const char *gcc_path) {
    printf("=== Test 3: Target configuration variations ===\n");
    
    char *source_file = create_temp_source(
        "int main(void) {\n"
        "    return 0;\n"
        "}\n"
    );
    
    if (!source_file) {
        fprintf(stderr, "Failed to create source file\n");
        return;
    }
    
    /* Test with different machine specs */
    char *argv[] = {
        (char *)gcc_path,
        "-dumpdir", ".",
        "-dumpbase", "test",
        "-march=x86-64",            /* may affect spec_machine */
        "-mtune=generic",
        "-c",
        source_file,
        "-o", "/tmp/test3.o",
        NULL
    };
    
    printf("Running with architecture-specific flags\n");
    int result = run_gcc(gcc_path, argv);
    printf("Exit code: %d\n\n", result);
    
    /* Cleanup */
    unlink(source_file);
    unlink("/tmp/test3.o");
    free(source_file);
}

/* Test case 4: Multiple invocations to stress cleanup */
static void test_multiple_invocations(const char *gcc_path) {
    printf("=== Test 4: Multiple rapid invocations ===\n");
    
    char *source_file = create_temp_source(
        "int main(void) {\n"
        "    return 0;\n"
        "}\n"
    );
    
    if (!source_file) {
        fprintf(stderr, "Failed to create source file\n");
        return;
    }
    
    /* Run multiple times with different dumpdir values */
    for (int i = 0; i < 3; i++) {
        char dumpdir[50];
        char dumpbase[50];
        char output_file[50];
        
        snprintf(dumpdir, sizeof(dumpdir), "/tmp/run%d", i);
        snprintf(dumpbase, sizeof(dumpbase), "test%d", i);
        snprintf(output_file, sizeof(output_file), "/tmp/output%d.o", i);
        
        char *argv[] = {
            (char *)gcc_path,
            "-save-temps",
            "-dumpdir", dumpdir,
            "-dumpbase", dumpbase,
            "-c",
            source_file,
            "-o", output_file,
            NULL
        };
        
        printf("Run %d: dumpdir=%s, dumpbase=%s\n", i, dumpdir, dumpbase);
        int result = run_gcc(gcc_path, argv);
        printf("  Exit code: %d\n", result);
        
        /* Cleanup output */
        unlink(output_file);
    }
    
    printf("\n");
    
    /* Cleanup source */
    unlink(source_file);
    free(source_file);
}

int main(int argc, char *argv[]) {
    const char *gcc_path;
    
    /* Determine GCC path to test */
    if (argc > 1) {
        gcc_path = argv[1];
    } else {
        /* Default to the gcc in PATH, or adjust as needed */
        gcc_path = "gcc";
    }
    
    printf("Testing GCC driver cleanup with: %s\n\n", gcc_path);
    
    /* Run all test cases */
    test_complex_compilation(gcc_path);
    test_help_and_version(gcc_path);
    test_target_configs(gcc_path);
    test_multiple_invocations(gcc_path);
    
    printf("All tests completed.\n");
    return 0;
}
