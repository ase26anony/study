/* driver_coverage_test.c - Test program to cover GCC driver initialization logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

/* Temporary file management */
typedef struct {
    char *filename;
    char *content;
} temp_file_t;

static temp_file_t temp_files[] = {
    {"temp1.c", "int foo(void) { return 0; }\n"},
    {"temp2.c", "int bar(void) { return 1; }\n"},
    {"syntax_error.c", "int baz(void) { return /* missing semicolon and value */ }\n"},
    {"temp3.c", "int qux(void) { return 2; }\n"}
};

#define NUM_TEMP_FILES (sizeof(temp_files)/sizeof(temp_files[0]))

/* Create temporary files with given content */
static int create_temp_files(void) {
    for (size_t i = 0; i < NUM_TEMP_FILES; i++) {
        FILE *fp = fopen(temp_files[i].filename, "w");
        if (!fp) {
            perror("Failed to create temp file");
            return -1;
        }
        fputs(temp_files[i].content, fp);
        fclose(fp);
        printf("Created %s\n", temp_files[i].filename);
    }
    return 0;
}

/* Clean up temporary files */
static void cleanup_temp_files(void) {
    for (size_t i = 0; i < NUM_TEMP_FILES; i++) {
        remove(temp_files[i].filename);
    }
    /* Remove dump directory if created */
    system("rm -rf ./dump1 ./dump2 2>/dev/null");
}

/* Execute GCC command and check status */
static int execute_gcc(const char *cmd, int check_status) {
    printf("Executing: %s\n", cmd);
    int status = system(cmd);
    if (check_status && status != 0) {
        printf("Command failed with status %d\n", status);
    }
    return status;
}

/* Main test orchestration */
int main(void) {
    int overall_result = 0;
    int checksum = 0;
    
    printf("=== GCC Driver Initialization Coverage Test ===\n");
    
    /* Create temporary source files */
    if (create_temp_files() != 0) {
        fprintf(stderr, "Failed to create temp files\n");
        return 1;
    }
    
    /* Sequence 1: Help/Version flags then compilation
       This should set print_help_list/print_version, then reset them */
    printf("\n--- Sequence 1: Help then Compile ---\n");
    execute_gcc("gcc --help=common 2>&1 | head -5", 0);  /* Sets print_help_list */
    checksum += execute_gcc("gcc -c temp1.c -o temp1.o", 1) == 0 ? 1 : 0;
    
    /* Clean up object file */
    remove("temp1.o");
    
    /* Sequence 2: Save-temps with dumpdir then plain compile
       This exercises save_temps_flag, dumpdir, dumpbase reset logic */
    printf("\n--- Sequence 2: Save-temps with dumpdir then plain compile ---\n");
    execute_gcc("mkdir -p ./dump1", 0);
    execute_gcc("gcc -save-temps -dumpdir ./dump1 -dumpbase mytest -c temp2.c -o temp2.o 2>&1", 0);
    checksum += execute_gcc("gcc -c temp3.c -o temp3.o", 1) == 0 ? 2 : 0;
    
    /* Clean up */
    remove("temp2.o");
    remove("temp3.o");
    remove("temp2.i");
    remove("temp2.s");
    remove("temp2.o");
    
    /* Sequence 3: Error then success - exercises greatest_status reset */
    printf("\n--- Sequence 3: Error then Success ---\n");
    execute_gcc("gcc -c syntax_error.c 2>/dev/null", 0);  /* Should fail */
    checksum += execute_gcc("gcc -c temp1.c -o temp1.o 2>&1", 1) == 0 ? 4 : 0;
    
    remove("temp1.o");
    
    /* Sequence 4: Verbose and linker selection then plain compile
       Exercises verbose_only_flag and use_ld reset */
    printf("\n--- Sequence 4: Verbose with linker selection then plain ---\n");
    execute_gcc("gcc -v -fuse-ld=bfd -c temp1.c 2>&1 | grep -q 'COLLECT_GCC' && echo 'Found COLLECT_GCC'", 0);
    checksum += execute_gcc("gcc -c temp2.c -o temp2.o", 1) == 0 ? 8 : 0;
    
    remove("temp2.o");
    
    /* Sequence 5: Multiple inputs in single invocation (multiple jobs)
       This should trigger initialization between internal jobs */
    printf("\n--- Sequence 5: Multiple inputs in single invocation ---\n");
    execute_gcc("gcc -c temp1.c temp2.c temp3.c 2>&1", 0);
    checksum += execute_gcc("gcc --version 2>&1 | head -1", 1) == 0 ? 16 : 0;
    
    remove("temp1.o");
    remove("temp2.o");
    remove("temp3.o");
    
    /* Sequence 6: Using -x to specify language (creates distinct jobs) */
    printf("\n--- Sequence 6: Using -x for language specification ---\n");
    execute_gcc("echo 'int x=1;' | gcc -x c - -c -o temp1.o", 0);
    execute_gcc("echo 'int y=2;' | gcc -x c - -c -o temp2.o", 0);
    checksum += execute_gcc("gcc temp1.o temp2.o -o combined 2>&1", 1) == 0 ? 32 : 0;
    
    remove("temp1.o");
    remove("temp2.o");
    remove("combined");
    
    /* Sequence 7: Target system root simulation */
    printf("\n--- Sequence 7: Target system root options ---\n");
    execute_gcc("gcc --sysroot=/ -c temp1.c 2>&1", 0);
    checksum += execute_gcc("gcc -c temp2.c", 1) == 0 ? 64 : 0;
    
    /* Final cleanup */
    cleanup_temp_files();
    
    /* Report results */
    printf("\n=== Test Results ===\n");
    printf("Checksum (bitmask of successful sequences): %d\n", checksum);
    printf("Binary: ");
    for (int i = 7; i >= 0; i--) {
        printf("%d", (checksum >> i) & 1);
    }
    printf("\n");
    
    /* All sequences should succeed except those designed to fail */
    if (checksum >= 127) {  /* Most bits set (except possibly sequence 3) */
        printf("SUCCESS: Driver initialization logic likely exercised\n");
        overall_result = 0;
    } else {
        printf("PARTIAL: Some sequences failed\n");
        overall_result = 1;
    }
    
    return overall_result;
}

/* Prevent unused function warnings */
void __attribute__((unused)) reference_functions(void) {
    (void)foo(); (void)bar(); (void)qux();
}
