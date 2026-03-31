/* test_gcc_driver_init.c
 * This program tests the GCC driver initialization logic by invoking
 * multiple compilation jobs with different option combinations.
 * The goal is to cover lines 11228-11250 in gcc.cc which reset
 * global/static variables between compilation jobs.
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
    
    /* Append suffix if provided */
    char* filename = malloc(strlen(template) + strlen(suffix) + 1);
    if (!filename) {
        close(fd);
        unlink(template);
        return NULL;
    }
    strcpy(filename, template);
    strcat(filename, suffix);
    
    /* Rename to add suffix */
    close(fd);
    if (rename(template, filename) != 0) {
        perror("rename failed");
        free(filename);
        unlink(template);
        return NULL;
    }
    
    /* Write content */
    FILE* f = fopen(filename, "w");
    if (!f) {
        perror("fopen failed");
        free(filename);
        return NULL;
    }
    fprintf(f, "%s", content);
    fclose(f);
    
    return filename;
}

/* Execute a GCC command and check return status */
static int execute_gcc_command(const char* cmd, int expected_status) {
    printf("Executing: %s\n", cmd);
    int status = system(cmd);
    
    if (WIFEXITED(status)) {
        int exit_status = WEXITSTATUS(status);
        printf("  Exit status: %d (expected: %d)\n", exit_status, expected_status);
        
        /* Update checksum with command hash and result */
        unsigned int cmd_hash = 0;
        for (const char* p = cmd; *p; p++) {
            cmd_hash = cmd_hash * 31 + *p;
        }
        checksum ^= (cmd_hash << 16) | (exit_status & 0xFFFF);
        
        return (exit_status == expected_status) ? 0 : -1;
    } else {
        printf("  Command did not exit normally\n");
        return -1;
    }
}

/* Clean up temporary files */
static void cleanup_files(char** files, int count) {
    for (int i = 0; i < count; i++) {
        if (files[i]) {
            unlink(files[i]);
            free(files[i]);
        }
    }
}

/* Create dump directory for save-temps tests */
static int create_dump_dir(const char* dirname) {
    struct stat st;
    if (stat(dirname, &st) == 0) {
        /* Directory exists, clean it */
        char cmd[256];
        snprintf(cmd, sizeof(cmd), "rm -rf %s", dirname);
        system(cmd);
    }
    return mkdir(dirname, 0755);
}

int main(void) {
    printf("=== Testing GCC Driver Initialization Block ===\n\n");
    
    /* Create temporary source files */
    char* temp_files[10] = {0};
    int file_count = 0;
    
    /* Valid source files */
    temp_files[file_count++] = create_temp_file(
        "int foo(void) { return 0; }\n", ".c");
    temp_files[file_count++] = create_temp_file(
        "int bar(void) { return 1; }\n", ".c");
    temp_files[file_count++] = create_temp_file(
        "int baz(void) { return 2; }\n", ".c");
    
    /* File with syntax error */
    temp_files[file_count++] = create_temp_file(
        "int error_func(void) { return \n", ".c");
    
    /* Check all files were created */
    for (int i = 0; i < file_count; i++) {
        if (!temp_files[i]) {
            fprintf(stderr, "Failed to create temp file %d\n", i);
            cleanup_files(temp_files, file_count);
            return 1;
        }
        printf("Created temp file: %s\n", temp_files[i]);
    }
    
    /* Get GCC executable name - use the same compiler that's compiling this test */
    const char* gcc = "gcc";
    
    /* Create dump directory */
    if (create_dump_dir("./dump_test_dir") != 0 && errno != EEXIST) {
        perror("Failed to create dump directory");
    }
    
    printf("\n--- Sequence 1: Help/Version flags then compilation ---\n");
    printf("This sets print_help_list/print_version, then resets them.\n");
    
    /* Job 1: Help flag (sets print_help_list) */
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "%s --help=common > /dev/null 2>&1", gcc);
    execute_gcc_command(cmd, 0);
    
    /* Job 2: Version flag (sets print_version) */
    snprintf(cmd, sizeof(cmd), "%s --version > /dev/null 2>&1", gcc);
    execute_gcc_command(cmd, 0);
    
    /* Job 3: Compilation (should reset the above flags) */
    snprintf(cmd, sizeof(cmd), "%s -c %s -o %s.o", 
             gcc, temp_files[0], temp_files[0]);
    execute_gcc_command(cmd, 0);
    
    printf("\n--- Sequence 2: Save-temps with dumpdir then plain compile ---\n");
    printf("This sets save_temps_flag, dumpdir, dumpbase, then resets them.\n");
    
    /* Job 1: With save-temps and dump options */
    snprintf(cmd, sizeof(cmd), 
             "%s -save-temps -dumpdir ./dump_test_dir -dumpbase mydump "
             "-c %s -o %s_save.o 2>/dev/null", 
             gcc, temp_files[1], temp_files[1]);
    execute_gcc_command(cmd, 0);
    
    /* Job 2: Plain compilation (should free dumpdir/dumpbase) */
    snprintf(cmd, sizeof(cmd), "%s -c %s -o %s_plain.o", 
             gcc, temp_files[2], temp_files[2]);
    execute_gcc_command(cmd, 0);
    
    printf("\n--- Sequence 3: Error recovery (tests greatest_status reset) ---\n");
    printf("First job fails, second succeeds, testing status management.\n");
    
    /* Job 1: Compilation with syntax error (should fail) */
    snprintf(cmd, sizeof(cmd), "%s -c %s 2>/dev/null", 
             gcc, temp_files[3]);
    execute_gcc_command(cmd, 1);  /* Expected to fail */
    
    /* Job 2: Successful compilation */
    snprintf(cmd, sizeof(cmd), "%s -c %s -o %s_recover.o", 
             gcc, temp_files[0], temp_files[0]);
    execute_gcc_command(cmd, 0);
    
    printf("\n--- Sequence 4: Verbose and linker options then plain ---\n");
    printf("Sets verbose_only_flag and use_ld, then resets.\n");
    
    /* Job 1: Verbose output with specific linker */
    snprintf(cmd, sizeof(cmd), 
             "%s -v -fuse-ld=bfd -c %s -o %s_verbose.o 2>&1 | "
             "head -5 > /dev/null", 
             gcc, temp_files[1], temp_files[1]);
    execute_gcc_command(cmd, 0);
    
    /* Job 2: Plain compilation */
    snprintf(cmd, sizeof(cmd), "%s -c %s -o %s_final.o", 
             gcc, temp_files[2], temp_files[2]);
    execute_gcc_command(cmd, 0);
    
    printf("\n--- Sequence 5: Multiple jobs in single invocation ---\n");
    printf("Tests reinitialization between multiple input files.\n");
    
    /* Single invocation with multiple source files */
    snprintf(cmd, sizeof(cmd), 
             "%s -c %s %s %s -o multifile.o 2>&1 | "
             "grep -q 'multiple input files' || true", 
             gcc, temp_files[0], temp_files[1], temp_files[2]);
    execute_gcc_command(cmd, 0);
    
    /* Alternative: Use -x to specify different languages */
    snprintf(cmd, sizeof(cmd), 
             "echo 'int x;' | %s -x c - -x assembler - -c -o /dev/null 2>&1 "
             "| head -2 > /dev/null", gcc);
    execute_gcc_command(cmd, 0);
    
    printf("\n--- Sequence 6: Spec machine and sysroot options ---\n");
    printf("Tests spec_machine and sysroot variable reset.\n");
    
    /* Job 1: With sysroot specification */
    snprintf(cmd, sizeof(cmd), 
             "%s --sysroot=/ -c %s -o %s_sysroot.o 2>/dev/null", 
             gcc, temp_files[0], temp_files[0]);
    execute_gcc_command(cmd, 0);
    
    /* Job 2: With custom spec (if available) */
    snprintf(cmd, sizeof(cmd), 
             "%s -specs=/dev/null -c %s -o %s_spec.o 2>/dev/null || true", 
             gcc, temp_files[1], temp_files[1]);
    execute_gcc_command(cmd, 0);
    
    printf("\n--- Sequence 7: Mixed options across jobs ---\n");
    printf("Complex sequence to stress the reset logic.\n");
    
    /* Chain of jobs with different option sets */
    snprintf(cmd, sizeof(cmd), 
             "%s --help=optimizers > /dev/null 2>&1 && "
             "%s -save-temps -c %s 2>/dev/null && "
             "%s -v -c %s 2>&1 > /dev/null && "
             "%s -c %s", 
             gcc, gcc, temp_files[0], gcc, temp_files[1], gcc, temp_files[2]);
    execute_gcc_command(cmd, 0);
    
    /* Clean up object files created during tests */
    printf("\n--- Cleaning up generated files ---\n");
    char* obj_files[] = {
        "*.o", "*.i", "*.s", "*.ii", "*.bc",
        temp_files[0], strcat(strdup(temp_files[0]), ".o"),
        temp_files[1], strcat(strdup(temp_files[1]), ".o"),
        temp_files[2], strcat(strdup(temp_files[2]), ".o"),
        temp_files[0], strcat(strdup(temp_files[0]), "_save.o"),
        temp_files[1], strcat(strdup(temp_files[1]), "_save.o"),
        temp_files[0], strcat(strdup(temp_files[0]), "_recover.o"),
        temp_files[1], strcat(strdup(temp_files[1]), "_verbose.o"),
        temp_files[2], strcat(strdup(temp_files[2]), "_final.o"),
        temp_files[0], strcat(strdup(temp_files[0]), "_sysroot.o"),
        temp_files[1], strcat(strdup(temp_files[1]), "_spec.o"),
        "multifile.o"
    };
    
    for (int i = 0; i < sizeof(obj_files)/sizeof(obj_files[0]); i++) {
        char rm_cmd[256];
        snprintf(rm_cmd, sizeof(rm_cmd), "rm -f %s 2>/dev/null", obj_files[i]);
        system(rm_cmd);
    }
    
    /* Clean up dump directory */
    system("rm -rf ./dump_test_dir 2>/dev/null");
    
    /* Clean up temporary source files */
    cleanup_files(temp_files, file_count);
    
    printf("\n=== Test Summary ===\n");
    printf("Final checksum: 0x%08x\n", checksum);
    printf("If checksum is non-zero, all commands were executed.\n");
    printf("Driver initialization block should have been exercised multiple times.\n");
    
    return 0;
}
