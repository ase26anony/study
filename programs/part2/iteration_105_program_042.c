#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_CMD_LEN 1024

void create_temp_file(const char *filename, const char *content) {
    FILE *f = fopen(filename, "w");
    if (f) {
        fprintf(f, "%s", content);
        fclose(f);
    }
}

int run_gcc(const char *cmd) {
    printf("Running: %s\n", cmd);
    int status = system(cmd);
    if (WIFEXITED(status)) {
        printf("Exit status: %d\n\n", WEXITSTATUS(status));
        return WEXITSTATUS(status);
    }
    return -1;
}

int main(int argc, char **argv) {
    char cmd[MAX_CMD_LEN];
    const char *tmp_prefix = "gcc_reset_test_";
    
    // Create a minimal valid C source file
    const char *source_content = 
        "int main(void) {\n"
        "    return 0;\n"
        "}\n";
    create_temp_file("test_source.c", source_content);
    
    // Create a response file with various options
    const char *response_content = 
        "-v\n"
        "-save-temps=obj\n"
        "-ftime-report\n";
    create_temp_file("args.rsp", response_content);
    
    printf("=== Testing GCC driver reset logic ===\n\n");
    
    // Invocation 1: Set print_help_list
    printf("1. Testing -print-help-list (sets print_help_list):\n");
    run_gcc("gcc -print-help-list");
    
    // Invocation 2: Set version flag
    printf("2. Testing --version (sets print_version):\n");
    run_gcc("gcc --version");
    
    // Invocation 3: Set verbose flag and use response file (sets at_file_supplied)
    printf("3. Testing with response file (sets at_file_supplied, verbose_only_flag):\n");
    run_gcc("gcc @args.rsp -c test_source.c -o test1.o");
    
    // Invocation 4: Set save_temps_flag and related variables
    printf("4. Testing -save-temps with dumpdir (sets save_temps_flag, dumpdir, etc.):\n");
    run_gcc("gcc -save-temps -fdumpdir=./dumpdir/ -fdumpbase=test -c test_source.c -o test2.o");
    
    // Invocation 5: Set use_ld and target_system_root
    printf("5. Testing -fuse-ld and sysroot options (sets use_ld, target_system_root*):\n");
    // Note: Using / as sysroot - it exists on all systems
    run_gcc("gcc -fuse-ld=bfd --sysroot=/ -c test_source.c -o test3.o");
    
    // Invocation 6: Cause compilation failure (sets greatest_status != 1)
    printf("6. Testing failure case (should set greatest_status != 1):\n");
    run_gcc("gcc -c non_existent_file.c -o fail.o 2>/dev/null");
    
    // Invocation 7: Set spec_machine via -march/-mtune
    printf("7. Testing machine-specific options (sets spec_machine):\n");
    run_gcc("gcc -march=x86-64 -mtune=generic -c test_source.c -o test4.o");
    
    // Invocation 8: Complex combination
    printf("8. Testing complex combination of options:\n");
    run_gcc("gcc -v -save-temps=obj -ftime-report -fuse-ld=gold --sysroot=/ -march=native -c test_source.c -o test5.o");
    
    // Invocation 9: Test with different output base
    printf("9. Testing with dumpbase and outbase:\n");
    run_gcc("gcc -fdumpdir=./ -fdumpbase=mybase -fdumpbase-ext=.ext -o test6.o -c test_source.c");
    
    // Invocation 10: Final simple compilation to ensure reset worked
    printf("10. Final simple compilation (verifies reset worked):\n");
    run_gcc("gcc -c test_source.c -o final.o");
    
    // Cleanup
    remove("test_source.c");
    remove("args.rsp");
    remove("test1.o");
    remove("test2.o");
    remove("test3.o");
    remove("test4.o");
    remove("test5.o");
    remove("test6.o");
    remove("final.o");
    // Clean up dump files if they exist
    system("rm -f test_source.* test.* mybase.* *.s *.i *.o 2>/dev/null");
    
    printf("=== Test completed ===\n");
    return 0;
}
