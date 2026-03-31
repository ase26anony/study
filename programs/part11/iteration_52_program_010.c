/* Test program to cover driver initialization reset logic in gcc.cc lines 11228-11250 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

/* Function prototypes for the dummy functions we'll create */
int foo(void);
int bar(void);
int baz(void);  /* Will have syntax error */

/* Helper to create temporary files */
static char* create_temp_file(const char* content, const char* suffix) {
    char template[] = "/tmp/gcc_test_XXXXXX";
    int fd = mkstemp(template);
    if (fd == -1) {
        perror("mkstemp");
        return NULL;
    }
    
    if (content) {
        write(fd, content, strlen(content));
    }
    
    close(fd);
    
    /* Create a copy with the right suffix */
    char* result = malloc(strlen(template) + strlen(suffix) + 1);
    sprintf(result, "%s%s", template, suffix);
    rename(template, result);
    
    return result;
}

/* Helper to run a command and check status */
static int run_command(const char* cmd, int check_status) {
    printf("Running: %s\n", cmd);
    int status = system(cmd);
    if (check_status) {
        return WEXITSTATUS(status);
    }
    return 0;
}

/* Cleanup helper */
static void cleanup_file(const char* filename) {
    if (filename) {
        unlink(filename);
        free((void*)filename);
    }
}

int main(void) {
    int checksum = 0;
    
    /* Create temporary source files */
    char* temp1_c = create_temp_file("int foo(void) { return 0; }\n", ".c");
    char* temp2_c = create_temp_file("int bar(void) { return 1; }\n", ".c");
    char* error_c = create_temp_file("int baz(void) { return \n", ".c");  /* Syntax error */
    
    if (!temp1_c || !temp2_c || !error_c) {
        fprintf(stderr, "Failed to create temp files\n");
        return 1;
    }
    
    /* Create object file names */
    char temp1_o[256], temp2_o[256];
    snprintf(temp1_o, sizeof(temp1_o), "%s.o", temp1_c);
    snprintf(temp2_o, sizeof(temp2_o), "%s.o", temp2_c);
    
    /* Create dump directory */
    mkdir("./dump_test", 0755);
    
    printf("=== Testing GCC driver initialization reset logic ===\n\n");
    
    /* Sequence A: Help then Compile - tests print_help_list reset */
    printf("--- Sequence A: Help then Compile ---\n");
    char cmd_a1[512];
    snprintf(cmd_a1, sizeof(cmd_a1), "gcc --help=common > /dev/null 2>&1");
    checksum += run_command(cmd_a1, 1);
    
    char cmd_a2[512];
    snprintf(cmd_a2, sizeof(cmd_a2), "gcc -c %s -o %s", temp1_c, temp1_o);
    checksum += run_command(cmd_a2, 1) << 1;
    
    /* Sequence B: Save-Temps with Dumpdir then Compile - tests save_temps_flag and dumpdir reset */
    printf("\n--- Sequence B: Save-Temps with Dumpdir then Compile ---\n");
    char cmd_b1[512];
    snprintf(cmd_b1, sizeof(cmd_b1), 
             "gcc -save-temps -dumpdir ./dump_test -dumpbase testbase -c %s -o %s",
             temp1_c, temp1_o);
    checksum += run_command(cmd_b1, 1) << 2;
    
    char cmd_b2[512];
    snprintf(cmd_b2, sizeof(cmd_b2), "gcc -c %s -o %s", temp2_c, temp2_o);
    checksum += run_command(cmd_b2, 1) << 3;
    
    /* Sequence C: Error then Success - tests greatest_status reset */
    printf("\n--- Sequence C: Error then Success ---\n");
    char cmd_c1[512];
    snprintf(cmd_c1, sizeof(cmd_c1), "gcc -c %s -o /tmp/error.o 2>/dev/null", error_c);
    int status_c1 = run_command(cmd_c1, 0);  /* Don't check status - we expect failure */
    checksum += (status_c1 != 0) << 4;  /* Track that it failed */
    
    char cmd_c2[512];
    snprintf(cmd_c2, sizeof(cmd_c2), "gcc -c %s -o %s", temp1_c, temp1_o);
    checksum += run_command(cmd_c2, 1) << 5;
    
    /* Sequence D: Verbose and Linker then Plain - tests verbose_only_flag and use_ld reset */
    printf("\n--- Sequence D: Verbose and Linker then Plain ---\n");
    char cmd_d1[512];
    snprintf(cmd_d1, sizeof(cmd_d1), 
             "gcc -v -fuse-ld=bfd -c %s -o %s 2>&1 | grep -q 'COLLECT_GCC_OPTIONS'",
             temp1_c, temp1_o);
    checksum += run_command(cmd_d1, 1) << 6;
    
    char cmd_d2[512];
    snprintf(cmd_d2, sizeof(cmd_d2), "gcc -c %s -o %s", temp2_c, temp2_o);
    checksum += run_command(cmd_d2, 1) << 7;
    
    /* Additional test: Multiple inputs in single invocation */
    printf("\n--- Sequence E: Multiple inputs in single invocation ---\n");
    char cmd_e[512];
    snprintf(cmd_e, sizeof(cmd_e), 
             "gcc -save-temps -dumpdir ./dump_test2 -c %s %s -o /tmp/multi.o",
             temp1_c, temp2_c);
    checksum += run_command(cmd_e, 1) << 8;
    
    /* Test with -x option to force language specification */
    printf("\n--- Sequence F: Using -x option ---\n");
    char cmd_f1[512];
    snprintf(cmd_f1, sizeof(cmd_f1),
             "gcc -x c -save-temps -c %s -o %s",
             temp1_c, temp1_o);
    checksum += run_command(cmd_f1, 1) << 9;
    
    char cmd_f2[512];
    snprintf(cmd_f2, sizeof(cmd_f2),
             "gcc -c %s -o %s",  /* No -x, no save-temps */
             temp2_c, temp2_o);
    checksum += run_command(cmd_f2, 1) << 10;
    
    /* Cleanup */
    printf("\n--- Cleaning up ---\n");
    cleanup_file(temp1_c);
    cleanup_file(temp2_c);
    cleanup_file(error_c);
    unlink(temp1_o);
    unlink(temp2_o);
    
    /* Clean dump directories */
    system("rm -rf ./dump_test ./dump_test2");
    
    /* Clean up any .i, .s, .o files created by -save-temps */
    system("rm -f /tmp/gcc_test_*.i /tmp/gcc_test_*.s /tmp/gcc_test_*.o /tmp/error.o /tmp/multi.o");
    
    printf("\n=== Test completed ===\n");
    printf("Checksum: 0x%x\n", checksum);
    
    /* Reference the functions to avoid unused warnings */
    (void)foo;
    (void)bar;
    (void)baz;
    
    return 0;
}
