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
    "-I. -I../../include -I../../gcc -c gengtype.cc -o gengtype.o && " \
    "g++ -O0 -fprofile-arcs -ftest-coverage -DIN_GCC -DHAVE_CONFIG_H " \
    "-I. -I../../include -I../../gcc -c gengtype-state.cc -o gengtype-state.o && " \
    "g++ -O0 -fprofile-arcs -ftest-coverage gengtype.o gengtype-state.o " \
    "-lgcov -liberty -o gengtype_coverage"

/* GT file 1: Basic types and forward declarations */
const char *gt_file1_content = 
"%{\n"
"/* Test file 1: Basic type definitions */\n"
"%}\n"
"\n"
"/* TYPE_UNDEFINED - forward declaration */\n"
"struct undefined_struct;\n"
"\n"
"/* TYPE_SCALAR - scalar typedefs */\n"
"typedef int my_scalar;\n"
"typedef unsigned long scalar_ulong;\n"
"\n"
"/* TYPE_STRING - string type usage */\n"
"struct string_struct {\n"
"  const char *name;          /* TYPE_STRING */\n"
"  char *dynamic_string;\n"
"};\n"
"\n"
"/* TYPE_STRUCT - regular struct */\n"
"struct my_struct {\n"
"  int a;\n"
"  float b;\n"
"};\n"
"\n"
"/* TYPE_POINTER - pointer typedef */\n"
"typedef struct my_struct *my_struct_ptr;\n"
"\n"
"/* TYPE_ARRAY - array typedef */\n"
"typedef int int_array[10];\n"
"typedef struct my_struct struct_array[5];\n"
"%}";

/* GT file 2: Complex types and user-defined structures */
const char *gt_file2_content =
"%{\n"
"/* Test file 2: Complex type combinations */\n"
"%}\n"
"\n"
"/* TYPE_USER_STRUCT - struct with user marking */\n"
"struct user_struct {\n"
"  int *p;\n"
"  void *data;\n"
"} GTY((user));\n"
"\n"
"/* TYPE_UNION - union definition */\n"
"union my_union {\n"
"  int i;\n"
"  void *p;\n"
"  double d;\n"
"};\n"
"\n"
"/* TYPE_CALLBACK - callback function pointer */\n"
"typedef void (*callback_fn)(void);\n"
"typedef int (*compare_fn)(const void *, const void *);\n"
"\n"
"/* Nested complex type: struct containing pointer to union of arrays */\n"
"struct complex_nested {\n"
"  union {\n"
"    int int_member;\n"
"    struct my_struct *struct_ptr;\n"
"  } u;\n"
"  callback_fn handler;\n"
"  int_array numbers;\n"
"};\n"
"\n"
"/* Array of pointers to user structs */\n"
"struct user_struct *user_ptr_array[20];\n"
"\n"
"/* Pointer to array of callbacks */\n"
"callback_fn (*callback_array_ptr)[10];\n"
"%}";

/* GT file 3: Language-specific and edge cases */
const char *gt_file3_content =
"%{\n"
"/* Test file 3: Language structs and edge cases */\n"
"%}\n"
"\n"
"/* TYPE_LANG_STRUCT - language-specific struct */\n"
"struct lang_struct {\n"
"  int data;\n"
"  void *lang_data;\n"
"} GTY ((lang));\n"
"\n"
"/* Another language struct with nested types */\n"
"struct lang_tree_node {\n"
"  struct lang_tree_node *left;\n"
"  struct lang_tree_node *right;\n"
"  union {\n"
"    int ival;\n"
"    double dval;\n"
"  } value;\n"
"} GTY ((lang));\n"
"\n"
"/* Multiple scalar types */\n"
"typedef char byte;\n"
"typedef short int16;\n"
"typedef long long int64;\n"
"\n"
"/* String array */\n"
"const char *string_array[] = {\"hello\", \"world\", \"test\"};\n"
"\n"
"/* Complex pointer chain */\n"
"struct pointer_chain {\n"
"  struct pointer_chain **next;\n"
"  void ***data_ppp;\n"
"};\n"
"\n"
"/* Mixed array types */\n"
"union my_union union_array[15];\n"
"callback_fn callback_list[5];\n"
"%}";

/* GT file 4: With syntax error (for error path testing) */
const char *gt_file4_content =
"%{\n"
"/* Test file 4: Contains deliberate syntax error */\n"
"\n"
"struct error_struct {\n"
"  int missing_semicolon\n"  /* Missing semicolon */
"};\n"
"\n"
"/* Missing closing %} */\n";

/* GT file 5: Duplicate definitions (for warning testing) */
const char *gt_file5_content =
"%{\n"
"/* Test file 5: Duplicate type definitions */\n"
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
"typedef int my_int;\n"
"typedef int my_int;  /* Duplicate typedef */\n"
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
    
    /* Rename to include suffix */
    rename(template, filename);
    
    FILE *f = fdopen(fd, "w");
    if (!f) {
        perror("fdopen failed");
        free(filename);
        return NULL;
    }
    
    fwrite(content, 1, strlen(content), f);
    fclose(f);
    
    return filename;
}

/* Execute command and return exit status */
int execute_command(const char *cmd) {
    printf("Executing: %s\n", cmd);
    int status = system(cmd);
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    return -1;
}

/* Run gengtype with various patterns */
void run_gengtype_tests(const char *gengtype_exe, char **files, int file_count) {
    char cmd[4096];
    
    /* Pattern A: Process each file individually */
    printf("\n=== Pattern A: Individual file processing ===\n");
    for (int i = 0; i < file_count; i++) {
        snprintf(cmd, sizeof(cmd), "%s -g output_header_%d.h %s", 
                 gengtype_exe, i, files[i]);
        execute_command(cmd);
    }
    
    /* Pattern B: Batch processing with -p flag */
    printf("\n=== Pattern B: Batch processing with -p ===\n");
    char *filelist = create_temp_file("", ".filelist");
    FILE *fl = fopen(filelist, "w");
    if (fl) {
        for (int i = 0; i < file_count; i++) {
            fprintf(fl, "%s\n", files[i]);
        }
        fclose(fl);
        
        snprintf(cmd, sizeof(cmd), "%s -p %s", gengtype_exe, filelist);
        execute_command(cmd);
        
        unlink(filelist);
        free(filelist);
    }
    
    /* Pattern C: Multiple files with header generation */
    printf("\n=== Pattern C: Multiple files, header generation ===\n");
    snprintf(cmd, sizeof(cmd), "%s -g combined_header.h ", gengtype_exe);
    char *cmd_ptr = cmd + strlen(cmd);
    for (int i = 0; i < file_count && i < 3; i++) {  /* Use first 3 valid files */
        cmd_ptr += snprintf(cmd_ptr, sizeof(cmd) - (cmd_ptr - cmd), "%s ", files[i]);
    }
    execute_command(cmd);
    
    /* Pattern D: Error and warning cases */
    printf("\n=== Pattern D: Error/warning cases ===\n");
    if (file_count >= 4) {
        /* Test with syntax error file */
        snprintf(cmd, sizeof(cmd), "%s -g error_output.h %s", 
                 gengtype_exe, files[3]);
        execute_command(cmd);
    }
    
    if (file_count >= 5) {
        /* Test with duplicate definitions file */
        snprintf(cmd, sizeof(cmd), "%s -g warning_output.h %s", 
                 gengtype_exe, files[4]);
        execute_command(cmd);
    }
    
    /* Additional test: Run with debug flag if available */
    printf("\n=== Additional: Run with -d flag for debug ===\n");
    snprintf(cmd, sizeof(cmd), "%s -d 1 -g debug_output.h %s %s", 
             gengtype_exe, files[0], files[1]);
    execute_command(cmd);
}

/* Main test driver */
int main() {
    char *gt_files[5];
    int file_count = 0;
    
    printf("=== GCC gengtype Coverage Test ===\n");
    
    /* Step 1: Compile gengtype with coverage instrumentation */
    printf("\n1. Compiling gengtype with coverage flags...\n");
    if (execute_command(COMPILE_GENGTYPE) != 0) {
        fprintf(stderr, "Failed to compile gengtype with coverage\n");
        return 1;
    }
    
    /* Step 2: Create test GT files */
    printf("\n2. Creating test .gt files...\n");
    
    gt_files[file_count++] = create_temp_file(gt_file1_content, ".gt");
    gt_files[file_count++] = create_temp_file(gt_file2_content, ".gt");
    gt_files[file_count++] = create_temp_file(gt_file3_content, ".gt");
    gt_files[file_count++] = create_temp_file(gt_file4_content, ".gt");
    gt_files[file_count++] = create_temp_file(gt_file5_content, ".gt");
    
    for (int i = 0; i < file_count; i++) {
        if (!gt_files[i]) {
            fprintf(stderr, "Failed to create temp file %d\n", i);
            return 1;
        }
        printf("Created: %s\n", gt_files[i]);
    }
    
    /* Step 3: Run gengtype with various patterns */
    printf("\n3. Running gengtype tests...\n");
    run_gengtype_tests("./gengtype_coverage", gt_files, file_count);
    
    /* Step 4: Generate coverage report */
    printf("\n4. Generating coverage report...\n");
    execute_command("gcov gengtype.cc gengtype-state.cc");
    
    /* Step 5: Display relevant coverage information */
    printf("\n5. Coverage information for target switch (lines 182-213):\n");
    execute_command("grep -n -A 30 -B 5 'case TYPE_UNDEFINED' gengtype.cc.gcov");
    
    /* Step 6: Cleanup */
    printf("\n6. Cleaning up...\n");
    for (int i = 0; i < file_count; i++) {
        unlink(gt_files[i]);
        free(gt_files[i]);
    }
    
    /* Remove generated files */
    unlink("gengtype_coverage");
    unlink("gengtype.o");
    unlink("gengtype-state.o");
    unlink("output_header_0.h");
    unlink("output_header_1.h");
    unlink("output_header_2.h");
    unlink("combined_header.h");
    unlink("error_output.h");
    unlink("warning_output.h");
    unlink("debug_output.h");
    
    printf("\n=== Test completed ===\n");
    printf("Check gengtype.cc.gcov for line-by-line coverage\n");
    printf("Specifically verify lines 182-213 have execution counts > 0\n");
    
    return 0;
}
