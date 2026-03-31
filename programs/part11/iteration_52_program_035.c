/* Test program to exercise GCC driver initialization logic (lines 11228-11250 in gcc.cc) */
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

static char* create_temp_file(const char* content, const char* suffix) {
    char template[] = "/tmp/gcc_test_XXXXXX";
    int fd = mkstemp(template);
    if (fd == -1) {
        perror("mkstemp failed");
        return NULL;
    }
    
    /* Write content */
    if (content) {
        write(fd, content, strlen(content));
    }
    
    close(fd);
    
    /* Create final filename with suffix */
    char* final_name = malloc(strlen(template) + strlen(suffix) + 1);
    sprintf(final_name, "%s%s", template, suffix);
    
    /* Rename to add suffix */
    if (rename(template, final_name) != 0) {
        free(final_name);
        return NULL;
    }
    
    /* Store for cleanup */
    if (temp_file_count < 10) {
        strcpy(temp_files[temp_file_count++], final_name);
    }
    
    return final_name;
}

static void cleanup_temp_files(void) {
    for (int i = 0; i < temp_file_count; i++) {
        if (temp_files[i][0]) {
            unlink(temp_files[i]);
        }
    }
}

static int run_gcc_command(const char* cmd) {
    printf("Running: %s\n", cmd);
    int status = system(cmd);
    if (status != 0) {
        printf("Command returned non-zero: %d\n", WEXITSTATUS(status));
    }
    return status;
}

int main(void) {
    int checksum = 0;
    
    /* Create temporary source files */
    char* valid1 = create_temp_file("int foo(void) { return 0; }\n", ".c");
    char* valid2 = create_temp_file("int bar(void) { return 1; }\n", ".c");
    char* error = create_temp_file("int baz(void) { return \n", ".c");
    
    if (!valid1 || !valid2 || !error) {
        fprintf(stderr, "Failed to create temp files\n");
        cleanup_temp_files();
        return 1;
    }
    
    /* Create dump directory */
    mkdir("./dump_test_dir", 0755);
    
    /* Sequence 1: Help flag followed by compilation
       This sets print_help_list, then resets it */
    printf("\n=== Sequence 1: Help then compile ===\n");
    checksum += (run_gcc_command("gcc --help=common > /dev/null 2>&1") == 0 ? 1 : 0);
    checksum += (run_gcc_command("gcc -c ") + valid1 + " -o /tmp/test1.o 2>/dev/null") == 0 ? 2 : 0);
    
    /* Sequence 2: Save-temps with dumpdir followed by plain compilation
       This sets save_temps_flag, dumpdir, dumpbase, then resets them */
    printf("\n=== Sequence 2: Save-temps with dumpdir then compile ===\n");
    checksum += (run_gcc_command("gcc -save-temps -dumpdir ./dump_test_dir -dumpbase mydump -c ") + 
                 valid1 + " -o /tmp/test2.o 2>/dev/null") == 0 ? 4 : 0);
    checksum += (run_gcc_command("gcc -c ") + valid2 + " -o /tmp/test3.o 2>/dev/null") == 0 ? 8 : 0);
    
    /* Sequence 3: Version flag followed by compilation
       This sets print_version, then resets it */
    printf("\n=== Sequence 3: Version then compile ===\n");
    checksum += (run_gcc_command("gcc --version > /dev/null 2>&1") == 0 ? 16 : 0);
    checksum += (run_gcc_command("gcc -c ") + valid2 + " -o /tmp/test4.o 2>/dev/null") == 0 ? 32 : 0);
    
    /* Sequence 4: Verbose flag with linker selection followed by plain compile
       This sets verbose_only_flag and use_ld, then resets them */
    printf("\n=== Sequence 4: Verbose with linker then plain ===\n");
    checksum += (run_gcc_command("gcc -v -fuse-ld=bfd -c ") + valid1 + 
                 " -o /tmp/test5.o 2>&1 | grep -q 'COLLECT_GCC_OPTIONS'") == 0 ? 64 : 0);
    checksum += (run_gcc_command("gcc -c ") + valid2 + " -o /tmp/test6.o 2>/dev/null") == 0 ? 128 : 0);
    
    /* Sequence 5: Error recovery - failed compilation followed by success
       This exercises greatest_status reset logic */
    printf("\n=== Sequence 5: Error then success ===\n");
    checksum += (run_gcc_command("gcc -c ") + error + " 2>/dev/null") != 0 ? 256 : 0);
    checksum += (run_gcc_command("gcc -c ") + valid1 + " -o /tmp/test7.o 2>/dev/null") == 0 ? 512 : 0);
    
    /* Sequence 6: Multiple inputs in single invocation (two compilation jobs)
       This should trigger initialization between the two internal jobs */
    printf("\n=== Sequence 6: Multiple source files in one command ===\n");
    checksum += (run_gcc_command("gcc -c ") + valid1 + " " + valid2 + 
                 " -o /tmp/test8.o 2>/dev/null") != 0 ? 1024 : 0);
    
    /* Sequence 7: Using -x to specify language (creates distinct jobs) */
    printf("\n=== Sequence 7: Using -x for explicit language specification ===\n");
    char* cpp_content = create_temp_file("#include <stdio.h>\nint main() { return 0; }", ".cpp");
    if (cpp_content) {
        checksum += (run_gcc_command("gcc -x c ") + valid1 + " -x c++ " + cpp_content + 
                     " -c -o /tmp/test9.o 2>/dev/null") != 0 ? 2048 : 0);
    }
    
    /* Cleanup */
    printf("\n=== Cleaning up ===\n");
    system("rm -f /tmp/test*.o");
    system("rm -rf ./dump_test_dir");
    cleanup_temp_files();
    
    /* Free allocated memory */
    free(valid1);
    free(valid2);
    free(error);
    if (cpp_content) free(cpp_content);
    
    printf("\nFinal checksum: %d (0x%x)\n", checksum, checksum);
    printf("Test completed. Each bit in checksum represents a successful state transition.\n");
    
    return 0;
}
