/* Test program to exercise GCC driver initialization logic (lines 11228-11250 in gcc.cc) */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

/* Temporary file management */
static char temp_files[6][64];
static int temp_file_count = 0;

/* Create a temporary C source file with given content */
static char* create_temp_file(const char* content, const char* suffix) {
    char template[] = "/tmp/gcc_test_XXXXXX";
    int fd = mkstemp(template);
    if (fd == -1) {
        perror("mkstemp failed");
        return NULL;
    }
    
    write(fd, content, strlen(content));
    close(fd);
    
    strcpy(temp_files[temp_file_count], template);
    if (suffix) {
        char newname[128];
        snprintf(newname, sizeof(newname), "%s.%s", template, suffix);
        rename(template, newname);
        strcpy(temp_files[temp_file_count], newname);
    }
    
    return temp_files[temp_file_count++];
}

/* Clean up temporary files */
static void cleanup_temp_files(void) {
    for (int i = 0; i < temp_file_count; i++) {
        if (temp_files[i][0]) {
            unlink(temp_files[i]);
        }
    }
    
    /* Clean up any dump directories we created */
    system("rm -rf ./dump1 ./dump2 2>/dev/null");
}

/* Execute a GCC command and check return code */
static int execute_gcc(const char* cmd, int expect_failure) {
    int result = system(cmd);
    int exit_status = WEXITSTATUS(result);
    
    if (expect_failure) {
        return (exit_status != 0) ? 1 : 0;  /* Success if command failed as expected */
    } else {
        return (exit_status == 0) ? 1 : 0;  /* Success if command succeeded */
    }
}

/* Main test orchestration */
int main(void) {
    int checksum = 0;
    
    /* Create temporary source files */
    char* temp1 = create_temp_file("int foo(void) { return 0; }\n", "c");
    char* temp2 = create_temp_file("int bar(void) { return 1; }\n", "c");
    char* temp3 = create_temp_file("int baz(void) { return /* missing semicolon */ }\n", "c");
    char* temp4 = create_temp_file("int qux(void) { return 2; }\n", "c");
    
    if (!temp1 || !temp2 || !temp3 || !temp4) {
        fprintf(stderr, "Failed to create temporary files\n");
        cleanup_temp_files();
        return 1;
    }
    
    printf("Testing GCC driver initialization block coverage...\n");
    
    /* Sequence 1: Help flag then compilation (tests print_help_list, print_version reset) */
    printf("\n1. Testing help flag -> compilation reset...\n");
    char cmd1[512];
    snprintf(cmd1, sizeof(cmd1), "%s --help=common > /dev/null 2>&1", 
             getenv("CC") ? getenv("CC") : "gcc");
    checksum += execute_gcc(cmd1, 0);
    
    snprintf(cmd1, sizeof(cmd1), "%s -c %s -o /tmp/temp1.o 2>/dev/null",
             getenv("CC") ? getenv("CC") : "gcc", temp1);
    checksum += execute_gcc(cmd1, 0);
    
    /* Sequence 2: Save-temps with dumpdir then plain compilation 
       (tests save_temps_flag, dumpdir, dumpbase reset) */
    printf("\n2. Testing save-temps/dumpdir -> compilation reset...\n");
    mkdir("./dump1", 0755);
    
    char cmd2[512];
    snprintf(cmd2, sizeof(cmd2), "%s -save-temps -dumpdir ./dump1 -dumpbase mydump -c %s -o /tmp/temp2.o 2>/dev/null",
             getenv("CC") ? getenv("CC") : "gcc", temp2);
    checksum += execute_gcc(cmd2, 0);
    
    /* Check if dump files were created */
    struct stat st;
    if (stat("./dump1/mydump.i", &st) == 0) {
        checksum += 1;  /* dumpdir worked */
    }
    
    snprintf(cmd2, sizeof(cmd2), "%s -c %s -o /tmp/temp3.o 2>/dev/null",
             getenv("CC") ? getenv("CC") : "gcc", temp4);
    checksum += execute_gcc(cmd2, 0);
    
    /* Sequence 3: Error then success (tests greatest_status reset) */
    printf("\n3. Testing error -> success reset...\n");
    char cmd3[512];
    snprintf(cmd3, sizeof(cmd3), "%s -c %s -o /tmp/temp_err.o 2>/dev/null",
             getenv("CC") ? getenv("CC") : "gcc", temp3);
    checksum += execute_gcc(cmd3, 1);  /* Expect failure */
    
    snprintf(cmd3, sizeof(cmd3), "%s -c %s -o /tmp/temp_ok.o 2>/dev/null",
             getenv("CC") ? getenv("CC") : "gcc", temp1);
    checksum += execute_gcc(cmd3, 0);  /* Expect success */
    
    /* Sequence 4: Verbose and linker selection then plain compilation
       (tests verbose_only_flag, use_ld reset) */
    printf("\n4. Testing verbose/linker -> compilation reset...\n");
    char cmd4[512];
    const char* cc = getenv("CC") ? getenv("CC") : "gcc";
    
    /* Try different linker options based on what's available */
    snprintf(cmd4, sizeof(cmd4), "%s -v -fuse-ld=bfd -c %s -o /tmp/temp4.o 2>&1 | grep -q 'COLLECT_GCC_OPTIONS'",
             cc, temp1);
    int res1 = system(cmd4);
    
    /* If bfd not available, try gold */
    if (WEXITSTATUS(res1) != 0) {
        snprintf(cmd4, sizeof(cmd4), "%s -v -fuse-ld=gold -c %s -o /tmp/temp4.o 2>&1 | grep -q 'COLLECT_GCC_OPTIONS'",
                 cc, temp1);
        res1 = system(cmd4);
    }
    
    checksum += (WEXITSTATUS(res1) == 0) ? 1 : 0;
    
    /* Plain compilation after verbose/linker */
    snprintf(cmd4, sizeof(cmd4), "%s -c %s -o /tmp/temp5.o 2>/dev/null",
             cc, temp2);
    checksum += execute_gcc(cmd4, 0);
    
    /* Sequence 5: Multiple inputs in single invocation (tests per-job reset) */
    printf("\n5. Testing multiple inputs in single command...\n");
    char cmd5[512];
    snprintf(cmd5, sizeof(cmd5), "%s -c %s %s -save-temps -dumpdir ./dump2 2>/dev/null",
             cc, temp1, temp2);
    checksum += execute_gcc(cmd5, 0);
    
    /* Sequence 6: Version flag then compilation (tests print_version reset) */
    printf("\n6. Testing version flag -> compilation reset...\n");
    char cmd6[512];
    snprintf(cmd6, sizeof(cmd6), "%s --version > /dev/null 2>&1", cc);
    checksum += execute_gcc(cmd6, 0);
    
    snprintf(cmd6, sizeof(cmd6), "%s -c %s -o /tmp/temp6.o 2>/dev/null", cc, temp4);
    checksum += execute_gcc(cmd6, 0);
    
    /* Clean up object files */
    system("rm -f /tmp/temp*.o 2>/dev/null");
    
    /* Final checksum and cleanup */
    printf("\nTest completed. Checksum: %d\n", checksum);
    printf("Expected checksum range: 12-15 (depends on linker availability)\n");
    
    cleanup_temp_files();
    
    return 0;
}
