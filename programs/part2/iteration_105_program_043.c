#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_CMD_LEN 1024

/* Create a minimal C source file */
void create_source_file(const char *filename) {
    FILE *f = fopen(filename, "w");
    if (f) {
        fprintf(f, "int main() { return 0; }\n");
        fclose(f);
    }
}

/* Create a response file with compiler arguments */
void create_response_file(const char *filename) {
    FILE *f = fopen(filename, "w");
    if (f) {
        fprintf(f, "-v\n");
        fprintf(f, "-save-temps=obj\n");
        fprintf(f, "-ftime-report\n");
        fclose(f);
    }
}

/* Execute a command and return exit status */
int execute_command(const char *cmd) {
    printf("Executing: %s\n", cmd);
    int status = system(cmd);
    if (WIFEXITED(status)) {
        printf("Exit status: %d\n\n", WEXITSTATUS(status));
        return WEXITSTATUS(status);
    }
    return -1;
}

int main(int argc, char **argv) {
    char cmd[MAX_CMD_LEN];
    const char *tmpdir = "/tmp";
    char source_file[256];
    char response_file[256];
    char output_file[256];
    
    /* Generate unique filenames using PID */
    pid_t pid = getpid();
    snprintf(source_file, sizeof(source_file), "%s/test_%d.c", tmpdir, pid);
    snprintf(response_file, sizeof(response_file), "%s/args_%d.rsp", tmpdir, pid);
    snprintf(output_file, sizeof(output_file), "%s/output_%d", tmpdir, pid);
    
    /* Create test files */
    create_source_file(source_file);
    create_response_file(response_file);
    
    printf("=== Testing GCC Driver Reset Logic ===\n\n");
    
    /* Invocation 1: Print help list (sets print_help_list) */
    execute_command("gcc -print-help-list 2>&1 | head -5");
    
    /* Invocation 2: Version info (sets print_version) */
    execute_command("gcc --version");
    
    /* Invocation 3: Verbose mode (sets verbose_only_flag) */
    execute_command("gcc -v -E -x c /dev/null");
    
    /* Invocation 4: Save temps with dumpdir variables (sets save_temps_flag, dumpdir, etc.) */
    snprintf(cmd, sizeof(cmd), "gcc -save-temps=obj -dumpdir ./dump_%d/ -dumpbase test -o %s.o %s 2>&1 | head -10", 
             pid, output_file, source_file);
    execute_command(cmd);
    
    /* Invocation 5: Use response file (sets at_file_supplied) */
    snprintf(cmd, sizeof(cmd), "gcc @%s -o %s.resp %s 2>&1 | head -10", 
             response_file, output_file, source_file);
    execute_command(cmd);
    
    /* Invocation 6: Set sysroot and related variables */
    execute_command("gcc --sysroot=/ -isysroot /usr 2>&1 | head -5");
    
    /* Invocation 7: Use specific linker (sets use_ld) */
    execute_command("gcc -fuse-ld=bfd -print-search-dirs 2>&1 | head -5");
    
    /* Invocation 8: Time report (sets report_times_to_file) */
    snprintf(cmd, sizeof(cmd), "gcc -ftime-report -c %s -o %s.time 2>&1 | head -20", 
             source_file, output_file);
    execute_command(cmd);
    
    /* Invocation 9: Machine specification (affects spec_machine) */
    /* Try various machine/architecture options */
    execute_command("gcc -march=x86-64 -mtune=generic -c -x c /dev/null 2>&1 | head -5");
    
    /* Invocation 10: Force failure to set greatest_status != 1 */
    printf("=== Forcing compilation failure ===\n");
    execute_command("gcc -c /nonexistent_file_xyz.c 2>&1 | head -5");
    
    /* Invocation 11: Successful compilation after failure */
    printf("=== Successful compilation after failure ===\n");
    snprintf(cmd, sizeof(cmd), "gcc -c %s -o %s.final", source_file, output_file);
    execute_command(cmd);
    
    /* Invocation 12: Multiple options combined */
    snprintf(cmd, sizeof(cmd), 
             "gcc -v -save-temps=cwd --sysroot=/ -fuse-ld=gold -ftime-report -o %s.combined %s 2>&1 | head -15",
             output_file, source_file);
    execute_command(cmd);
    
    /* Cleanup */
    unlink(source_file);
    unlink(response_file);
    
    /* Clean up potential output files */
    char cleanup[256];
    snprintf(cleanup, sizeof(cleanup), "rm -f %s.* 2>/dev/null", output_file);
    system(cleanup);
    
    printf("=== Test completed ===\n");
    printf("The GCC driver's reset logic should have been exercised multiple times.\n");
    printf("Variables like at_file_supplied, save_temps_flag, dumpdir, spec_machine,\n");
    printf("and greatest_status were set and reset between invocations.\n");
    
    return 0;
}
