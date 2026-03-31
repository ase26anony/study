/* gcc_driver_cleanup_test.c
 * Test program to cover GCC driver cleanup code in gcc.cc lines 11228-11250
 * This program invokes the GCC driver with various flags that set global state,
 * ensuring the cleanup/reset logic is executed.
 */

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
        perror("Failed to create test source");
        exit(1);
    }
    fprintf(f, "int main(void) { return 0; }\n");
    fclose(f);
}

/* Remove temporary files */
void cleanup_files(const char *src, const char *obj, const char *dumpdir) {
    if (src) unlink(src);
    if (obj) unlink(obj);
    
    /* Remove dump directory contents if created */
    if (dumpdir) {
        char cmd[256];
        snprintf(cmd, sizeof(cmd), "rm -rf %s", dumpdir);
        system(cmd);
    }
}

/* Execute GCC with specific flags to set various global state variables */
int run_gcc_with_flags(const char *gcc_path, const char *src_file, 
                       const char *output_file, int test_num) {
    pid_t pid;
    int status;
    
    /* Different flag combinations for different test cases */
    const char *flags[][20] = {
        /* Test 1: Comprehensive flags setting many state variables */
        {
            gcc_path,
            "-save-temps",              /* sets save_temps_flag */
            "-dumpdir", "/tmp/gcc_dump", /* allocates dumpdir */
            "-dumpbase", "test_dump",   /* allocates dumpbase */
            "-dumpbase-ext", ".ext",    /* allocates dumpbase_ext */
            "--sysroot=/opt/mysysroot", /* sets target_system_root, target_system_root_changed */
            "-fuse-ld=gold",            /* sets use_ld */
            "-ftime-report",            /* sets report_times_to_file */
            "-v",                       /* sets verbose_only_flag */
            "-o", output_file,
            src_file,
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
        
        /* Test 3: Subprocess help and verbose */
        {
            gcc_path,
            "-###",                     /* may set print_subprocess_help */
            "-v",
            "-o", output_file,
            src_file,
            NULL
        },
        
        /* Test 4: Different machine spec */
        {
            gcc_path,
            "-march=x86-64",            /* may affect spec_machine */
            "-mtune=generic",
            "-save-temps=obj",
            "-dumpdir", "./my_dumps",
            "-dumpbase", "mytest",
            "-o", output_file,
            src_file,
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
    
    pid = fork();
    if (pid == 0) {
        /* Child process */
        
        /* Set environment variables that affect driver state */
        setenv("GCC_EXEC_PREFIX", "/usr/local/lib/gcc/", 1);
        setenv("COMPILER_PATH", "/usr/bin:/usr/local/bin", 1);
        
        /* Execute GCC */
        execv(gcc_path, (char *const *)flags[test_num]);
        
        /* If execv fails */
        perror("execv failed");
        exit(1);
    } else if (pid > 0) {
        /* Parent process */
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status)) {
            printf("Test %d exited with status %d\n", test_num + 1, WEXITSTATUS(status));
        } else {
            printf("Test %d terminated abnormally\n", test_num + 1);
        }
        
        return status;
    } else {
        perror("fork failed");
        return -1;
    }
}

int main(int argc, char *argv[]) {
    const char *gcc_path;
    char src_file[] = "/tmp/gcc_test_XXXXXX.c";
    char obj_file[] = "/tmp/gcc_test_XXXXXX.o";
    char dump_dir[] = "/tmp/gcc_dump";
    int fd;
    int overall_status = 0;
    
    /* Determine GCC path - use first argument or default to xgcc in build dir */
    if (argc > 1) {
        gcc_path = argv[1];
    } else {
        /* Try to find the GCC driver in common build locations */
        gcc_path = "./xgcc";
        if (access(gcc_path, X_OK) != 0) {
            gcc_path = "./gcc/xgcc";
            if (access(gcc_path, X_OK) != 0) {
                gcc_path = "gcc";  /* Fall back to system gcc */
            }
        }
    }
    
    printf("Using GCC driver: %s\n", gcc_path);
    
    /* Create unique temporary filenames */
    fd = mkstemps(src_file, 2);  /* Creates /tmp/gcc_test_XXXXXX.c */
    if (fd < 0) {
        perror("Failed to create temp source file");
        return 1;
    }
    close(fd);
    
    /* Create object filename */
    strncpy(obj_file, src_file, sizeof(obj_file));
    char *dot = strrchr(obj_file, '.');
    if (dot) *dot = '\0';
    strcat(obj_file, ".o");
    
    /* Create test source file */
    create_test_source(src_file);
    
    /* Create dump directory */
    mkdir(dump_dir, 0755);
    
    /* Run multiple test cases to cover different state variables */
    for (int i = 0; i < 4; i++) {
        char test_obj[256];
        
        /* Use different output files for each test to avoid conflicts */
        if (i > 0) {
            snprintf(test_obj, sizeof(test_obj), "%s.%d", obj_file, i);
        } else {
            strncpy(test_obj, obj_file, sizeof(test_obj));
        }
        
        int status = run_gcc_with_flags(gcc_path, src_file, test_obj, i);
        if (status != 0 && i != 1) {  /* Test 1 (help/version) may exit early */
            printf("Warning: Test %d returned non-zero status\n", i + 1);
        }
        
        /* Clean up test-specific output */
        if (i > 0) {
            unlink(test_obj);
            /* Also clean up any .i, .s files from -save-temps */
            char temp_file[256];
            snprintf(temp_file, sizeof(temp_file), "%s.i", test_obj);
            unlink(temp_file);
            snprintf(temp_file, sizeof(temp_file), "%s.s", test_obj);
            unlink(temp_file);
        }
        
        /* Small delay between tests */
        sleep(1);
    }
    
    /* Additional test: Run GCC with environment variables that affect state */
    printf("\nRunning additional test with modified environment...\n");
    {
        pid_t pid = fork();
        if (pid == 0) {
            /* Set multiple environment variables */
            setenv("GCC_EXEC_PREFIX", "/test/prefix", 1);
            setenv("GCC_COLORS", "always", 1);
            setenv("LC_ALL", "C", 1);
            
            /* Run with flags that allocate dump strings */
            const char *args[] = {
                gcc_path,
                "-save-temps",
                "-dumpdir", "/tmp/special_dump",
                "-dumpbase", "env_test",
                "-dumpbase-ext", ".special",
                "-o", "/tmp/env_test.o",
                src_file,
                NULL
            };
            
            execv(gcc_path, (char *const *)args);
            perror("execv failed");
            exit(1);
        } else if (pid > 0) {
            waitpid(pid, &overall_status, 0);
            unlink("/tmp/env_test.o");
            unlink("/tmp/env_test.i");
            unlink("/tmp/env_test.s");
        }
    }
    
    /* Cleanup */
    cleanup_files(src_file, obj_file, dump_dir);
    
    /* Also clean up any files from the first test */
    char base_name[256];
    strncpy(base_name, src_file, sizeof(base_name));
    char *dot = strrchr(base_name, '.');
    if (dot) *dot = '\0';
    
    char to_delete[256];
    snprintf(to_delete, sizeof(to_delete), "%s.i", base_name);
    unlink(to_delete);
    snprintf(to_delete, sizeof(to_delete), "%s.s", base_name);
    unlink(to_delete);
    
    printf("\nAll tests completed. The GCC driver's cleanup code should have been executed.\n");
    printf("Check coverage data to verify lines 11228-11250 in gcc.cc are covered.\n");
    
    return overall_status;
}
