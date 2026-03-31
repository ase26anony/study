/**
 * test_gcov_dump_switches.c
 * 
 * Test driver to exercise specific command-line switch cases in gcov-dump
 * Targets lines 111-130 of gcov-dump.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>

/* Simple test program to generate coverage data */
const char *test_source = 
"#include <stdio.h>\n"
"int main() {\n"
"    int i, sum = 0;\n"
"    for (i = 0; i < 10; i++) {\n"
"        sum += i;\n"
"    }\n"
"    printf(\"Sum: %d\\n\", sum);\n"
"    return 0;\n"
"}\n";

/**
 * Create a temporary filename with given suffix
 */
char *create_temp_filename(const char *suffix) {
    char *template = strdup("/tmp/gcov_test_XXXXXX");
    int fd = mkstemp(template);
    if (fd == -1) {
        perror("mkstemp failed");
        free(template);
        return NULL;
    }
    close(fd);
    
    char *result = malloc(strlen(template) + strlen(suffix) + 1);
    sprintf(result, "%s%s", template, suffix);
    free(template);
    return result;
}

/**
 * Execute a command and capture its stderr
 * Returns 1 if "unknown flag" message found, 0 otherwise
 */
int check_for_unknown_flag(const char *cmd) {
    char buffer[1024];
    int found = 0;
    
    /* Use popen to capture stderr (2>&1 redirects stderr to stdout) */
    char full_cmd[2048];
    snprintf(full_cmd, sizeof(full_cmd), "%s 2>&1", cmd);
    
    FILE *fp = popen(full_cmd, "r");
    if (fp == NULL) {
        perror("popen failed");
        return 0;
    }
    
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        if (strstr(buffer, "unknown flag") != NULL) {
            found = 1;
            printf("  -> Successfully triggered 'unknown flag' message: %s", buffer);
        }
    }
    
    pclose(fp);
    return found;
}

/**
 * Execute a command and print status
 */
void execute_command(const char *description, const char *cmd, int check_error) {
    printf("\n=== %s ===\n", description);
    printf("Command: %s\n", cmd);
    
    if (check_error) {
        if (check_for_unknown_flag(cmd)) {
            printf("  -> Default switch case triggered successfully\n");
        } else {
            printf("  -> Warning: 'unknown flag' message not found\n");
        }
    } else {
        int result = system(cmd);
        if (result != 0) {
            printf("  -> Command exited with code %d\n", result);
        } else {
            printf("  -> Command executed successfully\n");
        }
    }
}

int main(int argc, char *argv[]) {
    char *source_file = NULL;
    char *binary_file = NULL;
    char *gcda_file = NULL;
    char *gcno_file = NULL;
    int cleanup = 1;
    
    printf("========================================\n");
    printf("Testing gcov-dump command-line switches\n");
    printf("Target: lines 111-130 of gcov-dump.cc\n");
    printf("========================================\n");
    
    /* Create temporary filenames */
    source_file = create_temp_filename(".c");
    binary_file = create_temp_filename("");
    gcda_file = create_temp_filename(".gcda");
    gcno_file = create_temp_filename(".gcno");
    
    if (!source_file || !binary_file || !gcda_file || !gcno_file) {
        fprintf(stderr, "Failed to create temporary filenames\n");
        goto cleanup;
    }
    
    /* Step 1: Create test source file */
    printf("\n1. Creating test source file: %s\n", source_file);
    FILE *fp = fopen(source_file, "w");
    if (!fp) {
        perror("Failed to create source file");
        goto cleanup;
    }
    fputs(test_source, fp);
    fclose(fp);
    
    /* Step 2: Compile with coverage instrumentation */
    printf("\n2. Compiling with coverage flags...\n");
    char compile_cmd[1024];
    snprintf(compile_cmd, sizeof(compile_cmd),
             "gcc -fprofile-arcs -ftest-coverage %s -o %s",
             source_file, binary_file);
    
    printf("Compile command: %s\n", compile_cmd);
    if (system(compile_cmd) != 0) {
        fprintf(stderr, "Compilation failed\n");
        goto cleanup;
    }
    
    /* The .gcno file should be created during compilation */
    char expected_gcno[1024];
    snprintf(expected_gcno, sizeof(expected_gcno), "%s.gcno", source_file);
    if (rename(expected_gcno, gcno_file) != 0) {
        /* Try copying instead if rename fails */
        char cp_cmd[1024];
        snprintf(cp_cmd, sizeof(cp_cmd), "cp %s %s", expected_gcno, gcno_file);
        system(cp_cmd);
    }
    
    /* Step 3: Execute to generate .gcda file */
    printf("\n3. Executing program to generate coverage data...\n");
    if (system(binary_file) != 0) {
        fprintf(stderr, "Execution failed\n");
        goto cleanup;
    }
    
    /* The .gcda file should be created during execution */
    char expected_gcda[1024];
    snprintf(expected_gcda, sizeof(expected_gcda), "%s.gcda", source_file);
    if (rename(expected_gcda, gcda_file) != 0) {
        /* Try copying instead if rename fails */
        char cp_cmd[1024];
        snprintf(cp_cmd, sizeof(cp_cmd), "cp %s %s", expected_gcda, gcda_file);
        system(cp_cmd);
    }
    
    /* Verify .gcda file exists */
    struct stat st;
    if (stat(gcda_file, &st) != 0 || st.st_size == 0) {
        fprintf(stderr, "Warning: .gcda file not created or empty\n");
        printf("Trying alternative approach...\n");
        
        /* Try direct execution with env var */
        char exec_cmd[1024];
        snprintf(exec_cmd, sizeof(exec_cmd), 
                 "GCOV_PREFIX=%s GCOV_PREFIX_STRIP=1 %s",
                 "/tmp", binary_file);
        system(exec_cmd);
    }
    
    printf("\n4. Testing gcov-dump switches...\n");
    
    /* Test case 1: -h flag (help) - triggers print_usage() */
    execute_command("Testing -h flag (help)", "gcov-dump -h", 0);
    
    /* Test case 2: -v flag (version) - triggers print_version() */
    execute_command("Testing -v flag (version)", "gcov-dump -v", 0);
    
    /* Test case 3: -l flag with .gcda file - sets flag_dump_contents */
    char cmd_l[1024];
    snprintf(cmd_l, sizeof(cmd_l), "gcov-dump -l %s", gcda_file);
    execute_command("Testing -l flag (dump contents)", cmd_l, 0);
    
    /* Test case 4: -p flag with .gcda file - sets flag_dump_positions */
    char cmd_p[1024];
    snprintf(cmd_p, sizeof(cmd_p), "gcov-dump -p %s", gcda_file);
    execute_command("Testing -p flag (dump positions)", cmd_p, 0);
    
    /* Test case 5: -r flag with .gcda file - sets flag_dump_raw */
    char cmd_r[1024];
    snprintf(cmd_r, sizeof(cmd_r), "gcov-dump -r %s", gcda_file);
    execute_command("Testing -r flag (dump raw)", cmd_r, 0);
    
    /* Test case 6: -s flag with .gcda file - sets flag_dump_stable */
    char cmd_s[1024];
    snprintf(cmd_s, sizeof(cmd_s), "gcov-dump -s %s", gcda_file);
    execute_command("Testing -s flag (dump stable)", cmd_s, 0);
    
    /* Test case 7: Combined flags -l -p */
    char cmd_lp[1024];
    snprintf(cmd_lp, sizeof(cmd_lp), "gcov-dump -l -p %s", gcda_file);
    execute_command("Testing combined -l -p flags", cmd_lp, 0);
    
    /* Test case 8: Invalid flag -X - triggers default case with fprintf */
    char cmd_invalid[1024];
    snprintf(cmd_invalid, sizeof(cmd_invalid), "gcov-dump -X %s", gcda_file);
    execute_command("Testing invalid -X flag (should trigger default case)", 
                   cmd_invalid, 1);
    
    /* Additional test: Try with .gcno file as well */
    printf("\n5. Additional tests with .gcno file...\n");
    char cmd_gcno[1024];
    snprintf(cmd_gcno, sizeof(cmd_gcno), "gcov-dump -l %s", gcno_file);
    execute_command("Testing -l flag with .gcno file", cmd_gcno, 0);
    
    /* Test invalid flag without file argument */
    execute_command("Testing invalid flag without file argument", 
                   "gcov-dump -Z", 1);
    
    printf("\n========================================\n");
    printf("All test cases executed\n");
    printf("Check coverage of gcov-dump.cc lines 111-130\n");
    printf("========================================\n");

cleanup:
    if (cleanup) {
        printf("\nCleaning up temporary files...\n");
        if (source_file) {
            unlink(source_file);
            free(source_file);
        }
        if (binary_file) {
            unlink(binary_file);
            free(binary_file);
        }
        if (gcda_file) {
            unlink(gcda_file);
            free(gcda_file);
        }
        if (gcno_file) {
            unlink(gcno_file);
            free(gcno_file);
        }
        /* Also clean up any default named files */
        unlink("test_coverage.gcda");
        unlink("test_coverage.gcno");
    }
    
    return 0;
}
