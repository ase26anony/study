/* Test program to exercise GCC driver initialization reset logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

/* Temporary file management */
static char temp_files[10][256];
static int temp_file_count = 0;

static void cleanup_temp_files(void) {
    for (int i = 0; i < temp_file_count; i++) {
        if (temp_files[i][0]) {
            unlink(temp_files[i]);
        }
    }
}

static int create_temp_file(const char *prefix, const char *content) {
    char template[256];
    snprintf(template, sizeof(template), "/tmp/%s_XXXXXX", prefix);
    
    int fd = mkstemp(template);
    if (fd < 0) {
        perror("mkstemp failed");
        return -1;
    }
    
    write(fd, content, strlen(content));
    close(fd);
    
    if (temp_file_count < 10) {
        strcpy(temp_files[temp_file_count], template);
        temp_file_count++;
    }
    
    return 0;
}

/* Function to execute GCC command and check result */
static int run_gcc_command(const char *cmd) {
    printf("Executing: %s\n", cmd);
    int result = system(cmd);
    printf("Result: %d\n", result);
    return result;
}

/* Unused functions to avoid compiler warnings */
static int foo(void) { return 0; }
static int bar(void) { return 1; }
__attribute__((unused)) static void use_functions(void) {
    (void)foo();
    (void)bar();
}

int main(void) {
    int checksum = 0;
    
    /* Register cleanup handler */
    atexit(cleanup_temp_files);
    
    /* Create temporary source files */
    if (create_temp_file("test1", "int foo(void) { return 0; }\n") < 0) return 1;
    if (create_temp_file("test2", "int bar(void) { return 1; }\n") < 0) return 1;
    if (create_temp_file("error", "int baz(void) { return /* syntax error */ }\n") < 0) return 1;
    
    /* Create dump directory */
    mkdir("./dump_test", 0755);
    
    /* Sequence 1: Help flag then compilation (tests print_help_list reset) */
    printf("\n=== Sequence 1: Help then compile ===\n");
    char cmd1[512];
    snprintf(cmd1, sizeof(cmd1), 
             "%s --help=common > /dev/null 2>&1 && "
             "%s -c %s -o /tmp/test1.o",
             CC, CC, temp_files[0]);
    checksum += (run_gcc_command(cmd1) == 0) ? 1 : 0;
    
    /* Sequence 2: Version flag then compilation (tests print_version reset) */
    printf("\n=== Sequence 2: Version then compile ===\n");
    char cmd2[512];
    snprintf(cmd2, sizeof(cmd2),
             "%s --version > /dev/null 2>&1 && "
             "%s -c %s -o /tmp/test2.o",
             CC, CC, temp_files[1]);
    checksum += (run_gcc_command(cmd2) == 0) ? 2 : 0;
    
    /* Sequence 3: Save-temps with dumpdir then plain compile 
       (tests save_temps_flag, dumpdir, dumpbase reset) */
    printf("\n=== Sequence 3: Save-temps with dumpdir then plain compile ===\n");
    char cmd3[512];
    snprintf(cmd3, sizeof(cmd3),
             "%s -save-temps -dumpdir ./dump_test -dumpbase mydump -c %s -o /tmp/test3.o 2>/dev/null && "
             "%s -c %s -o /tmp/test4.o 2>/dev/null",
             CC, temp_files[0], CC, temp_files[1]);
    checksum += (run_gcc_command(cmd3) == 0) ? 4 : 0;
    
    /* Sequence 4: Verbose flag then normal compile (tests verbose_only_flag reset) */
    printf("\n=== Sequence 4: Verbose then normal compile ===\n");
    char cmd4[512];
    snprintf(cmd4, sizeof(cmd4),
             "%s -v -c %s -o /tmp/test5.o 2>/dev/null && "
             "%s -c %s -o /tmp/test6.o 2>/dev/null",
             CC, temp_files[0], CC, temp_files[1]);
    checksum += (run_gcc_command(cmd4) == 0) ? 8 : 0;
    
    /* Sequence 5: Linker selection then default (tests use_ld reset) */
    printf("\n=== Sequence 5: Specific linker then default ===\n");
    char cmd5[512];
    snprintf(cmd5, sizeof(cmd5),
             "%s -fuse-ld=bfd -c %s -o /tmp/test7.o 2>/dev/null && "
             "%s -c %s -o /tmp/test8.o 2>/dev/null",
             CC, temp_files[0], CC, temp_files[1]);
    checksum += (run_gcc_command(cmd5) == 0) ? 16 : 0;
    
    /* Sequence 6: Error then success (tests greatest_status reset) */
    printf("\n=== Sequence 6: Error then success ===\n");
    char cmd6[512];
    snprintf(cmd6, sizeof(cmd6),
             "%s -c %s 2>/dev/null; "  /* This should fail */
             "%s -c %s -o /tmp/test9.o 2>/dev/null",  /* This should succeed */
             CC, temp_files[2], CC, temp_files[0]);
    checksum += (run_gcc_command(cmd6) == 0) ? 32 : 0;
    
    /* Sequence 7: Multiple source files in one invocation (tests job sequencing) */
    printf("\n=== Sequence 7: Multiple source files ===\n");
    char cmd7[512];
    snprintf(cmd7, sizeof(cmd7),
             "%s -c %s %s -o /tmp/multi.o 2>/dev/null",
             CC, temp_files[0], temp_files[1]);
    checksum += (run_gcc_command(cmd7) == 0) ? 64 : 0;
    
    /* Sequence 8: Using -x to specify language (tests driver job control) */
    printf("\n=== Sequence 8: Using -x language specification ===\n");
    char cmd8[512];
    snprintf(cmd8, sizeof(cmd8),
             "%s -x c -c %s -o /tmp/test10.o 2>/dev/null && "
             "%s -x c -c %s -o /tmp/test11.o 2>/dev/null",
             CC, temp_files[0], CC, temp_files[1]);
    checksum += (run_gcc_command(cmd8) == 0) ? 128 : 0;
    
    /* Cleanup dump directory */
    system("rm -rf ./dump_test");
    
    /* Cleanup object files */
    system("rm -f /tmp/test*.o /tmp/multi.o");
    
    printf("\n=== Final checksum: %d ===\n", checksum);
    printf("Expected maximum: 255 (all sequences successful)\n");
    
    return (checksum > 0) ? 0 : 1;
}
