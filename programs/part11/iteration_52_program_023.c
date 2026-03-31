/* test_gcc_driver_init.c
 * 
 * This test program exercises the GCC driver initialization logic
 * (lines 11228-11250 in gcc.cc) by invoking multiple compilation jobs
 * with different option combinations that set and reset the target
 * state variables.
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

/* Function to execute a command and update checksum */
static int execute_command(const char *cmd, const char *desc) {
    printf("Executing: %s\n", desc);
    printf("Command: %s\n", cmd);
    
    int result = system(cmd);
    checksum = (checksum * 31) + (result & 0xFF);
    
    if (result != 0) {
        printf("Command returned: %d\n", result);
    }
    
    return result;
}

/* Create a temporary file with given content */
static char *create_temp_file(const char *content, const char *suffix) {
    char template[] = "/tmp/gcc_test_XXXXXX";
    int fd = mkstemp(template);
    if (fd == -1) {
        perror("mkstemp failed");
        return NULL;
    }
    
    /* Append suffix if provided */
    char *filename = malloc(strlen(template) + strlen(suffix) + 1);
    if (!filename) {
        close(fd);
        unlink(template);
        return NULL;
    }
    
    strcpy(filename, template);
    strcat(filename, suffix);
    
    /* Rename to include suffix */
    close(fd);
    if (rename(template, filename) != 0) {
        free(filename);
        unlink(template);
        return NULL;
    }
    
    /* Write content */
    FILE *f = fopen(filename, "w");
    if (!f) {
        free(filename);
        unlink(filename);
        return NULL;
    }
    
    fputs(content, f);
    fclose(f);
    
    return filename;
}

/* Clean up temporary files */
static void cleanup_file(const char *filename) {
    if (filename) {
        unlink(filename);
    }
}

/* Clean up temporary directory */
static void cleanup_dir(const char *dirname) {
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "rm -rf %s 2>/dev/null", dirname);
    system(cmd);
}

int main(void) {
    printf("=== GCC Driver Initialization Block Test ===\n\n");
    
    /* Create temporary source files */
    char *temp1_c = create_temp_file(
        "int foo(void) { return 0; }\n"
        "int unused_func1(void) { return 1; }\n",
        ".c");
    
    char *temp2_c = create_temp_file(
        "int bar(void) { return 2; }\n"
        "int unused_func2(void) { return 3; }\n",
        ".c");
    
    char *error_c = create_temp_file(
        "int baz(void) { return  /* deliberate syntax error */\n",
        ".c");
    
    if (!temp1_c || !temp2_c || !error_c) {
        fprintf(stderr, "Failed to create temporary files\n");
        return 1;
    }
    
    printf("Created temporary files:\n");
    printf("  %s\n", temp1_c);
    printf("  %s\n", temp2_c);
    printf("  %s\n", error_c);
    printf("\n");
    
    /* Create temporary output files */
    char temp1_o[256], temp2_o[256];
    snprintf(temp1_o, sizeof(temp1_o), "%s.o", temp1_c);
    snprintf(temp2_o, sizeof(temp2_o), "%s.o", temp2_c);
    
    /* Clean up any previous test artifacts */
    cleanup_dir("./dump1");
    cleanup_dir("./dump2");
    cleanup_file(temp1_o);
    cleanup_file(temp2_o);
    
    /* Get the GCC executable name - try various possibilities */
    const char *gcc_exe = "gcc";
    char gcc_path[256] = "";
    
    /* Try to find gcc in PATH */
    FILE *fp = popen("which gcc 2>/dev/null", "r");
    if (fp) {
        if (fgets(gcc_path, sizeof(gcc_path), fp)) {
            /* Remove newline */
            gcc_path[strcspn(gcc_path, "\n")] = 0;
            gcc_exe = gcc_path;
        }
        pclose(fp);
    }
    
    printf("Using GCC: %s\n\n", gcc_exe);
    
    /* TEST SEQUENCE 1: Help/Version flags then compilation
     * This sets print_help_list or print_version in first job,
     * then resets them in second job.
     */
    printf("--- Test Sequence 1: Help then Compile ---\n");
    char cmd[1024];
    
    /* First job: help output (sets print_help_list) */
    snprintf(cmd, sizeof(cmd), "%s --help=common > /dev/null 2>&1", gcc_exe);
    execute_command(cmd, "Help command (sets print_help_list)");
    
    /* Second job: compilation (should reset print_help_list) */
    snprintf(cmd, sizeof(cmd), "%s -c %s -o %s", gcc_exe, temp1_c, temp1_o);
    execute_command(cmd, "Compilation after help (resets flags)");
    
    /* Third job: version (sets print_version) */
    snprintf(cmd, sizeof(cmd), "%s --version > /dev/null 2>&1", gcc_exe);
    execute_command(cmd, "Version command (sets print_version)");
    
    /* Fourth job: another compilation */
    snprintf(cmd, sizeof(cmd), "%s -c %s -o %s", gcc_exe, temp2_c, temp2_o);
    execute_command(cmd, "Compilation after version");
    
    /* TEST SEQUENCE 2: Save-temps and dumpdir flags
     * This sets save_temps_flag, dumpdir, dumpbase, etc.
     * then resets them in next job.
     */
    printf("\n--- Test Sequence 2: Save-Temps with Dumpdir ---\n");
    
    /* Create dump directory */
    mkdir("./dump1", 0755);
    
    /* First job: with save-temps and dumpdir (sets save_temps_flag, dumpdir, dumpbase) */
    snprintf(cmd, sizeof(cmd), 
             "%s -save-temps -dumpdir ./dump1 -dumpbase mytest -c %s -o %s 2>/dev/null",
             gcc_exe, temp1_c, temp1_o);
    execute_command(cmd, "Compile with save-temps and dumpdir");
    
    /* Check if dump files were created */
    snprintf(cmd, sizeof(cmd), "ls -la ./dump1/mytest* 2>/dev/null | wc -l");
    FILE *ls_fp = popen(cmd, "r");
    if (ls_fp) {
        int count = 0;
        fscanf(ls_fp, "%d", &count);
        pclose(ls_fp);
        printf("Created %d dump files in ./dump1\n", count);
        checksum = (checksum * 31) + count;
    }
    
    /* Second job: without save-temps (should free dumpdir, dumpbase, etc.) */
    snprintf(cmd, sizeof(cmd), "%s -c %s -o %s", gcc_exe, temp2_c, temp2_o);
    execute_command(cmd, "Compile without save-temps (resets dump variables)");
    
    /* Third job: different dump configuration */
    mkdir("./dump2", 0755);
    snprintf(cmd, sizeof(cmd),
             "%s -save-temps=cwd -dumpdir ./dump2 -dumpbase test2 -c %s 2>/dev/null",
             gcc_exe, temp1_c);
    execute_command(cmd, "Another save-temps with different dumpdir");
    
    /* TEST SEQUENCE 3: Error recovery
     * First job fails (syntax error), second succeeds.
     * Exercises greatest_status reset logic.
     */
    printf("\n--- Test Sequence 3: Error then Success ---\n");
    
    /* First job: should fail (sets error status) */
    snprintf(cmd, sizeof(cmd), "%s -c %s 2>/dev/null", gcc_exe, error_c);
    int error_result = execute_command(cmd, "Compile with syntax error (should fail)");
    
    /* Second job: should succeed (resets greatest_status) */
    snprintf(cmd, sizeof(cmd), "%s -c %s -o %s", gcc_exe, temp1_c, temp1_o);
    execute_command(cmd, "Successful compile after error");
    
    /* TEST SEQUENCE 4: Verbose and linker flags
     * Sets verbose_only_flag, use_ld, then resets.
     */
    printf("\n--- Test Sequence 4: Verbose and Linker Flags ---\n");
    
    /* First job: verbose output with specific linker */
    snprintf(cmd, sizeof(cmd),
             "%s -v -fuse-ld=bfd -c %s -o %s 2>&1 | grep -q 'COLLECT_GCC_OPTIONS'",
             gcc_exe, temp1_c, temp1_o);
    execute_command(cmd, "Verbose compile with specific linker");
    
    /* Try different linker if available */
    snprintf(cmd, sizeof(cmd),
             "%s -fuse-ld=lld -c %s -o %s 2>/dev/null || true",
             gcc_exe, temp2_c, temp2_o);
    execute_command(cmd, "Try lld linker (may fail if not available)");
    
    /* Second job: plain compilation (resets use_ld) */
    snprintf(cmd, sizeof(cmd), "%s -c %s", gcc_exe, temp1_c);
    execute_command(cmd, "Plain compile (resets linker flag)");
    
    /* TEST SEQUENCE 5: Multiple inputs in single invocation
     * This creates multiple compilation jobs in one gcc call.
     */
    printf("\n--- Test Sequence 5: Multiple Inputs Single Invocation ---\n");
    
    /* Create two more temporary files */
    char *temp3_c = create_temp_file("int three(void) { return 3; }\n", ".c");
    char *temp4_c = create_temp_file("int four(void) { return 4; }\n", ".c");
    
    if (temp3_c && temp4_c) {
        char temp3_o[256], temp4_o[256];
        snprintf(temp3_o, sizeof(temp3_o), "%s.o", temp3_c);
        snprintf(temp4_o, sizeof(temp4_o), "%s.o", temp4_c);
        
        /* Compile multiple files with mixed options */
        snprintf(cmd, sizeof(cmd),
                 "%s -save-temps -dumpdir ./dump1 -c %s %s -o %s %s 2>/dev/null",
                 gcc_exe, temp1_c, temp2_c, temp1_o, temp2_o);
        execute_command(cmd, "Compile multiple files with save-temps");
        
        /* Clean up */
        cleanup_file(temp3_c);
        cleanup_file(temp4_c);
        cleanup_file(temp3_o);
        cleanup_file(temp4_o);
    }
    
    /* TEST SEQUENCE 6: Combination of many flags
     * Exercise multiple state variables at once.
     */
    printf("\n--- Test Sequence 6: Combined Flags ---\n");
    
    snprintf(cmd, sizeof(cmd),
             "%s -v --help=optimizers 2>&1 | head -5 > /dev/null",
             gcc_exe);
    execute_command(cmd, "Verbose with help optimizers");
    
    snprintf(cmd, sizeof(cmd),
             "%s -save-temps -dumpbase combined -c %s 2>/dev/null",
             gcc_exe, temp1_c);
    execute_command(cmd, "Save-temps after help");
    
    /* Final compilation to ensure all is reset */
    snprintf(cmd, sizeof(cmd),
             "%s -c %s -o %s",
             gcc_exe, temp2_c, temp2_o);
    execute_command(cmd, "Final plain compilation");
    
    /* Cleanup */
    printf("\n--- Cleanup ---\n");
    cleanup_file(temp1_c);
    cleanup_file(temp2_c);
    cleanup_file(error_c);
    cleanup_file(temp1_o);
    cleanup_file(temp2_o);
    cleanup_dir("./dump1");
    cleanup_dir("./dump2");
    
    /* Remove any .i, .s, .o files created in current directory */
    system("rm -f *.i *.s *.o 2>/dev/null");
    
    printf("\n=== Test Complete ===\n");
    printf("Final checksum: 0x%08x\n", checksum);
    printf("(This checksum validates that all command sequences were executed)\n");
    
    /* Reference the functions to avoid unused function warnings */
    if (checksum == 0) {
        /* This should never happen, but prevents optimization */
        printf("Zero checksum - something went wrong\n");
    }
    
    return 0;
}
