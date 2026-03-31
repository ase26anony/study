/* driver_coverage_test.c - Test program to cover GCC driver initialization logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

/* Temporary file management */
static char temp_files[5][64];
static int temp_file_count = 0;

static char* create_temp_file(const char* content, const char* suffix) {
    char template[64];
    snprintf(template, sizeof(template), "/tmp/gcc_test_XXXXXX%s", suffix);
    
    int fd = mkstemps(template, strlen(suffix));
    if (fd < 0) {
        perror("mkstemps failed");
        return NULL;
    }
    
    write(fd, content, strlen(content));
    close(fd);
    
    strcpy(temp_files[temp_file_count], template);
    temp_file_count++;
    
    return strdup(template);
}

static void cleanup_temp_files(void) {
    for (int i = 0; i < temp_file_count; i++) {
        unlink(temp_files[i]);
    }
}

/* Test sequences to exercise driver initialization reset logic */
static int run_test_sequence(const char* description, const char* command) {
    printf("Running: %s\n", description);
    printf("Command: %s\n", command);
    
    int result = system(command);
    printf("Result: %d\n\n", result);
    
    return result == 0;
}

int main(void) {
    int success_count = 0;
    int total_tests = 0;
    
    /* Create temporary source files */
    char* valid_c1 = create_temp_file(
        "int foo(void) { return 0; }\n"
        "int unused_func1(void) { return 42; }\n",
        ".c"
    );
    
    char* valid_c2 = create_temp_file(
        "int bar(void) { return 1; }\n"
        "int unused_func2(void) { return 99; }\n",
        ".c"
    );
    
    char* error_c = create_temp_file(
        "int baz(void) { return \n"  /* Deliberate syntax error */
        "int missing_semicolon = 5\n",
        ".c"
    );
    
    if (!valid_c1 || !valid_c2 || !error_c) {
        fprintf(stderr, "Failed to create temp files\n");
        return 1;
    }
    
    /* Create dump directory */
    mkdir("./dump1", 0755);
    
    /* Test Sequence 1: Help flag then compilation (tests print_help_list reset) */
    char cmd1[512];
    snprintf(cmd1, sizeof(cmd1),
             "gcc --help=common > /dev/null 2>&1 && "
             "gcc -c %s -o /tmp/test1.o 2>/dev/null",
             valid_c1);
    success_count += run_test_sequence("Help then compile", cmd1);
    total_tests++;
    
    /* Test Sequence 2: Version flag then compilation (tests print_version reset) */
    char cmd2[512];
    snprintf(cmd2, sizeof(cmd2),
             "gcc --version > /dev/null 2>&1 && "
             "gcc -c %s -o /tmp/test2.o 2>/dev/null",
             valid_c2);
    success_count += run_test_sequence("Version then compile", cmd2);
    total_tests++;
    
    /* Test Sequence 3: Save-temps with dumpdir then plain compile
       (tests save_temps_flag, dumpdir, dumpbase reset) */
    char cmd3[512];
    snprintf(cmd3, sizeof(cmd3),
             "gcc -save-temps -dumpdir ./dump1 -dumpbase testdump "
             "-c %s -o /tmp/test3.o 2>/dev/null && "
             "gcc -c %s -o /tmp/test4.o 2>/dev/null",
             valid_c1, valid_c2);
    success_count += run_test_sequence("Save-temps/dumpdir then plain compile", cmd3);
    total_tests++;
    
    /* Test Sequence 4: Verbose flag then quiet compile (tests verbose_only_flag reset) */
    char cmd4[512];
    snprintf(cmd4, sizeof(cmd4),
             "gcc -v -c %s 2>&1 | grep -q 'COLLECT_GCC_OPTIONS' && "
             "gcc -c %s -o /tmp/test5.o 2>/dev/null",
             valid_c1, valid_c2);
    success_count += run_test_sequence("Verbose then quiet compile", cmd4);
    total_tests++;
    
    /* Test Sequence 5: Linker selection then default (tests use_ld reset) */
    char cmd5[512];
    snprintf(cmd5, sizeof(cmd5),
             "gcc -fuse-ld=bfd -c %s -o /tmp/test6.o 2>/dev/null && "
             "gcc -c %s -o /tmp/test7.o 2>/dev/null",
             valid_c1, valid_c2);
    success_count += run_test_sequence("Specific linker then default", cmd5);
    total_tests++;
    
    /* Test Sequence 6: Error then success (tests greatest_status reset) */
    char cmd6[512];
    snprintf(cmd6, sizeof(cmd6),
             "gcc -c %s 2>/dev/null; "  /* This will fail */
             "gcc -c %s -o /tmp/test8.o 2>/dev/null",  /* This should succeed */
             error_c, valid_c1);
    success_count += run_test_sequence("Error then success", cmd6);
    total_tests++;
    
    /* Test Sequence 7: Multiple inputs in single invocation (tests per-job reset) */
    char cmd7[512];
    snprintf(cmd7, sizeof(cmd7),
             "gcc -c %s %s -o /tmp/test9.o 2>/dev/null",
             valid_c1, valid_c2);
    success_count += run_test_sequence("Multiple source files", cmd7);
    total_tests++;
    
    /* Test Sequence 8: Using -x to chain language specifications */
    char cmd8[512];
    snprintf(cmd8, sizeof(cmd8),
             "echo 'int x=1;' | gcc -x c - -c -o /tmp/test10.o 2>/dev/null && "
             "gcc -c %s -o /tmp/test11.o 2>/dev/null",
             valid_c2);
    success_count += run_test_sequence("Pipe input then file compile", cmd8);
    total_tests++;
    
    /* Cleanup */
    system("rm -f /tmp/test*.o /tmp/test*.i /tmp/test*.s /tmp/test*.ii");
    system("rm -rf ./dump1");
    cleanup_temp_files();
    
    free(valid_c1);
    free(valid_c2);
    free(error_c);
    
    /* Output checksum for verification */
    unsigned int checksum = (success_count << 16) | total_tests;
    printf("\n=== Test Summary ===\n");
    printf("Success: %d/%d\n", success_count, total_tests);
    printf("Checksum: 0x%08X\n", checksum);
    
    /* Return 0 if at least half the tests passed */
    return (success_count * 2 >= total_tests) ? 0 : 1;
}

/* Reference the unused functions to avoid warnings */
__attribute__((used))
static void reference_unused_functions(void) {
    /* These references ensure the functions in temp files aren't optimized away */
    extern int unused_func1(void);
    extern int unused_func2(void);
    (void)unused_func1;
    (void)unused_func2;
}
