/* test_gengtype_coverage.c - Driver program to test gengtype type counting */
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
    "gengtype.cc gengtype-state.cc gengtype-lex.cc " \
    "-lgcov -liberty -o gengtype_coverage"

/* GT file 1: Basic types and forward declarations */
const char *gt_file1 = 
"%{\n"
"/* Test file 1: Basic types and forward declarations */\n"
"#include \"config.h\"\n"
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
"  const char *name;  /* TYPE_STRING */\n"
"  char *buffer;\n"
"};\n"
"\n"
"/* TYPE_POINTER: Pointer types */\n"
"typedef struct string_struct *string_ptr;\n"
"typedef my_scalar *scalar_ptr;\n"
"%}";

/* GT file 2: Structs, unions, and arrays */
const char *gt_file2 =
"%{\n"
"/* Test file 2: Structs, unions, and arrays */\n"
"#include \"config.h\"\n"
"%}\n"
"\n"
"/* TYPE_STRUCT: Regular struct */\n"
"struct my_struct {\n"
"  int a;\n"
"  double b;\n"
"  struct my_struct *next;  /* Self-referential pointer */\n"
"};\n"
"\n"
"/* TYPE_USER_STRUCT: Struct with user marking */\n"
"struct user_struct {\n"
"  int *p;\n"
"  void *data;\n"
"} GTY((user));\n"
"\n"
"/* TYPE_UNION: Union type */\n"
"union my_union {\n"
"  int i;\n"
"  void *p;\n"
"  double d;\n"
"};\n"
"\n"
"/* TYPE_ARRAY: Array types */\n"
"typedef int my_array[10];\n"
"typedef struct my_struct struct_array[5];\n"
"\n"
"/* Nested complex type */\n"
"struct complex_type {\n"
"  union my_union u;\n"
"  my_array arr;\n"
"  struct user_struct *user_ptr;\n"
"};\n"
"%}";

/* GT file 3: Callbacks, lang structs, and more complex types */
const char *gt_file3 =
"%{\n"
"/* Test file 3: Callbacks and language-specific structs */\n"
"#include \"config.h\"\n"
"%}\n"
"\n"
"/* TYPE_CALLBACK: Callback function type */\n"
"typedef void (*callback_fn)(void);\n"
"typedef int (*compare_fn)(const void *, const void *);\n"
"\n"
"/* TYPE_LANG_STRUCT: Language-specific struct */\n"
"struct lang_struct {\n"
"  int data;\n"
"  void *lang_data;\n"
"} GTY((lang));\n"
"\n"
"/* More complex nested types */\n"
"struct container {\n"
"  callback_fn handler;\n"
"  struct lang_struct *lang;\n"
"  union {\n"
"    int tag;\n"
"    void *ptr;\n"
"  } variant;\n"
"};\n"
"\n"
"/* Pointer to callback */\n"
"typedef callback_fn *callback_ptr;\n"
"\n"
"/* Array of pointers to lang structs */\n"
"typedef struct lang_struct *lang_ptr_array[8];\n"
"%}";

/* GT file 4: File with syntax error (for error path testing) */
const char *gt_file4 =
"%{\n"
"/* Test file 4: File with syntax error */\n"
"#include \"config.h\"\n"
"/* Missing closing %} to trigger error */\n"
"\n"
"struct bad_struct {\n"
"  int x;\n";

/* GT file 5: Duplicate definitions (for warning testing) */
const char *gt_file5 =
"%{\n"
"/* Test file 5: Duplicate type definitions */\n"
"#include \"config.h\"\n"
"%}\n"
"\n"
"typedef int my_scalar;  /* Duplicate from file1 */\n"
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

/* Write a string to a temporary file */
char *write_temp_file(const char *content, const char *prefix, const char *suffix) {
    char template[256];
    snprintf(template, sizeof(template), "/tmp/%s_XXXXXX%s", prefix, suffix);
    int fd = mkstemps(template, strlen(suffix));
    if (fd < 0) {
        perror("mkstemps failed");
        return NULL;
    }
    
    write(fd, content, strlen(content));
    close(fd);
    
    return strdup(template);
}

/* Run gengtype on a set of files */
int run_gengtype(const char *output_header, const char **input_files, int count, 
                 const char *gengtype_exe) {
    char cmd[4096];
    int status;
    
    /* Pattern C: Generate header with multiple input files */
    snprintf(cmd, sizeof(cmd), "%s -g %s", gengtype_exe, output_header);
    for (int i = 0; i < count; i++) {
        strncat(cmd, " ", sizeof(cmd) - strlen(cmd) - 1);
        strncat(cmd, input_files[i], sizeof(cmd) - strlen(cmd) - 1);
    }
    
    printf("Executing: %s\n", cmd);
    status = system(cmd);
    
    if (WIFEXITED(status)) {
        printf("gengtype exited with status %d\n", WEXITSTATUS(status));
    }
    
    return status;
}

/* Pattern B: Batch processing with -p flag */
int run_gengtype_batch(const char *filelist, const char *gengtype_exe) {
    char cmd[1024];
    int status;
    
    snprintf(cmd, sizeof(cmd), "%s -p %s", gengtype_exe, filelist);
    printf("Executing batch: %s\n", cmd);
    status = system(cmd);
    
    return status;
}

/* Pattern A: Process files individually */
int process_files_individually(const char **files, int count, const char *gengtype_exe) {
    int overall_status = 0;
    
    for (int i = 0; i < count; i++) {
        char cmd[1024];
        snprintf(cmd, sizeof(cmd), "%s -c gtype-desc.c %s", gengtype_exe, files[i]);
        printf("Processing %s individually\n", files[i]);
        int status = system(cmd);
        if (status != 0) {
            printf("Warning: File %s returned status %d\n", files[i], status);
        }
        overall_status |= status;
    }
    
    return overall_status;
}

/* Main test driver */
int main(int argc, char **argv) {
    char *temp_files[10];
    char *filelist = NULL;
    int num_files = 0;
    int ret = 0;
    
    printf("=== GCC gengtype Type Coverage Test ===\n");
    
    /* Step 1: Compile gengtype with coverage instrumentation */
    printf("\n1. Compiling gengtype with coverage flags...\n");
    int compile_status = system(COMPILE_GENGTYPE);
    if (compile_status != 0) {
        fprintf(stderr, "Failed to compile gengtype\n");
        return 1;
    }
    
    /* Step 2: Create temporary GT files */
    printf("\n2. Creating temporary GT files...\n");
    
    temp_files[num_files++] = write_temp_file(gt_file1, "test1", ".gt");
    temp_files[num_files++] = write_temp_file(gt_file2, "test2", ".gt");
    temp_files[num_files++] = write_temp_file(gt_file3, "test3", ".gt");
    temp_files[num_files++] = write_temp_file(gt_file4, "test4", ".gt");  /* Error case */
    temp_files[num_files++] = write_temp_file(gt_file5, "test5", ".gt");  /* Warning case */
    
    for (int i = 0; i < num_files; i++) {
        if (!temp_files[i]) {
            fprintf(stderr, "Failed to create temp file %d\n", i);
            return 1;
        }
        printf("  Created: %s\n", temp_files[i]);
    }
    
    /* Step 3: Create file list for batch processing */
    printf("\n3. Creating file list for batch processing...\n");
    filelist = write_temp_file("", "filelist", ".txt");
    if (!filelist) {
        fprintf(stderr, "Failed to create file list\n");
        return 1;
    }
    
    FILE *fl = fopen(filelist, "w");
    if (!fl) {
        perror("Failed to open file list");
        return 1;
    }
    
    /* Write first 3 valid files to the list */
    for (int i = 0; i < 3; i++) {
        fprintf(fl, "%s\n", temp_files[i]);
    }
    fclose(fl);
    printf("  File list: %s\n", filelist);
    
    /* Step 4: Run gengtype with various patterns */
    printf("\n4. Running gengtype with different patterns...\n");
    
    /* Pattern C: Generate header from multiple files */
    printf("\n--- Pattern C: Header generation from multiple files ---\n");
    char *output_header = write_temp_file("", "output", ".h");
    run_gengtype(output_header, temp_files, 3, "./gengtype_coverage");
    
    /* Pattern B: Batch processing */
    printf("\n--- Pattern B: Batch processing with -p flag ---\n");
    run_gengtype_batch(filelist, "./gengtype_coverage");
    
    /* Pattern A: Individual file processing */
    printf("\n--- Pattern A: Individual file processing ---\n");
    process_files_individually(temp_files, 3, "./gengtype_coverage");
    
    /* Pattern D: Error and warning cases */
    printf("\n--- Pattern D: Error and warning cases ---\n");
    printf("Processing file with syntax error (should fail):\n");
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "./gengtype_coverage -c gtype-desc.c %s", temp_files[3]);
    system(cmd);
    
    printf("\nProcessing file with duplicate definitions (should warn):\n");
    snprintf(cmd, sizeof(cmd), "./gengtype_coverage -c gtype-desc.c %s", temp_files[4]);
    system(cmd);
    
    /* Step 5: Generate routine file to ensure full processing */
    printf("\n5. Generating routine file for complete processing...\n");
    char *output_routine = write_temp_file("", "routines", ".c");
    snprintf(cmd, sizeof(cmd), "./gengtype_coverage -r %s %s %s %s", 
             output_routine, temp_files[0], temp_files[1], temp_files[2]);
    system(cmd);
    
    /* Step 6: Cleanup */
    printf("\n6. Cleaning up temporary files...\n");
    for (int i = 0; i < num_files; i++) {
        unlink(temp_files[i]);
        free(temp_files[i]);
    }
    unlink(filelist);
    free(filelist);
    unlink(output_header);
    free(output_header);
    unlink(output_routine);
    free(output_routine);
    
    /* Remove gengtype_coverage executable */
    unlink("./gengtype_coverage");
    
    /* Clean up coverage files if they exist */
    unlink("gengtype.gcda");
    unlink("gengtype-state.gcda");
    unlink("gengtype-lex.gcda");
    
    printf("\n=== Test completed ===\n");
    printf("Coverage data should be in .gcda files\n");
    printf("Run 'gcov gengtype.cc' to see line coverage\n");
    
    return 0;
}
