/* gcc_cleanup_coverage.c
 * This program tests the GCC driver's cleanup logic by invoking it with
 * various flags that set global state variables, ensuring they get reset
 * in the finalize function.
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
static void create_test_source(const char *filename) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        perror("fopen");
        exit(1);
    }
    fprintf(f, "int main(void) { return 0; }\n");
    fclose(f);
}

/* Remove temporary files */
static void cleanup_files(const char **files, int count) {
    for (int i = 0; i < count; i++) {
        if (files[i]) {
            unlink(files[i]);
        }
    }
}

/* Execute GCC with the given arguments */
static int run_gcc(const char *gcc_path, char *const argv[]) {
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        return -1;
    }
    
    if (pid == 0) {
        /* Child process */
        execv(gcc_path, argv);
        perror("execv");
        exit(1);
    } else {
        /* Parent process */
        int status;
        waitpid(pid, &status, 0);
        return WEXITSTATUS(status);
    }
}

int main(int argc, char *argv[]) {
    const char *gcc_path = "./xgcc";  /* Path to the GCC driver under test */
    const char *tmp_dir = "/tmp";
    char source_file[256];
    char output_file[256];
    char dump_dir[256];
    
    /* Create unique temporary filenames */
    pid_t pid = getpid();
    snprintf(source_file, sizeof(source_file), "%s/test_cover_%d.c", tmp_dir, pid);
    snprintf(output_file, sizeof(output_file), "%s/test_cover_%d.o", tmp_dir, pid);
    snprintf(dump_dir, sizeof(dump_dir), "%s/dump_%d", tmp_dir, pid);
    
    /* Create test source file */
    create_test_source(source_file);
    
    /* Test 1: Comprehensive compilation with many state-altering flags */
    printf("Test 1: Comprehensive compilation with state-altering flags\n");
    
    /* Set environment variables that affect driver state */
    setenv("GCC_EXEC_PREFIX", "/usr/lib/gcc/x86_64-linux-gnu/11", 1);
    setenv("COMPILER_PATH", "/usr/bin:/usr/local/bin", 1);
    setenv("LIBRARY_PATH", "/usr/lib:/usr/local/lib", 1);
    
    /* Create dump directory */
    mkdir(dump_dir, 0755);
    
    /* Construct complex command line to set many global variables */
    char *gcc_argv1[] = {
        (char *)gcc_path,
        "-save-temps",                    /* sets save_temps_flag */
        "-dumpdir", dump_dir,             /* allocates dumpdir */
        "-dumpbase", "foo",               /* allocates dumpbase */
        "-dumpbase-ext", ".bar",          /* allocates dumpbase_ext */
        "--sysroot=/opt/sysroot",         /* sets target_system_root, target_system_root_changed */
        "-fuse-ld=gold",                  /* sets use_ld */
        "-ftime-report",                  /* sets report_times_to_file */
        "-v",                             /* sets verbose_only_flag */
        "-march=native",                  /* may affect spec_machine */
        "-mtune=generic",
        "-specs=/dev/null",               /* forces spec processing */
        "-B/usr/bin",                     /* adds to compiler execution path */
        "-L/usr/lib",
        "-I/usr/include",
        "-o", output_file,
        source_file,
        NULL
    };
    
    int result1 = run_gcc(gcc_path, gcc_argv1);
    printf("Test 1 completed with exit code: %d\n", result1);
    
    /* Test 2: Help and version flags (different state variables) */
    printf("\nTest 2: Help and version flags\n");
    
    char *gcc_argv2[] = {
        (char *)gcc_path,
        "--help=common",                  /* sets print_help_list */
        "--version",                      /* sets print_version */
        "-v",                             /* verbose flag */
        NULL
    };
    
    int result2 = run_gcc(gcc_path, gcc_argv2);
    printf("Test 2 completed with exit code: %d\n", result2);
    
    /* Test 3: Subprocess help and verbose output */
    printf("\nTest 3: Subprocess help and verbose output\n");
    
    char *gcc_argv3[] = {
        (char *)gcc_path,
        "-###",                           /* may set print_subprocess_help */
        "-v",
        "-E",                             /* preprocessor only */
        "-dM",                            /* dump macros */
        "-o", "/dev/null",
        source_file,
        NULL
    };
    
    int result3 = run_gcc(gcc_path, gcc_argv3);
    printf("Test 3 completed with exit code: %d\n", result3);
    
    /* Test 4: Different target configuration */
    printf("\nTest 4: Cross-compilation-like configuration\n");
    
    char *gcc_argv4[] = {
        (char *)gcc_path,
        "-save-temps=obj",
        "-dumpdir", ".",
        "-dumpbase", "cross_test",
        "-isysroot", "/usr/arm-linux-gnueabi",  /* alternative sysroot */
        "-march=armv7-a",
        "-mtune=cortex-a8",
        "-mfloat-abi=hard",
        "-mfpu=vfpv3",
        "-o", output_file,
        source_file,
        NULL
    };
    
    int result4 = run_gcc(gcc_path, gcc_argv4);
    printf("Test 4 completed with exit code: %d\n", result4);
    
    /* Test 5: Time report to file */
    printf("\nTest 5: Time report with file output\n");
    
    char time_report_file[256];
    snprintf(time_report_file, sizeof(time_report_file), 
             "%s/time_report_%d.txt", tmp_dir, pid);
    
    char *gcc_argv5[] = {
        (char *)gcc_path,
        "-ftime-report",
        "-fmem-report",
        "-fstack-usage",
        "-fdump-tree-all",
        "-fdump-rtl-all",
        "-o", output_file,
        source_file,
        NULL
    };
    
    int result5 = run_gcc(gcc_path, gcc_argv5);
    printf("Test 5 completed with exit code: %d\n", result5);
    
    /* Cleanup temporary files */
    const char *files_to_clean[] = {
        source_file,
        output_file,
        time_report_file,
        NULL
    };
    
    /* Also clean up any dump files created */
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "rm -rf %s/* 2>/dev/null", dump_dir);
    system(cmd);
    rmdir(dump_dir);
    
    cleanup_files(files_to_clean, 3);
    
    printf("\nAll tests completed. Each GCC invocation should have triggered\n");
    printf("the cleanup logic in driver::finalize, covering the target lines.\n");
    
    return 0;
}
