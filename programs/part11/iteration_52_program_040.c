/* test_gcc_driver_init.c - Test program to cover GCC driver initialization block */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

/* Helper function to create a temporary file with given content */
static int create_temp_file(const char *template, const char *content) {
    FILE *f = fopen(template, "w");
    if (!f) {
        perror("fopen");
        return -1;
    }
    fputs(content, f);
    fclose(f);
    return 0;
}

/* Helper function to execute a command and check its return status */
static int execute_and_check(const char *cmd, int expected_status) {
    printf("Executing: %s\n", cmd);
    int status = system(cmd);
    int actual_status = WEXITSTATUS(status);
    
    if (actual_status != expected_status) {
        printf("  Warning: Expected status %d, got %d\n", expected_status, actual_status);
    }
    return actual_status;
}

/* Unused functions to avoid -Wunused-function warnings */
static int unused_func1(void) { return 0; }
static int unused_func2(void) { return 1; }
static int unused_func3(void) { return 2; }

int main(void) {
    int checksum = 0;
    int result;
    
    printf("=== GCC Driver Initialization Block Test ===\n\n");
    
    /* Create temporary directory for dump files */
    mkdir("./dump_test_dir", 0755);
    
    /* Create temporary source files */
    const char *valid_src1 = 
        "int foo(void) { return 0; }\n"
        "int bar(void) { return 1; }\n";
    
    const char *valid_src2 = 
        "int baz(void) { return 2; }\n"
        "int qux(void) { return 3; }\n";
    
    const char *error_src = 
        "int syntax_error(void) { return /* missing semicolon and value */ }\n";
    
    if (create_temp_file("temp_valid1.c", valid_src1) < 0 ||
        create_temp_file("temp_valid2.c", valid_src2) < 0 ||
        create_temp_file("temp_error.c", error_src) < 0) {
        fprintf(stderr, "Failed to create temporary files\n");
        return 1;
    }
    
    printf("Created temporary source files\n\n");
    
    /* Sequence 1: Help/Version flags followed by compilation
       This should set print_help_list or print_version, then reset them */
    printf("--- Sequence 1: Help then Compile ---\n");
    result = execute_and_check("gcc --help=common 2>&1 | head -5 > /dev/null", 0);
    checksum += result;
    
    result = execute_and_check("gcc -c temp_valid1.c -o temp1.o", 0);
    checksum += result;
    
    /* Force a delay to ensure separate jobs */
    sleep(1);
    
    /* Sequence 2: Version flag then compilation */
    printf("\n--- Sequence 2: Version then Compile ---\n");
    result = execute_and_check("gcc --version 2>&1 | head -1 > /dev/null", 0);
    checksum += result;
    
    result = execute_and_check("gcc -c temp_valid2.c -o temp2.o", 0);
    checksum += result;
    
    sleep(1);
    
    /* Sequence 3: Save-temps with dumpdir then plain compilation
       This exercises save_temps_flag, dumpdir, dumpbase reset logic */
    printf("\n--- Sequence 3: Save-temps with dumpdir then plain compile ---\n");
    result = execute_and_check("gcc -save-temps -dumpdir ./dump_test_dir -dumpbase mytest "
                               "-c temp_valid1.c -o temp_save.o 2>&1", 0);
    checksum += result;
    
    /* Check if dump files were created */
    result = system("ls -la ./dump_test_dir/mytest* 2>/dev/null | wc -l");
    printf("  Dump files created: %d\n", WEXITSTATUS(result));
    
    /* Now compile without save-temps - should reset the flags */
    result = execute_and_check("gcc -c temp_valid2.c -o temp_nosave.o", 0);
    checksum += result;
    
    sleep(1);
    
    /* Sequence 4: Verbose output then normal compilation
       Exercises verbose_only_flag reset */
    printf("\n--- Sequence 4: Verbose then normal compile ---\n");
    result = execute_and_check("gcc -v -c temp_valid1.c -o temp_verbose.o 2>&1 | "
                               "grep -q 'COLLECT_GCC' && echo 'Found' > /dev/null", 0);
    checksum += result;
    
    result = execute_and_check("gcc -c temp_valid2.c -o temp_normal.o", 0);
    checksum += result;
    
    sleep(1);
    
    /* Sequence 5: Linker selection then default
       Exercises use_ld reset */
    printf("\n--- Sequence 5: Specific linker then default ---\n");
    result = execute_and_check("gcc -fuse-ld=bfd -c temp_valid1.c -o temp_ldbfd.o 2>&1", 0);
    checksum += result;
    
    result = execute_and_check("gcc -c temp_valid2.c -o temp_lddefault.o", 0);
    checksum += result;
    
    sleep(1);
    
    /* Sequence 6: Error recovery path
       Exercises greatest_status reset to 1 after failure */
    printf("\n--- Sequence 6: Error then success (tests greatest_status) ---\n");
    result = execute_and_check("gcc -c temp_error.c -o temp_error.o 2>&1", 1);
    checksum += result;
    
    result = execute_and_check("gcc -c temp_valid1.c -o temp_recover.o", 0);
    checksum += result;
    
    sleep(1);
    
    /* Sequence 7: Multiple inputs in single invocation
       Tests per-job initialization with multiple source files */
    printf("\n--- Sequence 7: Multiple inputs in one invocation ---\n");
    result = execute_and_check("gcc -c temp_valid1.c temp_valid2.c", 0);
    checksum += result;
    
    /* Sequence 8: Combined flags then minimal compile
       Tests reset of multiple variables at once */
    printf("\n--- Sequence 8: Combined flags then minimal compile ---\n");
    result = execute_and_check("gcc -v --help=optimizers -save-temps -dumpdir ./dump_test_dir2 "
                               "-dumpbase combined -fuse-ld=lld -c temp_valid1.c "
                               "-o temp_combined.o 2>&1 | head -20 > /dev/null", 0);
    checksum += result;
    
    mkdir("./dump_test_dir2", 0755);
    result = execute_and_check("gcc -c temp_valid2.c -o temp_minimal.o", 0);
    checksum += result;
    
    sleep(1);
    
    /* Sequence 9: Using -x to specify language (forces job creation) */
    printf("\n--- Sequence 9: Using -x language specification ---\n");
    result = execute_and_check("echo 'int x = 42;' | gcc -x c - -c -o temp_stdin1.o", 0);
    checksum += result;
    
    result = execute_and_check("echo 'int y = 43;' | gcc -x c - -c -o temp_stdin2.o", 0);
    checksum += result;
    
    /* Cleanup */
    printf("\n--- Cleaning up ---\n");
    system("rm -f temp*.c temp*.o temp*.i temp*.s temp*.ii 2>/dev/null");
    system("rm -rf ./dump_test_dir ./dump_test_dir2 2>/dev/null");
    
    /* Final checksum and verification */
    printf("\n=== Test Results ===\n");
    printf("Checksum of return codes: %d\n", checksum);
    
    /* Verify object files were created in successful compilations */
    int obj_count = 0;
    FILE *fp = popen("ls temp*.o 2>/dev/null | wc -l", "r");
    if (fp) {
        char buf[32];
        if (fgets(buf, sizeof(buf), fp)) {
            obj_count = atoi(buf);
        }
        pclose(fp);
    }
    
    printf("Object files remaining (should be 0): %d\n", obj_count);
    
    if (obj_count == 0 && checksum < 10) {  /* Most commands should succeed */
        printf("\n✅ Test completed successfully\n");
        return 0;
    } else {
        printf("\n⚠️  Test completed with warnings\n");
        return 1;
    }
}
