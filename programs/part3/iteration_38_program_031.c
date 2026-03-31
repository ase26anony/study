/* test_gengtype_coverage.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>

/* Compile gengtype with coverage instrumentation */
#define COMPILE_GENGTYPE \
    "g++ -O0 -fprofile-arcs -ftest-coverage -DIN_GCC -DHAVE_CONFIG_H " \
    "-I. -I../../include -I../../gcc " \
    "gengtype.cc gengtype-state.cc gengtype-lex.cc gengtype-parse.cc " \
    "-o gengtype_coverage -lgcov -liberty"

/* GT file 1: Basic types with forward declarations */
const char *gt_file1_content = 
"%{\n"
"/* File 1: Basic types and forward declarations */\n"
"#include \"config.h\"\n"
"#include \"system.h\"\n"
"%}\n"
"\n"
"/* TYPE_UNDEFINED: Forward declaration */\n"
"struct undefined_struct;\n"
"\n"
"/* TYPE_SCALAR: Scalar typedefs */\n"
"typedef int my_scalar;\n"
"typedef unsigned long scalar2;\n"
"\n"
"/* TYPE_STRING: String types */\n"
"struct string_struct {\n"
"    const char *name;  /* TYPE_STRING */\n"
"    char *buffer;\n"
"};\n"
"\n"
"/* TYPE_POINTER: Pointer types */\n"
"typedef struct string_struct *string_ptr;\n"
"typedef my_scalar *scalar_ptr;\n"
"%}\n";

/* GT file 2: Structs, unions, and arrays */
const char *gt_file2_content =
"%{\n"
"/* File 2: Structs, unions, arrays */\n"
"#include \"config.h\"\n"
"#include \"system.h\"\n"
"%}\n"
"\n"
"/* TYPE_STRUCT: Regular struct */\n"
"struct my_struct {\n"
"    int a;\n"
"    double b;\n"
"    struct my_struct *next;  /* Self-referential pointer */\n"
"};\n"
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
"/* TYPE_ARRAY: Array types */\n"
"typedef int my_array[10];\n"
"typedef struct my_struct struct_array[5];\n"
"\n"
"/* Nested complex type: struct containing pointer to union of arrays */\n"
"struct complex_nested {\n"
"    union {\n"
"        my_array arr1;\n"
"        struct_array arr2;\n"
"    } data_union;\n"
"    union my_union *union_ptr;  /* TYPE_POINTER to TYPE_UNION */\n"
"};\n"
"%}\n";

/* GT file 3: Callbacks, lang structs, and more */
const char *gt_file3_content =
"%{\n"
"/* File 3: Callbacks and language-specific structs */\n"
"#include \"config.h\"\n"
"#include \"system.h\"\n"
"%}\n"
"\n"
"/* TYPE_CALLBACK: Callback function type */\n"
"typedef void (*callback_fn)(void);\n"
"typedef int (*compare_fn)(const void *, const void *);\n"
"\n"
"/* TYPE_LANG_STRUCT: Language-specific struct */\n"
"struct lang_struct {\n"
"    int data;\n"
"    void *extra;\n"
"} GTY ((lang));\n"
"\n"
"/* More TYPE_POINTER variations */\n"
"typedef callback_fn (*callback_ptr)(void);  /* Pointer to callback */\n"
"\n"
"/* Struct with callback member */\n"
"struct with_callback {\n"
"    callback_fn handler;\n"
"    compare_fn comparator;\n"
"};\n"
"\n"
"/* Mixed complex type */\n"
"struct mixed_types {\n"
"    my_scalar scalar_field;          /* TYPE_SCALAR */\n"
"    const char *string_field;        /* TYPE_STRING */\n"
"    struct my_struct *struct_ptr;    /* TYPE_POINTER to TYPE_STRUCT */\n"
"    union my_union union_field;      /* TYPE_UNION */\n"
"    my_array array_field;            /* TYPE_ARRAY */\n"
"    callback_fn callback_field;      /* TYPE_CALLBACK */\n"
"};\n"
"%}\n";

/* GT file 4: With syntax error (for error path testing) */
const char *gt_file4_content =
"%{\n"
"/* File 4: Deliberate syntax error - missing closing %}\n"
"#include \"config.h\"\n"
"#include \"system.h\"\n"
"\n"
"struct error_struct {\n"
"    int x;\n"
"    int y;\n"
"};\n"
"/* Missing %} here to trigger error */\n";

/* GT file 5: Duplicate definitions (for warning testing) */
const char *gt_file5_content =
"%{\n"
"/* File 5: Duplicate type definitions */\n"
"#include \"config.h\"\n"
"#include \"system.h\"\n"
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
"%}\n";

/* Create temporary file with given content */
char *create_temp_file(const char *content, const char *suffix) {
    char template[] = "/tmp/gengtype_test_XXXXXX";
    int fd = mkstemp(template);
    if (fd == -1) {
        perror("mkstemp failed");
        return NULL;
    }
    
    /* Write content */
    write(fd, content, strlen(content));
    close(fd);
    
    /* Rename with proper suffix */
    char *new_name = malloc(strlen(template) + strlen(suffix) + 1);
    sprintf(new_name, "%s%s", template, suffix);
    rename(template, new_name);
    
    return new_name;
}

/* Run gengtype with given arguments */
int run_gengtype(const char *gengtype_path, const char *args) {
    char command[1024];
    snprintf(command, sizeof(command), "%s %s", gengtype_path, args);
    
    printf("Executing: %s\n", command);
    int status = system(command);
    
    if (WIFEXITED(status)) {
        printf("Exit code: %d\n", WEXITSTATUS(status));
        return WEXITSTATUS(status);
    }
    return -1;
}

/* Create file list for batch processing */
char *create_file_list(char **files, int count) {
    char template[] = "/tmp/gengtype_filelist_XXXXXX";
    int fd = mkstemp(template);
    if (fd == -1) {
        perror("mkstemp failed for file list");
        return NULL;
    }
    
    for (int i = 0; i < count; i++) {
        write(fd, files[i], strlen(files[i]));
        write(fd, "\n", 1);
    }
    close(fd);
    
    return strdup(template);
}

int main() {
    printf("=== GCC gengtype Coverage Test ===\n\n");
    
    /* Step 1: Compile gengtype with coverage instrumentation */
    printf("1. Compiling gengtype with coverage flags...\n");
    int compile_status = system(COMPILE_GENGTYPE);
    if (compile_status != 0) {
        fprintf(stderr, "Failed to compile gengtype\n");
        return 1;
    }
    printf("Compilation successful\n\n");
    
    /* Step 2: Create temporary GT files */
    printf("2. Creating test GT files...\n");
    char *gt_files[5];
    
    gt_files[0] = create_temp_file(gt_file1_content, ".gt");
    gt_files[1] = create_temp_file(gt_file2_content, ".gt");
    gt_files[2] = create_temp_file(gt_file3_content, ".gt");
    gt_files[3] = create_temp_file(gt_file4_content, ".gt");
    gt_files[4] = create_temp_file(gt_file5_content, ".gt");
    
    for (int i = 0; i < 5; i++) {
        if (gt_files[i]) {
            printf("  Created: %s\n", gt_files[i]);
        } else {
            fprintf(stderr, "Failed to create file %d\n", i);
            return 1;
        }
    }
    printf("\n");
    
    /* Step 3: Pattern A - Process each file individually */
    printf("3. Pattern A: Processing files individually...\n");
    for (int i = 0; i < 3; i++) {  /* Only first 3 valid files */
        char args[1024];
        snprintf(args, sizeof(args), "-g /tmp/output%d.h %s", i, gt_files[i]);
        run_gengtype("./gengtype_coverage", args);
    }
    printf("\n");
    
    /* Step 4: Pattern B - Batch processing with -p flag */
    printf("4. Pattern B: Batch processing with -p flag...\n");
    char *file_list = create_file_list(gt_files, 3);  /* First 3 valid files */
    if (file_list) {
        char args[1024];
        snprintf(args, sizeof(args), "-p %s", file_list);
        run_gengtype("./gengtype_coverage", args);
        free(file_list);
    }
    printf("\n");
    
    /* Step 5: Pattern C - Multiple files with header generation */
    printf("5. Pattern C: Multiple files with header generation...\n");
    char multi_args[2048] = "-g /tmp/combined.h ";
    for (int i = 0; i < 3; i++) {
        strcat(multi_args, gt_files[i]);
        strcat(multi_args, " ");
    }
    run_gengtype("./gengtype_coverage", multi_args);
    printf("\n");
    
    /* Step 6: Pattern D - Error and warning cases */
    printf("6. Pattern D: Testing error and warning paths...\n");
    
    /* Test syntax error file */
    printf("  Testing syntax error file...\n");
    char error_args[1024];
    snprintf(error_args, sizeof(error_args), "%s", gt_files[3]);
    run_gengtype("./gengtype_coverage", error_args);
    
    /* Test duplicate definition file */
    printf("  Testing duplicate definition file...\n");
    snprintf(error_args, sizeof(error_args), "%s", gt_files[4]);
    run_gengtype("./gengtype_coverage", error_args);
    printf("\n");
    
    /* Step 7: Generate routine file with all types */
    printf("7. Generating routine file with all valid types...\n");
    char routine_args[2048] = "-r /tmp/routines.c ";
    for (int i = 0; i < 3; i++) {
        strcat(routine_args, gt_files[i]);
        strcat(routine_args, " ");
    }
    run_gengtype("./gengtype_coverage", routine_args);
    printf("\n");
    
    /* Step 8: Clean up temporary files */
    printf("8. Cleaning up temporary files...\n");
    for (int i = 0; i < 5; i++) {
        unlink(gt_files[i]);
        free(gt_files[i]);
    }
    
    /* Remove generated output files */
    for (int i = 0; i < 3; i++) {
        char fname[256];
        snprintf(fname, sizeof(fname), "/tmp/output%d.h", i);
        unlink(fname);
    }
    unlink("/tmp/combined.h");
    unlink("/tmp/routines.c");
    
    printf("\n=== Test completed ===\n");
    printf("Coverage data should be in gengtype.gcda and related files\n");
    printf("Run 'gcov gengtype.cc' to see coverage results\n");
    
    return 0;
}
