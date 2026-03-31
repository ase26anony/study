/* test_gengtype_coverage.c - Coverage test for gengtype type counting switch */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>

/* Compile gengtype with coverage flags */
#define GENGTYPE_SOURCE "gengtype.cc"
#define GENGTYPE_STATE_SOURCE "gengtype-state.cc"
#define GENGTYPE_EXEC "gengtype_coverage"
#define MAX_FILES 10
#define MAX_PATH 256

/* GT file 1: Basic types and undefined */
const char *gt_file1 = 
"%{\n"
"/* Test file 1: Basic types and undefined */\n"
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
"/* TYPE_POINTER */\n"
"typedef struct string_struct *string_ptr;\n"
"typedef my_scalar *scalar_ptr;\n"
"\n"
"/* TYPE_ARRAY */\n"
"typedef int my_array[10];\n"
"typedef struct string_struct struct_array[5];\n";

/* GT file 2: Structs, unions, and user structs */
const char *gt_file2 =
"%{\n"
"/* Test file 2: Structs, unions, and user structs */\n"
"%}\n"
"\n"
"/* TYPE_STRUCT */\n"
"struct my_struct {\n"
"  int a;\n"
"  double b;\n"
"  my_array arr;\n"
"};\n"
"\n"
"/* TYPE_USER_STRUCT */\n"
"struct user_struct {\n"
"  int *p;\n"
"  void *data;\n"
"} GTY((user));\n"
"\n"
"/* TYPE_UNION */\n"
"union my_union {\n"
"  int i;\n"
"  void *p;\n"
"  struct my_struct *s;\n"
"};\n"
"\n"
"/* Nested complex type */\n"
"struct complex_struct {\n"
"  union my_union u;\n"
"  struct my_struct nested;\n"
"  struct user_struct *user_ptr;\n"
"};\n"
"\n"
"/* Another pointer type */\n"
"typedef union my_union *union_ptr;";

/* GT file 3: Callback, lang struct, and more arrays */
const char *gt_file3 =
"%{\n"
"/* Test file 3: Callback, lang struct, and arrays */\n"
"%}\n"
"\n"
"/* TYPE_CALLBACK */\n"
"typedef void (*callback_fn)(void);\n"
"typedef int (*int_callback)(int, char*);\n"
"\n"
"/* TYPE_LANG_STRUCT */\n"
"struct lang_struct {\n"
"  int data;\n"
"  void *lang_data;\n"
"} GTY ((lang));\n"
"\n"
"struct another_lang_struct {\n"
"  callback_fn handler;\n"
"} GTY((lang));\n"
"\n"
"/* Multi-dimensional array */\n"
"typedef int matrix[10][10];\n"
"\n"
"/* Struct with callback */\n"
"struct with_callback {\n"
"  callback_fn cb;\n"
"  int_callback int_cb;\n"
"};\n"
"\n"
"/* Pointer to lang struct */\n"
"typedef struct lang_struct *lang_ptr;\n"
"\n"
"/* Union with array */\n"
"union array_union {\n"
"  matrix m;\n"
"  my_array a;\n"
"};";

/* GT file 4: With syntax error (for error path testing) */
const char *gt_file4 =
"%{\n"
"/* Test file 4: With deliberate syntax error */\n"
"%}\n"
"\n"
"struct error_struct {\n"
"  int x;\n"
"  /* Missing semicolon to cause error */\n"
"  int y\n"
"};\n"
"\n"
"typedef int error_type";

/* GT file 5: Duplicate definitions (for warning testing) */
const char *gt_file5 =
"%{\n"
"/* Test file 5: Duplicate definitions */\n"
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
"\n"
"typedef int my_scalar;\n"
"typedef int my_scalar;  /* Duplicate typedef */";

/* Create temporary GT file */
int create_temp_gt_file(const char *content, char *filename) {
    char template[] = "/tmp/gt_test_XXXXXX.gt";
    int fd = mkstemps(template, 3);  /* .gt is 3 chars */
    if (fd < 0) {
        perror("mkstemps failed");
        return -1;
    }
    
    write(fd, content, strlen(content));
    close(fd);
    
    strcpy(filename, template);
    return 0;
}

/* Compile gengtype with coverage instrumentation */
int compile_gengtype_with_coverage() {
    char cmd[1024];
    int status;
    
    printf("Compiling gengtype with coverage instrumentation...\n");
    
    /* Compile gengtype.cc */
    snprintf(cmd, sizeof(cmd),
             "g++ -O0 -fprofile-arcs -ftest-coverage -DIN_GCC "
             "-DHAVE_CONFIG_H -I. -I../../include -I../../gcc "
             "-c " GENGTYPE_SOURCE " -o gengtype_coverage.o 2>&1");
    
    printf("Running: %s\n", cmd);
    status = system(cmd);
    if (status != 0) {
        fprintf(stderr, "Failed to compile gengtype.cc\n");
        return -1;
    }
    
    /* Compile gengtype-state.cc */
    snprintf(cmd, sizeof(cmd),
             "g++ -O0 -fprofile-arcs -ftest-coverage -DIN_GCC "
             "-DHAVE_CONFIG_H -I. -I../../include -I../../gcc "
             "-c " GENGTYPE_STATE_SOURCE " -o gengtype_state_coverage.o 2>&1");
    
    printf("Running: %s\n", cmd);
    status = system(cmd);
    if (status != 0) {
        fprintf(stderr, "Failed to compile gengtype-state.cc\n");
        return -1;
    }
    
    /* Link gengtype executable with coverage */
    snprintf(cmd, sizeof(cmd),
             "g++ -O0 -fprofile-arcs -ftest-coverage "
             "gengtype_coverage.o gengtype_state_coverage.o "
             "-lgcov -liberty -o " GENGTYPE_EXEC " 2>&1");
    
    printf("Running: %s\n", cmd);
    status = system(cmd);
    if (status != 0) {
        fprintf(stderr, "Failed to link gengtype executable\n");
        return -1;
    }
    
    printf("Successfully compiled " GENGTYPE_EXEC "\n");
    return 0;
}

/* Run gengtype on a single file */
int run_gengtype_single(const char *gt_file, const char *output_base) {
    char cmd[1024];
    int status;
    
    /* Run with -g to generate header (forces full parsing) */
    snprintf(cmd, sizeof(cmd),
             "./" GENGTYPE_EXEC " -g %s.h %s 2>&1",
             output_base, gt_file);
    
    printf("Running: %s\n", cmd);
    status = system(cmd);
    
    if (status != 0) {
        printf("Note: gengtype returned %d for %s (may be expected for error cases)\n",
               WEXITSTATUS(status), gt_file);
    }
    
    return status;
}

/* Run gengtype on multiple files using -p option */
int run_gengtype_batch(const char **gt_files, int count, const char *list_file) {
    char cmd[1024];
    FILE *list_fp;
    int i, status;
    
    /* Create file list */
    list_fp = fopen(list_file, "w");
    if (!list_fp) {
        perror("Failed to create file list");
        return -1;
    }
    
    for (i = 0; i < count; i++) {
        fprintf(list_fp, "%s\n", gt_files[i]);
    }
    fclose(list_fp);
    
    /* Run gengtype with -p option */
    snprintf(cmd, sizeof(cmd),
             "./" GENGTYPE_EXEC " -p %s 2>&1",
             list_file);
    
    printf("Running batch: %s\n", cmd);
    status = system(cmd);
    
    if (status != 0) {
        printf("Batch processing returned %d\n", WEXITSTATUS(status));
    }
    
    return status;
}

/* Run gengtype with -r to generate routines */
int run_gengtype_routines(const char **gt_files, int count, const char *output_base) {
    char cmd[2048];
    int i, status;
    
    /* Build command with all files */
    snprintf(cmd, sizeof(cmd), "./" GENGTYPE_EXEC " -r %s.c ", output_base);
    size_t len = strlen(cmd);
    
    for (i = 0; i < count && len < sizeof(cmd) - 100; i++) {
        len += snprintf(cmd + len, sizeof(cmd) - len, "%s ", gt_files[i]);
    }
    
    strcat(cmd, "2>&1");
    
    printf("Running routines generation: %s\n", cmd);
    status = system(cmd);
    
    if (status != 0) {
        printf("Routines generation returned %d\n", WEXITSTATUS(status));
    }
    
    return status;
}

/* Main test driver */
int main(int argc, char **argv) {
    char gt_filenames[5][MAX_PATH];
    char file_list[MAX_PATH];
    char output_base[64];
    int i, status;
    const char *gt_contents[5] = {gt_file1, gt_file2, gt_file3, gt_file4, gt_file5};
    
    printf("=== GCC gengtype Coverage Test ===\n");
    printf("Testing type counting switch (lines 182-213 in gengtype.cc)\n\n");
    
    /* Step 1: Compile gengtype with coverage */
    if (compile_gengtype_with_coverage() != 0) {
        fprintf(stderr, "Failed to compile gengtype\n");
        return 1;
    }
    
    /* Step 2: Create temporary GT files */
    printf("\nCreating test GT files...\n");
    for (i = 0; i < 5; i++) {
        if (create_temp_gt_file(gt_contents[i], gt_filenames[i]) != 0) {
            fprintf(stderr, "Failed to create GT file %d\n", i + 1);
            return 1;
        }
        printf("Created: %s\n", gt_filenames[i]);
    }
    
    /* Create file list for batch processing */
    snprintf(file_list, sizeof(file_list), "/tmp/gt_filelist_%d.txt", getpid());
    
    /* Step 3: Pattern A - Process each file individually */
    printf("\n--- Pattern A: Individual file processing ---\n");
    for (i = 0; i < 5; i++) {
        snprintf(output_base, sizeof(output_base), "/tmp/output_%d", i);
        printf("\nProcessing %s:\n", gt_filenames[i]);
        run_gengtype_single(gt_filenames[i], output_base);
    }
    
    /* Step 4: Pattern B - Batch processing with -p */
    printf("\n--- Pattern B: Batch processing with -p ---\n");
    run_gengtype_batch((const char **)gt_filenames, 5, file_list);
    
    /* Step 5: Pattern C - Header generation with multiple files */
    printf("\n--- Pattern C: Header generation with all files ---\n");
    run_gengtype_routines((const char **)gt_filenames, 3, "/tmp/all_types");  /* Only first 3 valid files */
    
    /* Step 6: Pattern D - Error cases already covered by files 4 and 5 */
    printf("\n--- Pattern D: Error/warning cases processed above ---\n");
    printf("File 4 has syntax error, File 5 has duplicate definitions\n");
    
    /* Step 7: Cleanup */
    printf("\nCleaning up temporary files...\n");
    for (i = 0; i < 5; i++) {
        unlink(gt_filenames[i]);
    }
    unlink(file_list);
    
    /* Remove generated output files */
    for (i = 0; i < 5; i++) {
        char header_file[MAX_PATH];
        snprintf(header_file, sizeof(header_file), "/tmp/output_%d.h", i);
        unlink(header_file);
    }
    unlink("/tmp/all_types.c");
    unlink("/tmp/all_types.h");
    
    /* Check for coverage data */
    printf("\nChecking for coverage data...\n");
    if (access("gengtype_coverage.gcda", F_OK) == 0) {
        printf("Coverage data generated: gengtype_coverage.gcda\n");
        printf("Run 'gcov gengtype_coverage.gcda' to see coverage results\n");
    } else {
        printf("No coverage data found. Ensure gengtype was built with -fprofile-arcs\n");
    }
    
    printf("\n=== Test Complete ===\n");
    printf("The switch statement should have been executed for:\n");
    printf("  - TYPE_UNDEFINED: struct undefined_struct;\n");
    printf("  - TYPE_SCALAR: typedef int my_scalar;\n");
    printf("  - TYPE_STRING: const char *name;\n");
    printf("  - TYPE_STRUCT: struct my_struct\n");
    printf("  - TYPE_USER_STRUCT: struct user_struct GTY((user))\n");
    printf("  - TYPE_UNION: union my_union\n");
    printf("  - TYPE_POINTER: typedef struct string_struct *string_ptr\n");
    printf("  - TYPE_ARRAY: typedef int my_array[10]\n");
    printf("  - TYPE_CALLBACK: typedef void (*callback_fn)(void)\n");
    printf("  - TYPE_LANG_STRUCT: struct lang_struct GTY((lang))\n");
    
    return 0;
}
