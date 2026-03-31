/* gcc_cleanup_test.c - Test program to cover driver cleanup code in gcc.cc */
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
        if (files[i]) unlink(files[i]);
    }
}

/* Execute GCC with specific arguments and environment */
static int run_gcc(const char *gcc_path, char *const argv[], char *const envp[]) {
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        return -1;
    }
    
    if (pid == 0) {
        /* Child process */
        execve(gcc_path, argv, envp);
        perror("execve");
        exit(1);
    } else {
        /* Parent process */
        int status;
        waitpid(pid, &status, 0);
        return WEXITSTATUS(status);
    }
}

int main(int argc, char *argv[]) {
    const char *gcc_path = "./gcc/xgcc";  /* Path to the GCC driver under test */
    const char *tmp_source = "/tmp/gcc_cover_test.c";
    const char *tmp_output = "/tmp/gcc_cover_test.o";
    const char *tmp_dumpdir = "/tmp/gcc_dumpdir";
    const char *files_to_clean[] = {tmp_source, tmp_output, NULL};
    
    /* Create test source file */
    create_test_source(tmp_source);
    
    /* Create dump directory */
    mkdir(tmp_dumpdir, 0755);
    
    printf("=== GCC Driver Cleanup Coverage Test ===\n\n");
    
    /* Test 1: Comprehensive compilation with many state-altering flags */
    printf("Test 1: Comprehensive compilation with state-altering flags\n");
    {
        /* Prepare environment variables */
        char *envp[] = {
            "GCC_EXEC_PREFIX=/usr/lib/gcc/",
            "COMPILER_PATH=/usr/bin:/usr/local/bin",
            "LIBRARY_PATH=/usr/lib:/usr/local/lib",
            "C_INCLUDE_PATH=/usr/include",
            "CPLUS_INCLUDE_PATH=/usr/include/c++",
            "PATH=/usr/bin:/bin",
            NULL
        };
        
        /* Build complex command line to set many global variables */
        char *gcc_argv[] = {
            (char *)gcc_path,
            "-save-temps",              /* Sets save_temps_flag */
            "-dumpdir", (char *)tmp_dumpdir, /* Allocates dumpdir */
            "-dumpbase", "test_dump",   /* Allocates dumpbase */
            "-dumpbase-ext", ".ext",    /* Allocates dumpbase_ext */
            "--sysroot=/opt/mysysroot", /* Sets target_system_root, target_system_root_changed */
            "-isysroot", "/opt/myisysroot", /* Also affects system roots */
            "-fuse-ld=gold",            /* Sets use_ld */
            "-ftime-report",            /* Sets report_times_to_file */
            "-v",                       /* Sets verbose_only_flag */
            "-mtune=generic",           /* Affects spec_machine */
            "-march=x86-64",            /* Affects spec_machine */
            "-specs=/dev/null",         /* Triggers spec processing */
            "-o", (char *)tmp_output,
            (char *)tmp_source,
            NULL
        };
        
        printf("Running: ");
        for (int i = 0; gcc_argv[i]; i++) {
            printf("%s ", gcc_argv[i]);
        }
        printf("\n");
        
        int result = run_gcc(gcc_path, gcc_argv, envp);
        printf("Exit code: %d\n\n", result);
    }
    
    /* Test 2: Help and version flags (different code paths) */
    printf("Test 2: Help and version flags\n");
    {
        char *envp[] = {"PATH=/usr/bin:/bin", NULL};
        
        /* First with --help */
        char *help_argv[] = {
            (char *)gcc_path,
            "--help=common",            /* Sets print_help_list */
            "--version",                /* Sets print_version */
            NULL
        };
        
        printf("Running with --help=common --version\n");
        int result = run_gcc(gcc_path, help_argv, envp);
        printf("Exit code: %d\n", result);
        
        /* Then with subprocess help */
        char *subproc_argv[] = {
            (char *)gcc_path,
            "-###",                     /* May set print_subprocess_help */
            (char *)tmp_source,
            NULL
        };
        
        printf("Running with -###\n");
        result = run_gcc(gcc_path, subproc_argv, envp);
        printf("Exit code: %d\n\n", result);
    }
    
    /* Test 3: Different target configurations */
    printf("Test 3: Cross-compilation-like configurations\n");
    {
        char *envp[] = {
            "GCC_EXEC_PREFIX=/cross/gcc/",
            "PATH=/usr/bin:/bin",
            NULL
        };
        
        /* Try with different machine specs */
        char *cross_argv[] = {
            (char *)gcc_path,
            "-dumpdir", "/tmp/other_dump",
            "-dumpbase", "cross_test",
            "-mtune=cortex-a53",        /* Different from default */
            "-march=armv8-a",           /* Different architecture */
            "--target=arm-linux-gnueabihf",
            "--sysroot=/cross/sysroot",
            "-o", "/tmp/cross_test.o",
            (char *)tmp_source,
            NULL
        };
        
        printf("Running cross-compilation-like command\n");
        int result = run_gcc(gcc_path, cross_argv, envp);
        printf("Exit code: %d\n\n", result);
    }
    
    /* Test 4: Error case to test cleanup after failure */
    printf("Test 4: Error case (cleanup should still happen)\n");
    {
        char *envp[] = {NULL};
        
        char *error_argv[] = {
            (char *)gcc_path,
            "-save-temps",
            "-dumpdir", "/tmp/error_dump",
            "-o", "/tmp/error_output",
            "/nonexistent/file.c",      /* This will fail */
            NULL
        };
        
        printf("Running with nonexistent source file\n");
        int result = run_gcc(gcc_path, error_argv, envp);
        printf("Exit code: %d\n\n", result);
    }
    
    /* Test 5: Multiple rapid invocations to stress cleanup */
    printf("Test 5: Multiple rapid invocations\n");
    {
        char *envp[] = {"PATH=/usr/bin:/bin", NULL};
        
        for (int i = 0; i < 3; i++) {
            char output[64];
            snprintf(output, sizeof(output), "/tmp/rapid%d.o", i);
            
            char *rapid_argv[] = {
                (char *)gcc_path,
                "-save-temps",
                "-dumpdir", "/tmp/rapid_dump",
                "-dumpbase", "rapid",
                "-v",
                "-o", output,
                (char *)tmp_source,
                NULL
            };
            
            printf("Rapid invocation %d\n", i + 1);
            int result = run_gcc(gcc_path, rapid_argv, envp);
            printf("  Exit code: %d\n", result);
            
            /* Clean up output file */
            unlink(output);
        }
        printf("\n");
    }
    
    /* Cleanup */
    printf("Cleaning up temporary files...\n");
    cleanup_files(files_to_clean, 2);
    rmdir(tmp_dumpdir);
    
    printf("\n=== Test completed ===\n");
    printf("The GCC driver should have executed the cleanup code block multiple times,\n");
    printf("resetting all the global variables in the uncovered lines.\n");
    
    return 0;
}
