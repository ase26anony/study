#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>

#define MAX_CMD_LEN 1024

/* Create a minimal C source file */
void create_source_file(const char *filename) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        perror("fopen source");
        exit(1);
    }
    fprintf(f, "int main() { return 0; }\n");
    fclose(f);
}

/* Create a response file with various options */
void create_response_file(const char *filename) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        perror("fopen response");
        exit(1);
    }
    fprintf(f, "-v\n");
    fprintf(f, "-save-temps=obj\n");
    fprintf(f, "-ftime-report\n");
    fclose(f);
}

/* Execute a GCC command and return exit status */
int execute_gcc(const char *cmd) {
    printf("Executing: %s\n", cmd);
    
    int status = system(cmd);
    if (status == -1) {
        perror("system");
        return -1;
    }
    
    if (WIFEXITED(status)) {
        printf("Exit status: %d\n\n", WEXITSTATUS(status));
        return WEXITSTATUS(status);
    } else {
        printf("Command terminated abnormally\n\n");
        return -1;
    }
}

int main(int argc, char **argv) {
    char cmd[MAX_CMD_LEN];
    char tmpdir[] = "/tmp/gcc_test_XXXXXX";
    char source_file[256];
    char response_file[256];
    char output_file[256];
    char object_file[256];
    
    /* Create temporary directory */
    if (!mkdtemp(tmpdir)) {
        perror("mkdtemp");
        return 1;
    }
    
    /* Create file paths */
    snprintf(source_file, sizeof(source_file), "%s/test.c", tmpdir);
    snprintf(response_file, sizeof(response_file), "%s/args.rsp", tmpdir);
    snprintf(output_file, sizeof(output_file), "%s/test.exe", tmpdir);
    snprintf(object_file, sizeof(object_file), "%s/test.o", tmpdir);
    
    /* Create test files */
    create_source_file(source_file);
    create_response_file(response_file);
    
    printf("=== Testing GCC Driver Reset Logic ===\n\n");
    
    /* Invocation 1: Print help list (sets print_help_list) */
    execute_gcc("gcc -print-help-list");
    
    /* Invocation 2: Version flag (sets print_version) */
    execute_gcc("gcc --version");
    
    /* Invocation 3: Verbose only (sets verbose_only_flag) */
    execute_gcc("gcc -v");
    
    /* Invocation 4: Save temps with different modes (sets save_temps_flag, dumpdir, etc.) */
    execute_gcc("gcc -save-temps -c test.c -o /tmp/dummy.o 2>/dev/null");
    execute_gcc("gcc -save-temps=cwd -c test.c -o /tmp/dummy.o 2>/dev/null");
    execute_gcc("gcc -save-temps=obj -c test.c -o /tmp/dummy.o 2>/dev/null");
    
    /* Invocation 5: With response file (sets at_file_supplied) */
    snprintf(cmd, sizeof(cmd), "gcc @%s -c %s -o %s", response_file, source_file, object_file);
    execute_gcc(cmd);
    
    /* Invocation 6: Set sysroot (affects target_system_root) */
    execute_gcc("gcc --sysroot=/usr -c test.c -o /tmp/dummy.o 2>/dev/null");
    execute_gcc("gcc -isysroot /usr -c test.c -o /tmp/dummy.o 2>/dev/null");
    
    /* Invocation 7: Set linker (sets use_ld) */
    execute_gcc("gcc -fuse-ld=bfd -c test.c -o /tmp/dummy.o 2>/dev/null");
    execute_gcc("gcc -fuse-ld=gold -c test.c -o /tmp/dummy.o 2>/dev/null");
    
    /* Invocation 8: Time report (sets report_times_to_file) */
    snprintf(cmd, sizeof(cmd), "gcc -ftime-report -c %s -o %s", source_file, object_file);
    execute_gcc(cmd);
    
    /* Invocation 9: Machine specification (sets spec_machine) */
    execute_gcc("gcc -machine= -c test.c -o /tmp/dummy.o 2>/dev/null");
    
    /* Invocation 10: Combined options */
    snprintf(cmd, sizeof(cmd), "gcc -v -save-temps=obj -ftime-report -fuse-ld=gold --sysroot=/ -c %s -o %s", 
             source_file, object_file);
    execute_gcc(cmd);
    
    /* Invocation 11: Force failure (sets greatest_status to non-1) */
    execute_gcc("gcc -invalid-option 2>/dev/null");
    execute_gcc("gcc non_existent_file.c 2>/dev/null");
    
    /* Invocation 12: Successful compilation after failure (tests reset) */
    snprintf(cmd, sizeof(cmd), "gcc -c %s -o %s", source_file, object_file);
    execute_gcc(cmd);
    
    /* Invocation 13: Full link with various options */
    snprintf(cmd, sizeof(cmd), "gcc %s -o %s -Wl,--verbose", source_file, output_file);
    execute_gcc(cmd);
    
    /* Invocation 14: Print subprocess help (sets print_subprocess_help) */
    execute_gcc("gcc -print-prog-name=cc1");
    execute_gcc("gcc -print-prog-name=as");
    execute_gcc("gcc -print-prog-name=ld");
    
    /* Invocation 15: Dump options */
    execute_gcc("gcc -dumpspecs 2>/dev/null");
    execute_gcc("gcc -dumpmachine");
    execute_gcc("gcc -dumpversion");
    
    /* Invocation 16: With dumpdir */
    snprintf(cmd, sizeof(cmd), "gcc -dumpdir=./dump -dumpbase=test -c %s -o %s", 
             source_file, object_file);
    execute_gcc(cmd);
    
    /* Invocation 17: Multiple source files */
    char source_file2[256];
    snprintf(source_file2, sizeof(source_file2), "%s/test2.c", tmpdir);
    FILE *f = fopen(source_file2, "w");
    if (f) {
        fprintf(f, "int foo() { return 42; }\n");
        fclose(f);
    }
    snprintf(cmd, sizeof(cmd), "gcc -c %s %s -o %s", source_file, source_file2, object_file);
    execute_gcc(cmd);
    
    /* Cleanup */
    snprintf(cmd, sizeof(cmd), "rm -rf %s", tmpdir);
    system(cmd);
    
    printf("=== Test completed ===\n");
    return 0;
}
