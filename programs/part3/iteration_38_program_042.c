/* driver.c - Test driver for gengtype coverage */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>

/* Compile gengtype with coverage flags */
#define GENGTYPE_SOURCE "gengtype.cc"
#define GENGTYPE_STATE_SOURCE "gengtype-state.cc"
#define GENGTYPE_EXECUTABLE "gengtype_coverage"

/* Create multiple .gt files with diverse type definitions */
const char *gt_files[] = {
    /* File 1: Basic types and structs */
    "test_types1.gt",
    "test_types2.gt", 
    "test_types3.gt",
    "test_error.gt",  /* File with syntax error */
    "test_dup.gt"     /* File with duplicate definitions */
};

/* Content for test_types1.gt - Basic type definitions */
const char *gt_content1 = 
"%{\n"
"/* Test file 1: Basic types */\n"
"%}\n"
"\n"
"/* TYPE_UNDEFINED - forward declaration */\n"
"struct undefined_struct;\n"
"\n"
"/* TYPE_SCALAR - scalar typedef */\n"
"typedef int my_scalar;\n"
"typedef unsigned long scalar2;\n"
"\n"
"/* TYPE_STRING - string type */\n"
"struct string_struct {\n"
"  const char *name;  /* TYPE_STRING */\n"
"  char *data;\n"
"};\n"
"\n"
"/* TYPE_STRUCT - regular struct */\n"
"struct my_struct {\n"
"  int a;\n"
"  double b;\n"
"};\n"
"\n"
"/* TYPE_POINTER - pointer types */\n"
"typedef struct my_struct *my_ptr;\n"
"typedef my_scalar *scalar_ptr;\n"
"%}";

/* Content for test_types2.gt - Advanced types */
const char *gt_content2 =
"%{\n"
"/* Test file 2: Advanced types */\n"
"%}\n"
"\n"
"/* TYPE_USER_STRUCT - struct with user marking */\n"
"struct user_struct {\n"
"  int *p;\n"
"  void *data;\n"
"} GTY((user));\n"
"\n"
"/* TYPE_UNION */\n"
"union my_union {\n"
"  int i;\n"
"  float f;\n"
"  void *p;\n"
"};\n"
"\n"
"/* TYPE_ARRAY - array types */\n"
"typedef int my_array[10];\n"
"typedef struct my_struct struct_array[5];\n"
"\n"
"/* TYPE_CALLBACK - callback type */\n"
"typedef void (*callback_fn)(void);\n"
"typedef int (*process_fn)(int, char**);\n"
"\n"
"/* Complex nested type combining multiple categories */\n"
"struct complex_type {\n"
"  union my_union u;           /* TYPE_UNION */\n"
"  my_array arr;               /* TYPE_ARRAY */\n"
"  callback_fn cb;             /* TYPE_CALLBACK */\n"
"  struct user_struct *user;   /* TYPE_USER_STRUCT + TYPE_POINTER */\n"
"};\n"
"%}";

/* Content for test_types3.gt - Language-specific and more types */
const char *gt_content3 =
"%{\n"
"/* Test file 3: Language structs and more */\n"
"%}\n"
"\n"
"/* TYPE_LANG_STRUCT - language-specific struct */\n"
"struct lang_struct {\n"
"  int data;\n"
"  void *lang_data;\n"
"} GTY ((lang));\n"
"\n"
"/* More TYPE_STRUCT definitions */\n"
"struct another_struct {\n"
"  struct lang_struct *lang_ptr;  /* TYPE_POINTER to TYPE_LANG_STRUCT */\n"
"  const char *description;       /* TYPE_STRING */\n"
"};\n"
"\n"
"/* Union with array */\n"
"union union_with_array {\n"
"  int numbers[20];               /* TYPE_ARRAY inside TYPE_UNION */\n"
"  char *strings[10];             /* TYPE_ARRAY of TYPE_POINTER */\n"
"};\n"
"\n"
"/* Pointer to callback */\n"
"typedef callback_fn *callback_ptr;  /* TYPE_POINTER to TYPE_CALLBACK */\n"
"\n"
"/* Struct containing all major types */\n"
"struct mega_struct {\n"
"  my_scalar scalar_field;        /* TYPE_SCALAR */\n"
"  struct undefined_struct *undef;/* TYPE_POINTER to TYPE_UNDEFINED */\n"
"  union my_union u_field;        /* TYPE_UNION */\n"
"  callback_fn handler;           /* TYPE_CALLBACK */\n"
"  int matrix[5][10];             /* Multi-dimensional TYPE_ARRAY */\n"
"};\n"
"%}";

/* File with syntax error - missing %} */
const char *gt_error_content =
"%{\n"
"/* This file has a syntax error - missing closing %} */\n"
"struct error_struct {\n"
"  int x;\n"
"};\n"
"/* Missing %} here to trigger error path */";

/* File with duplicate definition to trigger warning */
const char *gt_dup_content =
"%{\n"
"/* File with duplicate type definition */\n"
"%}\n"
"\n"
"struct duplicate_struct {\n"
"  int a;\n"
"};\n"
"\n"
"/* Duplicate definition */\n"
"struct duplicate_struct {\n"
"  int b;\n"
"};\n"
"%}";

/* Write content to file */
int write_file(const char *filename, const char *content) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        perror("fopen");
        return 0;
    }
    fprintf(f, "%s", content);
    fclose(f);
    return 1;
}

/* Compile gengtype with coverage instrumentation */
int compile_gengtype() {
    printf("Compiling gengtype with coverage instrumentation...\n");
    
    /* Compile gengtype.cc */
    char cmd[1024];
    snprintf(cmd, sizeof(cmd),
        "g++ -O0 -fprofile-arcs -ftest-coverage -DIN_GCC "
        "-DHAVE_CONFIG_H -I. -I../../include -I../../gcc "
        "-c %s -o gengtype.o",
        GENGTYPE_SOURCE);
    
    printf("Running: %s\n", cmd);
    if (system(cmd) != 0) {
        fprintf(stderr, "Failed to compile gengtype.cc\n");
        return 0;
    }
    
    /* Compile gengtype-state.cc if it exists */
    struct stat st;
    if (stat(GENGTYPE_STATE_SOURCE, &st) == 0) {
        snprintf(cmd, sizeof(cmd),
            "g++ -O0 -fprofile-arcs -ftest-coverage -DIN_GCC "
            "-DHAVE_CONFIG_H -I. -I../../include -I../../gcc "
            "-c %s -o gengtype-state.o",
            GENGTYPE_STATE_SOURCE);
        
        printf("Running: %s\n", cmd);
        if (system(cmd) != 0) {
            fprintf(stderr, "Failed to compile gengtype-state.cc\n");
            return 0;
        }
        
        /* Link both object files */
        snprintf(cmd, sizeof(cmd),
            "g++ -O0 -fprofile-arcs -ftest-coverage gengtype.o gengtype-state.o "
            "-o %s -lgcov -liberty",
            GENGTYPE_EXECUTABLE);
    } else {
        /* Link single object file */
        snprintf(cmd, sizeof(cmd),
            "g++ -O0 -fprofile-arcs -ftest-coverage gengtype.o "
            "-o %s -lgcov -liberty",
            GENGTYPE_EXECUTABLE);
    }
    
    printf("Running: %s\n", cmd);
    if (system(cmd) != 0) {
        fprintf(stderr, "Failed to link gengtype\n");
        return 0;
    }
    
    printf("gengtype compiled successfully\n");
    return 1;
}

/* Run gengtype with various patterns */
int run_gengtype_patterns() {
    int success = 1;
    char cmd[1024];
    
    printf("\n=== Pattern A: Process each file individually ===\n");
    /* Pattern A: Process each file individually */
    for (int i = 0; i < 3; i++) {  /* First 3 valid files */
        snprintf(cmd, sizeof(cmd), "./%s -g output%d.h %s", 
                GENGTYPE_EXECUTABLE, i + 1, gt_files[i]);
        printf("Running: %s\n", cmd);
        
        int status = system(cmd);
        if (WIFEXITED(status)) {
            printf("Exit status: %d\n", WEXITSTATUS(status));
        }
        
        /* Check if output was generated */
        char output_file[64];
        snprintf(output_file, sizeof(output_file), "output%d.h", i + 1);
        struct stat st;
        if (stat(output_file, &st) == 0) {
            printf("Generated %s (%ld bytes)\n", output_file, st.st_size);
            /* Clean up */
            remove(output_file);
        }
    }
    
    printf("\n=== Pattern B: Batch processing with -p flag ===\n");
    /* Pattern B: Create file list and process with -p */
    FILE *list = fopen("filelist.txt", "w");
    if (list) {
        for (int i = 0; i < 3; i++) {
            fprintf(list, "%s\n", gt_files[i]);
        }
        fclose(list);
        
        snprintf(cmd, sizeof(cmd), "./%s -p filelist.txt -g batch_output.h", 
                GENGTYPE_EXECUTABLE);
        printf("Running: %s\n", cmd);
        
        int status = system(cmd);
        if (WIFEXITED(status)) {
            printf("Batch processing exit status: %d\n", WEXITSTATUS(status));
        }
        
        /* Check output */
        struct stat st;
        if (stat("batch_output.h", &st) == 0) {
            printf("Generated batch_output.h (%ld bytes)\n", st.st_size);
            remove("batch_output.h");
        }
        remove("filelist.txt");
    }
    
    printf("\n=== Pattern C: Multiple files with header generation ===\n");
    /* Pattern C: Process all valid files at once */
    snprintf(cmd, sizeof(cmd), "./%s -g combined.h %s %s %s", 
            GENGTYPE_EXECUTABLE, gt_files[0], gt_files[1], gt_files[2]);
    printf("Running: %s\n", cmd);
    
    int status = system(cmd);
    if (WIFEXITED(status)) {
        printf("Combined processing exit status: %d\n", WEXITSTATUS(status));
    }
    
    struct stat st;
    if (stat("combined.h", &st) == 0) {
        printf("Generated combined.h (%ld bytes)\n", st.st_size);
        remove("combined.h");
    }
    
    printf("\n=== Pattern D: Error and warning cases ===\n");
    /* Pattern D: Test error case */
    snprintf(cmd, sizeof(cmd), "./%s -g error_output.h %s", 
            GENGTYPE_EXECUTABLE, gt_files[3]);
    printf("Running (should fail): %s\n", cmd);
    
    status = system(cmd);
    if (WIFEXITED(status)) {
        int exit_status = WEXITSTATUS(status);
        printf("Error file exit status: %d (non-zero expected)\n", exit_status);
        if (exit_status == 0) {
            fprintf(stderr, "Warning: Error case didn't fail as expected\n");
        }
    }
    
    /* Pattern D: Test warning case (duplicate definition) */
    snprintf(cmd, sizeof(cmd), "./%s -g dup_output.h %s", 
            GENGTYPE_EXECUTABLE, gt_files[4]);
    printf("Running (should warn): %s\n", cmd);
    
    status = system(cmd);
    if (WIFEXITED(status)) {
        printf("Duplicate file exit status: %d\n", WEXITSTATUS(status));
    }
    
    /* Check if warning output was generated */
    if (stat("dup_output.h", &st) == 0) {
        printf("Generated dup_output.h (%ld bytes)\n", st.st_size);
        remove("dup_output.h");
    }
    
    printf("\n=== Additional test: Generate routine file ===\n");
    /* Test with -r flag for routine generation */
    snprintf(cmd, sizeof(cmd), "./%s -r routine.c %s", 
            GENGTYPE_EXECUTABLE, gt_files[0]);
    printf("Running: %s\n", cmd);
    
    status = system(cmd);
    if (WIFEXITED(status)) {
        printf("Routine generation exit status: %d\n", WEXITSTATUS(status));
    }
    
    if (stat("routine.c", &st) == 0) {
        printf("Generated routine.c (%ld bytes)\n", st.st_size);
        remove("routine.c");
    }
    
    return success;
}

/* Clean up temporary files */
void cleanup() {
    printf("\nCleaning up temporary files...\n");
    for (int i = 0; i < sizeof(gt_files)/sizeof(gt_files[0]); i++) {
        remove(gt_files[i]);
    }
    remove("gengtype.o");
    remove("gengtype-state.o");
    remove(GENGTYPE_EXECUTABLE);
    remove("*.gcda");
    remove("*.gcno");
}

int main() {
    printf("=== gengtype Coverage Test Driver ===\n");
    
    /* Create test .gt files */
    printf("\nCreating test .gt files...\n");
    if (!write_file(gt_files[0], gt_content1)) return 1;
    if (!write_file(gt_files[1], gt_content2)) return 1;
    if (!write_file(gt_files[2], gt_content3)) return 1;
    if (!write_file(gt_files[3], gt_error_content)) return 1;
    if (!write_file(gt_files[4], gt_dup_content)) return 1;
    
    printf("Created %zu test files\n", sizeof(gt_files)/sizeof(gt_files[0]));
    
    /* Compile gengtype with coverage */
    if (!compile_gengtype()) {
        cleanup();
        return 1;
    }
    
    /* Run gengtype with various patterns */
    if (!run_gengtype_patterns()) {
        cleanup();
        return 1;
    }
    
    /* Generate coverage report */
    printf("\n=== Generating coverage report ===\n");
    system("gcov gengtype.cc");
    
    /* Check if the specific switch was executed */
    FILE *gcov_output = fopen("gengtype.cc.gcov", "r");
    if (gcov_output) {
        char line[256];
        int in_target_block = 0;
        int lines_executed = 0;
        int total_lines = 0;
        
        while (fgets(line, sizeof(line), gcov_output)) {
            /* Look for lines around the target switch (lines 182-213) */
            if (strstr(line, ":") && strstr(line, "case TYPE_")) {
                /* Parse gcov output line: "count:lineno:source" */
                char *colon1 = strchr(line, ':');
                if (colon1) {
                    char *colon2 = strchr(colon1 + 1, ':');
                    if (colon2) {
                        /* Check execution count */
                        char count_str[32];
                        strncpy(count_str, line, colon1 - line);
                        count_str[colon1 - line] = '\0';
                        
                        long count = strtol(count_str, NULL, 10);
                        if (count > 0) {
                            lines_executed++;
                        }
                        total_lines++;
                    }
                }
            }
        }
        fclose(gcov_output);
        
        printf("\nCoverage results for target switch statement:\n");
        printf("Lines executed: %d out of %d\n", lines_executed, total_lines);
        printf("Coverage: %.1f%%\n", total_lines > 0 ? 
               (lines_executed * 100.0 / total_lines) : 0.0);
        
        if (lines_executed > 0) {
            printf("SUCCESS: Switch statement was executed!\n");
        } else {
            printf("WARNING: Switch statement may not have been executed\n");
        }
    }
    
    /* Clean up */
    cleanup();
    
    printf("\n=== Test completed ===\n");
    return 0;
}
