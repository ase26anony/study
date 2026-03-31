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
        /* Options that set various flags in the driver */
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
    snprintf(output_file, sizeof(output_file), "%s/output_%d.o", tmpdir, pid);
    
    /* Create test files */
    create_source_file(source_file);
    create_response_file(response_file);
    
    printf("=== Testing GCC Driver Reset Logic ===\n\n");
    
    /* Invocation 1: Print help list (sets print_help_list) */
    execute_command("gcc -print-help-list 2>&1");
    
    /* Invocation 2: Version (sets print_version) */
    execute_command("gcc --version 2>&1");
    
    /* Invocation 3: Verbose only (sets verbose_only_flag) */
    execute_command("gcc -v 2>&1");
    
    /* Invocation 4: Use response file (sets at_file_supplied) */
    snprintf(cmd, sizeof(cmd), "gcc @%s -c %s -o %s 2>&1", 
             response_file, source_file, output_file);
    execute_command(cmd);
    
    /* Invocation 5: Save temps with different options (sets save_temps_flag, dumpdir, etc.) */
    execute_command("gcc -save-temps -c test_nonexistent.c 2>&1");  /* This will fail */
    
    /* Invocation 6: Set sysroot and related variables */
    execute_command("gcc --sysroot=/usr -c test_nonexistent.c 2>&1");  /* This will fail */
    
    /* Invocation 7: Use specific linker (sets use_ld) */
    snprintf(cmd, sizeof(cmd), "gcc -fuse-ld=gold -c %s -o %s.2 2>&1", 
             source_file, output_file);
    execute_command(cmd);
    
    /* Invocation 8: Time report to file (sets report_times_to_file) */
    snprintf(cmd, sizeof(cmd), "gcc -ftime-report -c %s -o %s.3 2>&1", 
             source_file, output_file);
    execute_command(cmd);
    
    /* Invocation 9: Machine specification (attempts to set spec_machine) */
    /* Note: -machine option may not exist on all targets, but driver will process it */
    snprintf(cmd, sizeof(cmd), "gcc -machine=x86_64 -c %s -o %s.4 2>&1", 
             source_file, output_file);
    execute_command(cmd);
    
    /* Invocation 10: Multiple options combined */
    snprintf(cmd, sizeof(cmd), "gcc -v -save-temps=cwd -ftime-report -fuse-ld=bfd -c %s -o %s.5 2>&1", 
             source_file, output_file);
    execute_command(cmd);
    
    /* Invocation 11: With dumpbase and related options */
    snprintf(cmd, sizeof(cmd), "gcc -dumpbase mytest -dumpdir ./dumps -c %s -o %s.6 2>&1", 
             source_file, output_file);
    execute_command(cmd);
    
    /* Invocation 12: Successful compilation to reset after failures */
    snprintf(cmd, sizeof(cmd), "gcc -c %s -o %s.final 2>&1", source_file, output_file);
    execute_command(cmd);
    
    /* Cleanup */
    unlink(source_file);
    unlink(response_file);
    
    /* Clean up output files if they exist */
    char cleanup[256];
    snprintf(cleanup, sizeof(cleanup), "rm -f %s* 2>/dev/null", output_file);
    system(cleanup);
    
    printf("=== Test completed ===\n");
    return 0;
}
