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
static char* create_temp_source(const char* content) {
    char template[] = "/tmp/gcc_test_XXXXXX.c";
    int fd = mkstemps(template, 2);  /* .c suffix is 2 chars */
    if (fd < 0) {
        perror("mkstemps failed");
        return NULL;
    }
    
    write(fd, content, strlen(content));
    close(fd);
    
    return strdup(template);
}

/* Helper function to run GCC with specific arguments */
static int run_gcc(const char* gcc_path, char* const argv[], char* const envp[]) {
    pid_t pid = fork();
    
    if (pid == 0) {
        /* Child process */
        execve(gcc_path, argv, envp);
        perror("execve failed");
        _exit(1);
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

/* Test 1: Comprehensive compilation with many state-altering flags */
static void test_comprehensive_compilation(const char* gcc_path) {
    printf("=== Test 1: Comprehensive compilation ===\n");
    
    /* Create a minimal C source file */
    char* src_file = create_temp_source(
        "int main() {\n"
        "    return 0;\n"
        "}\n"
    );
    if (!src_file) return;
    
    /* Prepare environment variables */
    char* env[] = {
        "GCC_EXEC_PREFIX=/usr/lib/gcc/",
        "COMPILER_PATH=/usr/bin:/usr/local/bin",
        "LIBRARY_PATH=/usr/lib:/usr/local/lib",
        "C_INCLUDE_PATH=/usr/include",
        "CPLUS_INCLUDE_PATH=/usr/include/c++/11",
        NULL
    };
    
    /* Build complex command line to set many global variables */
    char* args[] = {
        (char*)gcc_path,
        "-save-temps",              /* sets save_temps_flag */
        "-dumpdir", "/tmp/my_dumps", /* allocates dumpdir */
        "-dumpbase", "test_coverage", /* allocates dumpbase */
        "-dumpbase-ext", ".ext",    /* allocates dumpbase_ext */
        "--sysroot=/opt/custom_sysroot", /* sets target_system_root */
        "-fuse-ld=gold",            /* sets use_ld */
        "-ftime-report",            /* sets report_times_to_file */
        "-v",                       /* sets verbose_only_flag */
        "-march=native",            /* may affect spec_machine */
        "-mtune=generic",
        "-specs=/dev/null",         /* forces spec processing */
        "-B/usr/local/lib/gcc",
        "-idirafter", "/usr/local/include",
        "-I/tmp/include",
        "-L/tmp/lib",
        "-Wl,-rpath=/tmp/lib",
        "-o", "/tmp/test_output",
        src_file,
        NULL
    };
    
    printf("Running: ");
    for (int i = 0; args[i]; i++) {
        printf("%s ", args[i]);
    }
    printf("\n");
    
    int result = run_gcc(gcc_path, args, env);
    printf("Exit code: %d\n\n", result);
    
    /* Cleanup */
    unlink(src_file);
    unlink("/tmp/test_output");
    unlink("/tmp/test_output.i");   /* -save-temps files */
    unlink("/tmp/test_output.s");
    unlink("/tmp/test_output.o");
    free(src_file);
}

/* Test 2: Help and version flags to set print_help_list and print_version */
static void test_help_and_version(const char* gcc_path) {
    printf("=== Test 2: Help and version flags ===\n");
    
    char* env[] = {
        "GCC_EXEC_PREFIX=/dummy/path",
        NULL
    };
    
    /* Test with --help=common (sets print_help_list) */
    printf("Testing --help=common:\n");
    char* help_args[] = {
        (char*)gcc_path,
        "--help=common",
        "--sysroot=/dummy",         /* Also set sysroot */
        NULL
    };
    
    int result = run_gcc(gcc_path, help_args, env);
    printf("Exit code: %d\n", result);
    
    /* Test with --version (sets print_version) */
    printf("\nTesting --version:\n");
    char* version_args[] = {
        (char*)gcc_path,
        "--version",
        "-dumpbase", "version_test", /* Combine with dump flags */
        NULL
    };
    
    result = run_gcc(gcc_path, version_args, env);
    printf("Exit code: %d\n\n", result);
}

/* Test 3: Subprocess help and verbose flags */
static void test_subprocess_and_verbose(const char* gcc_path) {
    printf("=== Test 3: Subprocess help and verbose ===\n");
    
    char* env[] = {
        "GCC_EXEC_PREFIX=/another/dummy",
        NULL
    };
    
    /* Test -### (may set print_subprocess_help) */
    printf("Testing -### (dry run):\n");
    char* src_file = create_temp_source("int x = 1;");
    if (!src_file) return;
    
    char* dry_run_args[] = {
        (char*)gcc_path,
        "-###",                     /* dry run mode */
        "-save-temps=cwd",          /* different save_temps_flag value */
        "-dumpdir", ".",
        "-dumpbase", "dryrun",
        "-v",                       /* verbose */
        "-ftime-report",
        src_file,
        NULL
    };
    
    int result = run_gcc(gcc_path, dry_run_args, env);
    printf("Exit code: %d\n\n", result);
    
    free(src_file);
}

/* Test 4: Multiple invocations to stress cleanup between runs */
static void test_multiple_invocations(const char* gcc_path) {
    printf("=== Test 4: Multiple rapid invocations ===\n");
    
    char* src_file = create_temp_source(
        "#include <stdio.h>\n"
        "int main() { printf(\"test\\n\"); return 0; }\n"
    );
    if (!src_file) return;
    
    char* env[] = {
        "GCC_EXEC_PREFIX=/test/prefix",
        NULL
    };
    
    /* Run multiple times with different combinations */
    for (int i = 0; i < 3; i++) {
        printf("Invocation %d:\n", i + 1);
        
        char dumpdir[64];
        char dumpbase[64];
        snprintf(dumpdir, sizeof(dumpdir), "/tmp/run%d", i);
        snprintf(dumpbase, sizeof(dumpbase), "multi%d", i);
        
        char* args[] = {
            (char*)gcc_path,
            "-save-temps",
            "-dumpdir", dumpdir,
            "-dumpbase", dumpbase,
            i == 0 ? "--sysroot=/tmp/sysroot1" : 
             i == 1 ? "--sysroot=/tmp/sysroot2" : 
                     "--sysroot=/tmp/sysroot3",
            i == 0 ? "-fuse-ld=bfd" : 
             i == 1 ? "-fuse-ld=lld" : 
                     "-fuse-ld=mold",
            "-v",
            "-o", "/tmp/multi_output",
            src_file,
            NULL
        };
        
        int result = run_gcc(gcc_path, args, env);
        printf("  Exit code: %d\n", result);
        
        /* Clean output file */
        unlink("/tmp/multi_output");
    }
    
    printf("\n");
    free(src_file);
}

/* Test 5: Error case to ensure cleanup still happens */
static void test_error_case(const char* gcc_path) {
    printf("=== Test 5: Error case with invalid options ===\n");
    
    char* env[] = {
        "GCC_EXEC_PREFIX=/error/path",
        NULL
    };
    
    /* Invalid combination that should cause error but still trigger cleanup */
    char* error_args[] = {
        (char*)gcc_path,
        "-save-temps=invalid",      /* Invalid value */
        "-dumpdir", "/tmp/error_dump",
        "-dumpbase", "error",
        "--sysroot=/nonexistent",
        "-fuse-ld=nonexistent",
        "-invalid-flag",            /* Definitely invalid */
        "-o", "/tmp/error_out",
        "/nonexistent.c",           /* Non-existent source */
        NULL
    };
    
    printf("Running with invalid options (should fail):\n");
    int result = run_gcc(gcc_path, error_args, env);
    printf("Exit code: %d (expected non-zero)\n\n", result);
}

int main(int argc, char* argv[]) {
    const char* gcc_path;
    
    /* Determine GCC path to test */
    if (argc > 1) {
        gcc_path = argv[1];
    } else {
        /* Default to the gcc in PATH, or use a common location */
        gcc_path = "gcc";
    }
    
    printf("Testing GCC driver cleanup with: %s\n\n", gcc_path);
    
    /* Run all tests */
    test_comprehensive_compilation(gcc_path);
    test_help_and_version(gcc_path);
    test_subprocess_and_verbose(gcc_path);
    test_multiple_invocations(gcc_path);
    test_error_case(gcc_path);
    
    printf("All tests completed. Check coverage for driver cleanup block.\n");
    
    /* Final cleanup of any remaining temp files */
    system("rm -f /tmp/gcc_test_*.c /tmp/*.i /tmp/*.s /tmp/*.o /tmp/*.ii "
           "/tmp/test_output* /tmp/multi_output* /tmp/error_out* "
           "/tmp/my_dumps/* 2>/dev/null");
    system("rmdir /tmp/my_dumps 2>/dev/null");
    
    return 0;
}
