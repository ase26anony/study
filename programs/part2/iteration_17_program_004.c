/* test_gcc_cleanup.c - Test program to cover driver cleanup logic in gcc.cc */
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

/* Run GCC with specific flags to set driver state variables */
int run_gcc_with_flags(const char *gcc_path, const char *source_file, 
                       const char *output_file, int test_num) {
    char *argv[64];
    int argc = 0;
    
    argv[argc++] = (char *)gcc_path;
    
    /* Set various flags to populate the global variables in gcc.cc */
    
    /* For save_temps_flag */
    argv[argc++] = "-save-temps";
    
    /* For dumpdir, dumpbase, dumpbase_ext allocation */
    argv[argc++] = "-dumpdir";
    argv[argc++] = "/tmp/gcc_test_dump";
    argv[argc++] = "-dumpbase";
    argv[argc++] = "test_dumpbase";
    argv[argc++] = "-dumpbase-ext";
    argv[argc++] = ".ext";
    
    /* For target_system_root and target_system_root_changed */
    argv[argc++] = "--sysroot=/opt/test_sysroot";
    
    /* For use_ld */
    argv[argc++] = "-fuse-ld=gold";
    
    /* For report_times_to_file */
    argv[argc++] = "-ftime-report";
    
    /* For verbose_only_flag */
    argv[argc++] = "-v";
    
    /* For print_help_list (test 1) or print_version (test 2) */
    if (test_num == 1) {
        argv[argc++] = "--help=common";
    } else if (test_num == 2) {
        argv[argc++] = "--version";
    }
    
    /* For print_subprocess_help */
    if (test_num == 3) {
        argv[argc++] = "-###";
    }
    
    /* Add optimization flag to affect spec processing */
    argv[argc++] = "-O2";
    
    /* Add machine-specific flags to affect spec_machine */
    argv[argc++] = "-march=x86-64";
    argv[argc++] = "-mtune=generic";
    
    /* Input and output files */
    argv[argc++] = (char *)source_file;
    argv[argc++] = "-o";
    argv[argc++] = (char *)output_file;
    
    argv[argc] = NULL;
    
    printf("Running GCC with %d arguments:\n", argc);
    for (int i = 0; i < argc; i++) {
        printf("  argv[%d] = %s\n", i, argv[i]);
    }
    
    pid_t pid = fork();
    if (pid == 0) {
        /* Child process */
        
        /* Set environment variables that affect driver state */
        setenv("GCC_EXEC_PREFIX", "/usr/lib/gcc/", 1);
        setenv("COMPILER_PATH", "/usr/bin:/usr/local/bin", 1);
        setenv("LIBRARY_PATH", "/usr/lib:/usr/local/lib", 1);
        
        execv(gcc_path, argv);
        
        /* If execv fails */
        perror("execv");
        exit(1);
    } else if (pid > 0) {
        /* Parent process */
        int status;
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status)) {
            printf("GCC exited with status %d\n", WEXITSTATUS(status));
            return WEXITSTATUS(status);
        } else {
            printf("GCC terminated abnormally\n");
            return -1;
        }
    } else {
        perror("fork");
        return -1;
    }
}

/* Test with different flag combinations to cover all reset lines */
void run_comprehensive_test(const char *gcc_path) {
    char source_file[] = "/tmp/test_gcc_cover_XXXXXX.c";
    char output_file[] = "/tmp/test_gcc_output_XXXXXX.o";
    
    /* Create unique temp filenames */
    int fd = mkstemps(source_file, 2);  /* .c extension is 2 chars */
    if (fd < 0) {
        perror("mkstemps");
        return;
    }
    close(fd);
    
    strcpy(output_file, source_file);
    strcpy(output_file + strlen(output_file) - 2, ".o");
    
    /* Create test source file */
    create_test_source(source_file);
    
    printf("\n=== Test 1: Compilation with help flag ===\n");
    run_gcc_with_flags(gcc_path, source_file, output_file, 1);
    
    printf("\n=== Test 2: Compilation with version flag ===\n");
    run_gcc_with_flags(gcc_path, source_file, output_file, 2);
    
    printf("\n=== Test 3: Compilation with subprocess help flag ===\n");
    run_gcc_with_flags(gcc_path, source_file, output_file, 3);
    
    printf("\n=== Test 4: Normal compilation with all state-setting flags ===\n");
    /* Run without early-exit flags to ensure full cleanup */
    char *argv[32];
    int argc = 0;
    
    argv[argc++] = (char *)gcc_path;
    argv[argc++] = "-save-temps=cwd";
    argv[argc++] = "-dumpdir";
    argv[argc++] = "/tmp/my_dumps";
    argv[argc++] = "-dumpbase";
    argv[argc++] = "mydump";
    argv[argc++] = "-dumpbase-ext";
    argv[argc++] = ".myext";
    argv[argc++] = "--sysroot=/";
    argv[argc++] = "-fuse-ld=bfd";
    argv[argc++] = "-ftime-report";
    argv[argc++] = "-v";
    argv[argc++] = "-march=native";
    argv[argc++] = "-mtune=native";
    argv[argc++] = (char *)source_file;
    argv[argc++] = "-o";
    argv[argc++] = (char *)output_file;
    argv[argc] = NULL;
    
    pid_t pid = fork();
    if (pid == 0) {
        /* Set environment to trigger more allocations */
        setenv("GCC_EXEC_PREFIX", "/test/prefix/", 1);
        setenv("GCC_COLORS", "always", 1);
        
        execv(gcc_path, argv);
        perror("execv");
        exit(1);
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
        printf("Test 4 completed with status %d\n", 
               WIFEXITED(status) ? WEXITSTATUS(status) : -1);
    }
    
    /* Clean up temporary files */
    unlink(source_file);
    unlink(output_file);
    
    /* Also clean up any save-temps files */
    char base_name[256];
    strcpy(base_name, source_file);
    char *dot = strrchr(base_name, '.');
    if (dot) *dot = '\0';
    
    char temp_files[][32] = {".i", ".s", ".o", ".ii"};
    for (int i = 0; i < 4; i++) {
        char fname[512];
        snprintf(fname, sizeof(fname), "%s%s", base_name, temp_files[i]);
        unlink(fname);
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <path-to-gcc-driver>\n", argv[0]);
        fprintf(stderr, "Example: %s ./gcc/xgcc\n", argv[0]);
        return 1;
    }
    
    const char *gcc_path = argv[1];
    
    /* Verify the GCC driver exists */
    if (access(gcc_path, X_OK) != 0) {
        fprintf(stderr, "Cannot execute GCC driver at: %s\n", gcc_path);
        return 1;
    }
    
    printf("Testing GCC driver cleanup coverage\n");
    printf("Driver path: %s\n", gcc_path);
    
    run_comprehensive_test(gcc_path);
    
    printf("\n=== All tests completed ===\n");
    
    return 0;
}
