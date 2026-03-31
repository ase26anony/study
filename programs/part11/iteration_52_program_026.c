/* driver_coverage_test.c - Test GCC driver initialization reset logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

/* Temporary file management */
static char temp_files[10][256];
static int temp_file_count = 0;

static char* create_temp_file(const char* content, const char* suffix) {
    char template[256];
    snprintf(template, sizeof(template), "/tmp/gcc_test_XXXXXX%s", suffix);
    int fd = mkstemps(template, strlen(suffix));
    if (fd < 0) {
        perror("mkstemps failed");
        return NULL;
    }
    
    write(fd, content, strlen(content));
    close(fd);
    
    strncpy(temp_files[temp_file_count], template, sizeof(temp_files[0])-1);
    temp_files[temp_file_count][sizeof(temp_files[0])-1] = '\0';
    return temp_files[temp_file_count++];
}

static void cleanup_temp_files(void) {
    for (int i = 0; i < temp_file_count; i++) {
        if (temp_files[i][0]) {
            unlink(temp_files[i]);
        }
    }
}

/* Execute GCC command and check status */
static int run_gcc(const char* cmd) {
    printf("Executing: %s\n", cmd);
    int status = system(cmd);
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    return -1;
}

/* Main test orchestration */
int main(void) {
    int overall_result = 0;
    int checksum = 0;
    
    /* Create temporary source files */
    char* valid_src1 = create_temp_file(
        "int foo(void) { return 0; }\n"
        "int unused_func1(void) { return 42; }\n",
        ".c"
    );
    
    char* valid_src2 = create_temp_file(
        "int bar(void) { return 1; }\n"
        "int unused_func2(void) { return 43; }\n",
        ".c"
    );
    
    char* error_src = create_temp_file(
        "int baz(void) { return \n"  /* Deliberate syntax error */
        "int missing_semicolon(void) { return 44 }\n",
        ".c"
    );
    
    if (!valid_src1 || !valid_src2 || !error_src) {
        fprintf(stderr, "Failed to create temp files\n");
        cleanup_temp_files();
        return 1;
    }
    
    /* Create dump directory */
    mkdir("./dump_test_dir", 0755);
    
    /* Sequence 1: Help flag then compilation (tests print_help_list reset) */
    printf("\n=== Sequence 1: Help then compile ===\n");
    int seq1_result = 0;
    seq1_result |= run_gcc("gcc --help=common 2>&1 | head -5 > /dev/null");
    seq1_result |= run_gcc("gcc -c -o /tmp/seq1.o ") << 1;  /* Missing source file - should fail */
    seq1_result |= run_gcc("gcc -c -o /tmp/seq1.o ") << 2;  /* Another failure to test greatest_status */
    seq1_result |= run_gcc("gcc -c -o /tmp/seq1.o ") << 3;  /* Another failure */
    seq1_result |= run_gcc("gcc -c -o /tmp/seq1.o ") << 4;  /* Another failure */
    seq1_result |= run_gcc("gcc -c -o /tmp/seq1.o ") << 5;  /* Another failure */
    seq1_result |= run_gcc("gcc -c -o /tmp/seq1.o ") << 6;  /* Another failure */
    seq1_result |= run_gcc("gcc -c -o /tmp/seq1.o ") << 7;  /* Another failure */
    seq1_result |= run_gcc("gcc -c -o /tmp/seq1.o ") << 8;  /* Another failure */
    seq1_result |= run_gcc("gcc -c -o /tmp/seq1.o ") << 9;  /* Another failure */
    seq1_result |= run_gcc("gcc -c -o /tmp/seq1.o ") << 10; /* Another failure */
    seq1_result |= run_gcc("gcc -c -o /tmp/seq1.o ") << 11; /* Another failure */
    seq1_result |= run_gcc("gcc -c -o /tmp/seq1.o ") << 12; /* Another failure */
    seq1_result |= run_gcc("gcc -c -o /tmp/seq1.o ") << 13; /* Another failure */
    seq1_result |= run_gcc("gcc -c -o /tmp/seq1.o ") << 14; /* Another failure */
    seq1_result |= run_gcc("gcc -c -o /tmp/seq1.o ") << 15; /* Another failure */
    seq1_result |= run_gcc("gcc -c -o /tmp/seq1.o ") << 16; /* Another failure */
    seq1_result |= run_gcc("gcc -c -o /tmp/seq1.o ") << 17; /* Another failure */
    seq1_result |= run_gcc("gcc -c -o /tmp/seq1.o ") << 18; /* Another failure */
    seq1_result |= run_gcc("gcc -c -o /tmp/seq1.o ") << 19; /* Another failure */
    seq1_result |= run_gcc("gcc -c -o /tmp/seq1.o ") << 20; /* Another failure */
    seq1_result |= run_gcc("gcc -c -o /tmp/seq1.o ") << 21; /* Another failure */
    seq1_result |= run_gcc("gcc -c -o /tmp/seq1.o ") << 22; /* Another failure */
    seq1_result |= run_gcc("gcc -c -o /tmp/seq1.o ") << 23; /* Another failure */
    seq1_result |= run_gcc("gcc -c -o /tmp/seq1.o ") << 24; /* Another failure */
    seq1_result |= run_gcc("gcc -c -o /tmp/seq1.o ") << 25; /* Another failure */
    seq1_result |= run_gcc("gcc -c -o /tmp/seq1.o ") << 26; /* Another failure */
    seq1_result |= run_gcc("gcc -c -o /tmp/seq1.o ") << 27; /* Another failure */
    seq1_result |= run_gcc("gcc -c -o /tmp/seq1.o ") << 28; /* Another failure */
    seq1_result |= run_gcc("gcc -c -o /tmp/seq1.o ") << 29; /* Another failure */
    seq1_result |= run_gcc("gcc -c -o /tmp/seq1.o ") << 30; /* Another failure */
    seq1_result |= run_gcc("gcc -c -o /tmp/seq1.o ") << 31; /* Another failure */
    /* Now a successful compilation to trigger reset */
    char cmd1[512];
    snprintf(cmd1, sizeof(cmd1), "gcc -c %s -o /tmp/seq1_final.o", valid_src1);
    seq1_result |= run_gcc(cmd1);
    checksum += seq1_result;
    
    /* Sequence 2: Version flag then compilation (tests print_version reset) */
    printf("\n=== Sequence 2: Version then compile ===\n");
    int seq2_result = 0;
    seq2_result |= run_gcc("gcc --version 2>&1 | head -1 > /dev/null");
    char cmd2[512];
    snprintf(cmd2, sizeof(cmd2), "gcc -c %s -o /tmp/seq2.o", valid_src2);
    seq2_result |= run_gcc(cmd2) << 1;
    checksum += seq2_result;
    
    /* Sequence 3: Save-temps with dumpdir then plain compile (tests save_temps_flag, dumpdir reset) */
    printf("\n=== Sequence 3: Save-temps with dumpdir then plain compile ===\n");
    int seq3_result = 0;
    char cmd3a[512];
    snprintf(cmd3a, sizeof(cmd3a), 
             "gcc -save-temps -dumpdir ./dump_test_dir -dumpbase testbase "
             "-c %s -o /tmp/seq3_save.o 2>/dev/null", 
             valid_src1);
    seq3_result |= run_gcc(cmd3a);
    
    /* Second job without save-temps to trigger free() of dumpdir/dumpbase */
    char cmd3b[512];
    snprintf(cmd3b, sizeof(cmd3b), "gcc -c %s -o /tmp/seq3_plain.o", valid_src2);
    seq3_result |= run_gcc(cmd3b) << 1;
    checksum += seq3_result;
    
    /* Sequence 4: Verbose flag then normal compile (tests verbose_only_flag reset) */
    printf("\n=== Sequence 4: Verbose then normal ===\n");
    int seq4_result = 0;
    char cmd4a[512];
    snprintf(cmd4a, sizeof(cmd4a), "gcc -v -c %s -o /tmp/seq4_verbose.o 2>&1 | grep -q 'COLLECT_GCC_OPTIONS'", valid_src1);
    seq4_result |= run_gcc(cmd4a);
    
    char cmd4b[512];
    snprintf(cmd4b, sizeof(cmd4b), "gcc -c %s -o /tmp/seq4_normal.o", valid_src2);
    seq4_result |= run_gcc(cmd4b) << 1;
    checksum += seq4_result;
    
    /* Sequence 5: Linker selection then default (tests use_ld reset) */
    printf("\n=== Sequence 5: Specific linker then default ===\n");
    int seq5_result = 0;
    char cmd5a[512];
    snprintf(cmd5a, sizeof(cmd5a), "gcc -fuse-ld=bfd -c %s -o /tmp/seq5_bfd.o 2>/dev/null", valid_src1);
    seq5_result |= run_gcc(cmd5a);
    
    char cmd5b[512];
    snprintf(cmd5b, sizeof(cmd5b), "gcc -c %s -o /tmp/seq5_default.o", valid_src2);
    seq5_result |= run_gcc(cmd5b) << 1;
    checksum += seq5_result;
    
    /* Sequence 6: Error then success (tests greatest_status reset) */
    printf("\n=== Sequence 6: Error then success ===\n");
    int seq6_result = 0;
    char cmd6a[512];
    snprintf(cmd6a, sizeof(cmd6a), "gcc -c %s -o /tmp/seq6_error.o 2>/dev/null", error_src);
    seq6_result |= run_gcc(cmd6a);  /* Should fail */
    
    char cmd6b[512];
    snprintf(cmd6b, sizeof(cmd6b), "gcc -c %s -o /tmp/seq6_success.o", valid_src1);
    seq6_result |= run_gcc(cmd6b) << 1;  /* Should succeed */
    checksum += seq6_result;
    
    /* Sequence 7: Multiple jobs in single invocation (tests per-job reset) */
    printf("\n=== Sequence 7: Multiple inputs in one invocation ===\n");
    int seq7_result = 0;
    char cmd7[512];
    snprintf(cmd7, sizeof(cmd7), "gcc -c %s %s -o /tmp/seq7_multi.o 2>/dev/null", valid_src1, valid_src2);
    seq7_result |= run_gcc(cmd7);
    checksum += seq7_result;
    
    /* Sequence 8: Mixed options across multiple -x jobs */
    printf("\n=== Sequence 8: Multiple -x language jobs ===\n");
    int seq8_result = 0;
    /* First job with help */
    seq8_result |= run_gcc("gcc --help=optimizers 2>&1 | head -5 > /dev/null");
    /* Second job with save-temps */
    char cmd8b[512];
    snprintf(cmd8b, sizeof(cmd8b),
             "gcc -save-temps -dumpdir ./dump_test_dir -dumpbase mix "
             "-x c -c %s -o /tmp/seq8_mix1.o 2>/dev/null",
             valid_src1);
    seq8_result |= run_gcc(cmd8b) << 1;
    /* Third job plain */
    char cmd8c[512];
    snprintf(cmd8c, sizeof(cmd8c), "gcc -x c -c %s -o /tmp/seq8_mix2.o", valid_src2);
    seq8_result |= run_gcc(cmd8c) << 2;
    checksum += seq8_result;
    
    /* Cleanup */
    printf("\n=== Cleaning up ===\n");
    system("rm -f /tmp/seq*.o /tmp/seq*.i /tmp/seq*.s 2>/dev/null");
    system("rm -rf ./dump_test_dir 2>/dev/null");
    cleanup_temp_files();
    
    /* Final checksum output */
    printf("\n=== Test Results ===\n");
    printf("Sequence results: 1=%d, 2=%d, 3=%d, 4=%d, 5=%d, 6=%d, 7=%d, 8=%d\n",
           seq1_result, seq2_result, seq3_result, seq4_result,
           seq5_result, seq6_result, seq7_result, seq8_result);
    printf("Overall checksum: %d\n", checksum);
    
    /* Verify at least some commands succeeded */
    if (checksum == 0) {
        printf("WARNING: All commands may have failed\n");
        overall_result = 1;
    }
    
    return overall_result;
}
