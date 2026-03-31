/* test_gcc_cleanup.c - Test program to cover driver cleanup lines in gcc.cc */

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

/* Run GCC with specific flags to set various global state variables */
int run_gcc_with_flags(const char *gcc_path, const char *source_file, 
                       const char *output_file, int test_num) {
    /* Construct command line with flags that set the uncovered variables */
    const char *argv[32];
    int argc = 0;
    
    argv[argc++] = gcc_path;
    
    /* Flags to set variables in the uncovered block */
    if (test_num == 1) {
        /* Comprehensive test with most flags */
        argv[argc++] = "-save-temps";          /* sets save_temps_flag */
        argv[argc++] = "-dumpdir";             /* allocates dumpdir */
        argv[argc++] = "/tmp/gcc_test_dump";
        argv[argc++] = "-dumpbase";            /* allocates dumpbase */
        argv[argc++] = "test_dumpbase";
        argv[argc++] = "-dumpbase-ext";        /* allocates dumpbase_ext */
        argv[argc++] = ".test_ext";
        argv[argc++] = "--sysroot=/opt/test_sysroot"; /* sets target_system_root */
        argv[argc++] = "-fuse-ld=gold";        /* sets use_ld */
        argv[argc++] = "-ftime-report";        /* sets report_times_to_file */
        argv[argc++] = "-v";                   /* sets verbose_only_flag */
        argv[argc++] = "-specs=/dev/null";     /* affects spec processing */
    } else if (test_num == 2) {
        /* Test with help and version flags */
        argv[argc++] = "--help=common";        /* sets print_help_list */
        argv[argc++] = "--version";            /* sets print_version */
        argv[argc++] = "-###";                 /* may set print_subprocess_help */
    } else if (test_num == 3) {
        /* Test with cross-compilation-like flags */
        argv[argc++] = "-march=x86-64";
        argv[argc++] = "-mtune=generic";
        argv[argc++] = "-isysroot";            /* alternative sysroot */
        argv[argc++] = "/usr/local/sysroot";
        argv[argc++] = "-save-temps=obj";
    }
    
    /* Common arguments */
    argv[argc++] = source_file;
    argv[argc++] = "-o";
    argv[argc++] = output_file;
    argv[argc++] = NULL;
    
    printf("Running GCC test %d with command:\n", test_num);
    for (int i = 0; i < argc; i++) {
        printf("%s ", argv[i] ? argv[i] : "(null)");
    }
    printf("\n\n");
    
    /* Fork and exec */
    pid_t pid = fork();
    if (pid == 0) {
        /* Child process */
        execv(argv[0], (char * const *)argv);
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

int main(int argc, char *argv[]) {
    const char *gcc_path = "./xgcc";  /* Path to the GCC driver under test */
    const char *source_file = "/tmp/test_cover.c";
    const char *output_file = "/tmp/test_output";
    
    /* Create test source file */
    create_test_source(source_file);
    
    /* Set environment variables that affect driver state */
    setenv("GCC_EXEC_PREFIX", "/usr/local/lib/gcc/", 1);
    setenv("COMPILER_PATH", "/usr/local/bin:/usr/bin", 1);
    
    /* Run multiple GCC invocations with different flag combinations */
    printf("=== Starting GCC driver cleanup coverage test ===\n\n");
    
    /* Test 1: Comprehensive flags to set most variables */
    printf("Test 1: Comprehensive flag test\n");
    printf("--------------------------------\n");
    int result1 = run_gcc_with_flags(gcc_path, source_file, 
                                    "/tmp/test_output1", 1);
    printf("Exit code: %d\n\n", result1);
    
    /* Test 2: Help and version flags */
    printf("Test 2: Help and version flags\n");
    printf("--------------------------------\n");
    int result2 = run_gcc_with_flags(gcc_path, source_file,
                                    "/tmp/test_output2", 2);
    printf("Exit code: %d\n\n", result2);
    
    /* Test 3: Cross-compilation-like flags */
    printf("Test 3: Cross-compilation flags\n");
    printf("--------------------------------\n");
    int result3 = run_gcc_with_flags(gcc_path, source_file,
                                    "/tmp/test_output3", 3);
    printf("Exit code: %d\n\n", result3);
    
    /* Cleanup temporary files */
    unlink(source_file);
    unlink("/tmp/test_output1");
    unlink("/tmp/test_output2");
    unlink("/tmp/test_output3");
    
    /* Also clean up any dump files created */
    system("rm -f /tmp/gcc_test_dump* /tmp/test_dumpbase* 2>/dev/null");
    
    printf("=== Test completed ===\n");
    printf("Each GCC invocation should have triggered the cleanup block\n");
    printf("resetting: is_cpp_driver, at_file_supplied, print_help_list,\n");
    printf("print_version, verbose_only_flag, print_subprocess_help,\n");
    printf("use_ld, report_times_to_file, target_system_root,\n");
    printf("target_system_root_changed, save_temps_flag, dumpdir,\n");
    printf("dumpbase, dumpbase_ext, outbase, spec_machine, etc.\n");
    
    return 0;
}
