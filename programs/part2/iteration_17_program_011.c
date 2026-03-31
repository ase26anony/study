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

/* Execute GCC with specific flags to set driver state */
int run_gcc_with_flags(const char *gcc_path, const char *source_file, 
                       const char *output_file, int test_num) {
    char *argv[64];
    int argc = 0;
    
    argv[argc++] = (char *)gcc_path;
    
    /* Set various flags to populate driver state variables */
    switch (test_num) {
        case 1:
            /* Comprehensive test with many flags set */
            argv[argc++] = "-save-temps";
            argv[argc++] = "-dumpdir";
            argv[argc++] = "/tmp/gcc_dump";
            argv[argc++] = "-dumpbase";
            argv[argc++] = "test_dump";
            argv[argc++] = "-dumpbase-ext";
            argv[argc++] = ".ext";
            argv[argc++] = "--sysroot=/opt/custom_sysroot";
            argv[argc++] = "-fuse-ld=gold";
            argv[argc++] = "-ftime-report";
            argv[argc++] = "-v";
            argv[argc++] = "-mtune=generic";
            argv[argc++] = "-march=x86-64";
            break;
            
        case 2:
            /* Test help and version flags */
            argv[argc++] = "--help=common";
            argv[argc++] = "--version";
            argv[argc++] = "-v";
            break;
            
        case 3:
            /* Test with different dump options */
            argv[argc++] = "-save-temps=cwd";
            argv[argc++] = "-dumpdir";
            argv[argc++] = ".";
            argv[argc++] = "-dumpbase";
            argv[argc++] = "another_dump";
            argv[argc++] = "-isysroot";
            argv[argc++] = "/usr/include";
            argv[argc++] = "-###";
            break;
            
        default:
            fprintf(stderr, "Unknown test number: %d\n", test_num);
            return -1;
    }
    
    /* Add source and output files for compilation tests */
    if (test_num == 1 || test_num == 3) {
        argv[argc++] = "-o";
        argv[argc++] = (char *)output_file;
        argv[argc++] = (char *)source_file;
    }
    
    argv[argc] = NULL;
    
    /* Print the command for debugging */
    printf("Running GCC with: ");
    for (int i = 0; i < argc; i++) {
        printf("%s ", argv[i]);
    }
    printf("\n");
    
    /* Fork and execute */
    pid_t pid = fork();
    if (pid == 0) {
        /* Child process */
        execv(argv[0], argv);
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
    const char *temp_source = "/tmp/test_cover.c";
    const char *temp_output = "/tmp/test_cover.o";
    
    /* Create test source file */
    create_test_source(temp_source);
    
    /* Set environment variables that affect driver state */
    setenv("GCC_EXEC_PREFIX", "/usr/lib/gcc/", 1);
    setenv("COMPILER_PATH", "/usr/bin:/usr/local/bin", 1);
    setenv("LIBRARY_PATH", "/usr/lib:/usr/local/lib", 1);
    
    printf("=== Test 1: Comprehensive compilation with many flags ===\n");
    int result1 = run_gcc_with_flags(gcc_path, temp_source, temp_output, 1);
    printf("Test 1 exited with status: %d\n\n", result1);
    
    /* Small delay to ensure cleanup completes */
    sleep(1);
    
    printf("=== Test 2: Help and version flags ===\n");
    int result2 = run_gcc_with_flags(gcc_path, temp_source, temp_output, 2);
    printf("Test 2 exited with status: %d\n\n", result2);
    
    sleep(1);
    
    printf("=== Test 3: Different dump options and dry run ===\n");
    int result3 = run_gcc_with_flags(gcc_path, temp_source, temp_output, 3);
    printf("Test 3 exited with status: %d\n\n", result3);
    
    /* Cleanup temporary files */
    unlink(temp_source);
    unlink(temp_output);
    unlink("/tmp/test_cover.i");   /* Created by -save-temps */
    unlink("/tmp/test_cover.s");   /* Created by -save-temps */
    
    /* Also clean up dump directory files if they exist */
    system("rm -f /tmp/gcc_dump* ./test_dump* ./another_dump*");
    
    printf("All tests completed. Driver cleanup should have been triggered.\n");
    
    return 0;
}
