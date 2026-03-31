/* test_gcc_driver_init.c - Test program to cover GCC driver initialization block */
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
    char template[256];
    if (suffix) {
        snprintf(template, sizeof(template), "/tmp/gcc_test_XXXXXX%s", suffix);
    } else {
        snprintf(template, sizeof(template), "/tmp/gcc_test_XXXXXX.c");
    }
    
    int fd = mkstemps(template, suffix ? (int)strlen(suffix) : 2);
    if (fd < 0) {
        perror("mkstemps failed");
        return NULL;
    }
    
    FILE* f = fdopen(fd, "w");
    if (!f) {
        perror("fdopen failed");
        close(fd);
        return NULL;
    }
    
    fwrite(content, 1, strlen(content), f);
    fclose(f);
    
    char* filename = strdup(template);
    if (filename && temp_file_count < 10) {
        strcpy(temp_files[temp_file_count++], filename);
    }
    
    return filename;
}

static void cleanup_temp_files(void) {
    for (int i = 0; i < temp_file_count; i++) {
        if (temp_files[i][0]) {
            unlink(temp_files[i]);
            temp_files[i][0] = '\0';
        }
    }
    temp_file_count = 0;
}

static int run_gcc_command(const char* format, ...) {
    char cmd[1024];
    va_list args;
    va_start(args, format);
    vsnprintf(cmd, sizeof(cmd), format, args);
    va_end(args);
    
    printf("Executing: %s\n", cmd);
    int status = system(cmd);
    
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    return -1;
}

/* Dummy functions to avoid unused function warnings */
static int foo(void) { return 0; }
static int bar(void) { return 1; }
static int baz(void) { return 2; }

int main(void) {
    int checksum = 0;
    int result;
    
    /* Create temporary source files */
    char* temp1 = create_temp_file("int foo(void) { return 0; }\n", ".c");
    char* temp2 = create_temp_file("int bar(void) { return 1; }\n", ".c");
    char* temp3 = create_temp_file("int baz(void) { return \n", ".c"); /* Syntax error */
    char* temp4 = create_temp_file("int qux(void) { return 3; }\n", ".c");
    
    if (!temp1 || !temp2 || !temp3 || !temp4) {
        fprintf(stderr, "Failed to create temp files\n");
        cleanup_temp_files();
        return 1;
    }
    
    /* Create temporary directory for dump tests */
    mkdir("./dump_test_dir", 0755);
    
    printf("=== Testing GCC Driver Initialization Block (lines 11228-11250) ===\n\n");
    
    /* Sequence 1: Help flag then compilation - tests print_help_list reset */
    printf("--- Sequence 1: Help then Compile ---\n");
    result = run_gcc_command("gcc --help=common 2>&1 | head -5 > /dev/null");
    checksum += (result == 0) ? 1 : 0;
    
    result = run_gcc_command("gcc -c %s -o %s.o", temp1, temp1);
    checksum += (result == 0) ? 2 : 0;
    
    /* Sequence 2: Version flag then compilation - tests print_version reset */
    printf("\n--- Sequence 2: Version then Compile ---\n");
    result = run_gcc_command("gcc --version 2>&1 | head -1 > /dev/null");
    checksum += (result == 0) ? 4 : 0;
    
    result = run_gcc_command("gcc -c %s -o %s_v2.o", temp2, temp2);
    checksum += (result == 0) ? 8 : 0;
    
    /* Sequence 3: Save-temps with dumpdir then plain compile - tests save_temps_flag, dumpdir reset */
    printf("\n--- Sequence 3: Save-temps with dumpdir then Plain Compile ---\n");
    result = run_gcc_command("gcc -save-temps -dumpdir ./dump_test_dir -dumpbase mytest -c %s -o %s_save.o 2>&1", 
                            temp1, temp1);
    checksum += (result == 0) ? 16 : 0;
    
    /* Check if dump files were created */
    struct stat st;
    if (stat("./dump_test_dir/mytest.i", &st) == 0) {
        checksum += 32;
    }
    
    result = run_gcc_command("gcc -c %s -o %s_plain.o", temp2, temp2);
    checksum += (result == 0) ? 64 : 0;
    
    /* Sequence 4: Verbose flag then normal compile - tests verbose_only_flag reset */
    printf("\n--- Sequence 4: Verbose then Normal ---\n");
    result = run_gcc_command("gcc -v -c %s -o %s_verbose.o 2>&1 | grep -q 'COLLECT_GCC_OPTIONS'", 
                            temp1, temp1);
    checksum += (result == 0) ? 128 : 0;
    
    result = run_gcc_command("gcc -c %s -o %s_normal.o", temp2, temp2);
    checksum += (result == 0) ? 256 : 0;
    
    /* Sequence 5: Linker selection then default - tests use_ld reset */
    printf("\n--- Sequence 5: Specific Linker then Default ---\n");
    result = run_gcc_command("gcc -fuse-ld=bfd -c %s -o %s_bfd.o 2>&1", temp1, temp1);
    checksum += (result == 0) ? 512 : 0;
    
    result = run_gcc_command("gcc -c %s -o %s_default.o", temp2, temp2);
    checksum += (result == 0) ? 1024 : 0;
    
    /* Sequence 6: Error then success - tests greatest_status reset */
    printf("\n--- Sequence 6: Error then Success ---\n");
    result = run_gcc_command("gcc -c %s 2>/dev/null", temp3); /* This should fail */
    checksum += (result != 0) ? 2048 : 0;
    
    result = run_gcc_command("gcc -c %s -o %s_after_error.o", temp4, temp4);
    checksum += (result == 0) ? 4096 : 0;
    
    /* Sequence 7: Multiple inputs in single invocation - tests per-job initialization */
    printf("\n--- Sequence 7: Multiple Inputs Single Invocation ---\n");
    result = run_gcc_command("gcc -c %s %s -save-temps -dumpdir ./dump_test_dir/multi", 
                            temp1, temp2);
    checksum += (result == 0) ? 8192 : 0;
    
    /* Sequence 8: Different language specs with -x flag */
    printf("\n--- Sequence 8: Different Language Specs ---\n");
    char* temp_cpp = create_temp_file("#include <iostream>\nint main() { return 0; }", ".cpp");
    if (temp_cpp) {
        result = run_gcc_command("gcc -x c++ -c %s -o %s.o 2>&1", temp_cpp, temp_cpp);
        checksum += (result == 0) ? 16384 : 0;
    }
    
    /* Sequence 9: Combined flags then reset */
    printf("\n--- Sequence 9: Combined Flags then Clean ---\n");
    result = run_gcc_command("gcc -v --help=optimizers -save-temps -dumpbase combined -c %s 2>&1 | head -20 > /dev/null", 
                            temp1);
    checksum += (result == 0) ? 32768 : 0;
    
    result = run_gcc_command("gcc -c %s", temp2);
    checksum += (result == 0) ? 65536 : 0;
    
    /* Cleanup */
    printf("\n--- Cleaning Up ---\n");
    
    /* Remove generated object files */
    char* objs[] = {
        "*.o", "*.i", "*.s", "*.ii", "*.bc", 
        "./dump_test_dir/*"
    };
    
    for (size_t i = 0; i < sizeof(objs)/sizeof(objs[0]); i++) {
        char cmd[256];
        snprintf(cmd, sizeof(cmd), "rm -f %s 2>/dev/null", objs[i]);
        system(cmd);
    }
    
    /* Remove dump directory */
    system("rm -rf ./dump_test_dir 2>/dev/null");
    
    cleanup_temp_files();
    
    /* Reference dummy functions to avoid unused warnings */
    (void)foo();
    (void)bar();
    (void)baz();
    
    printf("\n=== Test Complete ===\n");
    printf("Checksum: 0x%08x\n", checksum);
    printf("Expected minimum: 0x0000FFFF (multiple sequences successful)\n");
    
    return (checksum > 0xFFFF) ? 0 : 1;
}
