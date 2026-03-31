/* test_gengtype_coverage.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>

/* Compile gengtype with coverage instrumentation */
#define COMPILE_GENGTYPE \
    "g++ -O0 -fprofile-arcs -ftest-coverage -DIN_GCC " \
    "-I. -I../../include -I../../gcc " \
    "-c gengtype.cc -o gengtype.o && " \
    "g++ -O0 -fprofile-arcs -ftest-coverage -DIN_GCC " \
    "-I. -I../../include -I../../gcc " \
    "-c gengtype-state.cc -o gengtype-state.o && " \
    "g++ -O0 -fprofile-arcs -ftest-coverage " \
    "gengtype.o gengtype-state.o -lgcov -liberty -o gengtype_coverage"

/* GT file 1: Basic types and undefined */
const char *gt_file1 = 
"%{\n"
"/* File 1: Basic types and undefined */\n"
"%}\n"
"\n"
"/* TYPE_UNDEFINED - forward declaration */\n"
"struct undefined_struct;\n"
"\n"
"/* TYPE_SCALAR */\n"
"typedef int my_scalar;\n"
"typedef unsigned long scalar2;\n"
"\n"
"/* TYPE_STRING */\n"
"struct string_struct {\n"
"  const char *name;  /* string type */\n"
"  char *data;\n"
"};\n"
"\n"
"/* TYPE_STRUCT */\n"
"struct my_struct {\n"
"  int a;\n"
"  double b;\n"
"};\n"
"\n"
"/* TYPE_POINTER */\n"
"typedef struct my_struct *struct_ptr;\n"
"typedef int *int_ptr;\n"
"\n"
"/* TYPE_ARRAY */\n"
"typedef int my_array[10];\n"
"typedef struct my_struct struct_array[5];\n"
"%}";

/* GT file 2: User structs, unions, and callbacks */
const char *gt_file2 =
"%{\n"
"/* File 2: User structs, unions, and callbacks */\n"
"%}\n"
"\n"
"/* TYPE_USER_STRUCT with user-provided marking */\n"
"struct user_struct {\n"
"  int *p;\n"
"  void *data;\n"
"} GTY((user));\n"
"\n"
"/* TYPE_UNION */\n"
"union my_union {\n"
"  int i;\n"
"  void *p;\n"
"  double d;\n"
"};\n"
"\n"
"/* TYPE_CALLBACK */\n"
"typedef void (*callback_fn)(void);\n"
"typedef int (*compare_fn)(const void *, const void *);\n"
"\n"
"/* Complex nested type combining multiple categories */\n"
"struct complex_type {\n"
"  union my_union u;          /* union member */\n"
"  struct user_struct *usr;   /* pointer to user struct */\n"
"  callback_fn cb;            /* callback */\n"
"  int array[20];             /* array */\n"
"  const char *str;           /* string */\n"
"};\n"
"\n"
"/* Another union with struct members */\n"
"union nested_union {\n"
"  struct my_struct s;\n"
"  struct user_struct u;\n"
"  callback_fn fn;\n"
"};\n"
"%}";

/* GT file 3: Lang structs, arrays of pointers, and error cases */
const char *gt_file3 =
"%{\n"
"/* File 3: Lang structs and complex arrays */\n"
"%}\n"
"\n"
"/* TYPE_LANG_STRUCT */\n"
"struct lang_struct {\n"
"  int data;\n"
"  void *lang_data;\n"
"} GTY((lang));\n"
"\n"
"/* More TYPE_ARRAY variations */\n"
"typedef union my_union union_array[8];\n"
"typedef callback_fn callback_array[4];\n"
"\n"
"/* Pointer to array */\n"
"typedef int (*array_ptr)[10];\n"
"\n"
"/* Struct containing array of pointers */\n"
"struct container {\n"
"  struct my_struct *items[5];\n"
"  union my_union *unions[3];\n"
"  callback_fn handlers[2];\n"
"};\n"
"\n"
"/* Forward declaration for undefined */\n"
"struct another_undefined;\n"
"\n"
"/* Scalar typedefs */\n"
"typedef short small_scalar;\n"
"typedef long long big_scalar;\n"
"%}";

/* GT file 4: With syntax error to test error paths */
const char *gt_file4 =
"%{\n"
"/* File 4: Contains syntax error */\n"
"/* Missing closing %} */\n"
"\n"
"struct error_struct {\n"
"  int x;\n"
"};\n"
"/* No closing %} - deliberate error */";

/* GT file 5: Duplicate definition for warning test */
const char *gt_file5 =
"%{\n"
"/* File 5: Duplicate definitions */\n"
"%}\n"
"\n"
"struct duplicate_struct {\n"
"  int a;\n"
"};\n"
"\n"
"/* Duplicate definition to trigger warning */\n"
"struct duplicate_struct {\n"
"  int b;\n"
"};\n"
"%}";

/* Create temporary file with given content */
char *create_temp_file(const char *content, const char *suffix) {
    char template[] = "/tmp/gengtype_test_XXXXXX";
    int fd = mkstemp(template);
    if (fd == -1) {
        perror("mkstemp failed");
        return NULL;
    }
    
    char *filename = strdup(template);
    strcat(filename, suffix);
    
    FILE *f = fdopen(fd, "w");
    if (!f) {
        perror("fdopen failed");
        free(filename);
        close(fd);
        return NULL;
    }
    
    fwrite(content, 1, strlen(content), f);
    fclose(f);
    
    return filename;
}

/* Run gengtype on a single file */
int run_gengtype_single(const char *gengtype_exe, const char *input_file, 
                       const char *output_header, const char *output_routine) {
    char cmd[1024];
    int status;
    
    /* Pattern C: Generate header and routine files */
    if (output_header && output_routine) {
        snprintf(cmd, sizeof(cmd), "%s -g %s -r %s %s", 
                gengtype_exe, output_header, output_routine, input_file);
    } else {
        /* Simple parse */
        snprintf(cmd, sizeof(cmd), "%s %s", gengtype_exe, input_file);
    }
    
    printf("Executing: %s\n", cmd);
    status = system(cmd);
    
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    return -1;
}

/* Pattern B: Batch processing with -p flag */
int run_gengtype_batch(const char *gengtype_exe, char **input_files, int count) {
    char listfile[] = "/tmp/gengtype_filelist_XXXXXX";
    int fd = mkstemp(listfile);
    if (fd == -1) {
        perror("mkstemp for filelist failed");
        return -1;
    }
    
    FILE *f = fdopen(fd, "w");
    if (!f) {
        perror("fdopen failed");
        close(fd);
        return -1;
    }
    
    for (int i = 0; i < count; i++) {
        fprintf(f, "%s\n", input_files[i]);
    }
    fclose(f);
    
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "%s -p %s", gengtype_exe, listfile);
    
    printf("Executing batch: %s\n", cmd);
    int status = system(cmd);
    
    unlink(listfile);
    
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    return -1;
}

/* Main test driver */
int main(int argc, char *argv[]) {
    char *gt_files[5];
    char *header_file = NULL;
    char *routine_file = NULL;
    int i, result;
    
    printf("=== GCC gengtype Coverage Test ===\n");
    
    /* Step 1: Compile gengtype with coverage instrumentation */
    printf("\n1. Compiling gengtype with coverage flags...\n");
    result = system(COMPILE_GENGTYPE);
    if (result != 0) {
        fprintf(stderr, "Failed to compile gengtype\n");
        return 1;
    }
    
    /* Step 2: Create temporary GT files */
    printf("\n2. Creating test GT files...\n");
    
    gt_files[0] = create_temp_file(gt_file1, ".gt");
    gt_files[1] = create_temp_file(gt_file2, ".gt");
    gt_files[2] = create_temp_file(gt_file3, ".gt");
    gt_files[3] = create_temp_file(gt_file4, ".gt");  /* Error case */
    gt_files[4] = create_temp_file(gt_file5, ".gt");  /* Warning case */
    
    for (i = 0; i < 5; i++) {
        if (!gt_files[i]) {
            fprintf(stderr, "Failed to create temp file %d\n", i);
            return 1;
        }
        printf("  Created: %s\n", gt_files[i]);
    }
    
    /* Create output files */
    header_file = create_temp_file("", ".h");
    routine_file = create_temp_file("", ".c");
    
    /* Step 3: Execute gengtype in various patterns */
    printf("\n3. Executing gengtype with different patterns...\n");
    
    /* Pattern A: Process each file individually */
    printf("\nPattern A: Individual file processing\n");
    for (i = 0; i < 3; i++) {  /* Skip error files for individual processing */
        printf("\nProcessing %s:\n", gt_files[i]);
        result = run_gengtype_single("./gengtype_coverage", gt_files[i], 
                                   i == 0 ? header_file : NULL, 
                                   i == 0 ? routine_file : NULL);
        printf("  Exit status: %d\n", result);
    }
    
    /* Pattern B: Batch processing */
    printf("\nPattern B: Batch processing (-p flag)\n");
    result = run_gengtype_batch("./gengtype_coverage", gt_files, 3);
    printf("  Batch exit status: %d\n", result);
    
    /* Pattern C: Header generation with multiple files */
    printf("\nPattern C: Header generation with multiple inputs\n");
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), 
             "./gengtype_coverage -g %s -r %s %s %s %s",
             header_file, routine_file, gt_files[0], gt_files[1], gt_files[2]);
    printf("Executing: %s\n", cmd);
    result = system(cmd);
    printf("  Exit status: %d\n", result);
    
    /* Pattern D: Error and warning cases */
    printf("\nPattern D: Error and warning cases\n");
    
    printf("\nProcessing file with syntax error:\n");
    result = run_gengtype_single("./gengtype_coverage", gt_files[3], NULL, NULL);
    printf("  Exit status (expected non-zero): %d\n", result);
    
    printf("\nProcessing file with duplicate definition:\n");
    result = run_gengtype_single("./gengtype_coverage", gt_files[4], NULL, NULL);
    printf("  Exit status: %d\n", result);
    
    /* Step 4: Verify output was generated */
    printf("\n4. Verifying outputs...\n");
    struct stat st;
    if (stat(header_file, &st) == 0 && st.st_size > 0) {
        printf("  Header file generated: %s (%ld bytes)\n", header_file, st.st_size);
    }
    if (stat(routine_file, &st) == 0 && st.st_size > 0) {
        printf("  Routine file generated: %s (%ld bytes)\n", routine_file, st.st_size);
    }
    
    /* Step 5: Check coverage data */
    printf("\n5. Checking coverage data...\n");
    if (stat("gengtype.gcda", &st) == 0) {
        printf("  Coverage data generated: gengtype.gcda\n");
        printf("  Run 'gcov gengtype.cc' to see line coverage\n");
    } else {
        printf("  No coverage data found. Ensure -fprofile-arcs was used.\n");
    }
    
    /* Step 6: Cleanup */
    printf("\n6. Cleaning up temporary files...\n");
    for (i = 0; i < 5; i++) {
        unlink(gt_files[i]);
        free(gt_files[i]);
    }
    
    if (header_file) {
        unlink(header_file);
        free(header_file);
    }
    if (routine_file) {
        unlink(routine_file);
        free(routine_file);
    }
    
    unlink("gengtype_coverage");
    unlink("gengtype.o");
    unlink("gengtype-state.o");
    
    printf("\n=== Test completed ===\n");
    printf("To view coverage results:\n");
    printf("  gcov gengtype.cc\n");
    printf("  cat gengtype.cc.gcov | grep -A2 -B2 '^#####'\n");
    
    return 0;
}
