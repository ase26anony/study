#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>

#define MAX_CMD_LEN 4096

/* Execute a command and return its exit status */
static int execute_command(const char *cmd) {
    printf("Executing: %s\n", cmd);
    int status = system(cmd);
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    return -1;
}

/* Create a minimal C source file */
static void create_source_file(const char *filename) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        perror("fopen source");
        exit(1);
    }
    fprintf(f, "int main(void) { return 0; }\n");
    fclose(f);
}

/* Create a response file with various options */
static void create_response_file(const char *filename) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        perror("fopen response");
        exit(1);
    }
    fprintf(f, "-v\n");
    fprintf(f, "-save-temps=obj\n");
    fprintf(f, "-Wall\n");
    fprintf(f, "-Wextra\n");
    fclose(f);
}

int main(int argc, char **argv) {
    char cmd[MAX_CMD_LEN];
    const char *base_name = "test_gcc_reset";
    char source_file[256];
    char response_file[256];
    char output_file[256];
    char object_file[256];
    char exec_file[256];
    
    /* Create temporary file names */
    snprintf(source_file, sizeof(source_file), "%s.c", base_name);
    snprintf(response_file, sizeof(response_file), "%s.rsp", base_name);
    snprintf(output_file, sizeof(output_file), "%s.out", base_name);
    snprintf(object_file, sizeof(object_file), "%s.o", base_name);
    snprintf(exec_file, sizeof(exec_file), "%s.exe", base_name);
    
    /* Create source and response files */
    create_source_file(source_file);
    create_response_file(response_file);
    
    printf("=== Testing GCC driver reset logic ===\n\n");
    
    /* 1. Invocation with -print-help-list (sets print_help_list) */
    printf("1. Testing -print-help-list:\n");
    snprintf(cmd, sizeof(cmd), "gcc -print-help-list 2>&1 | head -5");
    execute_command(cmd);
    printf("\n");
    
    /* 2. Invocation with --version (sets print_version) */
    printf("2. Testing --version:\n");
    snprintf(cmd, sizeof(cmd), "gcc --version");
    execute_command(cmd);
    printf("\n");
    
    /* 3. Invocation with verbose flag (sets verbose_only_flag) */
    printf("3. Testing verbose flag:\n");
    snprintf(cmd, sizeof(cmd), "gcc -v -E %s 2>&1 | head -10", source_file);
    execute_command(cmd);
    printf("\n");
    
    /* 4. Invocation with save-temps options (sets save_temps_flag, dumpdir, etc.) */
    printf("4. Testing save-temps options:\n");
    snprintf(cmd, sizeof(cmd), "gcc -save-temps=cwd -c %s -o %s", 
             source_file, object_file);
    execute_command(cmd);
    printf("\n");
    
    /* 5. Invocation with response file (sets at_file_supplied) */
    printf("5. Testing response file (@file syntax):\n");
    snprintf(cmd, sizeof(cmd), "gcc @%s -c %s -o %s", 
             response_file, source_file, object_file);
    execute_command(cmd);
    printf("\n");
    
    /* 6. Invocation with sysroot options (affects target_system_root) */
    printf("6. Testing sysroot options:\n");
    snprintf(cmd, sizeof(cmd), "gcc --sysroot=/usr -c %s -o %s", 
             source_file, object_file);
    execute_command(cmd);
    printf("\n");
    
    /* 7. Invocation with fuse-ld (sets use_ld) */
    printf("7. Testing fuse-ld option:\n");
    snprintf(cmd, sizeof(cmd), "gcc -fuse-ld=bfd -o %s %s 2>&1 | head -5", 
             exec_file, source_file);
    execute_command(cmd);
    printf("\n");
    
    /* 8. Invocation with time report (sets report_times_to_file) */
    printf("8. Testing time report option:\n");
    snprintf(cmd, sizeof(cmd), "gcc -ftime-report -c %s -o %s 2>&1 | head -20", 
             source_file, object_file);
    execute_command(cmd);
    printf("\n");
    
    /* 9. Invocation that fails (sets greatest_status to non-1) */
    printf("9. Testing failure case (invalid option):\n");
    snprintf(cmd, sizeof(cmd), "gcc -invalid-option-xyz %s 2>&1 | head -5", 
             source_file);
    execute_command(cmd);
    printf("\n");
    
    /* 10. Invocation with machine-specific options (affects spec_machine) */
    printf("10. Testing machine-specific options:\n");
    snprintf(cmd, sizeof(cmd), "gcc -march=native -mtune=native -c %s -o %s", 
             source_file, object_file);
    execute_command(cmd);
    printf("\n");
    
    /* 11. Invocation with dump options (sets dumpdir, dumpbase, etc.) */
    printf("11. Testing dump options:\n");
    snprintf(cmd, sizeof(cmd), "gcc -fdump-rtl-all -c %s -o %s 2>&1 | head -5", 
             source_file, object_file);
    execute_command(cmd);
    printf("\n");
    
    /* 12. Final successful compilation (ensures reset after previous failures) */
    printf("12. Final successful compilation (verifies reset):\n");
    snprintf(cmd, sizeof(cmd), "gcc -o %s %s", exec_file, source_file);
    int final_status = execute_command(cmd);
    printf("\nFinal compilation status: %d\n", final_status);
    
    /* Cleanup */
    printf("\n=== Cleaning up temporary files ===\n");
    unlink(source_file);
    unlink(response_file);
    unlink(object_file);
    unlink(exec_file);
    
    /* Also clean up any save-temps files */
    char temp_files[][32] = {
        "test_gcc_reset.i",
        "test_gcc_reset.s",
        "test_gcc_reset.ii",
        "test_gcc_reset.o",
        "test_gcc_reset.exe"
    };
    
    for (size_t i = 0; i < sizeof(temp_files)/sizeof(temp_files[0]); i++) {
        if (unlink(temp_files[i]) == 0) {
            printf("Removed: %s\n", temp_files[i]);
        }
    }
    
    printf("\n=== Test complete ===\n");
    return 0;
}
