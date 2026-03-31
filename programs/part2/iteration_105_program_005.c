#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>

#define MAX_CMD_LEN 1024
#define TEMP_FILE_TEMPLATE "/tmp/gcc_test_XXXXXX"

/* Helper function to create a temporary file with given content */
char* create_temp_file(const char* content, const char* suffix) {
    char template[256];
    snprintf(template, sizeof(template), "%s%s", TEMP_FILE_TEMPLATE, suffix);
    int fd = mkstemps(template, strlen(suffix));
    if (fd < 0) {
        perror("mkstemps failed");
        return NULL;
    }
    
    if (content) {
        write(fd, content, strlen(content));
    }
    close(fd);
    
    return strdup(template);
}

/* Helper function to execute a command and capture exit status */
int execute_command(const char* cmd) {
    printf("Executing: %s\n", cmd);
    int status = system(cmd);
    if (WIFEXITED(status)) {
        printf("Exit status: %d\n\n", WEXITSTATUS(status));
        return WEXITSTATUS(status);
    }
    return -1;
}

/* Helper to clean up temporary files */
void cleanup_files(char** files, int count) {
    for (int i = 0; i < count; i++) {
        if (files[i]) {
            unlink(files[i]);
            free(files[i]);
        }
    }
}

int main(int argc, char** argv) {
    char* temp_files[10] = {0};
    int file_count = 0;
    
    /* Create a minimal valid C source file */
    char* source_file = create_temp_file(
        "int main() {\n"
        "    return 0;\n"
        "}\n", ".c");
    if (!source_file) return 1;
    temp_files[file_count++] = source_file;
    
    /* Create a dummy source file that doesn't exist (for failure case) */
    char dummy_file[] = "/tmp/nonexistent_dummy.c";
    
    /* Create a response file with various options */
    char* response_file = create_temp_file(
        "-v\n"
        "-save-temps=obj\n"
        "-Wall\n"
        "-Wextra\n", ".rsp");
    if (!response_file) {
        cleanup_files(temp_files, file_count);
        return 1;
    }
    temp_files[file_count++] = response_file;
    
    /* Create output file names */
    char* output_obj = create_temp_file(NULL, ".o");
    char* output_exe = create_temp_file(NULL, "");
    if (output_obj) temp_files[file_count++] = output_obj;
    if (output_exe) temp_files[file_count++] = output_exe;
    
    /* Build commands with various options to set the targeted variables */
    char cmd[MAX_CMD_LEN];
    
    /* Invocation 1: Set print_help_list */
    printf("=== Invocation 1: Setting print_help_list ===\n");
    snprintf(cmd, sizeof(cmd), "gcc -print-help-list 2>&1 | head -5");
    execute_command(cmd);
    
    /* Invocation 2: Set version flag */
    printf("=== Invocation 2: Setting print_version ===\n");
    snprintf(cmd, sizeof(cmd), "gcc --version");
    execute_command(cmd);
    
    /* Invocation 3: Set verbose flag and use response file (triggers at_file_supplied) */
    printf("=== Invocation 3: Setting verbose_only_flag and at_file_supplied ===\n");
    snprintf(cmd, sizeof(cmd), "gcc -v -save-temps=obj -o %s @%s %s 2>&1 | head -10",
             output_obj ? output_obj : "test.o", 
             response_file, 
             source_file);
    execute_command(cmd);
    
    /* Invocation 4: Set save_temps_flag and related dumpdir variables */
    printf("=== Invocation 4: Setting save_temps_flag and dumpdir variables ===\n");
    snprintf(cmd, sizeof(cmd), "gcc -save-temps=cwd -fdumpdir=./dumps/ -fdump-base=dumpbase "
             "-o %s %s 2>&1 | head -5",
             output_exe ? output_exe : "test.exe",
             source_file);
    execute_command(cmd);
    
    /* Invocation 5: Set use_ld and sysroot variables */
    printf("=== Invocation 5: Setting use_ld and target_system_root ===\n");
    snprintf(cmd, sizeof(cmd), "gcc -fuse-ld=gold --sysroot=/usr -isysroot=/usr/include "
             "-o %s %s 2>&1 | head -5",
             output_exe ? output_exe : "test.exe",
             source_file);
    execute_command(cmd);
    
    /* Invocation 6: Set report_times_to_file */
    printf("=== Invocation 6: Setting report_times_to_file ===\n");
    snprintf(cmd, sizeof(cmd), "gcc -ftime-report -ftime-report-details "
             "-c %s -o %s 2>&1 | head -10",
             source_file,
             output_obj ? output_obj : "test.o");
    execute_command(cmd);
    
    /* Invocation 7: Cause failure to set greatest_status != 1 */
    printf("=== Invocation 7: Causing failure (should set greatest_status != 1) ===\n");
    snprintf(cmd, sizeof(cmd), "gcc -v -save-temps=obj -o %s @%s %s 2>&1 | head -5",
             output_obj ? output_obj : "test.o", 
             response_file, 
             dummy_file);  /* This file doesn't exist */
    execute_command(cmd);
    
    /* Invocation 8: Try to set spec_machine (may be target-specific) */
    printf("=== Invocation 8: Attempting to set spec_machine ===\n");
    snprintf(cmd, sizeof(cmd), "gcc -mtune=native -march=x86-64 -c %s -o %s 2>&1 | head -5",
             source_file,
             output_obj ? output_obj : "test.o");
    execute_command(cmd);
    
    /* Invocation 9: Complex combination to exercise multiple resets */
    printf("=== Invocation 9: Complex combination ===\n");
    snprintf(cmd, sizeof(cmd), "gcc -v --version -print-help-list -save-temps "
             "-ftime-report -fuse-ld=bfd -c %s -o %s 2>&1 | head -15",
             source_file,
             output_obj ? output_obj : "test.o");
    execute_command(cmd);
    
    /* Invocation 10: Final successful compilation to ensure reset after all */
    printf("=== Invocation 10: Final successful compilation ===\n");
    snprintf(cmd, sizeof(cmd), "gcc -O2 -o %s %s",
             output_exe ? output_exe : "test.exe",
             source_file);
    int final_status = execute_command(cmd);
    
    /* Clean up */
    printf("=== Cleaning up temporary files ===\n");
    cleanup_files(temp_files, file_count);
    
    /* Also clean up any generated dump files */
    system("rm -f ./dumps/* dumpbase.* *.i *.s *.o 2>/dev/null");
    
    printf("Test completed. Final exit status: %d\n", final_status);
    return 0;
}
