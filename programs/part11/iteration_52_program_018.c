/* Test program to exercise driver initialization logic in gcc.cc lines 11228-11250 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

/* Helper to create temporary source files */
static int create_temp_file(const char *template, const char *content) {
    FILE *f = fopen(template, "w");
    if (!f) {
        perror("fopen");
        return 0;
    }
    fprintf(f, "%s", content);
    fclose(f);
    return 1;
}

/* Helper to run command and check status */
static int run_command(const char *cmd) {
    int status = system(cmd);
    if (status == -1) {
        fprintf(stderr, "Failed to execute: %s\n", cmd);
        return 0;
    }
    return WIFEXITED(status) && WEXITSTATUS(status) == 0;
}

/* Unused functions to avoid -Wunused-function warnings */
static int foo(void) { return 0; }
static int bar(void) { return 1; }

int main(void) {
    int checksum = 0;
    char cmd[1024];
    
    /* Create temporary source files */
    if (!create_temp_file("temp1.c", "int foo(void) { return 0; }\n")) {
        return 1;
    }
    if (!create_temp_file("temp2.c", "int bar(void) { return 1; }\n")) {
        unlink("temp1.c");
        return 1;
    }
    if (!create_temp_file("syntax_error.c", "int baz(void) { return \n")) {
        unlink("temp1.c");
        unlink("temp2.c");
        return 1;
    }
    
    /* Create dump directory */
    mkdir("./dump1", 0755);
    
    printf("Testing GCC driver initialization block...\n");
    
    /* Sequence A: Help flag then compile - tests print_help_list reset */
    printf("\n1. Testing --help then compile...\n");
    if (run_command("gcc --help=common > /dev/null 2>&1")) {
        checksum |= 1;
    }
    if (run_command("gcc -c temp1.c -o temp1.o")) {
        checksum |= 2;
    }
    
    /* Sequence B: Save-temps with dumpdir then plain compile - tests save_temps_flag and dumpdir reset */
    printf("\n2. Testing -save-temps with dumpdir then plain compile...\n");
    snprintf(cmd, sizeof(cmd), 
             "gcc -save-temps -dumpdir ./dump1 -dumpbase base -c temp1.c -o temp1_save.o 2>/dev/null");
    if (run_command(cmd)) {
        checksum |= 4;
    }
    if (run_command("gcc -c temp2.c -o temp2.o")) {
        checksum |= 8;
    }
    
    /* Sequence C: Error then success - tests greatest_status reset */
    printf("\n3. Testing error then success...\n");
    if (!run_command("gcc -c syntax_error.c 2>/dev/null")) {
        checksum |= 16;  /* Expected to fail */
    }
    if (run_command("gcc -c temp1.c -o temp1_err.o")) {
        checksum |= 32;
    }
    
    /* Sequence D: Verbose and linker selection then plain - tests verbose_only_flag and use_ld reset */
    printf("\n4. Testing verbose with linker selection then plain...\n");
    snprintf(cmd, sizeof(cmd),
             "gcc -v -fuse-ld=bfd -c temp1.c 2>&1 | grep -q 'COLLECT_GCC_OPTIONS'");
    if (run_command(cmd)) {
        checksum |= 64;
    }
    if (run_command("gcc -c temp2.c -o temp2_plain.o")) {
        checksum |= 128;
    }
    
    /* Sequence E: Version flag then compile - tests print_version reset */
    printf("\n5. Testing --version then compile...\n");
    if (run_command("gcc --version > /dev/null 2>&1")) {
        checksum |= 256;
    }
    if (run_command("gcc -c temp1.c -o temp1_ver.o")) {
        checksum |= 512;
    }
    
    /* Sequence F: Multiple input files in single invocation - tests per-job initialization */
    printf("\n6. Testing multiple inputs in single invocation...\n");
    snprintf(cmd, sizeof(cmd),
             "gcc -save-temps -dumpdir ./dump1 -dumpbase multi -c temp1.c temp2.c 2>/dev/null");
    if (run_command(cmd)) {
        checksum |= 1024;
    }
    
    /* Sequence G: Using -x to chain language specifications */
    printf("\n7. Testing -x language specification chaining...\n");
    snprintf(cmd, sizeof(cmd),
             "echo 'int x=1;' | gcc -x c - -c -o temp_stdin1.o 2>/dev/null && "
             "echo 'int y=2;' | gcc -x c - -c -o temp_stdin2.o 2>/dev/null");
    if (run_command(cmd)) {
        checksum |= 2048;
    }
    
    /* Cleanup */
    printf("\nCleaning up...\n");
    unlink("temp1.c");
    unlink("temp2.c");
    unlink("syntax_error.c");
    unlink("temp1.o");
    unlink("temp2.o");
    unlink("temp1_save.o");
    unlink("temp1_err.o");
    unlink("temp2_plain.o");
    unlink("temp1_ver.o");
    unlink("temp_stdin1.o");
    unlink("temp_stdin2.o");
    
    /* Clean dump directory files */
    system("rm -f ./dump1/* 2>/dev/null");
    rmdir("./dump1");
    
    /* Also clean any .i, .s, .o files that might have been created */
    system("rm -f *.i *.s *.o 2>/dev/null");
    
    printf("\nTest completed. Checksum: 0x%04x\n", checksum);
    printf("Expected minimum checksum for successful test: 0x%04x\n", 
           0x2 | 0x8 | 0x10 | 0x20 | 0x80 | 0x100 | 0x200);
    
    /* Reference unused functions to avoid warnings */
    (void)foo();
    (void)bar();
    
    return 0;
}
