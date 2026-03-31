#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>

#define MAX_CMD_LEN 4096

/* Helper function to execute a command and return its exit status */
static int execute_command(const char *cmd) {
    printf("Executing: %s\n", cmd);
    int status = system(cmd);
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    return -1;
}

/* Helper to create a temporary file with given content */
static int create_temp_file(const char *filename, const char *content) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        perror("fopen");
        return -1;
    }
    fputs(content, f);
    fclose(f);
    return 0;
}

int main(int argc, char **argv) {
    char cmd[MAX_CMD_LEN];
    const char *base_name = "test_coverage";
    const char *source_file = "test_coverage_source.c";
    const char *response_file = "test_coverage_args.rsp";
    const char *output_obj = "test_coverage.o";
    const char *output_exe = "test_coverage.exe";
    const char *nonexistent_file = "nonexistent_dummy.c";
    
    /* Create a minimal valid C source file */
    const char *source_content = 
        "int main() {\n"
        "    return 0;\n"
        "}\n";
    
    if (create_temp_file(source_file, source_content) < 0) {
        return 1;
    }
    
    /* Create a response file with various options */
    const char *response_content = 
        "-v\n"
        "-save-temps=obj\n"
        "-O2\n";
    
    if (create_temp_file(response_file, response_content) < 0) {
        unlink(source_file);
        return 1;
    }
    
    printf("=== Starting GCC driver reset logic test ===\n\n");
    
    /* Invocation 1: Set print_help_list */
    printf("--- Invocation 1: Setting print_help_list ---\n");
    snprintf(cmd, sizeof(cmd), "gcc -print-help-list 2>&1 | head -5");
    execute_command(cmd);
    printf("\n");
    
    /* Invocation 2: Set print_version */
    printf("--- Invocation 2: Setting print_version ---\n");
    snprintf(cmd, sizeof(cmd), "gcc --version");
    execute_command(cmd);
    printf("\n");
    
    /* Invocation 3: Set verbose_only_flag and use response file (at_file_supplied) */
    printf("--- Invocation 3: Setting verbose_only_flag with response file ---\n");
    snprintf(cmd, sizeof(cmd), "gcc -v @%s -c %s -o %s", 
             response_file, source_file, output_obj);
    execute_command(cmd);
    printf("\n");
    
    /* Invocation 4: Set save_temps_flag and related dump variables */
    printf("--- Invocation 4: Setting save_temps and dump variables ---\n");
    snprintf(cmd, sizeof(cmd), "gcc -save-temps=cwd -fdump-rtl-all -fdump-tree-all "
             "-dumpdir dumpdir_test -dumpbase dumpbase_test "
             "-dumpbase-ext .ext -o %s %s", 
             output_exe, source_file);
    execute_command(cmd);
    printf("\n");
    
    /* Invocation 5: Set use_ld, sysroot, and time report (may fail gracefully) */
    printf("--- Invocation 5: Setting use_ld, sysroot, time report ---\n");
    snprintf(cmd, sizeof(cmd), "gcc -fuse-ld=gold --sysroot=/  -ftime-report "
             "-o %s_2 %s 2>&1 | head -20", 
             output_exe, source_file);
    execute_command(cmd);
    printf("\n");
    
    /* Invocation 6: Cause failure to set greatest_status != 1 */
    printf("--- Invocation 6: Causing failure (greatest_status != 1) ---\n");
    snprintf(cmd, sizeof(cmd), "gcc -c %s -o %s 2>&1 | head -5", 
             nonexistent_file, output_obj);
    int fail_status = execute_command(cmd);
    printf("Failure exit status: %d\n\n", fail_status);
    
    /* Invocation 7: Successful compilation after failure (should reset) */
    printf("--- Invocation 7: Successful compilation after failure ---\n");
    snprintf(cmd, sizeof(cmd), "gcc -c %s -o %s", 
             source_file, output_obj);
    execute_command(cmd);
    printf("\n");
    
    /* Invocation 8: Try to set spec_machine (target-specific) */
    printf("--- Invocation 8: Attempting to set spec_machine ---\n");
    /* Try various target/machine options */
    const char *machine_opts[] = {
        "-march=x86-64",
        "-mtune=generic",
        "-mcpu=generic",
        "-m32",  /* Change machine width */
        "-m64",
    };
    
    for (size_t i = 0; i < sizeof(machine_opts)/sizeof(machine_opts[0]); i++) {
        snprintf(cmd, sizeof(cmd), "gcc %s -c %s -o %s_machine%d.o 2>&1 | head -3",
                 machine_opts[i], source_file, base_name, (int)i);
        execute_command(cmd);
    }
    printf("\n");
    
    /* Invocation 9: Complex combination of many flags */
    printf("--- Invocation 9: Complex combination of flags ---\n");
    snprintf(cmd, sizeof(cmd), "gcc -v -save-temps=obj -ftime-report-details "
             "-fuse-ld=bfd --sysroot=/usr -isysroot /usr/include "
             "-dumpdir complex_dump -dumpbase complex "
             "-o %s_complex %s 2>&1 | tail -10",
             output_exe, source_file);
    execute_command(cmd);
    printf("\n");
    
    /* Cleanup */
    printf("=== Cleaning up temporary files ===\n");
    unlink(source_file);
    unlink(response_file);
    unlink(output_obj);
    
    /* Try to remove generated files (may not exist if commands failed) */
    remove(output_exe);
    remove("dumpdir_test.*");
    remove("dumpbase_test.*");
    for (int i = 0; i < 5; i++) {
        char fname[256];
        snprintf(fname, sizeof(fname), "%s_machine%d.o", base_name, i);
        remove(fname);
    }
    remove("complex_dump.*");
    
    printf("\n=== Test completed ===\n");
    printf("The GCC driver's reset logic should have been exercised multiple times.\n");
    printf("Check coverage reports for lines 11228-11250 in gcc.cc\n");
    
    return 0;
}
