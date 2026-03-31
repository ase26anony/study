#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

/* Trivial functions to compile */
int foo(void) { return 0; }
int bar(void) { return 1; }
int baz(void) { return 2; }

/* Function to create a temporary file with given content */
int create_temp_file(const char *filename, const char *content) {
    FILE *f = fopen(filename, "w");
    if (!f) return -1;
    fprintf(f, "%s", content);
    fclose(f);
    return 0;
}

/* Function to execute a command and check return status */
int execute_and_check(const char *cmd, int expected_status) {
    int status = system(cmd);
    int result = (WIFEXITED(status) && WEXITSTATUS(status) == expected_status);
    printf("Command: %s\n", cmd);
    printf("Status: %d (expected %d) -> %s\n\n", 
           WEXITSTATUS(status), expected_status, 
           result ? "PASS" : "FAIL");
    return result;
}

int main(void) {
    int checksum = 0;
    int success_count = 0;
    int total_tests = 0;
    
    /* Create temporary source files */
    const char *valid_src1 = "int foo(void) { return 0; }\n";
    const char *valid_src2 = "int bar(void) { return 1; }\n";
    const char *error_src = "int baz(void) { return \n";  /* Syntax error */
    
    if (create_temp_file("temp1.c", valid_src1) != 0 ||
        create_temp_file("temp2.c", valid_src2) != 0 ||
        create_temp_file("syntax_error.c", error_src) != 0) {
        fprintf(stderr, "Failed to create temporary files\n");
        return 1;
    }
    
    /* Create dump directory */
    mkdir("./dump1", 0755);
    
    printf("=== Testing GCC Driver Initialization Block ===\n\n");
    
    /* Sequence A: Help then Compile - tests print_help_list reset */
    printf("Sequence A: Help flag then compilation\n");
    total_tests++;
    if (execute_and_check("gcc --help=common > /dev/null 2>&1", 0)) {
        checksum += 1;
        total_tests++;
        if (execute_and_check("gcc -c temp1.c -o temp1.o", 0)) {
            success_count++;
            checksum += 2;
        }
    }
    
    /* Sequence B: Save-temps with dumpdir then compile - tests save_temps_flag, dumpdir reset */
    printf("Sequence B: Save-temps with dumpdir then plain compilation\n");
    total_tests++;
    if (execute_and_check("gcc -save-temps -dumpdir ./dump1 -dumpbase base -c temp1.c -o temp1_save.o 2>/dev/null", 0)) {
        checksum += 4;
        total_tests++;
        if (execute_and_check("gcc -c temp2.c -o temp2.o", 0)) {
            success_count++;
            checksum += 8;
        }
    }
    
    /* Sequence C: Error then success - tests greatest_status reset */
    printf("Sequence C: Error compilation then successful compilation\n");
    total_tests++;
    /* First command expected to fail (syntax error) */
    if (execute_and_check("gcc -c syntax_error.c 2>/dev/null", 1)) {
        checksum += 16;
        total_tests++;
        if (execute_and_check("gcc -c temp1.c -o temp1_err.o", 0)) {
            success_count++;
            checksum += 32;
        }
    }
    
    /* Sequence D: Verbose and linker then plain - tests verbose_only_flag, use_ld reset */
    printf("Sequence D: Verbose with linker selection then plain compilation\n");
    total_tests++;
    if (execute_and_check("gcc -v -fuse-ld=bfd -c temp1.c 2>&1 | grep -q 'COLLECT_GCC_OPTIONS' && echo 'Found' > /dev/null", 0)) {
        checksum += 64;
        total_tests++;
        if (execute_and_check("gcc -c temp2.c -o temp2_plain.o", 0)) {
            success_count++;
            checksum += 128;
        }
    }
    
    /* Additional test: Version flag then compilation - tests print_version reset */
    printf("Sequence E: Version flag then compilation\n");
    total_tests++;
    if (execute_and_check("gcc --version > /dev/null 2>&1", 0)) {
        checksum += 256;
        total_tests++;
        if (execute_and_check("gcc -c temp1.c -o temp1_ver.o", 0)) {
            success_count++;
            checksum += 512;
        }
    }
    
    /* Test with multiple inputs in single invocation (multiple jobs) */
    printf("Sequence F: Multiple source files in single invocation\n");
    total_tests++;
    if (execute_and_check("gcc -c temp1.c temp2.c", 0)) {
        success_count++;
        checksum += 1024;
    }
    
    /* Test with -x option to chain language specifications */
    printf("Sequence G: Using -x option for multiple language specs\n");
    total_tests++;
    if (execute_and_check("echo 'int x=1;' | gcc -x c - -c -o temp_stdin.o 2>/dev/null", 0)) {
        success_count++;
        checksum += 2048;
    }
    
    /* Cleanup */
    printf("=== Cleanup ===\n");
    system("rm -f temp1.c temp2.c syntax_error.c");
    system("rm -f temp*.o");
    system("rm -f base.*");  /* Files created by -dumpbase */
    system("rm -rf ./dump1");
    system("rm -f *.s *.i *.o");  /* Remove any leftover temp files */
    
    /* Final summary */
    printf("\n=== Test Summary ===\n");
    printf("Successfully executed: %d/%d test sequences\n", success_count, total_tests);
    printf("Final checksum: 0x%04X\n", checksum);
    
    /* Reference functions to avoid unused function warnings */
    (void)foo;
    (void)bar;
    (void)baz;
    
    return (success_count == total_tests) ? 0 : 1;
}
