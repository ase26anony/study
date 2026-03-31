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
    char template[] = "/tmp/gcc_test_XXXXXX";
    int fd = mkstemp(template);
    if (fd == -1) return NULL;
    
    write(fd, content, strlen(content));
    close(fd);
    
    char* filename = strdup(template);
    if (suffix) {
        char newname[128];
        snprintf(newname, sizeof(newname), "%s%s", template, suffix);
        rename(template, newname);
        free(filename);
        filename = strdup(newname);
    }
    
    if (temp_file_count < 5) {
        strcpy(temp_files[temp_file_count++], filename);
    }
    return filename;
}

static void cleanup_temp_files(void) {
    for (int i = 0; i < temp_file_count; i++) {
        unlink(temp_files[i]);
        free(temp_files[i]);
    }
}

/* Test functions to avoid unused function warnings */
static int foo(void) { return 0; }
static int bar(void) { return 1; }
__attribute__((unused)) static void use_functions(void) {
    (void)foo();
    (void)bar();
}

/* Execute command and check status */
static int execute_check(const char* cmd, const char* desc) {
    printf("Executing: %s\n", desc);
    int status = system(cmd);
    if (status != 0) {
        printf("  Command returned: %d\n", WEXITSTATUS(status));
    }
    return status == 0 ? 1 : 0;
}

int main(void) {
    int checksum = 0;
    
    /* Create temporary source files */
    char* temp1 = create_temp_file("int foo(void) { return 0; }\n", ".c");
    char* temp2 = create_temp_file("int bar(void) { return 1; }\n", ".c");
    char* temp_err = create_temp_file("int baz(void) { return \n", ".c");
    
    if (!temp1 || !temp2 || !temp_err) {
        fprintf(stderr, "Failed to create temp files\n");
        return 1;
    }
    
    /* Create dump directory */
    mkdir("./dump1", 0755);
    
    printf("=== GCC Driver State Reset Coverage Test ===\n\n");
    
    /* Sequence A: Help flag then compilation (tests print_help_list reset) */
    printf("--- Sequence A: Help then Compile ---\n");
    char cmd1[512];
    snprintf(cmd1, sizeof(cmd1), "gcc --help=common > /dev/null 2>&1 && gcc -c %s -o /tmp/temp1.o", temp1);
    checksum += execute_check(cmd1, "Help then compile");
    unlink("/tmp/temp1.o");
    
    /* Sequence B: Save-temps with dumpdir then plain compile 
       (tests save_temps_flag, dumpdir, dumpbase reset) */
    printf("\n--- Sequence B: Save-temps with dumpdir then plain compile ---\n");
    char cmd2[512];
    snprintf(cmd2, sizeof(cmd2), 
             "gcc -save-temps -dumpdir ./dump1 -dumpbase mydump -c %s -o /tmp/temp2.o 2>/dev/null && "
             "gcc -c %s -o /tmp/temp3.o", temp1, temp2);
    checksum += execute_check(cmd2, "Save-temps with dumpdir then plain compile");
    unlink("/tmp/temp2.o");
    unlink("/tmp/temp3.o");
    
    /* Sequence C: Failed compilation then successful compilation 
       (tests greatest_status reset) */
    printf("\n--- Sequence C: Error then Success ---\n");
    char cmd3[512];
    snprintf(cmd3, sizeof(cmd3), 
             "gcc -c %s -o /tmp/bad.o 2>/dev/null; "
             "gcc -c %s -o /tmp/good.o", temp_err, temp1);
    checksum += execute_check(cmd3, "Error then success");
    unlink("/tmp/bad.o");
    unlink("/tmp/good.o");
    
    /* Sequence D: Verbose with linker selection then plain compile
       (tests verbose_only_flag, use_ld reset) */
    printf("\n--- Sequence D: Verbose with linker then plain compile ---\n");
    char cmd4[512];
    snprintf(cmd4, sizeof(cmd4),
             "gcc -v -fuse-ld=bfd -c %s -o /tmp/verbose.o 2>&1 | grep -q 'COLLECT_GCC' && "
             "gcc -c %s -o /tmp/plain.o", temp1, temp2);
    checksum += execute_check(cmd4, "Verbose with linker then plain");
    unlink("/tmp/verbose.o");
    unlink("/tmp/plain.o");
    
    /* Sequence E: Version flag then compilation (tests print_version reset) */
    printf("\n--- Sequence E: Version then Compile ---\n");
    char cmd5[512];
    snprintf(cmd5, sizeof(cmd5),
             "gcc --version > /dev/null 2>&1 && "
             "gcc -c %s %s -o /tmp/combined.o", temp1, temp2);
    checksum += execute_check(cmd5, "Version then compile");
    unlink("/tmp/combined.o");
    
    /* Sequence F: Multiple input files processed as separate jobs
       (tests per-job reinitialization) */
    printf("\n--- Sequence F: Multiple inputs as separate jobs ---\n");
    char cmd6[512];
    snprintf(cmd6, sizeof(cmd6),
             "gcc -save-temps -c %s %s -o /tmp/multi1.o -o /tmp/multi2.o 2>/dev/null", 
             temp1, temp2);
    checksum += execute_check(cmd6, "Multiple inputs with save-temps");
    unlink("/tmp/multi1.o");
    unlink("/tmp/multi2.o");
    
    /* Sequence G: Using -x to specify language for multiple jobs */
    printf("\n--- Sequence G: Using -x for multiple jobs ---\n");
    char* temp_cpp = create_temp_file("int x = 1;\n", ".cpp");
    if (temp_cpp) {
        char cmd7[512];
        snprintf(cmd7, sizeof(cmd7),
                 "gcc -x c --help > /dev/null 2>&1 && "
                 "gcc -x c++ -c %s -o /tmp/cpp.o 2>/dev/null", temp_cpp);
        checksum += execute_check(cmd7, "-x language specification");
        unlink("/tmp/cpp.o");
        free(temp_cpp);
    }
    
    /* Cleanup */
    printf("\n=== Cleaning up ===\n");
    cleanup_temp_files();
    
    /* Remove dump directory */
    system("rm -rf ./dump1 2>/dev/null");
    
    /* Remove any remaining .i, .s, .o files from save-temps */
    system("rm -f /tmp/*.i /tmp/*.s /tmp/*.o 2>/dev/null");
    
    printf("\nFinal checksum (successful sequences): %d/7\n", checksum);
    printf("Test completed. Check coverage for gcc.cc lines 11228-11250.\n");
    
    return 0;
}
