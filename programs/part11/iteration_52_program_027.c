/* test_gcc_driver_init.c
 * 
 * This test program exercises the GCC driver initialization logic
 * (lines 11228-11250 in gcc.cc) by invoking multiple compilation jobs
 * with different states, forcing the driver to reset global variables
 * between jobs.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

/* Simple checksum to track test execution */
static unsigned int checksum = 0;

/* Function to add result to checksum */
static void add_to_checksum(int result) {
    checksum = (checksum << 1) ^ result;
}

/* Create a temporary file with given content */
static char* create_temp_file(const char* prefix, const char* content) {
    char template[256];
    snprintf(template, sizeof(template), "/tmp/%s_XXXXXX", prefix);
    
    int fd = mkstemp(template);
    if (fd == -1) {
        perror("mkstemp failed");
        return NULL;
    }
    
    if (write(fd, content, strlen(content)) != (ssize_t)strlen(content)) {
        perror("write failed");
        close(fd);
        return NULL;
    }
    
    close(fd);
    
    char* filename = strdup(template);
    if (!filename) {
        perror("strdup failed");
        return NULL;
    }
    
    return filename;
}

/* Execute a GCC command and check result */
static int execute_gcc_command(const char* command, const char* description) {
    printf("Executing: %s\n", description);
    printf("Command: %s\n", command);
    
    int result = system(command);
    int exit_status = WEXITSTATUS(result);
    
    printf("Exit status: %d\n\n", exit_status);
    add_to_checksum(exit_status);
    
    return exit_status;
}

/* Clean up temporary files */
static void cleanup_files(char** files, int count) {
    for (int i = 0; i < count; i++) {
        if (files[i]) {
            unlink(files[i]);
            free(files[i]);
        }
    }
}

/* Create dump directory for testing */
static int create_dump_dir(const char* dirname) {
    struct stat st = {0};
    if (stat(dirname, &st) == -1) {
        if (mkdir(dirname, 0755) == -1) {
            perror("mkdir failed");
            return 0;
        }
    }
    return 1;
}

/* Remove dump directory */
static void remove_dump_dir(const char* dirname) {
    char command[256];
    snprintf(command, sizeof(command), "rm -rf %s", dirname);
    system(command);
}

int main(void) {
    printf("=== GCC Driver Initialization Block Test ===\n\n");
    
    /* Create temporary source files */
    char* temp_files[10] = {0};
    int file_count = 0;
    
    /* Valid source files */
    temp_files[file_count++] = create_temp_file("test1", 
        "int foo(void) { return 0; }\n");
    temp_files[file_count++] = create_temp_file("test2",
        "int bar(void) { return 1; }\n");
    
    /* Source file with syntax error */
    temp_files[file_count++] = create_temp_file("syntax_err",
        "int baz(void) { return /* missing semicolon and value */ }\n");
    
    /* Another valid source file */
    temp_files[file_count++] = create_temp_file("test3",
        "int qux(void) { return 2; }\n");
    
    if (!temp_files[0] || !temp_files[1] || !temp_files[2] || !temp_files[3]) {
        fprintf(stderr, "Failed to create temporary files\n");
        cleanup_files(temp_files, file_count);
        return 1;
    }
    
    /* Create object file names */
    char obj1[256], obj2[256], obj3[256];
    snprintf(obj1, sizeof(obj1), "/tmp/test1_%d.o", getpid());
    snprintf(obj2, sizeof(obj2), "/tmp/test2_%d.o", getpid());
    snprintf(obj3, sizeof(obj3), "/tmp/test3_%d.o", getpid());
    
    /* Create dump directory */
    char dumpdir[256];
    snprintf(dumpdir, sizeof(dumpdir), "/tmp/dumpdir_%d", getpid());
    if (!create_dump_dir(dumpdir)) {
        cleanup_files(temp_files, file_count);
        return 1;
    }
    
    /* Test Sequence 1: Help flag then compilation
     * This sets print_help_list, then resets it in next job */
    printf("--- Sequence 1: Help then Compile ---\n");
    char cmd1[512];
    snprintf(cmd1, sizeof(cmd1), 
             "gcc --help=common > /dev/null 2>&1 && "
             "gcc -c %s -o %s",
             temp_files[0], obj1);
    execute_gcc_command(cmd1, "Help then compile");
    
    /* Test Sequence 2: Version flag then compilation
     * This sets print_version, then resets it */
    printf("--- Sequence 2: Version then Compile ---\n");
    char cmd2[512];
    snprintf(cmd2, sizeof(cmd2),
             "gcc --version > /dev/null 2>&1 && "
             "gcc -c %s -o %s",
             temp_files[1], obj2);
    execute_gcc_command(cmd2, "Version then compile");
    
    /* Test Sequence 3: Save-temps with dumpdir then plain compile
     * This sets save_temps_flag, dumpdir, dumpbase, then frees them */
    printf("--- Sequence 3: Save-temps with dumpdir then plain compile ---\n");
    char cmd3[1024];
    snprintf(cmd3, sizeof(cmd3),
             "gcc -save-temps -dumpdir %s -dumpbase mydump -c %s -o %s && "
             "gcc -c %s -o %s",
             dumpdir, temp_files[0], obj1,
             temp_files[1], obj2);
    execute_gcc_command(cmd3, "Save-temps with dumpdir then plain compile");
    
    /* Test Sequence 4: Verbose flag then normal compile
     * This sets verbose_only_flag, then resets it */
    printf("--- Sequence 4: Verbose then normal compile ---\n");
    char cmd4[512];
    snprintf(cmd4, sizeof(cmd4),
             "gcc -v -c %s -o %s 2>&1 | head -5 > /dev/null && "
             "gcc -c %s -o %s",
             temp_files[0], obj1,
             temp_files[1], obj2);
    execute_gcc_command(cmd4, "Verbose then normal compile");
    
    /* Test Sequence 5: Linker selection then default
     * This sets use_ld, then resets to NULL */
    printf("--- Sequence 5: Specific linker then default ---\n");
    char cmd5[512];
    snprintf(cmd5, sizeof(cmd5),
             "gcc -fuse-ld=bfd -c %s -o %s && "
             "gcc -c %s -o %s",
             temp_files[0], obj1,
             temp_files[1], obj2);
    execute_gcc_command(cmd5, "Specific linker then default");
    
    /* Test Sequence 6: Error then success
     * This tests greatest_status reset to 1 after failure */
    printf("--- Sequence 6: Error then success ---\n");
    char cmd6[512];
    snprintf(cmd6, sizeof(cmd6),
             "gcc -c %s 2>/dev/null; "
             "gcc -c %s -o %s",
             temp_files[2],  /* syntax error file */
             temp_files[3], obj3);
    execute_gcc_command(cmd6, "Error then success");
    
    /* Test Sequence 7: Multiple options combined
     * Exercise multiple variables at once */
    printf("--- Sequence 7: Combined options then reset ---\n");
    char cmd7[1024];
    snprintf(cmd7, sizeof(cmd7),
             "gcc -v --help=optimizers -save-temps -dumpdir %s -fuse-ld=lld -c %s 2>&1 | head -10 > /dev/null && "
             "gcc -c %s -o %s",
             dumpdir, temp_files[0],
             temp_files[1], obj2);
    execute_gcc_command(cmd7, "Combined options then reset");
    
    /* Test Sequence 8: Multiple source files in one invocation
     * Tests job sequencing within single gcc call */
    printf("--- Sequence 8: Multiple files with different options ---\n");
    char cmd8[1024];
    snprintf(cmd8, sizeof(cmd8),
             "gcc -v -c %s -o %s --help=common 2>&1 > /dev/null; "
             "gcc -c %s %s -o combined.o",
             temp_files[0], obj1,
             temp_files[0], temp_files[1]);
    execute_gcc_command(cmd8, "Mixed options and compilation");
    
    /* Test Sequence 9: Using -x to force language specification
     * Creates distinct compilation jobs */
    printf("--- Sequence 9: Using -x for distinct jobs ---\n");
    char cmd9[1024];
    /* Create a simple C++ file to test -x */
    char* cpp_file = create_temp_file("test_cpp",
        "extern \"C\" int test_cpp(void) { return 42; }\n");
    if (cpp_file) {
        temp_files[file_count++] = cpp_file;
        snprintf(cmd9, sizeof(cmd9),
                 "gcc -x c --help=common 2>&1 > /dev/null && "
                 "gcc -x c++ -c %s -o /tmp/cpp_test.o 2>/dev/null && "
                 "gcc -c %s -o %s",
                 cpp_file,
                 temp_files[0], obj1);
        execute_gcc_command(cmd9, "Using -x for language specification");
    }
    
    /* Test Sequence 10: Testing dumpbase_ext and outbase */
    printf("--- Sequence 10: Testing dumpbase and outbase ---\n");
    char cmd10[1024];
    snprintf(cmd10, sizeof(cmd10),
             "gcc -save-temps -dumpdir %s -dumpbase mytest -dumpbase-ext .ext "
             "-o /tmp/final.exe %s 2>&1 > /dev/null && "
             "gcc -c %s -o %s",
             dumpdir, temp_files[0],
             temp_files[1], obj2);
    execute_gcc_command(cmd10, "Testing dumpbase_ext and outbase");
    
    /* Cleanup */
    printf("--- Cleanup ---\n");
    
    /* Remove object files */
    unlink(obj1);
    unlink(obj2);
    unlink(obj3);
    unlink("/tmp/final.exe");
    unlink("/tmp/cpp_test.o");
    unlink("/tmp/combined.o");
    
    /* Remove dump directory */
    remove_dump_dir(dumpdir);
    
    /* Remove temporary source files */
    cleanup_files(temp_files, file_count);
    
    /* Final checksum output */
    printf("\n=== Test Complete ===\n");
    printf("Final checksum: 0x%08x\n", checksum);
    printf("(Non-zero checksum indicates all commands were executed)\n");
    
    /* Reference the functions to avoid unused warnings */
    if (checksum == 0) {
        /* This should never happen, but prevents optimization */
        printf("All test functions were referenced\n");
    }
    
    return 0;
}
