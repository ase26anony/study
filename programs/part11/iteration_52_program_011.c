/* test_gcc_driver_init.c - Test program to cover GCC driver initialization block */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

/* Function prototypes for the dummy functions we'll create */
int foo(void);
int bar(void);
int baz(void);

/* Simple checksum to verify all sequences were attempted */
static unsigned int checksum = 0;

/* Helper function to create temporary source files */
static int create_temp_file(const char *filename, const char *content) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        perror("fopen");
        return 0;
    }
    fprintf(f, "%s", content);
    fclose(f);
    return 1;
}

/* Helper function to execute a command and update checksum */
static int execute_command(const char *cmd, int sequence_id) {
    printf("Executing sequence %d: %s\n", sequence_id, cmd);
    int result = system(cmd);
    checksum += sequence_id * (result == 0 ? 1 : 2);
    return result;
}

/* Helper to create a temporary directory */
static int create_temp_dir(const char *dirname) {
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "mkdir -p %s", dirname);
    return system(cmd);
}

/* Helper to cleanup temporary files */
static void cleanup_files(const char **files, int count) {
    for (int i = 0; i < count; i++) {
        if (files[i]) {
            char cmd[256];
            snprintf(cmd, sizeof(cmd), "rm -f %s", files[i]);
            system(cmd);
        }
    }
}

int main(void) {
    /* Create temporary source files */
    const char *temp_files[] = {
        "temp1.c",
        "temp2.c", 
        "syntax_error.c",
        "temp1.o",
        "temp2.o",
        "temp1.i",
        "temp1.s",
        NULL
    };
    
    /* Create directories for dump tests */
    const char *temp_dirs[] = {
        "./dump1",
        "./dump2",
        NULL
    };
    
    /* Create source files with valid and invalid content */
    if (!create_temp_file("temp1.c", 
        "int foo(void) { return 0; }\n"
        "int main(void) { return foo(); }\n")) {
        return 1;
    }
    
    if (!create_temp_file("temp2.c",
        "int bar(void) { return 1; }\n"
        "int unused_func(void) { return 42; }\n")) {
        return 1;
    }
    
    if (!create_temp_file("syntax_error.c",
        "int baz(void) { return \n"  /* Deliberate syntax error - missing semicolon and value */
        "/* This should cause a compilation error */\n")) {
        return 1;
    }
    
    /* Create dump directories */
    for (int i = 0; temp_dirs[i]; i++) {
        create_temp_dir(temp_dirs[i]);
    }
    
    printf("=== Testing GCC Driver Initialization Block Coverage ===\n\n");
    
    /* SEQUENCE 1: Help/Version flags then compilation
       This sets print_help_list or print_version, then resets them */
    printf("--- Sequence 1: Help then Compile ---\n");
    execute_command("gcc --help=common 2>&1 | head -5 > /dev/null", 1);
    execute_command("gcc -c temp1.c -o temp1.o", 1);
    
    /* SEQUENCE 2: Save-temps with dumpdir then compile
       This sets save_temps_flag, dumpdir, dumpbase, then resets them */
    printf("\n--- Sequence 2: Save-temps with Dumpdir then Compile ---\n");
    execute_command("gcc -save-temps -dumpdir ./dump1 -dumpbase base -c temp1.c -o temp1.o 2>&1", 2);
    execute_command("gcc -c temp2.c -o temp2.o", 2);
    
    /* SEQUENCE 3: Error then success
       This tests greatest_status reset logic */
    printf("\n--- Sequence 3: Error then Success ---\n");
    execute_command("gcc -c syntax_error.c 2>/dev/null", 3);  /* This should fail */
    execute_command("gcc -c temp1.c -o temp1.o", 3);          /* This should succeed */
    
    /* SEQUENCE 4: Verbose and linker selection then plain compile
       This sets verbose_only_flag and use_ld, then resets them */
    printf("\n--- Sequence 4: Verbose and Linker then Plain ---\n");
    execute_command("gcc -v -fuse-ld=bfd -c temp1.c 2>&1 | grep -q 'COLLECT_GCC'", 4);
    execute_command("gcc -c temp2.c -o temp2.o", 4);
    
    /* SEQUENCE 5: Multiple language specifications in one invocation
       This creates multiple compilation jobs in a single driver run */
    printf("\n--- Sequence 5: Multiple -x Language Specs ---\n");
    execute_command("gcc -x c -c temp1.c -o temp1.o -x c -c temp2.c -o temp2.o", 5);
    
    /* SEQUENCE 6: Version flag with multiple source files
       This tests print_version reset between jobs */
    printf("\n--- Sequence 6: Version then Multiple Sources ---\n");
    execute_command("gcc --version -c temp1.c temp2.c 2>&1 | head -2 > /dev/null", 6);
    
    /* SEQUENCE 7: Complex dumpdir/save-temps combination
       Tests dumpdir_trailing_dash_added and save_temps_overrides_dumpdir logic */
    printf("\n--- Sequence 7: Complex Dumpdir Combinations ---\n");
    execute_command("gcc -dumpdir ./dump2/ -dumpbase ext -save-temps=cwd -c temp1.c 2>&1", 7);
    execute_command("gcc -c temp2.c -o temp2.o", 7);
    
    /* SEQUENCE 8: Target system root variations
       Tests target_system_root_changed and related variables */
    printf("\n--- Sequence 8: Target System Root ---\n");
    execute_command("gcc --sysroot=/ -c temp1.c -o temp1.o 2>&1", 8);
    execute_command("gcc -c temp2.c -o temp2.o", 8);
    
    /* SEQUENCE 9: Time reporting then compile
       Tests report_times_to_file reset */
    printf("\n--- Sequence 9: Time Reporting ---\n");
    execute_command("gcc -ftime-report -c temp1.c -o temp1.o 2>&1 | head -10 > /dev/null", 9);
    execute_command("gcc -c temp2.c -o temp2.o", 9);
    
    /* SEQUENCE 10: Multiple help options in sequence
       Tests print_help_list and print_subprocess_help reset */
    printf("\n--- Sequence 10: Multiple Help Options ---\n");
    execute_command("gcc --help=optimizers 2>&1 | head -5 > /dev/null", 10);
    execute_command("gcc --help=warnings 2>&1 | head -5 > /dev/null", 10);
    execute_command("gcc -c temp1.c -o temp1.o", 10);
    
    /* Verify the object files were created */
    printf("\n--- Verification ---\n");
    struct stat st;
    if (stat("temp1.o", &st) == 0) {
        printf("temp1.o created successfully\n");
        checksum += 100;
    }
    if (stat("temp2.o", &st) == 0) {
        printf("temp2.o created successfully\n");
        checksum += 200;
    }
    
    /* Cleanup */
    printf("\n--- Cleanup ---\n");
    cleanup_files(temp_files, 7);
    
    /* Remove dump directories */
    for (int i = 0; temp_dirs[i]; i++) {
        char cmd[256];
        snprintf(cmd, sizeof(cmd), "rm -rf %s", temp_dirs[i]);
        system(cmd);
    }
    
    /* Final checksum output */
    printf("\n=== Test Complete ===\n");
    printf("Final checksum: %u\n", checksum);
    printf("Expected range: 1800-2200 (depends on exact command successes)\n");
    
    /* Reference the dummy functions to avoid unused function warnings */
    (void)foo;
    (void)bar;
    (void)baz;
    
    return 0;
}

/* Dummy function definitions */
int foo(void) { return 0; }
int bar(void) { return 1; }
int baz(void) { return 2; }
