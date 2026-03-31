/* Test program to exercise gcc driver initialization logic (lines 11228-11250) */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

/* Temporary source files content */
static const char *valid_source1 = 
    "int foo(void) { return 0; }\n"
    "int unused_func1(void) __attribute__((unused));\n"
    "int unused_func1(void) { return 42; }\n";

static const char *valid_source2 = 
    "int bar(void) { return 1; }\n"
    "static int unused_func2(void) { return 99; }\n";

static const char *error_source = 
    "int baz(void) { return \n"  /* Deliberate syntax error - missing semicolon and value */
    "/* Unclosed comment to ensure error */\n";

/* Helper to create temporary files */
static int create_temp_file(const char *content, char *template) {
    int fd = mkstemp(template);
    if (fd < 0) {
        perror("mkstemp failed");
        return -1;
    }
    
    if (write(fd, content, strlen(content)) != (ssize_t)strlen(content)) {
        perror("write failed");
        close(fd);
        return -1;
    }
    
    close(fd);
    return 0;
}

/* Helper to run command and check status */
static int run_command(const char *cmd, int expect_failure) {
    printf("Running: %s\n", cmd);
    int status = system(cmd);
    int result = WEXITSTATUS(status);
    
    if (expect_failure) {
        if (result == 0) {
            printf("  WARNING: Expected failure but command succeeded\n");
            return 0;  /* Still count as covered */
        }
        return 1;
    } else {
        return (result == 0) ? 1 : 0;
    }
}

int main(void) {
    char temp1[] = "/tmp/gcc_test1_XXXXXX.c";
    char temp2[] = "/tmp/gcc_test2_XXXXXX.c";
    char temp_err[] = "/tmp/gcc_test_err_XXXXXX.c";
    char obj1[] = "/tmp/gcc_test1_XXXXXX.o";
    char obj2[] = "/tmp/gcc_test2_XXXXXX.o";
    
    int checksum = 0;
    
    /* Create temporary source files */
    if (create_temp_file(valid_source1, temp1) < 0 ||
        create_temp_file(valid_source2, temp2) < 0 ||
        create_temp_file(error_source, temp_err) < 0) {
        return EXIT_FAILURE;
    }
    
    /* Create temporary object file names */
    int fd1 = mkstemps(obj1, 2);  /* .o extension */
    if (fd1 >= 0) close(fd1);
    
    int fd2 = mkstemps(obj2, 2);
    if (fd2 >= 0) close(fd2);
    
    /* Create dump directory */
    mkdir("./dump_test", 0755);
    
    /* Sequence A: Help then Compile - exercises print_help_list reset */
    printf("\n=== Sequence A: Help then Compile ===\n");
    char cmd1[512];
    snprintf(cmd1, sizeof(cmd1), 
             "gcc --help=common > /dev/null 2>&1 && "
             "gcc -c %s -o %s", temp1, obj1);
    checksum += run_command(cmd1, 0) << 0;
    
    /* Sequence B: Save-Temps with Dumpdir then Compile - exercises save_temps_flag, dumpdir reset */
    printf("\n=== Sequence B: Save-Temps with Dumpdir ===\n");
    char cmd2[512];
    snprintf(cmd2, sizeof(cmd2),
             "gcc -save-temps -dumpdir ./dump_test -dumpbase testbase -c %s -o %s 2>/dev/null && "
             "gcc -c %s -o %s", temp1, obj1, temp2, obj2);
    checksum += run_command(cmd2, 0) << 1;
    
    /* Sequence C: Error then Success - exercises greatest_status reset */
    printf("\n=== Sequence C: Error then Success ===\n");
    char cmd3[512];
    snprintf(cmd3, sizeof(cmd3),
             "gcc -c %s -o /tmp/bad.o 2>/dev/null; "
             "gcc -c %s -o %s", temp_err, temp1, obj1);
    checksum += run_command(cmd3, 0) << 2;
    
    /* Sequence D: Verbose and Linker then Plain - exercises verbose_only_flag, use_ld reset */
    printf("\n=== Sequence D: Verbose and Linker ===\n");
    char cmd4[512];
    snprintf(cmd4, sizeof(cmd4),
             "gcc -v -fuse-ld=bfd -c %s 2>&1 | grep -q 'COLLECT_GCC_OPTIONS' && "
             "gcc -c %s -o %s", temp1, temp2, obj2);
    checksum += run_command(cmd4, 0) << 3;
    
    /* Sequence E: Version then Compile - exercises print_version reset */
    printf("\n=== Sequence E: Version then Compile ===\n");
    char cmd5[512];
    snprintf(cmd5, sizeof(cmd5),
             "gcc --version > /dev/null 2>&1 && "
             "gcc -c %s -o %s", temp1, obj1);
    checksum += run_command(cmd5, 0) << 4;
    
    /* Sequence F: Multiple jobs in single invocation - exercises per-job reset */
    printf("\n=== Sequence F: Multiple Input Files ===\n");
    char cmd6[512];
    snprintf(cmd6, sizeof(cmd6),
             "gcc -c %s %s -save-temps -dumpdir ./dump_test2", temp1, temp2);
    checksum += run_command(cmd6, 0) << 5;
    
    /* Sequence G: Using -x to chain language changes */
    printf("\n=== Sequence G: Using -x Language Specification ===\n");
    char cmd7[512];
    snprintf(cmd7, sizeof(cmd7),
             "echo 'int x=1;' | gcc -x c - -c -o /tmp/stdin1.o 2>/dev/null && "
             "echo 'int y=2;' | gcc -x c - -c -o /tmp/stdin2.o");
    checksum += run_command(cmd7, 0) << 6;
    
    /* Cleanup */
    printf("\n=== Cleaning up ===\n");
    unlink(temp1);
    unlink(temp2);
    unlink(temp_err);
    unlink(obj1);
    unlink(obj2);
    unlink("/tmp/stdin1.o");
    unlink("/tmp/stdin2.o");
    
    /* Clean dump directories */
    system("rm -rf ./dump_test ./dump_test2 2>/dev/null");
    
    /* Also clean any .i, .s, .o files that might have been created */
    system("rm -f /tmp/*.i /tmp/*.s /tmp/*.o 2>/dev/null");
    
    printf("\n=== Final Checksum: 0x%02x ===\n", checksum);
    
    /* Reference functions to avoid unused warnings */
    (void)foo;  /* Will be defined if we compiled successfully */
    (void)bar;
    
    return (checksum > 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

/* Dummy functions to satisfy potential linker references */
int foo(void) { return 0; }
int bar(void) { return 1; }
