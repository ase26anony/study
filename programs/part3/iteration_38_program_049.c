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

/* GT file 1: Basic types with all categories */
static const char *gt_file1_content =
"%{\n"
"/* Test file 1: Basic type definitions */\n"
"%}\n"
"\n"
"/* TYPE_UNDEFINED: Forward declaration */\n"
"struct undefined_struct;\n"
"\n"
"/* TYPE_SCALAR: Scalar typedef */\n"
"typedef int my_scalar;\n"
"typedef unsigned long scalar2;\n"
"\n"
"/* TYPE_STRING: String type */\n"
"struct string_struct {\n"
"  const char *name;  /* This is TYPE_STRING */\n"
"  char *data;\n"
"};\n"
"\n"
"/* TYPE_STRUCT: Regular struct */\n"
"struct my_struct {\n"
"  int a;\n"
"  float b;\n"
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
"typedef int (*callback_with_args)(int, char*);\n"
"\n"
"/* TYPE_LANG_STRUCT: Language-specific struct */\n"
"struct lang_struct {\n"
"  int data;\n"
"  void *extra;\n"
"} GTY ((lang));\n"
"\n"
"/* Complex nested type combining multiple categories */\n"
"struct complex_nested {\n"
"  my_array arr;           /* TYPE_ARRAY */\n"
"  my_ptr ptr;            /* TYPE_POINTER */\n"
"  union my_union uni;    /* TYPE_UNION */\n"
"  callback_fn cb;        /* TYPE_CALLBACK */\n"
"};\n"
"%}";

/* GT file 2: More complex types and variations */
static const char *gt_file2_content =
"%{\n"
"/* Test file 2: Advanced type combinations */\n"
"%}\n"
"\n"
"/* Another TYPE_UNDEFINED */\n"
"struct another_undefined;\n"
"\n"
"/* More scalars */\n"
"typedef char byte_scalar;\n"
"typedef double real_scalar;\n"
"\n"
"/* String variations */\n"
"struct string_container {\n"
"  const char *static_str;\n"
"  char *dynamic_str;\n"
"  const char * const constant_str;\n"
"};\n"
"\n"
"/* Struct with pointers to different types */\n"
"struct pointer_holder {\n"
"  int *int_ptr;\n"
"  struct my_struct *struct_ptr;\n"
"  void **void_ptr_ptr;\n"
"};\n"
"\n"
"/* Array of pointers */\n"
"typedef void* ptr_array[20];\n"
"\n"
"/* Union with struct member */\n"
"union complex_union {\n"
"  struct {\n"
"    int x;\n"
"    int y;\n"
"  } point;\n"
"  void *generic;\n"
"};\n"
"\n"
"/* Callback with specific signature */\n"
"typedef struct my_struct* (*factory_fn)(int);\n"
"\n"
"/* Another language struct */\n"
"struct lang_struct2 {\n"
"  factory_fn create;\n"
"  ptr_array pointers;\n"
"} GTY((lang));\n"
"\n"
"/* User struct with complex members */\n"
"struct complex_user_struct {\n"
"  ptr_array data;\n"
"  callback_fn handlers[5];\n"
"} GTY((user));\n"
"%}";

/* GT file 3: Edge cases and error testing */
static const char *gt_file3_content =
"%{\n"
"/* Test file 3: Edge cases and potential warnings */\n"
"\n"
"/* Duplicate type definition to test warning path */\n"
"typedef int my_scalar;  /* Duplicate from file1 */\n"
"\n"
"/* Empty struct */\n"
"struct empty_struct {\n"
"};\n"
"\n"
"/* Struct with only scalar members */\n"
"struct scalar_only {\n"
"  int a;\n"
"  float b;\n"
"  double c;\n"
"};\n"
"\n"
"/* Pointer to pointer */\n"
"typedef int ***triple_ptr;\n"
"\n"
"/* Multi-dimensional array */\n"
"typedef int matrix[10][20];\n"
"\n"
"/* Complex callback chain */\n"
"typedef void (*init_fn)(void);\n"
"typedef void (*cleanup_fn)(void);\n"
"struct callback_chain {\n"
"  init_fn init;\n"
"  cleanup_fn cleanup;\n"
"};\n"
"\n"
"/* Mixed language and user struct */\n"
"struct mixed_attributes {\n"
"  int counter;\n"
"} GTY((user, lang));\n"
"\n"
"/* File ends without %} to test error handling */\n"
"/* Deliberate syntax error - missing %} */";

/* GT file 4: Valid file for batch processing */
static const char *gt_file4_content =
"%{\n"
"/* Test file 4: For batch processing test */\n"
"%}\n"
"\n"
"struct batch_struct {\n"
"  int id;\n"
"  char name[50];\n"
"};\n"
"\n"
"union batch_union {\n"
"  struct batch_struct s;\n"
"  long long data;\n"
"};\n"
"\n"
"typedef void (*batch_callback)(struct batch_struct*);\n"
"%}";

/* Create a temporary file with given content */
char* create_temp_file(const char *content, const char *suffix) {
    char template[] = "/tmp/gengtype_test_XXXXXX";
    int fd = mkstemp(template);
    if (fd == -1) {
        perror("mkstemp failed");
        return NULL;
    }
    
    /* Append suffix */
    char *filename = malloc(strlen(template) + strlen(suffix) + 1);
    strcpy(filename, template);
    strcat(filename, suffix);
    
    /* Rename to include suffix */
    close(fd);
    rename(template, filename);
    
    /* Write content */
    FILE *f = fopen(filename, "w");
    if (!f) {
        perror("fopen failed");
        free(filename);
        return NULL;
    }
    
    fwrite(content, 1, strlen(content), f);
    fclose(f);
    
    return filename;
}

/* Run gengtype with given arguments */
int run_gengtype(const char *gengtype_path, const char *arg_format, ...) {
    char command[1024];
    va_list args;
    
    va_start(args, arg_format);
    vsnprintf(command, sizeof(command), arg_format, args);
    va_end(args);
    
    printf("Executing: %s\n", command);
    
    int status = system(command);
    if (WIFEXITED(status)) {
        printf("Exit code: %d\n", WEXITSTATUS(status));
        return WEXITSTATUS(status);
    }
    return -1;
}

/* Create file list for batch processing */
char* create_file_list(char **files, int count) {
    char *listname = create_temp_file("", ".list");
    if (!listname) return NULL;
    
    FILE *f = fopen(listname, "w");
    if (!f) {
        free(listname);
        return NULL;
    }
    
    for (int i = 0; i < count; i++) {
        fprintf(f, "%s\n", files[i]);
    }
    fclose(f);
    
    return listname;
}

/* Main test driver */
int main(int argc, char **argv) {
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
    char *gt_files[4];
    
    gt_files[0] = create_temp_file(gt_file1_content, ".gt");
    gt_files[1] = create_temp_file(gt_file2_content, ".gt");
    gt_files[2] = create_temp_file(gt_file3_content, ".gt");  /* Has syntax error */
    gt_files[3] = create_temp_file(gt_file4_content, ".gt");
    
    for (int i = 0; i < 4; i++) {
        if (!gt_files[i]) {
            fprintf(stderr, "Failed to create temp file %d\n", i);
            return 1;
        }
        printf("  Created: %s\n", gt_files[i]);
    }
    printf("\n");
    
    /* Step 3: Test Pattern A - Process each file individually */
    printf("3. Pattern A: Processing files individually...\n");
    for (int i = 0; i < 3; i++) {  /* Only first 3 files */
        printf("\n  Processing %s:\n", gt_files[i]);
        
        /* Test with header generation (-g) */
        run_gengtype("./gengtype_coverage", 
                    "./gengtype_coverage -g output_%d.h %s", 
                    i, gt_files[i]);
        
        /* Test with routine generation (-r) */
        run_gengtype("./gengtype_coverage", 
                    "./gengtype_coverage -r output_%d.c %s", 
                    i, gt_files[i]);
        
        /* Test with both flags */
        run_gengtype("./gengtype_coverage", 
                    "./gengtype_coverage -g output_both_%d.h -r output_both_%d.c %s", 
                    i, i, gt_files[i]);
    }
    printf("\n");
    
    /* Step 4: Test Pattern B - Batch processing with -p */
    printf("4. Pattern B: Batch processing with -p flag...\n");
    char *filelist = create_file_list(gt_files, 4);
    if (filelist) {
        printf("  File list: %s\n", filelist);
        run_gengtype("./gengtype_coverage", 
                    "./gengtype_coverage -p %s", filelist);
        
        /* Clean up file list */
        unlink(filelist);
        free(filelist);
    }
    printf("\n");
    
    /* Step 5: Test Pattern C - Multiple files at once */
    printf("5. Pattern C: Processing multiple files together...\n");
    run_gengtype("./gengtype_coverage", 
                "./gengtype_coverage -g combined.h %s %s %s", 
                gt_files[0], gt_files[1], gt_files[3]);
    
    run_gengtype("./gengtype_coverage", 
                "./gengtype_coverage -r combined.c %s %s", 
                gt_files[0], gt_files[3]);
    printf("\n");
    
    /* Step 6: Test Pattern D - Error cases */
    printf("6. Pattern D: Testing error cases...\n");
    printf("  Processing file with syntax error (file3):\n");
    run_gengtype("./gengtype_coverage", 
                "./gengtype_coverage -g error_output.h %s", 
                gt_files[2]);
    
    /* Test with non-existent file */
    printf("\n  Processing non-existent file:\n");
    run_gengtype("./gengtype_coverage", 
                "./gengtype_coverage -g dummy.h nonexistent.gt");
    printf("\n");
    
    /* Step 7: Generate coverage data with aggressive processing */
    printf("7. Aggressive processing with debug flags...\n");
    
    /* Recompile with DEBUG_GENGTYPE */
    printf("  Recompiling with DEBUG_GENGTYPE...\n");
    char debug_compile[1024];
    snprintf(debug_compile, sizeof(debug_compile),
             "g++ -O0 -fprofile-arcs -ftest-coverage -DIN_GCC -DHAVE_CONFIG_H "
             "-DDEBUG_GENGTYPE -I. -I../../include -I../../gcc "
             "-c gengtype.cc -o gengtype_debug.o && "
             "g++ -O0 -fprofile-arcs -ftest-coverage -DIN_GCC -DHAVE_CONFIG_H "
             "-DDEBUG_GENGTYPE -I. -I../../include -I../../gcc "
             "-c gengtype-state.cc -o gengtype-state_debug.o && "
             "g++ -O0 -fprofile-arcs -ftest-coverage gengtype_debug.o "
             "gengtype-state_debug.o -lgcov -liberty -o gengtype_debug");
    
    if (system(debug_compile) == 0) {
        /* Run debug version on all valid files */
        printf("  Running debug version...\n");
        run_gengtype("./gengtype_debug", 
                    "./gengtype_debug -g debug_output.h %s %s %s", 
                    gt_files[0], gt_files[1], gt_files[3]);
    }
    printf("\n");
    
    /* Step 8: Cleanup and generate coverage report */
    printf("8. Cleanup and coverage report...\n");
    
    /* Remove output files */
    for (int i = 0; i < 10; i++) {
        char fname[100];
        snprintf(fname, sizeof(fname), "output_%d.h", i);
        unlink(fname);
        snprintf(fname, sizeof(fname), "output_%d.c", i);
        unlink(fname);
        snprintf(fname, sizeof(fname), "output_both_%d.h", i);
        unlink(fname);
        snprintf(fname, sizeof(fname), "output_both_%d.c", i);
        unlink(fname);
    }
    unlink("combined.h");
    unlink("combined.c");
    unlink("error_output.h");
    unlink("debug_output.h");
    
    /* Remove temporary GT files */
    for (int i = 0; i < 4; i++) {
        unlink(gt_files[i]);
        free(gt_files[i]);
    }
    
    /* Generate gcov report */
    printf("\nGenerating coverage report...\n");
    system("gcov gengtype.cc gengtype-state.cc");
    
    printf("\n=== Test Complete ===\n");
    printf("Check gengtype.cc.gcov for line coverage\n");
    printf("Specifically verify lines 182-213 (type counting switch) are covered\n");
    
    return 0;
}
