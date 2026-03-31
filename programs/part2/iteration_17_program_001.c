/* test_gcc_cleanup.c - Test program to cover GCC driver cleanup lines */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

/* Create a minimal C source file for compilation */
void create_test_source(const char *filename) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        perror("fopen");
        exit(1);
    }
    fprintf(f, "int main(void) { return 0; }\n");
    fclose(f);
}

/* Execute GCC with specific flags to set various global state variables */
int run_gcc_with_flags(const char *gcc_path, const char *source_file, 
                       const char *output_file, int test_num) {
    /* Different flag combinations for different test cases */
    const char *flags[][20] = {
        /* Test 1: Comprehensive flags setting many variables */
        {
            gcc_path,
            "-save-temps",              /* sets save_temps_flag */
            "-dumpdir", "/tmp/gcc_dump",/* allocates dumpdir */
            "-dumpbase", "testdump",    /* allocates dumpbase */
            "-dumpbase-ext", ".ext",    /* allocates dumpbase_ext */
            "--sysroot=/opt/mysysroot", /* sets target_system_root, target_system_root_changed */
            "-fuse-ld=gold",            /* sets use_ld */
            "-ftime-report",            /* sets report_times_to_file */
            "-v",                       /* sets verbose_only_flag */
            "-o", output_file,
            source_file,
            NULL
        },
        /* Test 2: Help and version flags */
        {
            gcc_path,
            "--help=common",            /* sets print_help_list */
            "--version",                /* sets print_version */
            "-v",                       /* sets verbose_only_flag */
            NULL
        },
        /* Test 3: Subprocess help and machine specs */
        {
            gcc_path,
            "-###",                     /* may set print_subprocess_help */
            "-march=x86-64",            /* affects spec_machine */
            "-mtune=generic",
            "-save-temps=obj",
            "-o", output_file,
            source_file,
            NULL
        },
        /* Test 4: Different sysroot and dump options */
        {
            gcc_path,
            "--sysroot=/",
            "-isysroot", "/usr/include",
            "-dumpdir", ".",
            "-dumpbase", "out",
            "-o", output_file,
            source_file,
            NULL
        }
    };
    
    if (test_num < 0 || test_num >= (int)(sizeof(flags)/sizeof(flags[0]))) {
        fprintf(stderr, "Invalid test number\n");
        return -1;
    }
    
    printf("Running test %d with flags:", test_num + 1);
    for (int i = 0; flags[test_num][i]; i++) {
        printf(" %s", flags[test_num][i]);
    }
    printf("\n");
    
    pid_t pid = fork();
    if (pid == 0) {
        /* Child process */
        
        /* Set environment variables that affect driver state */
        setenv("GCC_EXEC_PREFIX", "/usr/lib/gcc/", 1);
        setenv("COMPILER_PATH", "/usr/bin:/usr/local/bin", 1);
        setenv("LIBRARY_PATH", "/usr/lib:/usr/local/lib", 1);
        
        /* Execute GCC */
        execv(gcc_path, (char * const *)flags[test_num]);
        
        /* If we get here, exec failed */
        perror("execv");
        exit(1);
    } else if (pid > 0) {
        /* Parent process */
        int status;
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status)) {
            printf("Test %d exited with status %d\n", test_num + 1, WEXITSTATUS(status));
            return WEXITSTATUS(status);
        } else {
            printf("Test %d terminated abnormally\n", test_num + 1);
            return -1;
        }
    } else {
        perror("fork");
        return -1;
    }
}

int main(int argc, char *argv[]) {
    const char *gcc_path;
    char source_file[] = "/tmp/test_gcc_cover_XXXXXX.c";
    char output_file[] = "/tmp/test_gcc_cover_XXXXXX.o";
    char dumpdir[] = "/tmp/gcc_dump_XXXXXX";
    
    /* Determine GCC path - use first argument or default */
    if (argc > 1) {
        gcc_path = argv[1];
    } else {
        /* Try to find gcc in common locations */
        gcc_path = "./xgcc";  /* Common build directory location */
        if (access(gcc_path, X_OK) != 0) {
            gcc_path = "gcc";  /* Fall back to system gcc */
        }
    }
    
    printf("Testing GCC driver at: %s\n", gcc_path);
    
    /* Create unique temporary filenames */
    int fd = mkstemps(source_file, 2);  /* Creates /tmp/test_gcc_cover_XXXXXX.c */
    if (fd < 0) {
        perror("mkstemps");
        return 1;
    }
    close(fd);
    
    /* Create output filename */
    strcpy(output_file, source_file);
    char *dot = strrchr(output_file, '.');
    if (dot) *dot = '\0';
    strcat(output_file, ".o");
    
    /* Create dump directory */
    strcpy(dumpdir, "/tmp/gcc_dump_XXXXXX");
    if (mkdtemp(dumpdir) == NULL) {
        perror("mkdtemp");
        return 1;
    }
    
    /* Create test source file */
    create_test_source(source_file);
    
    /* Run multiple test cases to cover different code paths */
    int results[4];
    int total_tests = 4;
    
    for (int i = 0; i < total_tests; i++) {
        /* Modify environment between tests to ensure different state */
        if (i == 1) {
            setenv("GCC_EXEC_PREFIX", "/custom/gcc/path", 1);
        } else if (i == 2) {
            unsetenv("GCC_EXEC_PREFIX");
            setenv("COMPILER_PATH", "/opt/cross/bin:/usr/cross/bin", 1);
        }
        
        results[i] = run_gcc_with_flags(gcc_path, source_file, output_file, i);
        
        /* Small delay to ensure cleanup completes */
        usleep(10000);
    }
    
    /* Clean up temporary files */
    unlink(source_file);
    unlink(output_file);
    
    /* Try to clean up dump directory (might have files from -save-temps) */
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "rm -rf %s", dumpdir);
    system(cmd);
    
    /* Also clean up any other temporary files GCC might have created */
    char *base = strrchr(source_file, '/');
    if (base) {
        *base = '\0';  /* Get directory path */
        char pattern[256];
        snprintf(pattern, sizeof(pattern), "%s/*test_gcc_cover*", source_file);
        snprintf(cmd, sizeof(cmd), "rm -f %s", pattern);
        system(cmd);
    }
    
    printf("\nTest summary:\n");
    for (int i = 0; i < total_tests; i++) {
        printf("Test %d: %s\n", i + 1, 
               results[i] == 0 ? "PASSED" : 
               results[i] > 0 ? "COMPILER_ERROR" : "FAILED");
    }
    
    return 0;
}
