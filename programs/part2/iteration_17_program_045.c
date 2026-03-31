/* test_gcc_cleanup.c - Test program to cover driver cleanup logic */
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

/* Execute GCC with specific flags to set driver state variables */
int run_gcc_with_flags(const char *gcc_path, const char *source_file, 
                       const char *output_file, int test_num) {
    pid_t pid;
    int status;
    
    /* Different flag combinations for different test cases */
    const char *flags[][20] = {
        /* Test 1: Comprehensive flags setting many state variables */
        {
            gcc_path,
            "-save-temps",              /* sets save_temps_flag */
            "-dumpdir", "/tmp/gcc_dump",/* allocates dumpdir */
            "-dumpbase", "test_dump",   /* allocates dumpbase */
            "-dumpbase-ext", ".ext",    /* allocates dumpbase_ext */
            "--sysroot=/opt/mysysroot", /* sets target_system_root, target_system_root_changed */
            "-fuse-ld=gold",            /* sets use_ld */
            "-ftime-report",            /* sets report_times_to_file */
            "-v",                       /* sets verbose_only_flag */
            "-specs=/dev/null",         /* triggers spec processing */
            "-o", output_file,
            source_file,
            NULL
        },
        /* Test 2: Help and version flags */
        {
            gcc_path,
            "--help=common",            /* sets print_help_list */
            "--version",                /* sets print_version */
            "-###",                     /* may set print_subprocess_help */
            source_file,
            NULL
        },
        /* Test 3: Different machine spec and sysroot suffix */
        {
            gcc_path,
            "-save-temps=obj",
            "-dumpdir", "/tmp/dump2/",  /* trailing slash for dumpdir_trailing_dash_added */
            "--sysroot=/usr/sysroot",
            "--no-sysroot-suffix",      /* affects target_sysroot_suffix */
            "-mtune=generic",           /* affects spec_machine */
            "-march=x86-64",            /* affects spec_machine */
            "-o", "/tmp/test3.o",
            source_file,
            NULL
        },
        /* Test 4: Environment variable influenced execution */
        {
            gcc_path,
            "-save-temps=cwd",
            "-dumpbase", "envtest",
            "-o", "/tmp/test4.o",
            source_file,
            NULL
        }
    };
    
    if (test_num < 0 || test_num >= (int)(sizeof(flags)/sizeof(flags[0]))) {
        fprintf(stderr, "Invalid test number\n");
        return -1;
    }
    
    printf("Running test %d with flags:\n", test_num + 1);
    for (int i = 0; flags[test_num][i]; i++) {
        printf("%s ", flags[test_num][i]);
    }
    printf("\n");
    
    pid = fork();
    if (pid < 0) {
        perror("fork");
        return -1;
    }
    
    if (pid == 0) {
        /* Child process */
        if (test_num == 3) {
            /* Set environment variables that affect driver state */
            setenv("GCC_EXEC_PREFIX", "/usr/lib/gcc/", 1);
            setenv("COMPILER_PATH", "/usr/bin:/usr/local/bin", 1);
            setenv("GCC_DEBUG_PREFIX", "/tmp/debug", 1);
        }
        
        execv(gcc_path, (char * const *)flags[test_num]);
        
        /* If execv fails */
        perror("execv");
        exit(1);
    } else {
        /* Parent process */
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status)) {
            printf("Test %d exited with status %d\n\n", 
                   test_num + 1, WEXITSTATUS(status));
            return WEXITSTATUS(status);
        } else {
            printf("Test %d terminated abnormally\n\n", test_num + 1);
            return -1;
        }
    }
}

/* Test with cross-compilation if available */
int test_cross_compilation(const char *gcc_path, const char *source_file) {
    pid_t pid;
    int status;
    
    /* Try different target triples to affect spec_machine */
    const char *targets[] = {
        "x86_64-linux-gnu",
        "arm-linux-gnueabihf",
        "aarch64-linux-gnu",
        NULL
    };
    
    for (int i = 0; targets[i]; i++) {
        char gcc_cmd[256];
        snprintf(gcc_cmd, sizeof(gcc_cmd), "%s-gcc", targets[i]);
        
        /* Check if cross-compiler exists by trying a simple invocation */
        pid = fork();
        if (pid == 0) {
            /* First check if this cross-compiler exists */
            execlp(gcc_cmd, gcc_cmd, "--version", NULL);
            exit(127); /* Command not found */
        }
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status) && WEXITSTATUS(status) != 127) {
            /* Cross-compiler exists, run it with state-setting flags */
            printf("Testing with cross-compiler: %s\n", gcc_cmd);
            
            const char *args[] = {
                gcc_cmd,
                "-save-temps",
                "-dumpdir", "/tmp/cross_dump",
                "--sysroot=/opt/cross/sysroot",
                "-v",
                "-o", "/tmp/cross_test.o",
                source_file,
                NULL
            };
            
            pid = fork();
            if (pid == 0) {
                execvp(gcc_cmd, (char * const *)args);
                exit(1);
            }
            waitpid(pid, &status, 0);
            
            if (WIFEXITED(status)) {
                printf("Cross-compiler test for %s completed\n\n", 
                       targets[i]);
            }
        }
    }
    
    return 0;
}

int main(int argc, char *argv[]) {
    const char *source_file = "/tmp/test_cover_gcc.c";
    const char *output_file = "/tmp/test_output.o";
    const char *gcc_path = "./xgcc";  /* Adjust based on your build directory */
    
    /* Use provided GCC path if specified */
    if (argc > 1) {
        gcc_path = argv[1];
    }
    
    printf("Testing GCC driver cleanup with: %s\n", gcc_path);
    
    /* Create test source file */
    create_test_source(source_file);
    
    /* Run multiple test cases with different flag combinations */
    for (int i = 0; i < 4; i++) {
        run_gcc_with_flags(gcc_path, source_file, 
                          i == 0 ? output_file : "/tmp/test_output2.o", i);
        
        /* Small delay to ensure cleanup completes */
        usleep(100000);
    }
    
    /* Test cross-compilation if available */
    test_cross_compilation(gcc_path, source_file);
    
    /* Additional test: Run GCC with error to ensure cleanup still happens */
    printf("Testing with compilation error (should still trigger cleanup):\n");
    {
        const char *error_args[] = {
            gcc_path,
            "-save-temps",
            "-dumpdir", "/tmp/error_dump",
            "-o", "/tmp/error.o",
            "/tmp/nonexistent.c",  /* This will cause an error */
            NULL
        };
        
        pid_t pid = fork();
        if (pid == 0) {
            execv(gcc_path, (char * const *)error_args);
            exit(1);
        }
        waitpid(pid, NULL, 0);
        printf("Error test completed\n\n");
    }
    
    /* Test with outbase (affects outbase variable) */
    printf("Testing with -save-temps=obj and output redirection:\n");
    {
        const char *outbase_args[] = {
            gcc_path,
            "-save-temps=obj",
            "-dumpdir", "/tmp/outbase_test",
            "-o", "/tmp/final_output.exe",
            source_file,
            NULL
        };
        
        pid_t pid = fork();
        if (pid == 0) {
            execv(gcc_path, (char * const *)outbase_args);
            exit(1);
        }
        waitpid(pid, NULL, 0);
        printf("Outbase test completed\n\n");
    }
    
    /* Cleanup temporary files */
    unlink(source_file);
    unlink(output_file);
    unlink("/tmp/test_output2.o");
    unlink("/tmp/test3.o");
    unlink("/tmp/test4.o");
    unlink("/tmp/cross_test.o");
    unlink("/tmp/error.o");
    unlink("/tmp/final_output.exe");
    
    /* Also clean up any .i, .s, .o files created by -save-temps */
    system("rm -f /tmp/*.i /tmp/*.s /tmp/*.o /tmp/gcc_dump/* /tmp/dump2/* /tmp/cross_dump/* 2>/dev/null");
    
    printf("All tests completed. Check coverage data for gcc.cc lines 11228-11250\n");
    
    return 0;
}
