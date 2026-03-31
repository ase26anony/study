/* test_gcc_driver_init.c - Test program to cover GCC driver initialization block */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

/* Helper function to create a temporary file with given content */
static char* create_temp_file(const char* content, const char* suffix) {
    char template[] = "/tmp/gcc_test_XXXXXX";
    int fd = mkstemp(template);
    if (fd == -1) {
        perror("mkstemp failed");
        return NULL;
    }
    
    if (suffix) {
        char newpath[256];
        snprintf(newpath, sizeof(newpath), "%s%s", template, suffix);
        rename(template, newpath);
        strcpy(template, newpath);
    }
    
    FILE* f = fdopen(fd, "w");
    if (!f) {
        perror("fdopen failed");
        close(fd);
        return NULL;
    }
    
    fputs(content, f);
    fclose(f);
    
    return strdup(template);
}

/* Helper function to delete temporary files */
static void cleanup_temp_file(const char* path) {
    if (path) {
        unlink(path);
        free((void*)path);
    }
}

/* Helper function to run a command and check status */
static int run_command(const char* cmd) {
    printf("Running: %s\n", cmd);
    int status = system(cmd);
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    return -1;
}

/* Unused functions to avoid compiler warnings */
static int foo(void) { return 0; }
static int bar(void) { return 1; }
static int baz(void) { return 2; }

int main(void) {
    int checksum = 0;
    int seq_result;
    
    printf("=== Testing GCC Driver Initialization Block Coverage ===\n");
    
    /* Create temporary source files */
    char* temp1 = create_temp_file("int foo(void) { return 0; }\n", ".c");
    char* temp2 = create_temp_file("int bar(void) { return 1; }\n", ".c");
    char* temp_err = create_temp_file("int baz(void) { return \n", ".c");
    
    if (!temp1 || !temp2 || !temp_err) {
        fprintf(stderr, "Failed to create temporary files\n");
        return 1;
    }
    
    /* Create temporary object files */
    char obj1[256], obj2[256];
    snprintf(obj1, sizeof(obj1), "%s.o", temp1);
    snprintf(obj2, sizeof(obj2), "%s.o", temp2);
    
    /* Create dump directory */
    mkdir("./dump1", 0755);
    
    /* Sequence A: Help then Compile - tests print_help_list reset */
    printf("\n--- Sequence A: Help then Compile ---\n");
    char cmd_a1[512];
    snprintf(cmd_a1, sizeof(cmd_a1), "gcc --help=common > /dev/null 2>&1");
    seq_result = run_command(cmd_a1);
    checksum += seq_result;
    
    char cmd_a2[512];
    snprintf(cmd_a2, sizeof(cmd_a2), "gcc -c %s -o %s", temp1, obj1);
    seq_result = run_command(cmd_a2);
    checksum += seq_result;
    
    /* Sequence B: Save-Temps with Dumpdir then Compile - tests save_temps_flag, dumpdir reset */
    printf("\n--- Sequence B: Save-Temps with Dumpdir then Compile ---\n");
    char cmd_b1[512];
    snprintf(cmd_b1, sizeof(cmd_b1), 
             "gcc -save-temps -dumpdir ./dump1 -dumpbase testbase -c %s -o %s 2>/dev/null", 
             temp1, obj1);
    seq_result = run_command(cmd_b1);
    checksum += seq_result;
    
    char cmd_b2[512];
    snprintf(cmd_b2, sizeof(cmd_b2), "gcc -c %s -o %s", temp2, obj2);
    seq_result = run_command(cmd_b2);
    checksum += seq_result;
    
    /* Sequence C: Version then Compile - tests print_version reset */
    printf("\n--- Sequence C: Version then Compile ---\n");
    char cmd_c1[512];
    snprintf(cmd_c1, sizeof(cmd_c1), "gcc --version > /dev/null 2>&1");
    seq_result = run_command(cmd_c1);
    checksum += seq_result;
    
    char cmd_c2[512];
    snprintf(cmd_c2, sizeof(cmd_c2), "gcc -c %s -o %s", temp2, obj2);
    seq_result = run_command(cmd_c2);
    checksum += seq_result;
    
    /* Sequence D: Error then Success - tests greatest_status reset */
    printf("\n--- Sequence D: Error then Success ---\n");
    char cmd_d1[512];
    snprintf(cmd_d1, sizeof(cmd_d1), "gcc -c %s 2>/dev/null", temp_err);
    seq_result = run_command(cmd_d1);
    checksum += seq_result;
    
    char cmd_d2[512];
    snprintf(cmd_d2, sizeof(cmd_d2), "gcc -c %s -o %s", temp1, obj1);
    seq_result = run_command(cmd_d2);
    checksum += seq_result;
    
    /* Sequence E: Verbose and Linker then Plain - tests verbose_only_flag, use_ld reset */
    printf("\n--- Sequence E: Verbose and Linker then Plain ---\n");
    char cmd_e1[512];
    snprintf(cmd_e1, sizeof(cmd_e1), 
             "gcc -v -fuse-ld=bfd -c %s -o %s 2>&1 | grep -q 'COLLECT_GCC_OPTIONS'", 
             temp1, obj1);
    seq_result = run_command(cmd_e1);
    checksum += seq_result;
    
    char cmd_e2[512];
    snprintf(cmd_e2, sizeof(cmd_e2), "gcc -c %s -o %s", temp2, obj2);
    seq_result = run_command(cmd_e2);
    checksum += seq_result;
    
    /* Sequence F: Multiple inputs in single invocation - tests per-job initialization */
    printf("\n--- Sequence F: Multiple Inputs Single Invocation ---\n");
    char cmd_f[512];
    snprintf(cmd_f, sizeof(cmd_f), 
             "gcc -c %s %s -save-temps -dumpdir ./dump2 -dumpbase multi 2>/dev/null", 
             temp1, temp2);
    seq_result = run_command(cmd_f);
    checksum += seq_result;
    
    /* Sequence G: Using -x to chain language specifications */
    printf("\n--- Sequence G: Chained -x Specifications ---\n");
    char cmd_g[512];
    snprintf(cmd_g, sizeof(cmd_g),
             "gcc -x c %s -x c %s -c -o combined.o 2>/dev/null",
             temp1, temp2);
    seq_result = run_command(cmd_g);
    checksum += seq_result;
    
    /* Sequence H: Help with specific categories then compile */
    printf("\n--- Sequence H: Multiple Help Categories then Compile ---\n");
    char cmd_h1[512];
    snprintf(cmd_h1, sizeof(cmd_h1), "gcc --help=optimizers > /dev/null 2>&1");
    seq_result = run_command(cmd_h1);
    checksum += seq_result;
    
    char cmd_h2[512];
    snprintf(cmd_h2, sizeof(cmd_h2), "gcc --help=warnings > /dev/null 2>&1");
    seq_result = run_command(cmd_h2);
    checksum += seq_result;
    
    char cmd_h3[512];
    snprintf(cmd_h3, sizeof(cmd_h3), "gcc -c %s -o %s", temp1, obj1);
    seq_result = run_command(cmd_h3);
    checksum += seq_result;
    
    /* Cleanup */
    printf("\n--- Cleaning up ---\n");
    cleanup_temp_file(temp1);
    cleanup_temp_file(temp2);
    cleanup_temp_file(temp_err);
    unlink(obj1);
    unlink(obj2);
    
    /* Clean up dump directories */
    system("rm -rf ./dump1 ./dump2 2>/dev/null");
    
    /* Clean up save-temps files */
    system("rm -f testbase.* *.i *.s *.o 2>/dev/null");
    
    /* Final checksum and result */
    printf("\n=== Test Complete ===\n");
    printf("Checksum (sum of exit codes): %d\n", checksum);
    printf("Note: Non-zero checksum is expected due to deliberate error cases.\n");
    
    /* Reference unused functions to avoid warnings */
    (void)foo();
    (void)bar();
    (void)baz();
    
    return 0;
}
