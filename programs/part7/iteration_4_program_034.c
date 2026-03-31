#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#define TEMP_SOURCE_FILE "test_coverage_XXXXXX.c"
#define TEMP_BINARY_FILE "test_coverage_XXXXXX"

int create_temp_file(char *template, const char *content) {
    int fd = mkstemps(template, 2);  // 2 for ".c" extension
    if (fd < 0) {
        perror("mkstemps failed");
        return -1;
    }
    
    FILE *f = fdopen(fd, "w");
    if (!f) {
        perror("fdopen failed");
        close(fd);
        return -1;
    }
    
    fputs(content, f);
    fclose(f);
    return 0;
}

int compile_with_coverage(const char *source_file, char *binary_file) {
    char compile_cmd[512];
    snprintf(compile_cmd, sizeof(compile_cmd),
             "gcc -fprofile-arcs -ftest-coverage %s -o %s",
             source_file, binary_file);
    
    printf("Compiling: %s\n", compile_cmd);
    return system(compile_cmd);
}

int execute_program(const char *binary_file) {
    char exec_cmd[256];
    snprintf(exec_cmd, sizeof(exec_cmd), "./%s", binary_file);
    return system(exec_cmd);
}

int invoke_gcov_dump(const char *args, int capture_stderr) {
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "gcov-dump %s 2>&1", args);
    
    if (capture_stderr) {
        FILE *pipe = popen(cmd, "r");
        if (!pipe) {
            perror("popen failed");
            return -1;
        }
        
        char buffer[1024];
        int found_error = 0;
        while (fgets(buffer, sizeof(buffer), pipe)) {
            if (strstr(buffer, "unknown flag")) {
                printf("SUCCESS: Triggered default case with error: %s", buffer);
                found_error = 1;
            }
        }
        
        pclose(pipe);
        return found_error ? 0 : -1;
    } else {
        return system(cmd);
    }
}

int main() {
    int ret = 0;
    char source_file[] = TEMP_SOURCE_FILE;
    char binary_file[] = TEMP_BINARY_FILE;
    char gcda_file[256];
    
    // Create a simple C source file with coverage instrumentation
    const char *source_code = 
        "#include <stdio.h>\n"
        "int main() {\n"
        "    int i, sum = 0;\n"
        "    for (i = 0; i < 10; i++) {\n"
        "        sum += i;\n"
        "    }\n"
        "    printf(\"Sum: %d\\n\", sum);\n"
        "    return 0;\n"
        "}\n";
    
    printf("=== Creating test coverage source file ===\n");
    if (create_temp_file(source_file, source_code) < 0) {
        fprintf(stderr, "Failed to create source file\n");
        return 1;
    }
    
    // Remove .c extension for binary file
    char *dot = strrchr(binary_file, '.');
    if (dot) *dot = '\0';
    
    printf("=== Compiling with coverage flags ===\n");
    if (compile_with_coverage(source_file, binary_file) != 0) {
        fprintf(stderr, "Compilation failed\n");
        ret = 1;
        goto cleanup;
    }
    
    printf("=== Executing program to generate .gcda file ===\n");
    if (execute_program(binary_file) != 0) {
        fprintf(stderr, "Execution failed\n");
        ret = 1;
        goto cleanup;
    }
    
    // Construct .gcda filename
    snprintf(gcda_file, sizeof(gcda_file), "%s.gcda", binary_file);
    
    // Check if .gcda file was created
    struct stat st;
    if (stat(gcda_file, &st) != 0) {
        fprintf(stderr, "No .gcda file generated: %s\n", gcda_file);
        ret = 1;
        goto cleanup;
    }
    
    printf("\n=== Testing gcov-dump with various flags ===\n");
    
    // 1. Test -h flag (help)
    printf("\n1. Testing -h flag (help):\n");
    if (invoke_gcov_dump("-h", 0) != 0) {
        fprintf(stderr, "Failed to invoke gcov-dump -h\n");
    }
    
    // 2. Test -v flag (version)
    printf("\n2. Testing -v flag (version):\n");
    if (invoke_gcov_dump("-v", 0) != 0) {
        fprintf(stderr, "Failed to invoke gcov-dump -v\n");
    }
    
    // 3. Test -l flag with .gcda file
    printf("\n3. Testing -l flag (dump contents):\n");
    char cmd_l[256];
    snprintf(cmd_l, sizeof(cmd_l), "-l %s", gcda_file);
    if (invoke_gcov_dump(cmd_l, 0) != 0) {
        fprintf(stderr, "Failed to invoke gcov-dump -l\n");
    }
    
    // 4. Test -p flag with .gcda file
    printf("\n4. Testing -p flag (dump positions):\n");
    char cmd_p[256];
    snprintf(cmd_p, sizeof(cmd_p), "-p %s", gcda_file);
    if (invoke_gcov_dump(cmd_p, 0) != 0) {
        fprintf(stderr, "Failed to invoke gcov-dump -p\n");
    }
    
    // 5. Test -r flag with .gcda file
    printf("\n5. Testing -r flag (dump raw):\n");
    char cmd_r[256];
    snprintf(cmd_r, sizeof(cmd_r), "-r %s", gcda_file);
    if (invoke_gcov_dump(cmd_r, 0) != 0) {
        fprintf(stderr, "Failed to invoke gcov-dump -r\n");
    }
    
    // 6. Test -s flag with .gcda file
    printf("\n6. Testing -s flag (dump stable):\n");
    char cmd_s[256];
    snprintf(cmd_s, sizeof(cmd_s), "-s %s", gcda_file);
    if (invoke_gcov_dump(cmd_s, 0) != 0) {
        fprintf(stderr, "Failed to invoke gcov-dump -s\n");
    }
    
    // 7. Test combined flags
    printf("\n7. Testing combined flags (-l -p):\n");
    char cmd_combined[256];
    snprintf(cmd_combined, sizeof(cmd_combined), "-l -p %s", gcda_file);
    if (invoke_gcov_dump(cmd_combined, 0) != 0) {
        fprintf(stderr, "Failed to invoke gcov-dump -l -p\n");
    }
    
    // 8. Test invalid flag to trigger default case
    printf("\n8. Testing invalid flag (should trigger default case):\n");
    char cmd_invalid[256];
    snprintf(cmd_invalid, sizeof(cmd_invalid), "-X %s", gcda_file);
    if (invoke_gcov_dump(cmd_invalid, 1) != 0) {
        fprintf(stderr, "Failed to trigger default case with invalid flag\n");
    }
    
    // 9. Additional test: multiple invalid flags
    printf("\n9. Testing multiple invalid flags:\n");
    if (invoke_gcov_dump("-X -Y -Z", 1) != 0) {
        fprintf(stderr, "Failed to trigger default case with multiple invalid flags\n");
    }

cleanup:
    printf("\n=== Cleaning up temporary files ===\n");
    
    // Remove generated files
    char rm_cmd[512];
    snprintf(rm_cmd, sizeof(rm_cmd), "rm -f %s %s %s %s.gcno %s.gcda",
             source_file, binary_file, 
             binary_file, binary_file, binary_file);
    system(rm_cmd);
    
    printf("Test completed with return code: %d\n", ret);
    return ret;
}
