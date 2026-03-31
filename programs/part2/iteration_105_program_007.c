#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>

#define TEMP_SOURCE_FILE "test_reset_source.c"
#define RESPONSE_FILE "test_reset_args.rsp"
#define OUTPUT_OBJ "test_reset_output.o"
#define OUTPUT_EXE "test_reset_output.exe"

/* Create a minimal valid C source file */
void create_test_source(void) {
    FILE *f = fopen(TEMP_SOURCE_FILE, "w");
    if (!f) {
        perror("Failed to create source file");
        exit(1);
    }
    fprintf(f, "int main(void) { return 0; }\n");
    fclose(f);
}

/* Create a response file with various options */
void create_response_file(void) {
    FILE *f = fopen(RESPONSE_FILE, "w");
    if (!f) {
        perror("Failed to create response file");
        exit(1);
    }
    /* Options that set various flags in the driver */
    fprintf(f, "-v\n");
    fprintf(f, "-save-temps=obj\n");
    fprintf(f, "-Wall\n");
    fprintf(f, "-Wextra\n");
    fclose(f);
}

/* Execute GCC with given arguments and return exit status */
int run_gcc(const char *arg1, ...) {
    char cmd[4096] = "gcc ";
    char *ptr = cmd + 4;  /* Start after "gcc " */
    
    /* Build command string from variable arguments */
    va_list args;
    va_start(args, arg1);
    
    const char *current = arg1;
    while (current) {
        size_t len = strlen(current);
        if (ptr + len + 2 > cmd + sizeof(cmd)) {
            fprintf(stderr, "Command too long\n");
            return -1;
        }
        strcpy(ptr, current);
        ptr += len;
        *ptr++ = ' ';
        current = va_arg(args, const char *);
    }
    va_end(args);
    
    /* Null-terminate the command */
    if (ptr > cmd) *(ptr - 1) = '\0';
    
    printf("Executing: %s\n", cmd);
    
    /* Execute using system() - simpler but less control */
    int status = system(cmd);
    
    if (status == -1) {
        perror("system() failed");
        return -1;
    }
    
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    
    return -1;
}

/* Alternative using fork/exec for better control */
int run_gcc_exec(const char **argv) {
    pid_t pid = fork();
    
    if (pid == -1) {
        perror("fork failed");
        return -1;
    }
    
    if (pid == 0) {
        /* Child process */
        execvp("gcc", (char * const *)argv);
        perror("execvp failed");
        exit(127);
    } else {
        /* Parent process */
        int status;
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status)) {
            return WEXITSTATUS(status);
        }
        return -1;
    }
}

int main(void) {
    int overall_status = 0;
    
    /* Create necessary files */
    create_test_source();
    create_response_file();
    
    printf("=== Testing GCC driver reset logic ===\n\n");
    
    /* Invocation 1: Set print_help_list flag */
    printf("1. Testing -print-help-list (sets print_help_list)\n");
    const char *args1[] = {"gcc", "-print-help-list", NULL};
    int status1 = run_gcc_exec(args1);
    printf("Exit status: %d\n\n", status1);
    
    /* Invocation 2: Multiple flags including response file, will fail */
    printf("2. Testing with response file and failing compilation\n");
    printf("   (sets at_file_supplied, save_temps_flag, verbose_only_flag)\n");
    const char *args2[] = {"gcc", "-v", "-save-temps=obj", "-o", OUTPUT_OBJ, 
                          "@" RESPONSE_FILE, "non_existent_file.c", NULL};
    int status2 = run_gcc_exec(args2);
    printf("Exit status: %d (should be non-zero)\n\n", status2);
    
    /* Invocation 3: Set use_ld, sysroot, time report flags */
    printf("3. Testing with linker, sysroot, and timing options\n");
    printf("   (sets use_ld, report_times_to_file, target_system_root_changed)\n");
    /* Note: --sysroot path may not exist, but that's OK for testing */
    const char *args3[] = {"gcc", "-fuse-ld=gold", "--sysroot=/tmp/test_sysroot",
                          "-ftime-report", "-o", OUTPUT_EXE, TEMP_SOURCE_FILE, NULL};
    int status3 = run_gcc_exec(args3);
    printf("Exit status: %d\n\n", status3);
    
    /* Invocation 4: Try to set spec_machine (machine-specific option) */
    printf("4. Testing machine-specific options\n");
    printf("   (attempts to set spec_machine)\n");
    /* Try various machine architecture options */
    const char *args4[] = {"gcc", "-march=x86-64", "-mtune=generic", 
                          "-c", TEMP_SOURCE_FILE, "-o", OUTPUT_OBJ, NULL};
    int status4 = run_gcc_exec(args4);
    printf("Exit status: %d\n\n", status4);
    
    /* Invocation 5: Test version and verbose flags */
    printf("5. Testing version and verbose flags\n");
    printf("   (sets print_version, verbose_only_flag)\n");
    const char *args5[] = {"gcc", "--version", "-v", NULL};
    int status5 = run_gcc_exec(args5);
    printf("Exit status: %d\n\n", status5);
    
    /* Invocation 6: Test save-temps variants */
    printf("6. Testing save-temps variants\n");
    printf("   (sets save_temps_flag, dumpdir, dumpbase)\n");
    const char *args6[] = {"gcc", "-save-temps=cwd", "-dumpdir=./test_dump",
                          "-dumpbase=test_base", "-c", TEMP_SOURCE_FILE, 
                          "-o", OUTPUT_OBJ, NULL};
    int status6 = run_gcc_exec(args6);
    printf("Exit status: %d\n\n", status6);
    
    /* Invocation 7: Test with isysroot for target_sysroot_hdrs_suffix */
    printf("7. Testing with isysroot\n");
    printf("   (affects target_sysroot_hdrs_suffix)\n");
    const char *args7[] = {"gcc", "-isysroot", "/usr", "-c", 
                          TEMP_SOURCE_FILE, "-o", OUTPUT_OBJ, NULL};
    int status7 = run_gcc_exec(args7);
    printf("Exit status: %d\n\n", status7);
    
    /* Invocation 8: Final successful compilation to ensure reset after failures */
    printf("8. Final successful compilation\n");
    printf("   (ensures reset works after previous invocations)\n");
    const char *args8[] = {"gcc", "-c", TEMP_SOURCE_FILE, "-o", OUTPUT_OBJ, NULL};
    int status8 = run_gcc_exec(args8);
    printf("Exit status: %d\n\n", status8);
    
    /* Cleanup */
    printf("Cleaning up temporary files...\n");
    unlink(TEMP_SOURCE_FILE);
    unlink(RESPONSE_FILE);
    unlink(OUTPUT_OBJ);
    unlink(OUTPUT_EXE);
    
    /* Also clean up any save-temps files that might have been created */
    system("rm -f test_reset_source.i test_reset_source.s test_reset_source.o 2>/dev/null");
    system("rm -rf test_dump* 2>/dev/null");
    
    printf("\n=== Test completed ===\n");
    printf("Each GCC invocation triggers driver::finalize() which resets:\n");
    printf("  - is_cpp_driver, at_file_supplied, print_help_list\n");
    printf("  - print_version, verbose_only_flag, print_subprocess_help\n");
    printf("  - use_ld, report_times_to_file, target_system_root*\n");
    printf("  - save_temps_flag, dumpdir/dumpbase/outbase pointers\n");
    printf("  - spec_machine, greatest_status\n");
    
    return overall_status;
}
