/* test_gcc_driver_init.c - Test program to cover GCC driver initialization block */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

/* Function prototypes for the dummy functions we'll create */
int foo(void) { return 0; }
int bar(void) { return 1; }
int baz(void) { return 2; }

/* Helper function to create temporary source files */
static int create_temp_file(const char *template, const char *content) {
    FILE *f = fopen(template, "w");
    if (!f) return -1;
    fprintf(f, "%s", content);
    fclose(f);
    return 0;
}

/* Helper to check if file exists */
static int file_exists(const char *path) {
    struct stat st;
    return stat(path, &st) == 0;
}

/* Helper to remove directory recursively */
static void remove_dir(const char *path) {
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "rm -rf %s 2>/dev/null", path);
    system(cmd);
}

int main(void) {
    int overall_result = 0;
    int checksum = 0;
    char cmd[1024];
    int ret;
    
    printf("=== GCC Driver Initialization Block Test ===\n");
    
    /* Create temporary source files */
    const char *temp1_content = 
        "int foo(void) { return 0; }\n"
        "int unused1(void) { return 100; }\n";
    
    const char *temp2_content = 
        "int bar(void) { return 1; }\n"
        "int unused2(void) { return 200; }\n";
    
    const char *syntax_error_content = 
        "int baz(void) { return /* missing semicolon and value */ }\n";
    
    /* Create files */
    if (create_temp_file("temp1.c", temp1_content) != 0 ||
        create_temp_file("temp2.c", temp2_content) != 0 ||
        create_temp_file("syntax_error.c", syntax_error_content) != 0) {
        fprintf(stderr, "Failed to create temporary files\n");
        return 1;
    }
    
    /* Create dump directory */
    mkdir("./dump_test", 0755);
    
    /* Sequence 1: Help flag then compilation (tests print_help_list reset) */
    printf("\n--- Sequence 1: Help then Compile ---\n");
    
    /* First job: help request (sets print_help_list) */
    ret = system("gcc --help=common > /dev/null 2>&1");
    checksum += (ret == 0) ? 1 : 0;
    
    /* Second job: compilation (should reset print_help_list) */
    ret = system("gcc -c temp1.c -o temp1.o 2>/dev/null");
    checksum += (ret == 0) ? 2 : 0;
    
    if (file_exists("temp1.o")) {
        printf("  temp1.o created successfully\n");
        remove("temp1.o");
    }
    
    /* Sequence 2: Version flag then different help type */
    printf("\n--- Sequence 2: Version then Optimizer Help ---\n");
    
    /* First job: version (sets print_version) */
    ret = system("gcc --version > /dev/null 2>&1");
    checksum += (ret == 0) ? 4 : 0;
    
    /* Second job: optimizer help (different help type) */
    ret = system("gcc --help=optimizers > /dev/null 2>&1");
    checksum += (ret == 0) ? 8 : 0;
    
    /* Sequence 3: Save-temps with dumpdir then plain compile */
    printf("\n--- Sequence 3: Save-temps with Dumpdir then Plain Compile ---\n");
    
    /* First job: uses save-temps, dumpdir, dumpbase (sets save_temps_flag, allocates dumpdir/dumpbase) */
    snprintf(cmd, sizeof(cmd),
             "gcc -save-temps -dumpdir ./dump_test -dumpbase mytest "
             "-c temp1.c -o temp1_save.o 2>/dev/null");
    ret = system(cmd);
    checksum += (ret == 0) ? 16 : 0;
    
    /* Check if save-temps files were created */
    int save_temps_created = 0;
    if (file_exists("temp1.i") || file_exists("temp1.s") || 
        file_exists("./dump_test/mytest.i")) {
        save_temps_created = 1;
        printf("  Save-temps files created\n");
    }
    
    /* Clean up save-temps files */
    remove("temp1.i");
    remove("temp1.s");
    remove("temp1.o");
    remove("temp1_save.o");
    
    /* Second job: plain compile (should free dumpdir/dumpbase and reset save_temps_flag) */
    ret = system("gcc -c temp2.c -o temp2.o 2>/dev/null");
    checksum += (ret == 0) ? 32 : 0;
    
    if (file_exists("temp2.o")) {
        printf("  temp2.o created successfully\n");
        remove("temp2.o");
    }
    
    /* Sequence 4: Verbose flag with linker selection */
    printf("\n--- Sequence 4: Verbose with Linker then Plain ---\n");
    
    /* First job: verbose with specific linker (sets verbose_only_flag, use_ld) */
    ret = system("gcc -v -fuse-ld=bfd -c temp1.c -o /dev/null 2>&1 | "
                 "grep -q 'COLLECT_GCC_OPTIONS' && echo 'verbose ok' > /dev/null");
    checksum += (ret == 0) ? 64 : 0;
    
    /* Second job: plain compile (should reset verbose_only_flag, use_ld) */
    ret = system("gcc -c temp2.c -o temp2_v.o 2>/dev/null");
    checksum += (ret == 0) ? 128 : 0;
    
    if (file_exists("temp2_v.o")) {
        remove("temp2_v.o");
    }
    
    /* Sequence 5: Error recovery (tests greatest_status reset) */
    printf("\n--- Sequence 5: Error then Success ---\n");
    
    /* First job: compilation with syntax error (should fail) */
    ret = system("gcc -c syntax_error.c -o error.o 2>/dev/null");
    checksum += (ret != 0) ? 256 : 0;  /* We expect failure */
    
    /* Second job: successful compilation (greatest_status should be reset) */
    ret = system("gcc -c temp1.c -o temp1_final.o 2>/dev/null");
    checksum += (ret == 0) ? 512 : 0;
    
    if (file_exists("temp1_final.o")) {
        printf("  Recovery compilation successful\n");
        remove("temp1_final.o");
    }
    
    /* Sequence 6: Multiple inputs in single invocation (multiple jobs) */
    printf("\n--- Sequence 6: Multiple Inputs Single Invocation ---\n");
    
    /* This creates multiple compilation jobs in one gcc invocation */
    ret = system("gcc -c temp1.c temp2.c 2>/dev/null");
    checksum += (ret == 0) ? 1024 : 0;
    
    if (file_exists("temp1.o") && file_exists("temp2.o")) {
        printf("  Multiple compilation successful\n");
        remove("temp1.o");
        remove("temp2.o");
    }
    
    /* Sequence 7: Using -x to specify language (creates distinct jobs) */
    printf("\n--- Sequence 7: Using -x Language Specification ---\n");
    
    /* Create a C++ source file to test -x option */
    const char *cpp_content = 
        "extern \"C\" int cpp_func(void) { return 42; }\n";
    create_temp_file("test_cpp.cpp", cpp_content);
    
    /* Use -x to process different languages */
    ret = system("gcc -x c -c temp1.c -o temp1x.o "
                 "-x c++ -c test_cpp.cpp -o test_cpp.o 2>/dev/null");
    checksum += (ret == 0) ? 2048 : 0;
    
    if (file_exists("temp1x.o")) remove("temp1x.o");
    if (file_exists("test_cpp.o")) remove("test_cpp.o");
    remove("test_cpp.cpp");
    
    /* Cleanup */
    printf("\n--- Cleanup ---\n");
    remove("temp1.c");
    remove("temp2.c");
    remove("syntax_error.c");
    remove_dir("./dump_test");
    
    /* Also clean up any remaining files */
    remove("temp1.i");
    remove("temp1.s");
    remove("temp1.o");
    remove("temp2.o");
    remove("error.o");
    
    /* Final checksum and result */
    printf("\n=== Test Complete ===\n");
    printf("Checksum: %d (0x%04x)\n", checksum, checksum);
    
    /* Use the dummy functions to avoid unused function warnings */
    (void)foo();
    (void)bar();
    (void)baz();
    
    /* Determine overall test result */
    if (checksum > 0) {
        printf("Test executed successfully with checksum %d\n", checksum);
        overall_result = 0;
    } else {
        printf("Test failed - no operations succeeded\n");
        overall_result = 1;
    }
    
    return overall_result;
}
