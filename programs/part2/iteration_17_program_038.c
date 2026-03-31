/* test_gcc_cleanup.c - Test program to cover driver cleanup block */
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

static void create_test_source(const char *path) {
    FILE *f = fopen(path, "w");
    if (!f) {
        perror("fopen source");
        exit(1);
    }
    fprintf(f, "int main(void) { return 0; }\n");
    fclose(f);
}

static void run_gcc(const char **argv, const char **envp) {
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        exit(1);
    }
    
    if (pid == 0) { /* Child */
        if (envp) {
            /* Clear environment and set new variables */
            extern char **environ;
            environ = (char **)envp;
        }
        execv(argv[0], (char * const *)argv);
        perror("execv");
        _exit(1);
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
    printf("\n=== Test Case 1: Full compilation with state-altering flags ===\n");
    
    /* Create temporary directory */
    mkdir(TMP_DIR, 0755);
    
    /* Create test source */
    char src_path[256];
    snprintf(src_path, sizeof(src_path), "%s/test.c", TMP_DIR);
    create_test_source(src_path);
    
    /* Complex GCC command line to set many state variables */
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
        "-specs=/dev/null",         /* affects spec processing */
        "-mtune=cortex-a72",        /* affects spec_machine */
        "-march=armv8-a",           /* affects spec_machine */
        "-B", "/usr/lib/gcc",       /* affects exec_prefix */
        "-idirafter", "/usr/include",
        "-iquote", TMP_DIR,
        "-isystem", "/usr/local/include",
        "-L", TMP_DIR,
        "-l", "m",
        "-o", "/tmp/test_output.o",
        src_path,
        NULL
    };
    
    /* Environment variables that affect driver state */
    const char *envp[] = {
        "GCC_EXEC_PREFIX=/usr/lib/gcc/",
        "COMPILER_PATH=/usr/bin:/usr/local/bin",
        "LIBRARY_PATH=/usr/lib:/usr/local/lib",
        "C_INCLUDE_PATH=/usr/include",
        "CPLUS_INCLUDE_PATH=/usr/include/c++",
        NULL
    };
    
    run_gcc(argv, envp);
}

static void test_case_2(void) {
    printf("\n=== Test Case 2: Help and version flags ===\n");
    
    /* This triggers print_help_list, print_version, print_subprocess_help */
    const char *argv1[] = {
        GCC_PATH,
        "--help=common",      /* sets print_help_list */
        "--version",          /* sets print_version */
        "-v",                 /* verbose flag */
        NULL
    };
    
    run_gcc(argv1, NULL);
    
    /* Test subprocess help */
    const char *argv2[] = {
        GCC_PATH,
        "--help=subprocess",  /* may set print_subprocess_help */
        NULL
    };
    
    run_gcc(argv2, NULL);
}

static void test_case_3(void) {
    printf("\n=== Test Case 3: Dry-run with dump options ===\n");
    
    /* Create another test source */
    char src_path[256];
    snprintf(src_path, sizeof(src_path), "%s/test2.c", TMP_DIR);
    create_test_source(src_path);
    
    /* Test with -### (dry run) which shows commands but doesn't execute */
    const char *argv[] = {
        GCC_PATH,
        "-###",               /* dry run, may affect print_subprocess_help */
        "-save-temps=obj",
        "-dumpdir", "/tmp/alt_dump",
        "-dumpbase", "altbase",
        "-dumpbase-ext", ".alt",
        "--sysroot=/",
        "-fuse-ld=bfd",
        "-ftime-report",
        "-wrapper", "/bin/true",
        "-v",
        "-o", "/tmp/test2.o",
        src_path,
        NULL
    };
    
    run_gcc(argv, NULL);
}

static void test_case_4(void) {
    printf("\n=== Test Case 4: Cross-compilation scenario ===\n");
    
    /* Try to simulate cross-compilation by targeting different machine */
    const char *argv[] = {
        GCC_PATH,
        "-dumpdir", TMP_DIR,
        "-dumpbase", "cross",
        "-mtune=generic",
        "-march=x86-64",
        "-mcpu=skylake",
        "--sysroot=/sysroot",
        "-target", "x86_64-linux-gnu",
        "-B", "/usr/x86_64-linux-gnu/bin",
        "-save-temps=cwd",
        "-v",
        "-E",  /* Preprocess only to ensure cleanup still runs */
        "-dD", /* Dump macros */
        "-",
        NULL
    };
    
    /* Pipe source via stdin */
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return;
    }
    
    if (pid == 0) {
        const char *src = "int main(void) { return 0; }\n";
        write(STDIN_FILENO, src, strlen(src));
        close(STDIN_FILENO);
        
        execv(argv[0], (char * const *)argv);
        perror("execv");
        _exit(1);
    } else {
        int status;
        waitpid(pid, &status, 0);
    }
}

static void cleanup_temp_files(void) {
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "rm -rf %s /tmp/test_output.o /tmp/test2.o /tmp/*.i /tmp/*.s /tmp/*.o 2>/dev/null", TMP_DIR);
    system(cmd);
}

int main(void) {
    printf("Testing GCC driver cleanup block coverage\n");
    
    /* Run multiple test cases to hit different state combinations */
    test_case_1();  /* Full compilation with many flags */
    test_case_2();  /* Help/version flags */
    test_case_3();  /* Dry-run mode */
    test_case_4();  /* Cross-compilation scenario */
    
    /* Clean up temporary files */
    cleanup_temp_files();
    
    printf("\nAll test cases completed. Check coverage with:\n");
    printf("  gcov gcc.cc\n");
    printf("  or\n");
    printf("  lcov --capture --directory . --output-file gcc.info\n");
    
    return 0;
}
