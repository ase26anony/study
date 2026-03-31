/* test_gengtype_coverage.c - Driver program to test gengtype type counting */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>

/* Coverage-instrumented gengtype executable name */
#define GENGTYPE_EXEC "./gengtype_coverage"

/* Temporary file templates */
#define GT_FILE_TEMPLATE "/tmp/test_gt_XXXXXX.gt"
#define FILELIST_TEMPLATE "/tmp/gt_filelist_XXXXXX.txt"

/* Create a temporary file with given content */
static char *create_temp_file(const char *template, const char *content) {
    char *filename = strdup(template);
    int fd = mkstemps(filename, 3); /* .gt suffix is 3 chars */
    if (fd < 0) {
        free(filename);
        return NULL;
    }
    
    FILE *f = fdopen(fd, "w");
    if (!f) {
        close(fd);
        free(filename);
        return NULL;
    }
    
    fputs(content, f);
    fclose(f);
    return filename;
}

/* Build gengtype with coverage instrumentation */
static int build_gengtype_with_coverage() {
    printf("Building gengtype with coverage instrumentation...\n");
    
    /* Compile gengtype.cc with coverage flags */
    const char *compile_cmd = 
        "g++ -O0 -fprofile-arcs -ftest-coverage -DIN_GCC "
        "-I. -I../../include -I../../gcc "
        "-c gengtype.cc -o gengtype_coverage.o 2>&1";
    
    if (system(compile_cmd) != 0) {
        fprintf(stderr, "Failed to compile gengtype.cc\n");
        return 0;
    }
    
    /* Compile gengtype-state.cc */
    const char *compile_state_cmd = 
        "g++ -O0 -fprofile-arcs -ftest-coverage -DIN_GCC "
        "-I. -I../../include -I../../gcc "
        "-c gengtype-state.cc -o gengtype_state_coverage.o 2>&1";
    
    if (system(compile_state_cmd) != 0) {
        fprintf(stderr, "Failed to compile gengtype-state.cc\n");
        return 0;
    }
    
    /* Link the executable */
    const char *link_cmd = 
        "g++ -O0 -fprofile-arcs -ftest-coverage "
        "gengtype_coverage.o gengtype_state_coverage.o "
        "-lgcov -liberty -o " GENGTYPE_EXEC " 2>&1";
    
    if (system(link_cmd) != 0) {
        fprintf(stderr, "Failed to link gengtype executable\n");
        return 0;
    }
    
    printf("Successfully built " GENGTYPE_EXEC "\n");
    return 1;
}

/* Test case 1: Basic types covering all categories */
static const char *gt_file1 = 
"%{\n"
"/* Test file 1: Basic type definitions */\n"
"#include \"config.h\"\n"
"%}\n"
"\n"
"/* TYPE_UNDEFINED: Forward declaration */\n"
"struct undefined_struct;\n"
"\n"
"/* TYPE_SCALAR: Scalar typedef */\n"
"typedef int my_scalar;\n"
"typedef unsigned long scalar2;\n"
"\n"
"/* TYPE_STRING: String type usage */\n"
"struct string_struct {\n"
"  const char *name;  /* This should be recognized as string type */\n"
"  char *dynamic_str;\n"
"};\n"
"\n"
"/* TYPE_STRUCT: Regular struct */\n"
"struct my_struct {\n"
"  int a;\n"
"  float b;\n"
"  struct my_struct *next;\n"
"};\n"
"\n"
"/* TYPE_USER_STRUCT: Struct with user marking */\n"
"struct user_struct {\n"
"  int *p;\n"
"  void *data;\n"
"} GTY((user));\n"
"\n"
"/* TYPE_UNION: Union definition */\n"
"union my_union {\n"
"  int i;\n"
"  float f;\n"
"  void *p;\n"
"};\n"
"\n"
"/* TYPE_POINTER: Pointer typedefs */\n"
"typedef struct my_struct *my_ptr;\n"
"typedef union my_union *union_ptr;\n"
"\n"
"/* TYPE_ARRAY: Array types */\n"
"typedef int my_array[10];\n"
"typedef struct my_struct struct_array[5];\n"
"\n"
"/* TYPE_CALLBACK: Callback function pointer */\n"
"typedef void (*callback_fn)(void);\n"
"typedef int (*predicate_fn)(const char *);\n"
"\n"
"/* TYPE_LANG_STRUCT: Language-specific struct */\n"
"struct lang_struct {\n"
"  int lang_data;\n"
"  void *lang_ptr;\n"
"} GTY((lang));\n"
"\n"
"/* Complex nested type combining multiple categories */\n"
"struct complex_nested {\n"
"  my_array data;           /* TYPE_ARRAY */\n"
"  callback_fn handler;     /* TYPE_CALLBACK */\n"
"  struct lang_struct *ls;  /* TYPE_POINTER to TYPE_LANG_STRUCT */\n"
"  union my_union choice;   /* TYPE_UNION */\n"
"};\n"
"%}";

/* Test case 2: More complex types and edge cases */
static const char *gt_file2 =
"%{\n"
"/* Test file 2: Advanced type combinations */\n"
"#include \"config.h\"\n"
"%}\n"
"\n"
"/* Another undefined struct */\n"
"struct another_undefined;\n"
"\n"
"/* More scalar types */\n"
"typedef double precision_scalar;\n"
"typedef _Bool bool_scalar;\n"
"\n"
"/* Struct with string arrays */\n"
"struct string_container {\n"
"  const char *names[5];     /* Array of strings */\n"
"  char *buffer;\n"
"};\n"
"\n"
"/* Union with struct members */\n"
"union complex_union {\n"
"  struct {\n"
"    int x;\n"
"    int y;\n"
"  } point;\n"
"  struct {\n"
"    float r;\n"
"    float i;\n"
"  } complex;\n"
"};\n"
"\n"
"/* Pointer to array */\n"
"typedef int (*array_ptr)[10];\n"
"\n"
"/* Multi-dimensional array */\n"
"typedef int matrix[3][3];\n"
"\n"
"/* Callback with parameters */\n"
"typedef void (*event_callback)(int event_id, void *user_data);\n"
"\n"
"/* Another lang struct with different attributes */\n"
"struct another_lang_struct {\n"
"  long identifier;\n"
"  struct string_container *str_container;\n"
"} GTY((lang));\n"
"\n"
"/* Struct containing all type categories */\n"
"struct mega_struct {\n"
"  precision_scalar value;      /* TYPE_SCALAR */\n"
"  const char *description;     /* TYPE_STRING */\n"
"  matrix transform;            /* TYPE_ARRAY */\n"
"  event_callback on_event;     /* TYPE_CALLBACK */\n"
"  union complex_union data;    /* TYPE_UNION */\n"
"  struct another_lang_struct *lang; /* TYPE_POINTER */\n"
"};\n"
"%}";

/* Test case 3: Error cases and warnings */
static const char *gt_file3 =
"%{\n"
"/* Test file 3: Error and warning cases */\n"
"#include \"config.h\"\n"
"\n"
"/* Deliberate duplicate to test warning */\n"
"typedef int my_scalar;  /* Duplicate from file1 */\n"
"\n"
"/* Struct with problematic nesting */\n"
"struct problematic {\n"
"  struct undefined_struct *undef;  /* Pointer to undefined */\n"
"  void (*weird_callback)(struct undefined_struct*);\n"
"};\n"
"\n"
"/* More pointer variations */\n"
"typedef void (*func_ptr)(void);\n"
"typedef func_ptr (*meta_func_ptr)(int);\n"
"\n"
"/* Array of pointers to callbacks */\n"
"typedef callback_fn callback_array[8];\n"
"\n"
"/* Note: Missing closing %} to test error handling */\n"
"/* This should trigger error path but still process types */\n";

/* Test case 4: Minimal but complete file */
static const char *gt_file4 =
"%{\n"
"#include \"config.h\"\n"
"%}\n"
"\n"
"/* Minimal coverage of all types in one file */\n"
"struct min_undefined;\n"
"typedef short min_scalar;\n"
"struct min_str { const char *s; };\n"
"struct min_struct { int a; };\n"
"struct min_user { void *p; } GTY((user));\n"
"union min_union { int i; void *p; };\n"
"typedef int* min_ptr;\n"
"typedef char min_arr[4];\n"
"typedef void (*min_cb)(void);\n"
"struct min_lang { int l; } GTY((lang));\n"
"%}";

/* Run gengtype with specific arguments */
static int run_gengtype(const char *args) {
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "%s %s", GENGTYPE_EXEC, args);
    
    printf("Running: %s\n", cmd);
    int status = system(cmd);
    
    if (WIFEXITED(status)) {
        printf("Exit code: %d\n", WEXITSTATUS(status));
        return WEXITSTATUS(status) == 0;
    }
    return 0;
}

/* Test Pattern A: Process each file individually */
static int test_pattern_a(char **files, int count) {
    printf("\n=== Testing Pattern A: Individual file processing ===\n");
    
    int success = 1;
    for (int i = 0; i < count; i++) {
        char args[512];
        snprintf(args, sizeof(args), "-g /tmp/output%d.h %s", i, files[i]);
        if (!run_gengtype(args)) {
            printf("Warning: File %d processing failed (may be expected for error cases)\n", i);
            /* Don't fail the test for error cases */
            if (i != 2) { /* file3 has deliberate error */
                success = 0;
            }
        }
    }
    
    return success;
}

/* Test Pattern B: Batch processing with -p flag */
static int test_pattern_b(char **files, int count) {
    printf("\n=== Testing Pattern B: Batch processing with -p ===\n");
    
    /* Create file list */
    char *listfile = create_temp_file(FILELIST_TEMPLATE, "");
    if (!listfile) {
        fprintf(stderr, "Failed to create file list\n");
        return 0;
    }
    
    FILE *list = fopen(listfile, "w");
    if (!list) {
        free(listfile);
        return 0;
    }
    
    for (int i = 0; i < count; i++) {
        fprintf(list, "%s\n", files[i]);
    }
    fclose(list);
    
    char args[512];
    snprintf(args, sizeof(args), "-p %s -g /tmp/batch_output.h", listfile);
    int result = run_gengtype(args);
    
    unlink(listfile);
    free(listfile);
    
    return result;
}

/* Test Pattern C: Multiple files in one command */
static int test_pattern_c(char **files, int count) {
    printf("\n=== Testing Pattern C: Multiple files in one command ===\n");
    
    char args[1024] = "-g /tmp/combined_output.h";
    int pos = strlen(args);
    
    for (int i = 0; i < count && i < 3; i++) { /* Limit to 3 files */
        pos += snprintf(args + pos, sizeof(args) - pos, " %s", files[i]);
    }
    
    return run_gengtype(args);
}

/* Test Pattern D: Generate routine file */
static int test_pattern_d(char **files, int count) {
    printf("\n=== Testing Pattern D: Generate routine file ===\n");
    
    char args[512];
    snprintf(args, sizeof(args), "-r /tmp/routines.c %s", files[0]); /* Use first valid file */
    
    return run_gengtype(args);
}

/* Clean up temporary files */
static void cleanup_files(char **files, int count) {
    for (int i = 0; i < count; i++) {
        if (files[i]) {
            unlink(files[i]);
            free(files[i]);
        }
    }
}

int main() {
    printf("=== gengtype Switch Coverage Test ===\n");
    
    /* Build gengtype with coverage */
    if (!build_gengtype_with_coverage()) {
        fprintf(stderr, "Failed to build gengtype with coverage\n");
        return 1;
    }
    
    /* Create temporary .gt files */
    char *gt_files[4];
    const char *contents[4] = {gt_file1, gt_file2, gt_file3, gt_file4};
    
    printf("\nCreating test .gt files...\n");
    for (int i = 0; i < 4; i++) {
        char template[64];
        snprintf(template, sizeof(template), "/tmp/test%d_XXXXXX.gt", i);
        gt_files[i] = create_temp_file(template, contents[i]);
        if (!gt_files[i]) {
            fprintf(stderr, "Failed to create temp file %d\n", i);
            for (int j = 0; j < i; j++) {
                free(gt_files[j]);
            }
            return 1;
        }
        printf("Created: %s\n", gt_files[i]);
    }
    
    /* Run all test patterns */
    int success = 1;
    
    success = success && test_pattern_a(gt_files, 4);
    success = success && test_pattern_b(gt_files, 4);
    success = success && test_pattern_c(gt_files, 4);
    success = success && test_pattern_d(gt_files, 4);
    
    /* Additional test: Run with debug flag */
    printf("\n=== Additional test: With debug flag ===\n");
    char debug_args[512];
    snprintf(debug_args, sizeof(debug_args), "-d -g /tmp/debug_output.h %s", gt_files[0]);
    run_gengtype(debug_args);
    
    /* Clean up */
    printf("\nCleaning up temporary files...\n");
    cleanup_files(gt_files, 4);
    
    /* Remove generated output files */
    system("rm -f /tmp/output*.h /tmp/batch_output.h /tmp/combined_output.h "
           "/tmp/routines.c /tmp/debug_output.h");
    
    /* Generate coverage report */
    printf("\n=== Generating coverage report ===\n");
    system("gcov gengtype_coverage.o");
    
    /* Check if the specific switch was executed */
    FILE *gcov_output = fopen("gengtype.cc.gcov", "r");
    if (gcov_output) {
        char line[256];
        int in_target_block = 0;
        int lines_executed = 0;
        int total_lines = 0;
        
        while (fgets(line, sizeof(line), gcov_output)) {
            if (strstr(line, "182:") || strstr(line, "182:")) {
                in_target_block = 1;
            }
            
            if (in_target_block) {
                /* Parse gcov output format: execution_count:line_number */
                int count;
                int line_num;
                if (sscanf(line, "%d:%d", &count, &line_num) == 2) {
                    total_lines++;
                    if (count > 0) lines_executed++;
                }
                
                /* Check if we've passed the target block */
                if (line_num > 213) {
                    break;
                }
            }
        }
        fclose(gcov_output);
        
        printf("\nCoverage results for lines 182-213:\n");
        printf("Lines executed: %d/%d\n", lines_executed, total_lines);
        
        if (lines_executed > 0) {
            printf("SUCCESS: Switch statement was executed!\n");
        } else {
            printf("WARNING: Switch statement may not have been executed\n");
            success = 0;
        }
    }
    
    printf("\n=== Test %s ===\n", success ? "PASSED" : "FAILED");
    return success ? 0 : 1;
}
