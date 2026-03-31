/* test_gcc_driver_init.c - Test GCC driver initialization logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

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
    
    strncpy(temp_files[temp_file_count], template, sizeof(temp_files[0]) - 1);
    temp_files[temp_file_count][sizeof(temp_files[0]) - 1] = '\0';
    
    return temp_files[temp_file_count++];
}

static void cleanup_temp_files(void) {
    for (int i = 0; i < temp_file_count; i++) {
        if (temp_files[i][0]) {
            unlink(temp_files[i]);
        }
    }
}

/* Function to avoid unused function warnings */
static void use_functions(void) {
    /* These functions exist only to be compiled */
    int (*fns[])(void) = {foo, bar};
    (void)fns;
}

int main(void) {
    int checksum = 0;
    int result;
    char cmd[1024];
    
    /* Create temporary source files with different content */
    const char* valid_code1 = 
        "int foo(void) { return 0; }\n"
        "int unused1(void) { return 1; }\n";
    
    const char* valid_code2 = 
        "int bar(void) { return 2; }\n"
        "int unused2(void) { return 3; }\n";
    
    const char* error_code = 
        "int baz(void) { return /* missing semicolon and value */ }\n";
    
    char* temp1 = create_temp_file(valid_code1, ".c");
    char* temp2 = create_temp_file(valid_code2, ".c");
    char* temp_err = create_temp_file(error_code, ".c");
    
    if (!temp1 || !temp2 || !temp_err) {
        fprintf(stderr, "Failed to create temp files\n");
        return 1;
    }
    
    /* Create temporary directory for dump files */
    mkdir("./dump_test_dir", 0755);
    
    printf("Testing GCC driver initialization block coverage...\n");
    
    /* SEQUENCE 1: Help flag then compilation - tests print_help_list reset */
    printf("\n1. Testing help flag reset...\n");
    snprintf(cmd, sizeof(cmd), 
             "%s --help=common > /dev/null 2>&1 && "
             "%s -c %s -o %s.o",
             CC, CC, temp1, temp1);
    result = system(cmd);
    checksum += (result == 0) ? 1 : 0;
    
    /* SEQUENCE 2: Version flag then compilation - tests print_version reset */
    printf("2. Testing version flag reset...\n");
    snprintf(cmd, sizeof(cmd),
             "%s --version > /dev/null 2>&1 && "
             "%s -c %s -o %s_v2.o",
             CC, CC, temp2, temp2);
    result = system(cmd);
    checksum += (result == 0) ? 2 : 0;
    
    /* SEQUENCE 3: Save-temps with dumpdir then plain compilation 
       - tests save_temps_flag, dumpdir, dumpbase reset */
    printf("3. Testing save-temps and dumpdir reset...\n");
    snprintf(cmd, sizeof(cmd),
             "%s -save-temps -dumpdir ./dump_test_dir -dumpbase testbase "
             "-c %s -o %s_save.o > /dev/null 2>&1 && "
             "%s -c %s -o %s_plain.o",
             CC, temp1, temp1, CC, temp2, temp2);
    result = system(cmd);
    checksum += (result == 0) ? 4 : 0;
    
    /* SEQUENCE 4: Verbose flag then quiet compilation 
       - tests verbose_only_flag reset */
    printf("4. Testing verbose flag reset...\n");
    snprintf(cmd, sizeof(cmd),
             "%s -v -c %s 2>&1 | grep -q 'COLLECT_GCC_OPTIONS' && "
             "%s -c %s -o %s_quiet.o",
             CC, temp1, CC, temp2, temp2);
    result = system(cmd);
    checksum += (result == 0) ? 8 : 0;
    
    /* SEQUENCE 5: Linker selection then default 
       - tests use_ld reset (if supported) */
    printf("5. Testing linker selection reset...\n");
    snprintf(cmd, sizeof(cmd),
             "%s -fuse-ld=bfd -c %s -o %s_ld.o 2>/dev/null; "
             "%s -c %s -o %s_nold.o",
             CC, temp1, temp1, CC, temp2, temp2);
    result = system(cmd);
    checksum += (result == 0) ? 16 : 0;
    
    /* SEQUENCE 6: Error then success - tests greatest_status reset */
    printf("6. Testing error recovery (greatest_status reset)...\n");
    snprintf(cmd, sizeof(cmd),
             "%s -c %s 2>/dev/null; "
             "%s -c %s -o %s_recover.o",
             CC, temp_err, CC, temp1, temp1);
    result = system(cmd);
    checksum += (result == 0) ? 32 : 0;
    
    /* SEQUENCE 7: Multiple inputs in single invocation 
       - tests per-job initialization */
    printf("7. Testing multiple inputs (per-job init)...\n");
    snprintf(cmd, sizeof(cmd),
             "%s -c %s %s -save-temps -dumpdir ./dump_test_dir/multi "
             "-dumpbase multi 2>/dev/null",
             CC, temp1, temp2);
    result = system(cmd);
    checksum += (result == 0) ? 64 : 0;
    
    /* SEQUENCE 8: -x language specification to force multiple jobs */
    printf("8. Testing -x language specification...\n");
    snprintf(cmd, sizeof(cmd),
             "echo 'int x=1;' | %s -x c - -c -o stdin.o 2>/dev/null && "
             "%s -c %s -o %s_x.o",
             CC, CC, temp1, temp1);
    result = system(cmd);
    checksum += (result == 0) ? 128 : 0;
    
    /* Cleanup temporary object files */
    system("rm -f *.o *.i *.s *.bc ./dump_test_dir/* 2>/dev/null");
    rmdir("./dump_test_dir");
    
    /* Final checksum and verification */
    printf("\nTest sequences completed. Checksum: %d\n", checksum);
    printf("Expected checksum if all sequences passed: %d\n", 255);
    
    if (checksum == 255) {
        printf("SUCCESS: All driver initialization sequences executed\n");
    } else {
        printf("PARTIAL: Some sequences failed (checksum: %d)\n", checksum);
    }
    
    cleanup_temp_files();
    return (checksum == 255) ? 0 : 1;
}

/* Dummy functions referenced in source files */
int foo(void) { return 0; }
int bar(void) { return 0; }
