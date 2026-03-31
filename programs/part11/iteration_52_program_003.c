/* Test program to cover driver initialization logic in gcc.cc lines 11228-11250
 * This program creates multiple compilation jobs to trigger state resets
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

/* Function to create a temporary file with given content */
static char* create_temp_file(const char* content, const char* suffix) {
    char template[] = "/tmp/gcc_test_XXXXXX";
    int fd = mkstemp(template);
    if (fd == -1) {
        perror("mkstemp failed");
        return NULL;
    }
    
    if (suffix) {
        char newname[256];
        snprintf(newname, sizeof(newname), "%s%s", template, suffix);
        close(fd);
        if (rename(template, newname) == 0) {
            strcpy(template, newname);
            fd = open(template, O_WRONLY);
        }
    }
    
    write(fd, content, strlen(content));
    close(fd);
    
    return strdup(template);
}

/* Function to execute a command and update checksum */
static int execute_command(const char* cmd, int expected_status) {
    printf("Executing: %s\n", cmd);
    int status = system(cmd);
    int exit_status = WEXITSTATUS(status);
    
    if (exit_status == expected_status) {
        checksum = (checksum * 31) + 1;  // Success
        printf("  [OK] Exit status: %d (expected %d)\n", exit_status, expected_status);
    } else {
        checksum = (checksum * 31) + 0;  // Failure
        printf("  [FAIL] Exit status: %d (expected %d)\n", exit_status, expected_status);
    }
    
    return exit_status;
}

/* Test functions to avoid unused function warnings */
static int foo(void) { return 0; }
static int bar(void) { return 1; }
__attribute__((unused)) static void use_functions(void) { 
    (void)foo(); 
    (void)bar(); 
}

int main(void) {
    printf("=== GCC Driver Initialization Block Test ===\n\n");
    
    /* Create temporary source files */
    char* temp1 = create_temp_file("int foo(void) { return 0; }\n", ".c");
    char* temp2 = create_temp_file("int bar(void) { return 1; }\n", ".c");
    char* temp3 = create_temp_file("int baz(void) { return  /* deliberate syntax error */\n", ".c");
    
    if (!temp1 || !temp2 || !temp3) {
        fprintf(stderr, "Failed to create temporary files\n");
        return 1;
    }
    
    /* Create output directory for dump files */
    mkdir("./dump_test", 0755);
    
    /* Get the actual gcc executable name */
    const char* gcc = "gcc";
    
    /* Test Sequence 1: Help then Compile
     * Sets print_help_list, then resets it in next job */
    printf("\n--- Sequence 1: Help flag then compilation ---\n");
    char cmd1[1024];
    snprintf(cmd1, sizeof(cmd1), "%s --help=common > /dev/null 2>&1", gcc);
    execute_command(cmd1, 0);
    
    snprintf(cmd1, sizeof(cmd1), "%s -c %s -o /tmp/test1.o", gcc, temp1);
    execute_command(cmd1, 0);
    
    /* Test Sequence 2: Version then Compile
     * Sets print_version, then resets it */
    printf("\n--- Sequence 2: Version flag then compilation ---\n");
    snprintf(cmd1, sizeof(cmd1), "%s --version > /dev/null 2>&1", gcc);
    execute_command(cmd1, 0);
    
    snprintf(cmd1, sizeof(cmd1), "%s -c %s -o /tmp/test2.o", gcc, temp2);
    execute_command(cmd1, 0);
    
    /* Test Sequence 3: Save-temps with dumpdir then plain compile
     * Sets save_temps_flag, dumpdir, dumpbase, then frees them */
    printf("\n--- Sequence 3: Save-temps with dumpdir then plain compile ---\n");
    snprintf(cmd1, sizeof(cmd1), "%s -save-temps -dumpdir ./dump_test -dumpbase mydump -c %s -o /tmp/test3.o 2>/dev/null", 
             gcc, temp1);
    execute_command(cmd1, 0);
    
    /* Second job without save-temps to trigger free() calls */
    snprintf(cmd1, sizeof(cmd1), "%s -c %s -o /tmp/test4.o", gcc, temp2);
    execute_command(cmd1, 0);
    
    /* Test Sequence 4: Verbose flag then normal compile
     * Sets verbose_only_flag, then resets it */
    printf("\n--- Sequence 4: Verbose flag then normal compile ---\n");
    snprintf(cmd1, sizeof(cmd1), "%s -v -c %s -o /tmp/test5.o 2>/dev/null", gcc, temp1);
    execute_command(cmd1, 0);
    
    snprintf(cmd1, sizeof(cmd1), "%s -c %s -o /tmp/test6.o", gcc, temp2);
    execute_command(cmd1, 0);
    
    /* Test Sequence 5: Linker selection then default
     * Sets use_ld, then resets to NULL */
    printf("\n--- Sequence 5: Linker selection then default ---\n");
    snprintf(cmd1, sizeof(cmd1), "%s -fuse-ld=bfd -c %s -o /tmp/test7.o 2>/dev/null", gcc, temp1);
    execute_command(cmd1, 0);
    
    snprintf(cmd1, sizeof(cmd1), "%s -c %s -o /tmp/test8.o", gcc, temp2);
    execute_command(cmd1, 0);
    
    /* Test Sequence 6: Error then success (tests greatest_status reset)
     * First job fails, second succeeds */
    printf("\n--- Sequence 6: Error recovery (tests greatest_status) ---\n");
    snprintf(cmd1, sizeof(cmd1), "%s -c %s -o /tmp/error.o 2>/dev/null", gcc, temp3);
    execute_command(cmd1, 1);  /* Expected to fail */
    
    snprintf(cmd1, sizeof(cmd1), "%s -c %s -o /tmp/success.o", gcc, temp1);
    execute_command(cmd1, 0);  /* Expected to succeed */
    
    /* Test Sequence 7: Multiple jobs in single invocation
     * Tests per-job initialization with multiple inputs */
    printf("\n--- Sequence 7: Multiple inputs in single invocation ---\n");
    snprintf(cmd1, sizeof(cmd1), "%s -c %s %s -o /tmp/multi1.o -o /tmp/multi2.o", gcc, temp1, temp2);
    execute_command(cmd1, 0);
    
    /* Test Sequence 8: Combined flags to set multiple variables
     * Sets several state variables at once */
    printf("\n--- Sequence 8: Combined flags then clean compile ---\n");
    snprintf(cmd1, sizeof(cmd1), "%s -v -save-temps -dumpdir ./dump_test2 -fuse-ld=bfd --help=optimizers > /dev/null 2>&1", gcc);
    execute_command(cmd1, 0);
    
    /* Follow with minimal compile to trigger full reset */
    snprintf(cmd1, sizeof(cmd1), "%s -c %s", gcc, temp1);
    execute_command(cmd1, 0);
    
    /* Test Sequence 9: Using -x to create distinct language jobs */
    printf("\n--- Sequence 9: Using -x for distinct language jobs ---\n");
    char* cpp_src = create_temp_file("#include <stdio.h>\nint main() { return 0; }", ".cpp");
    if (cpp_src) {
        snprintf(cmd1, sizeof(cmd1), "%s -x c -c %s -o /tmp/c.o", gcc, temp1);
        execute_command(cmd1, 0);
        
        snprintf(cmd1, sizeof(cmd1), "%s -x c++ -c %s -o /tmp/cpp.o", gcc, cpp_src);
        execute_command(cmd1, 0);
        free(cpp_src);
    }
    
    /* Cleanup */
    printf("\n--- Cleanup ---\n");
    unlink(temp1);
    unlink(temp2);
    unlink(temp3);
    free(temp1);
    free(temp2);
    free(temp3);
    
    /* Clean up generated files */
    system("rm -f /tmp/test*.o /tmp/error.o /tmp/success.o /tmp/multi*.o /tmp/c.o /tmp/cpp.o");
    system("rm -rf ./dump_test ./dump_test2");
    
    /* Remove any leftover temporary files */
    system("rm -f /tmp/gcc_test_* /tmp/cc* /tmp/*.s /tmp/*.i /tmp/*.o 2>/dev/null");
    
    printf("\n=== Test Complete ===\n");
    printf("Final checksum: %u (should be non-zero)\n", checksum);
    printf("All sequences executed. Check coverage for gcc.cc lines 11228-11250\n");
    
    /* Use the test functions to avoid warnings */
    use_functions();
    
    return 0;
}
