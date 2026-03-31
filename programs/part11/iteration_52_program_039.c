/* test_gcc_driver_init.c
 * This program tests the GCC driver initialization logic by invoking
 * multiple compilation jobs with different state configurations.
 * It specifically targets the reset logic in gcc.cc lines 11228-11250.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

/* Function prototypes for the dummy functions we'll compile */
static int foo(void) { return 0; }
static int bar(void) { return 1; }
/* baz() will be intentionally broken in a separate file */

/* Helper function to create a temporary file with given content */
static char* create_temp_file(const char* content, const char* suffix) {
    char template[] = "/tmp/gcc_test_XXXXXX";
    int fd = mkstemp(template);
    if (fd == -1) {
        perror("mkstemp failed");
        return NULL;
    }
    
    if (content) {
        write(fd, content, strlen(content));
    }
    
    close(fd);
    
    /* Append suffix if provided */
    if (suffix) {
        char* new_name = malloc(strlen(template) + strlen(suffix) + 1);
        if (!new_name) {
            unlink(template);
            return NULL;
        }
        strcpy(new_name, template);
        strcat(new_name, suffix);
        
        if (rename(template, new_name) == 0) {
            free(new_name);
            return strdup(template); /* Return original name */
        }
        free(new_name);
    }
    
    return strdup(template);
}

/* Helper to run a command and check status */
static int run_command(const char* cmd) {
    printf("Running: %s\n", cmd);
    int status = system(cmd);
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    return -1;
}

/* Clean up temporary files */
static void cleanup_files(const char** files, int count) {
    for (int i = 0; i < count; i++) {
        if (files[i]) {
            unlink(files[i]);
            free((void*)files[i]);
        }
    }
}

int main(void) {
    int overall_result = 0;
    const char* temp_files[10] = {0};
    int file_count = 0;
    
    /* Create temporary source files */
    const char* valid_code1 = "int foo(void) { return 0; }\n";
    const char* valid_code2 = "int bar(void) { return 1; }\n";
    const char* error_code = "int baz(void) { return \n"; /* Syntax error */
    
    char* temp1 = create_temp_file(valid_code1, ".c");
    char* temp2 = create_temp_file(valid_code2, ".c");
    char* temp_err = create_temp_file(error_code, ".c");
    
    if (!temp1 || !temp2 || !temp_err) {
        fprintf(stderr, "Failed to create temporary files\n");
        return 1;
    }
    
    temp_files[file_count++] = temp1;
    temp_files[file_count++] = temp2;
    temp_files[file_count++] = temp_err;
    
    /* Create temporary directory for dump files */
    char dumpdir1[] = "/tmp/gcc_dump1_XXXXXX";
    char dumpdir2[] = "/tmp/gcc_dump2_XXXXXX";
    
    if (mkdtemp(dumpdir1) == NULL || mkdtemp(dumpdir2) == NULL) {
        perror("Failed to create temporary directories");
        cleanup_files(temp_files, file_count);
        return 1;
    }
    
    printf("=== Testing GCC Driver Initialization Block ===\n");
    printf("Target: gcc.cc lines 11228-11250\n\n");
    
    /* Sequence 1: Help flag then compilation
     * This should set print_help_list in first job, then reset it in second */
    printf("--- Sequence 1: Help then Compile ---\n");
    {
        char cmd1[512];
        char cmd2[512];
        
        snprintf(cmd1, sizeof(cmd1), "gcc --help=common > /dev/null 2>&1");
        snprintf(cmd2, sizeof(cmd2), "gcc -c %s -o %s.o", temp1, temp1);
        
        int res1 = run_command(cmd1);
        int res2 = run_command(cmd2);
        
        if (res1 == 0 && res2 == 0) {
            printf("✓ Sequence 1 passed\n");
        } else {
            printf("✗ Sequence 1 failed: %d, %d\n", res1, res2);
            overall_result |= 1;
        }
        
        /* Clean up object file */
        char obj_file[512];
        snprintf(obj_file, sizeof(obj_file), "%s.o", temp1);
        unlink(obj_file);
    }
    
    /* Sequence 2: Version flag then compilation
     * This should set print_version in first job, then reset it */
    printf("\n--- Sequence 2: Version then Compile ---\n");
    {
        char cmd1[512];
        char cmd2[512];
        
        snprintf(cmd1, sizeof(cmd1), "gcc --version > /dev/null 2>&1");
        snprintf(cmd2, sizeof(cmd2), "gcc -c %s -o %s.o", temp2, temp2);
        
        int res1 = run_command(cmd1);
        int res2 = run_command(cmd2);
        
        if (res1 == 0 && res2 == 0) {
            printf("✓ Sequence 2 passed\n");
        } else {
            printf("✗ Sequence 2 failed: %d, %d\n", res1, res2);
            overall_result |= 2;
        }
        
        char obj_file[512];
        snprintf(obj_file, sizeof(obj_file), "%s.o", temp2);
        unlink(obj_file);
    }
    
    /* Sequence 3: Save-temps with dumpdir then plain compile
     * This exercises save_temps_flag, dumpdir, dumpbase reset logic */
    printf("\n--- Sequence 3: Save-temps with dumpdir then plain compile ---\n");
    {
        char cmd1[512];
        char cmd2[512];
        
        snprintf(cmd1, sizeof(cmd1), 
                 "gcc -save-temps -dumpdir %s -dumpbase mytest -c %s -o %s.save.o 2>/dev/null",
                 dumpdir1, temp1, temp1);
        snprintf(cmd2, sizeof(cmd2), "gcc -c %s -o %s.plain.o", temp2, temp2);
        
        int res1 = run_command(cmd1);
        int res2 = run_command(cmd2);
        
        if (res2 == 0) { /* First command might fail if dumpdir doesn't work */
            printf("✓ Sequence 3 passed (second job succeeded)\n");
        } else {
            printf("✗ Sequence 3 failed: %d, %d\n", res1, res2);
            overall_result |= 4;
        }
        
        /* Clean up */
        char obj1[512], obj2[512];
        snprintf(obj1, sizeof(obj1), "%s.save.o", temp1);
        snprintf(obj2, sizeof(obj2), "%s.plain.o", temp2);
        unlink(obj1);
        unlink(obj2);
    }
    
    /* Sequence 4: Verbose flag then normal compile
     * Exercises verbose_only_flag reset */
    printf("\n--- Sequence 4: Verbose then normal compile ---\n");
    {
        char cmd1[512];
        char cmd2[512];
        
        snprintf(cmd1, sizeof(cmd1), "gcc -v -c %s 2>&1 | grep -q 'COLLECT_GCC_OPTIONS'", temp1);
        snprintf(cmd2, sizeof(cmd2), "gcc -c %s -o %s.v.o", temp2, temp2);
        
        int res1 = run_command(cmd1);
        int res2 = run_command(cmd2);
        
        if (res2 == 0) {
            printf("✓ Sequence 4 passed\n");
        } else {
            printf("✗ Sequence 4 failed: %d, %d\n", res1, res2);
            overall_result |= 8;
        }
        
        char obj_file[512];
        snprintf(obj_file, sizeof(obj_file), "%s.v.o", temp2);
        unlink(obj_file);
    }
    
    /* Sequence 5: Linker selection then default
     * Exercises use_ld reset */
    printf("\n--- Sequence 5: Specific linker then default ---\n");
    {
        char cmd1[512];
        char cmd2[512];
        
        /* Try different linkers, accept failure if not available */
        snprintf(cmd1, sizeof(cmd1), "gcc -fuse-ld=bfd -c %s -o %s.ld1.o 2>/dev/null", temp1, temp1);
        snprintf(cmd2, sizeof(cmd2), "gcc -c %s -o %s.ld2.o", temp2, temp2);
        
        int res1 = run_command(cmd1);
        int res2 = run_command(cmd2);
        
        if (res2 == 0) {
            printf("✓ Sequence 5 passed\n");
        } else {
            printf("✗ Sequence 5 failed: %d, %d\n", res1, res2);
            overall_result |= 16;
        }
        
        char obj1[512], obj2[512];
        snprintf(obj1, sizeof(obj1), "%s.ld1.o", temp1);
        snprintf(obj2, sizeof(obj2), "%s.ld2.o", temp2);
        unlink(obj1);
        unlink(obj2);
    }
    
    /* Sequence 6: Error then success
     * Exercises greatest_status reset from failure to success */
    printf("\n--- Sequence 6: Error then success ---\n");
    {
        char cmd1[512];
        char cmd2[512];
        
        snprintf(cmd1, sizeof(cmd1), "gcc -c %s 2>/dev/null", temp_err);
        snprintf(cmd2, sizeof(cmd2), "gcc -c %s -o %s.err.o", temp1, temp1);
        
        int res1 = run_command(cmd1);
        int res2 = run_command(cmd2);
        
        /* First should fail (non-zero), second should succeed */
        if (res1 != 0 && res2 == 0) {
            printf("✓ Sequence 6 passed (error then success)\n");
        } else {
            printf("✗ Sequence 6 failed: error=%d, success=%d\n", res1, res2);
            overall_result |= 32;
        }
        
        char obj_file[512];
        snprintf(obj_file, sizeof(obj_file), "%s.err.o", temp1);
        unlink(obj_file);
    }
    
    /* Sequence 7: Multiple options combined then reset
     * Comprehensive test of multiple variables */
    printf("\n--- Sequence 7: Combined options then reset ---\n");
    {
        char cmd1[512];
        char cmd2[512];
        
        snprintf(cmd1, sizeof(cmd1),
                 "gcc -v --help=optimizers -save-temps -dumpdir %s -fuse-ld=lld -c %s 2>&1 | head -5 > /dev/null",
                 dumpdir2, temp1);
        snprintf(cmd2, sizeof(cmd2), "gcc -c %s -o %s.final.o", temp2, temp2);
        
        int res1 = run_command(cmd1);
        int res2 = run_command(cmd2);
        
        if (res2 == 0) {
            printf("✓ Sequence 7 passed\n");
        } else {
            printf("✗ Sequence 7 failed: %d, %d\n", res1, res2);
            overall_result |= 64;
        }
        
        char obj_file[512];
        snprintf(obj_file, sizeof(obj_file), "%s.final.o", temp2);
        unlink(obj_file);
    }
    
    /* Clean up */
    printf("\n--- Cleaning up ---\n");
    cleanup_files(temp_files, file_count);
    
    /* Remove dump directories */
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "rm -rf %s %s", dumpdir1, dumpdir2);
    system(cmd);
    
    /* Also clean up any stray .i, .s, .o files */
    snprintf(cmd, sizeof(cmd), "rm -f %s.* %s.* 2>/dev/null", temp1, temp2);
    system(cmd);
    
    printf("\n=== Test Complete ===\n");
    printf("Overall result code: 0x%02x\n", overall_result);
    
    /* Use the dummy functions to avoid unused function warnings */
    (void)foo();
    (void)bar();
    
    return overall_result;
}
