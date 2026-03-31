/* driver.c - Test driver for gengtype coverage */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>

/* Compile gengtype with coverage instrumentation */
#define COMPILE_GENGTYPE 1

/* Temporary file management */
typedef struct {
    char *filename;
    char *content;
} temp_file_t;

/* Generated .gt file contents */
static const char *gt_files[] = {
    /* File 1: Basic type definitions */
    "%{\n"
    "/* Test file 1: Basic types */\n"
    "%}\n"
    "\n"
    "/* TYPE_UNDEFINED: Forward declaration */\n"
    "struct undefined_struct;\n"
    "\n"
    "/* TYPE_SCALAR: Scalar typedefs */\n"
    "typedef int my_scalar;\n"
    "typedef unsigned long scalar2;\n"
    "\n"
    "/* TYPE_STRING: String type */\n"
    "struct string_struct {\n"
    "    const char *name;  /* string type */\n"
    "    char *data;\n"
    "};\n"
    "\n"
    "/* TYPE_STRUCT: Regular struct */\n"
    "struct my_struct {\n"
    "    int a;\n"
    "    float b;\n"
    "};\n"
    "\n"
    "/* TYPE_POINTER: Pointer types */\n"
    "typedef struct my_struct *my_ptr;\n"
    "typedef my_scalar *scalar_ptr;\n"
    "\n"
    "/* TYPE_ARRAY: Array types */\n"
    "typedef int my_array[10];\n"
    "typedef struct my_struct struct_array[5];\n"
    "\n"
    "%{\n"
    "/* End of file 1 */\n"
    "%}\n",

    /* File 2: Complex and user-defined types */
    "%{\n"
    "/* Test file 2: Complex types */\n"
    "%}\n"
    "\n"
    "/* TYPE_USER_STRUCT: Struct with user marking */\n"
    "struct user_struct {\n"
    "    int *p;\n"
    "    void *data;\n"
    "} GTY((user));\n"
    "\n"
    "/* TYPE_UNION: Union type */\n"
    "union my_union {\n"
    "    int i;\n"
    "    void *p;\n"
    "    double d;\n"
    "};\n"
    "\n"
    "/* TYPE_CALLBACK: Callback function type */\n"
    "typedef void (*callback_fn)(void);\n"
    "typedef int (*compare_fn)(const void *, const void *);\n"
    "\n"
    "/* Nested complex type: struct containing pointer to union of arrays */\n"
    "struct complex_nested {\n"
    "    union {\n"
    "        int int_array[5];\n"
    "        char *ptr_array[3];\n"
    "    } data_union;\n"
    "    struct user_struct *user_ptr;\n"
    "    callback_fn handler;\n"
    "};\n"
    "\n"
    "/* Array of pointers to callbacks */\n"
    "typedef callback_fn callback_array[8];\n"
    "\n"
    "%{\n"
    "/* End of file 2 */\n"
    "%}\n",

    /* File 3: Language-specific and edge cases */
    "%{\n"
    "/* Test file 3: Language structs and edge cases */\n"
    "%}\n"
    "\n"
    "/* TYPE_LANG_STRUCT: Language-specific struct */\n"
    "struct lang_struct {\n"
    "    int data;\n"
    "    void *lang_data;\n"
    "} GTY ((lang));\n"
    "\n"
    "/* Another language struct with nested types */\n"
    "struct lang_tree_node {\n"
    "    struct lang_tree_node *left;\n"
    "    struct lang_tree_node *right;\n"
    "    union {\n"
    "        int ival;\n"
    "        double dval;\n"
    "    } value;\n"
    "} GTY ((lang));\n"
    "\n"
    "/* Mixed complex type exercising multiple categories */\n"
    "struct mega_type {\n"
    "    /* scalar */\n"
    "    int count;\n"
    "    \n"
    "    /* string */\n"
    "    const char *description;\n"
    "    \n"
    "    /* pointer */\n"
    "    struct lang_struct *lang_ptr;\n"
    "    \n"
    "    /* array */\n"
    "    callback_fn handlers[4];\n"
    "    \n"
    "    /* union */\n"
    "    union {\n"
    "        my_array numbers;\n"
    "        char *strings[3];\n"
    "    } container;\n"
    "    \n"
    "    /* pointer to user struct */\n"
    "    struct user_struct *user_data;\n"
    "};\n"
    "\n"
    "/* Forward declaration for undefined type */\n"
    "struct another_undefined;\n"
    "\n"
    "%{\n"
    "/* End of file 3 */\n"
    "%}\n",

    /* File 4: With syntax error (for error path testing) */
    "%{\n"
    "/* Test file 4: Contains deliberate syntax error */\n"
    "%}\n"
    "\n"
    "struct error_struct {\n"
    "    int x;\n"
    "    /* Missing semicolon - will cause parse error */\n"
    "    int y\n"
    "};\n"
    "\n"
    "/* Note: Missing closing %} to test error recovery */\n",

    /* File 5: Duplicate definitions (for warning testing) */
    "%{\n"
    "/* Test file 5: Duplicate type definitions */\n"
    "%}\n"
    "\n"
    "struct duplicate_struct {\n"
    "    int a;\n"
    "};\n"
    "\n"
    "/* Duplicate definition to trigger warning */\n"
    "struct duplicate_struct {\n"
    "    int b;\n"
    "};\n"
    "\n"
    "%{\n"
    "/* End of file 5 */\n"
    "%}\n"
};

/* Create a temporary file with given content */
static char *create_temp_file(const char *content, const char *suffix) {
    char template[] = "/tmp/gengtype_test_XXXXXX";
    int fd = mkstemp(template);
    if (fd == -1) {
        perror("mkstemp");
        return NULL;
    }
    
    /* Write content */
    if (content) {
        ssize_t written = write(fd, content, strlen(content));
        if (written != (ssize_t)strlen(content)) {
            perror("write");
            close(fd);
            unlink(template);
            return NULL;
        }
    }
    
    close(fd);
    
    /* Add suffix if provided */
    if (suffix) {
        char *new_name = malloc(strlen(template) + strlen(suffix) + 1);
        if (!new_name) {
            unlink(template);
            return NULL;
        }
        sprintf(new_name, "%s%s", template, suffix);
        unlink(template);  /* Remove old name */
        return new_name;
    }
    
    return strdup(template);
}

/* Compile gengtype with coverage instrumentation */
static int compile_gengtype(void) {
    printf("Compiling gengtype with coverage instrumentation...\n");
    
    /* Compilation command for gengtype */
    const char *compile_cmd = 
        "g++ -O0 -fprofile-arcs -ftest-coverage "
        "-DIN_GCC -DHAVE_CONFIG_H "
        "-I. -I../../include -I../../gcc "
        "-c gengtype.cc -o gengtype.o 2>&1";
    
    printf("Running: %s\n", compile_cmd);
    int status = system(compile_cmd);
    if (status != 0) {
        fprintf(stderr, "Failed to compile gengtype.cc\n");
        return -1;
    }
    
    /* Compile gengtype-state.cc */
    const char *compile_state_cmd = 
        "g++ -O0 -fprofile-arcs -ftest-coverage "
        "-DIN_GCC -DHAVE_CONFIG_H "
        "-I. -I../../include -I../../gcc "
        "-c gengtype-state.cc -o gengtype-state.o 2>&1";
    
    printf("Running: %s\n", compile_state_cmd);
    status = system(compile_state_cmd);
    if (status != 0) {
        fprintf(stderr, "Failed to compile gengtype-state.cc\n");
        return -1;
    }
    
    /* Link gengtype executable */
    const char *link_cmd = 
        "g++ -O0 -fprofile-arcs -ftest-coverage "
        "gengtype.o gengtype-state.o "
        "-lgcov -liberty -o gengtype_coverage 2>&1";
    
    printf("Running: %s\n", link_cmd);
    status = system(link_cmd);
    if (status != 0) {
        fprintf(stderr, "Failed to link gengtype\n");
        return -1;
    }
    
    printf("gengtype compiled successfully as 'gengtype_coverage'\n");
    return 0;
}

/* Run gengtype with various patterns */
static int run_gengtype_patterns(char **temp_files, int file_count) {
    int overall_status = 0;
    
    /* Pattern A: Process each file individually */
    printf("\n=== Pattern A: Processing each file individually ===\n");
    for (int i = 0; i < file_count; i++) {
        char cmd[1024];
        snprintf(cmd, sizeof(cmd), 
                 "./gengtype_coverage -g output_%d.h %s 2>&1", 
                 i, temp_files[i]);
        
        printf("Running: %s\n", cmd);
        int status = system(cmd);
        if (status != 0 && i < 3) {  /* First 3 files should succeed */
            fprintf(stderr, "Warning: gengtype failed on file %d\n", i);
        }
        
        /* Clean up output file if created */
        char output_file[256];
        snprintf(output_file, sizeof(output_file), "output_%d.h", i);
        unlink(output_file);
    }
    
    /* Pattern B: Batch processing with -p flag */
    printf("\n=== Pattern B: Batch processing with -p flag ===\n");
    
    /* Create file list */
    char *filelist = create_temp_file(NULL, ".list");
    if (!filelist) {
        fprintf(stderr, "Failed to create file list\n");
        return -1;
    }
    
    FILE *fl = fopen(filelist, "w");
    if (!fl) {
        perror("fopen filelist");
        free(filelist);
        return -1;
    }
    
    /* Write only valid files to the list (first 3) */
    for (int i = 0; i < 3 && i < file_count; i++) {
        fprintf(fl, "%s\n", temp_files[i]);
    }
    fclose(fl);
    
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), 
             "./gengtype_coverage -p %s -g batch_output.h 2>&1", 
             filelist);
    
    printf("Running: %s\n", cmd);
    int status = system(cmd);
    if (status != 0) {
        fprintf(stderr, "Batch processing failed\n");
        overall_status = -1;
    }
    
    unlink("batch_output.h");
    unlink(filelist);
    free(filelist);
    
    /* Pattern C: Multiple files in one command */
    printf("\n=== Pattern C: Multiple files in one command ===\n");
    if (file_count >= 3) {
        char multi_cmd[2048];
        snprintf(multi_cmd, sizeof(multi_cmd),
                 "./gengtype_coverage -g multi_output.h %s %s %s 2>&1",
                 temp_files[0], temp_files[1], temp_files[2]);
        
        printf("Running: %s\n", multi_cmd);
        status = system(multi_cmd);
        if (status != 0) {
            fprintf(stderr, "Multi-file processing failed\n");
            overall_status = -1;
        }
        
        unlink("multi_output.h");
    }
    
    /* Pattern D: Error and warning cases */
    printf("\n=== Pattern D: Error and warning testing ===\n");
    
    /* Test with syntax error (file 3) */
    if (file_count > 3) {
        snprintf(cmd, sizeof(cmd),
                 "./gengtype_coverage -g error_output.h %s 2>&1",
                 temp_files[3]);
        
        printf("Testing syntax error: %s\n", cmd);
        status = system(cmd);
        if (status == 0) {
            fprintf(stderr, "Expected failure but got success\n");
        } else {
            printf("Correctly detected syntax error\n");
        }
        
        unlink("error_output.h");
    }
    
    /* Test with duplicate definition (file 4) */
    if (file_count > 4) {
        snprintf(cmd, sizeof(cmd),
                 "./gengtype_coverage -g warning_output.h %s 2>&1",
                 temp_files[4]);
        
        printf("Testing duplicate definition: %s\n", cmd);
        status = system(cmd);
        /* Duplicate definitions may or may not cause failure */
        
        unlink("warning_output.h");
    }
    
    return overall_status;
}

/* Generate coverage report */
static void generate_coverage_report(void) {
    printf("\n=== Generating coverage report ===\n");
    
    /* Run gcov on gengtype */
    const char *gcov_cmd = "gcov gengtype.cc 2>&1";
    printf("Running: %s\n", gcov_cmd);
    system(gcov_cmd);
    
    /* Display summary of gengtype.gcov */
    FILE *gcov_file = fopen("gengtype.cc.gcov", "r");
    if (gcov_file) {
        char line[256];
        int line_num = 0;
        printf("\nCoverage summary for target lines (182-213):\n");
        printf("Line  Executions\n");
        printf("----  ----------\n");
        
        while (fgets(line, sizeof(line), gcov_file)) {
            line_num++;
            if (line_num >= 182 && line_num <= 213) {
                /* Parse gcov output format: executions:line_num:content */
                char *colon1 = strchr(line, ':');
                if (colon1) {
                    char *colon2 = strchr(colon1 + 1, ':');
                    if (colon2) {
                        /* Extract execution count */
                        char exec_count[32];
                        strncpy(exec_count, line, colon1 - line);
                        exec_count[colon1 - line] = '\0';
                        
                        /* Extract line content */
                        char *content = colon2 + 1;
                        
                        printf("%4d: %-10s %s", line_num, exec_count, content);
                    }
                }
            }
        }
        fclose(gcov_file);
    }
}

/* Main test driver */
int main(int argc, char **argv) {
    printf("=== gengtype Switch Coverage Test Driver ===\n");
    
    /* Step 1: Compile gengtype with coverage */
    if (COMPILE_GENGTYPE) {
        if (compile_gengtype() != 0) {
            fprintf(stderr, "Failed to compile gengtype\n");
            return 1;
        }
    }
    
    /* Step 2: Create temporary .gt files */
    int file_count = sizeof(gt_files) / sizeof(gt_files[0]);
    char **temp_files = malloc(file_count * sizeof(char *));
    if (!temp_files) {
        perror("malloc");
        return 1;
    }
    
    printf("\nCreating %d temporary .gt files...\n", file_count);
    for (int i = 0; i < file_count; i++) {
        char suffix[32];
        snprintf(suffix, sizeof(suffix), "_%d.gt", i);
        temp_files[i] = create_temp_file(gt_files[i], suffix);
        
        if (!temp_files[i]) {
            fprintf(stderr, "Failed to create temp file %d\n", i);
            /* Clean up already created files */
            for (int j = 0; j < i; j++) {
                unlink(temp_files[j]);
                free(temp_files[j]);
            }
            free(temp_files);
            return 1;
        }
        
        printf("Created: %s\n", temp_files[i]);
    }
    
    /* Step 3: Run gengtype with various patterns */
    int run_status = run_gengtype_patterns(temp_files, file_count);
    
    /* Step 4: Clean up temporary files */
    printf("\nCleaning up temporary files...\n");
    for (int i = 0; i < file_count; i++) {
        if (temp_files[i]) {
            unlink(temp_files[i]);
            free(temp_files[i]);
        }
    }
    free(temp_files);
    
    /* Step 5: Generate coverage report */
    generate_coverage_report();
    
    /* Step 6: Clean up generated files */
    printf("\nCleaning up generated files...\n");
    unlink("gengtype.o");
    unlink("gengtype-state.o");
    unlink("gengtype_coverage");
    unlink("gengtype.gcno");
    unlink("gengtype.gcda");
    unlink("gengtype-state.gcno");
    unlink("gengtype-state.gcda");
    
    if (run_status == 0) {
        printf("\n=== Test completed successfully ===\n");
        printf("The switch statement in lines 182-213 should now have coverage.\n");
        printf("Check gengtype.cc.gcov for detailed line-by-line coverage.\n");
    } else {
        printf("\n=== Test completed with warnings ===\n");
    }
    
    return run_status;
}
