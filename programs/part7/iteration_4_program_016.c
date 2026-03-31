#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#define TEMP_SOURCE "test_coverage_XXXXXX.c"
#define TEMP_BINARY "test_coverage_XXXXXX"

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

int main() {
    char source_file[] = TEMP_SOURCE;
    char binary_file[] = TEMP_BINARY;
    char gcda_file[256];
    char command[512];
    FILE *fp;
    char buffer[1024];
    int result;
    
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
    
    printf("Creating test source file...\n");
    if (create_temp_file(source_file, source_code) < 0) {
        fprintf(stderr, "Failed to create source file\n");
        return 1;
    }
    
    // Remove .c extension from binary template
    char *dot = strrchr(binary_file, 'X');
    if (dot) *dot = '\0';
    
    // Compile with coverage flags
    printf("Compiling with coverage instrumentation...\n");
    snprintf(command, sizeof(command),
             "gcc -fprofile-arcs -ftest-coverage %s -o %s",
             source_file, binary_file);
    result = system(command);
    if (result != 0) {
        fprintf(stderr, "Compilation failed\n");
        unlink(source_file);
        return 1;
    }
    
    // Execute to generate .gcda file
    printf("Executing to generate coverage data...\n");
    snprintf(command, sizeof(command), "./%s", binary_file);
    result = system(command);
    if (result != 0) {
        fprintf(stderr, "Execution failed\n");
        unlink(source_file);
        unlink(binary_file);
        return 1;
    }
    
    // Construct .gcda filename
    snprintf(gcda_file, sizeof(gcda_file), "%s.gcda", binary_file);
    
    // Verify .gcda file exists
    struct stat st;
    if (stat(gcda_file, &st) != 0) {
        fprintf(stderr, "No .gcda file generated: %s\n", gcda_file);
        unlink(source_file);
        unlink(binary_file);
        return 1;
    }
    
    printf("\n=== Testing gcov-dump with various flags ===\n\n");
    
    // 1. Test -h flag (help)
    printf("1. Testing -h flag (help):\n");
    snprintf(command, sizeof(command), "gcov-dump -h 2>&1");
    system(command);
    printf("\n");
    
    // 2. Test -v flag (version)
    printf("2. Testing -v flag (version):\n");
    snprintf(command, sizeof(command), "gcov-dump -v 2>&1");
    system(command);
    printf("\n");
    
    // 3. Test -l flag (dump contents)
    printf("3. Testing -l flag (dump contents):\n");
    snprintf(command, sizeof(command), "gcov-dump -l %s 2>&1 | head -20", gcda_file);
    system(command);
    printf("\n");
    
    // 4. Test -p flag (dump positions)
    printf("4. Testing -p flag (dump positions):\n");
    snprintf(command, sizeof(command), "gcov-dump -p %s 2>&1 | head -20", gcda_file);
    system(command);
    printf("\n");
    
    // 5. Test -r flag (dump raw)
    printf("5. Testing -r flag (dump raw):\n");
    snprintf(command, sizeof(command), "gcov-dump -r %s 2>&1 | head -20", gcda_file);
    system(command);
    printf("\n");
    
    // 6. Test -s flag (dump stable)
    printf("6. Testing -s flag (dump stable):\n");
    snprintf(command, sizeof(command), "gcov-dump -s %s 2>&1 | head -20", gcda_file);
    system(command);
    printf("\n");
    
    // 7. Test combined flags (-l -p)
    printf("7. Testing combined flags (-l -p):\n");
    snprintf(command, sizeof(command), "gcov-dump -l -p %s 2>&1 | head -20", gcda_file);
    system(command);
    printf("\n");
    
    // 8. Test invalid flag (-X) and capture stderr
    printf("8. Testing invalid flag (-X):\n");
    snprintf(command, sizeof(command), "gcov-dump -X %s 2>&1", gcda_file);
    
    fp = popen(command, "r");
    if (fp) {
        int found_error = 0;
        while (fgets(buffer, sizeof(buffer), fp) != NULL) {
            printf("%s", buffer);
            if (strstr(buffer, "unknown flag")) {
                found_error = 1;
            }
        }
        pclose(fp);
        
        if (found_error) {
            printf("SUCCESS: Triggered 'unknown flag' error message\n");
        } else {
            printf("WARNING: Did not see expected error message\n");
        }
    } else {
        fprintf(stderr, "Failed to execute command\n");
    }
    printf("\n");
    
    // Cleanup
    printf("Cleaning up temporary files...\n");
    unlink(source_file);
    unlink(binary_file);
    unlink(gcda_file);
    
    // Also clean up .gcno file if it exists
    char gcno_file[256];
    snprintf(gcno_file, sizeof(gcno_file), "%s.gcno", binary_file);
    unlink(gcno_file);
    
    printf("Test completed successfully!\n");
    return 0;
}
