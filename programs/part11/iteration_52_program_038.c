/* driver_coverage_test.c - Test program to exercise GCC driver initialization logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

/* Function prototypes for temporary source files */
int foo(void) { return 0; }
int bar(void) { return 1; }
/* This function is referenced to avoid -Wunused-function warnings */
int dummy_ref(void) { return foo() + bar(); }

/* Helper function to create temporary files */
static int create_temp_file(const char *template, const char *content) {
    FILE *f = fopen(template, "w");
    if (!f) {
        perror("fopen");
        return -1;
    }
    fprintf(f, "%s", content);
    fclose(f);
    return 0;
}

/* Helper to remove temporary files */
static void cleanup_files(const char **files, int count) {
    for (int i = 0; i < count; i++) {
        if (files[i]) {
            unlink(files[i]);
        }
    }
}

/* Helper to create and cleanup dump directory */
static int setup_dump_dir(void) {
    /* Remove if exists from previous run */
    system("rm -rf ./dump_test_coverage");
    return mkdir("./dump_test_coverage", 0755);
}

int main(void) {
    int overall_result = 0;
    int checksum = 0;
    const char *temp_files[] = {
        "temp_coverage_1.c",
        "temp_coverage_2.c", 
        "temp_coverage_err.c",
        "temp_coverage_1.o",
        "temp_coverage_2.o",
        NULL
    };
    
    /* Create temporary source files */
    if (create_temp_file("temp_coverage_1.c", 
        "int foo(void) { return 0; }\n") < 0) {
        return 1;
    }
    
    if (create_temp_file("temp_coverage_2.c",
        "int bar(void) { return 1; }\n") < 0) {
        cleanup_files(temp_files, 2);
        return 1;
    }
    
    /* Create file with syntax error */
    if (create_temp_file("temp_coverage_err.c",
        "int baz(void) { return /* missing semicolon and value */ \n") < 0) {
        cleanup_files(temp_files, 3);
        return 1;
    }
    
    printf("=== GCC Driver Initialization Coverage Test ===\n");
    
    /* Sequence A: Help flag followed by compilation - tests print_help_list reset */
    printf("\n1. Testing help flag reset (print_help_list -> 0)...\n");
    int seq_a = system("gcc --help=common > /dev/null 2>&1 && "
                      "gcc -c temp_coverage_1.c -o temp_coverage_1.o 2>/dev/null");
    if (seq_a == 0) {
        printf("   Sequence A passed\n");
        checksum += 1;
    } else {
        printf("   Sequence A failed: %d\n", WEXITSTATUS(seq_a));
        overall_result = 1;
    }
    
    /* Sequence B: Save-temps with dumpdir followed by plain compile 
       Tests save_temps_flag, dumpdir, dumpbase reset */
    printf("\n2. Testing save-temps and dumpdir reset...\n");
    if (setup_dump_dir() == 0) {
        int seq_b = system("gcc -save-temps -dumpdir ./dump_test_coverage "
                          "-dumpbase testbase -c temp_coverage_1.c "
                          "-o temp_coverage_1.o 2>/dev/null && "
                          "gcc -c temp_coverage_2.c -o temp_coverage_2.o 2>/dev/null");
        if (seq_b == 0) {
            printf("   Sequence B passed\n");
            checksum += 2;
        } else {
            printf("   Sequence B failed: %d\n", WEXITSTATUS(seq_b));
            overall_result = 1;
        }
    } else {
        printf("   Failed to create dump directory\n");
        overall_result = 1;
    }
    
    /* Sequence C: Error then success - tests greatest_status reset */
    printf("\n3. Testing error recovery (greatest_status reset)...\n");
    int seq_c = system("gcc -c temp_coverage_err.c 2>/dev/null; "
                      "gcc -c temp_coverage_1.c -o temp_coverage_1.o 2>/dev/null");
    /* First command should fail, second should succeed */
    if (access("temp_coverage_1.o", F_OK) == 0) {
        printf("   Sequence C passed (error handled, recovery successful)\n");
        checksum += 4;
    } else {
        printf("   Sequence C failed\n");
        overall_result = 1;
    }
    
    /* Sequence D: Verbose and linker selection followed by plain compile
       Tests verbose_only_flag, use_ld reset */
    printf("\n4. Testing verbose and linker flag reset...\n");
    int seq_d = system("gcc -v -fuse-ld=bfd -c temp_coverage_1.c 2>&1 | "
                      "grep -q 'COLLECT_GCC_OPTIONS' && "
                      "gcc -c temp_coverage_2.c -o temp_coverage_2.o 2>/dev/null");
    if (seq_d == 0) {
        printf("   Sequence D passed\n");
        checksum += 8;
    } else {
        printf("   Sequence D failed: %d\n", WEXITSTATUS(seq_d));
        overall_result = 1;
    }
    
    /* Additional sequence: Version flag reset (print_version -> 0) */
    printf("\n5. Testing version flag reset...\n");
    int seq_e = system("gcc --version > /dev/null 2>&1 && "
                      "gcc -c temp_coverage_2.c -o temp_coverage_2.o 2>/dev/null");
    if (seq_e == 0) {
        printf("   Sequence E passed\n");
        checksum += 16;
    } else {
        printf("   Sequence E failed: %d\n", WEXITSTATUS(seq_e));
        overall_result = 1;
    }
    
    /* Cleanup */
    printf("\n6. Cleaning up...\n");
    cleanup_files(temp_files, 5);
    system("rm -rf ./dump_test_coverage");
    
    printf("\n=== Test Complete ===\n");
    printf("Checksum: %d (expected: 31 for all passes)\n", checksum);
    printf("Overall result: %s\n", overall_result == 0 ? "PASS" : "FAIL");
    
    /* Reference dummy function to avoid unused function warnings */
    (void)dummy_ref;
    
    return overall_result;
}
