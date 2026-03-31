#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_PATH 256

int main() {
    char source_file[MAX_PATH] = "/tmp/test_coverage_XXXXXX.c";
    char binary_file[MAX_PATH] = "/tmp/test_coverage_XXXXXX";
    char gcda_file[MAX_PATH];
    char gcno_file[MAX_PATH];
    char command[512];
    FILE *fp;
    int fd;
    
    // Create unique filenames
    char *temp_source = strdup("/tmp/test_coverage_XXXXXX.c");
    char *temp_binary = strdup("/tmp/test_coverage_XXXXXX");
    
    // Create source file
    fd = mkstemps(temp_source, 2);  // .c extension is 2 chars
    if (fd < 0) {
        perror("Failed to create temp source file");
        free(temp_source);
        free(temp_binary);
        return 1;
    }
    close(fd);
    
    strcpy(source_file, temp_source);
    strcpy(binary_file, temp_binary);
    
    // Generate .gcda and .gcno filenames
    snprintf(gcda_file, sizeof(gcda_file), "%s.gcda", binary_file);
    snprintf(gcno_file, sizeof(gcno_file), "%s.gcno", binary_file);
    
    // Write simple test program with coverage
    fp = fopen(source_file, "w");
    if (!fp) {
        perror("Failed to open source file");
        free(temp_source);
        free(temp_binary);
        return 1;
    }
    
    fprintf(fp, "#include <stdio.h>\n\n");
    fprintf(fp, "int main() {\n");
    fprintf(fp, "    int i, sum = 0;\n");
    fprintf(fp, "    for (i = 0; i < 10; i++) {\n");
    fprintf(fp, "        sum += i;\n");
    fprintf(fp, "    }\n");
    fprintf(fp, "    printf(\"Sum: %%d\\n\", sum);\n");
    fprintf(fp, "    return 0;\n");
    fprintf(fp, "}\n");
    fclose(fp);
    
    printf("Created source file: %s\n", source_file);
    
    // Compile with coverage flags
    snprintf(command, sizeof(command), 
             "gcc -fprofile-arcs -ftest-coverage %s -o %s",
             source_file, binary_file);
    printf("Compiling: %s\n", command);
    
    if (system(command) != 0) {
        fprintf(stderr, "Compilation failed\n");
        free(temp_source);
        free(temp_binary);
        return 1;
    }
    
    // Execute to generate .gcda file
    printf("Executing to generate coverage data...\n");
    if (system(binary_file) != 0) {
        fprintf(stderr, "Execution failed\n");
        free(temp_source);
        free(temp_binary);
        return 1;
    }
    
    // Verify .gcda file exists
    if (access(gcda_file, F_OK) != 0) {
        fprintf(stderr, "No .gcda file generated: %s\n", gcda_file);
        free(temp_source);
        free(temp_binary);
        return 1;
    }
    
    printf("Generated coverage file: %s\n", gcda_file);
    
    // Now invoke gcov-dump with various flags to trigger the switch cases
    
    printf("\n=== Triggering gcov-dump switch cases ===\n\n");
    
    // 1. Trigger case 'h' - help
    printf("1. Testing -h flag (help):\n");
    snprintf(command, sizeof(command), "gcov-dump -h");
    system(command);
    printf("\n");
    
    // 2. Trigger case 'v' - version
    printf("2. Testing -v flag (version):\n");
    snprintf(command, sizeof(command), "gcov-dump -v");
    system(command);
    printf("\n");
    
    // 3. Trigger case 'l' - dump contents
    printf("3. Testing -l flag (dump contents):\n");
    snprintf(command, sizeof(command), "gcov-dump -l %s", gcda_file);
    system(command);
    printf("\n");
    
    // 4. Trigger case 'p' - dump positions
    printf("4. Testing -p flag (dump positions):\n");
    snprintf(command, sizeof(command), "gcov-dump -p %s", gcda_file);
    system(command);
    printf("\n");
    
    // 5. Trigger case 'r' - dump raw
    printf("5. Testing -r flag (dump raw):\n");
    snprintf(command, sizeof(command), "gcov-dump -r %s", gcda_file);
    system(command);
    printf("\n");
    
    // 6. Trigger case 's' - dump stable
    printf("6. Testing -s flag (dump stable):\n");
    snprintf(command, sizeof(command), "gcov-dump -s %s", gcda_file);
    system(command);
    printf("\n");
    
    // 7. Test combined flags
    printf("7. Testing combined flags -l -p:\n");
    snprintf(command, sizeof(command), "gcov-dump -l -p %s", gcda_file);
    system(command);
    printf("\n");
    
    // 8. Trigger default case with invalid flag
    printf("8. Testing invalid flag -X (should trigger default case):\n");
    snprintf(command, sizeof(command), "gcov-dump -X %s 2>&1", gcda_file);
    
    // Use popen to capture stderr and check for "unknown flag" message
    fp = popen(command, "r");
    if (fp) {
        char buffer[256];
        int found_error = 0;
        
        while (fgets(buffer, sizeof(buffer), fp) != NULL) {
            printf("%s", buffer);
            if (strstr(buffer, "unknown flag") != NULL) {
                found_error = 1;
            }
        }
        
        pclose(fp);
        
        if (found_error) {
            printf("SUCCESS: Triggered default case with 'unknown flag' error\n");
        } else {
            printf("WARNING: Didn't see 'unknown flag' error message\n");
        }
    }
    printf("\n");
    
    // Cleanup
    printf("Cleaning up temporary files...\n");
    unlink(source_file);
    unlink(binary_file);
    unlink(gcda_file);
    unlink(gcno_file);
    
    free(temp_source);
    free(temp_binary);
    
    printf("Test completed successfully!\n");
    return 0;
}
