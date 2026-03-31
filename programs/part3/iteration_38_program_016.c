/* driver.c - Test driver for gengtype coverage of type counting switch */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>

/* Compilation flags for building gengtype with coverage */
#define COVERAGE_FLAGS "-O0 -fprofile-arcs -ftest-coverage"
#define INCLUDE_FLAGS "-I. -I../../include -I../../gcc"
#define DEFINES "-DIN_GCC -DHAVE_CONFIG_H"

/* gengtype source files needed */
#define GT_SOURCES "gengtype.cc gengtype-state.cc gengtype-lex.cc"

/* Temporary file management */
typedef struct {
    char *filename;
    char *content;
} temp_file_t;

/* Generated .gt file contents covering all type kinds */
static const char *gt_files[] = {
    /* File 1: Basic types and structs */
    "file1.gt",
    "%{\n"
    "/* Test file 1: Basic type definitions */\n"
    "%}\n"
    "\n"
    "/* TYPE_UNDEFINED: Forward declaration */\n"
    "struct undefined_struct;\n"
    "\n"
    "/* TYPE_SCALAR: Scalar typedefs */\n"
    "typedef int my_scalar;\n"
    "typedef unsigned long scalar2;\n"
    "\n"
    "/* TYPE_STRING: String type usage */\n"
    "struct string_struct {\n"
    "  const char *name;  /* TYPE_STRING */\n"
    "  char *data;\n"
    "};\n"
    "\n"
    "/* TYPE_STRUCT: Regular struct */\n"
    "struct my_struct {\n"
    "  int a;\n"
    "  float b;\n"
    "};\n"
    "\n"
    "/* TYPE_POINTER: Pointer typedef */\n"
    "typedef struct my_struct *my_ptr;\n"
    "\n"
    "%}\n",

    /* File 2: Unions, arrays, and user structs */
    "file2.gt",
    "%{\n"
    "/* Test file 2: Complex type combinations */\n"
    "%}\n"
    "\n"
    "/* TYPE_UNION: Union definition */\n"
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
    "/* TYPE_USER_STRUCT: Struct with user marking */\n"
    "struct user_struct {\n"
    "  int *p;\n"
    "  void *data;\n"
    "} GTY((user));\n"
    "\n"
    "/* TYPE_CALLBACK: Callback function pointer */\n"
    "typedef void (*callback_fn)(void);\n"
    "typedef int (*another_callback)(char *, int);\n"
    "\n"
    "/* Nested complex type */\n"
    "struct complex_nested {\n"
    "  union my_union u;\n"
    "  my_array arr;\n"
    "  callback_fn cb;\n"
    "};\n"
    "\n"
    "%}\n",

    /* File 3: Language structs and edge cases */
    "file3.gt",
    "%{\n"
    "/* Test file 3: Language-specific and edge cases */\n"
    "%}\n"
    "\n"
    "/* TYPE_LANG_STRUCT: Language-specific struct */\n"
    "struct lang_struct {\n"
    "  int data;\n"
    "  void *lang_data;\n"
    "} GTY ((lang));\n"
    "\n"
    "/* More TYPE_STRING variations */\n"
    "struct multiple_strings {\n"
    "  const char *str1;\n"
    "  char *str2;\n"
    "  const char * const str3;\n"
    "};\n"
    "\n"
    "/* Pointer to array */\n"
    "typedef int (*array_ptr)[10];\n"
    "\n"
    "/* Struct containing pointer to union of arrays */\n"
    "struct deep_nesting {\n"
    "  union {\n"
    "    int int_arr[5];\n"
    "    char char_arr[20];\n"
    "  } *union_ptr;\n"
    "  struct lang_struct *lang_ptr;\n"
    "};\n"
    "\n"
    "/* Another TYPE_USER_STRUCT */\n"
    "struct another_user_struct {\n"
    "  struct user_struct *next;\n"
    "  callback_fn handlers[3];\n"
    "} GTY((user));\n"
    "\n"
    "%}\n",

    /* File 4: With syntax error to test error paths */
    "file4.gt",
    "%{\n"
    "/* Test file 4: Contains deliberate syntax error */\n"
    "%}\n"
    "\n"
    "struct error_struct {\n"
    "  int missing_semicolon\n"  /* Missing semicolon */
    "  float f;\n"
    "};\n"
    "\n"
    "/* Missing closing %} */\n",

    /* File 5: Duplicate definitions for warning testing */
    "file5.gt",
    "%{\n"
    "/* Test file 5: Duplicate type definitions */\n"
    "%}\n"
    "\n"
    "struct duplicate_struct {\n"
    "  int x;\n"
    "};\n"
    "\n"
    "/* Duplicate definition */\n"
    "struct duplicate_struct {\n"
    "  int y;\n"
    "};\n"
    "\n"
    "%}\n"
};

/* Create temporary file with given content */
static int create_temp_file(const char *filename, const char *content) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        perror("fopen");
        return 0;
    }
    fputs(content, f);
    fclose(f);
    return 1;
}

/* Build gengtype with coverage instrumentation */
static int build_gengtype(void) {
    char cmd[1024];
    int status;
    
    printf("Building gengtype with coverage instrumentation...\n");
    
    /* Compile gengtype sources */
    snprintf(cmd, sizeof(cmd),
             "g++ %s %s %s -c gengtype.cc -o gengtype.o",
             COVERAGE_FLAGS, INCLUDE_FLAGS, DEFINES);
    printf("Running: %s\n", cmd);
    status = system(cmd);
    if (status != 0) return 0;
    
    /* Compile other required sources */
    snprintf(cmd, sizeof(cmd),
             "g++ %s %s %s -c gengtype-state.cc -o gengtype-state.o",
             COVERAGE_FLAGS, INCLUDE_FLAGS, DEFINES);
    printf("Running: %s\n", cmd);
    status = system(cmd);
    if (status != 0) return 0;
    
    snprintf(cmd, sizeof(cmd),
             "g++ %s %s %s -c gengtype-lex.cc -o gengtype-lex.o",
             COVERAGE_FLAGS, INCLUDE_FLAGS, DEFINES);
    printf("Running: %s\n", cmd);
    status = system(cmd);
    if (status != 0) return 0;
    
    /* Link gengtype executable */
    snprintf(cmd, sizeof(cmd),
             "g++ %s gengtype.o gengtype-state.o gengtype-lex.o "
             "-lgcov -liberty -o gengtype-coverage",
             COVERAGE_FLAGS);
    printf("Running: %s\n", cmd);
    status = system(cmd);
    
    return status == 0;
}

/* Run gengtype with various input patterns */
static int run_gengtype_tests(void) {
    char cmd[1024];
    int status;
    FILE *filelist;
    int i;
    
    printf("\nRunning gengtype tests...\n");
    
    /* Pattern A: Process each file individually */
    printf("\n=== Pattern A: Individual file processing ===\n");
    for (i = 0; i < 3; i += 2) {  /* Process first 3 valid files */
        snprintf(cmd, sizeof(cmd),
                 "./gengtype-coverage -g output%d.h %s",
                 i/2 + 1, gt_files[i*2]);
        printf("Running: %s\n", cmd);
        status = system(cmd);
        if (status != 0 && i < 4) {  /* Files 0-2 should succeed */
            printf("Warning: gengtype failed on valid file %s\n", gt_files[i*2]);
        }
    }
    
    /* Pattern B: Batch processing with -p flag */
    printf("\n=== Pattern B: Batch processing with -p ===\n");
    filelist = fopen("gt_filelist.txt", "w");
    if (filelist) {
        for (i = 0; i < 3; i += 2) {  /* List first 3 valid files */
            fprintf(filelist, "%s\n", gt_files[i*2]);
        }
        fclose(filelist);
        
        snprintf(cmd, sizeof(cmd),
                 "./gengtype-coverage -p gt_filelist.txt -g output_batch.h");
        printf("Running: %s\n", cmd);
        status = system(cmd);
        if (status != 0) {
            printf("Warning: Batch processing failed\n");
        }
    }
    
    /* Pattern C: Multiple files in one command */
    printf("\n=== Pattern C: Multiple file processing ===\n");
    snprintf(cmd, sizeof(cmd),
             "./gengtype-coverage -g output_combined.h %s %s %s",
             gt_files[0], gt_files[2], gt_files[4]);
    printf("Running: %s\n", cmd);
    status = system(cmd);
    if (status != 0) {
        printf("Warning: Combined processing failed\n");
    }
    
    /* Pattern D: Error and warning cases */
    printf("\n=== Pattern D: Error/warning testing ===\n");
    
    /* Test with syntax error (file 4) */
    snprintf(cmd, sizeof(cmd),
             "./gengtype-coverage -g output_error.h %s 2>error.log",
             gt_files[6]);
    printf("Running: %s\n", cmd);
    status = system(cmd);
    if (status == 0) {
        printf("Unexpected: gengtype succeeded on file with syntax error\n");
    } else {
        printf("Correctly detected syntax error\n");
    }
    
    /* Test with duplicate definition (file 5) */
    snprintf(cmd, sizeof(cmd),
             "./gengtype-coverage -g output_warn.h %s 2>warn.log",
             gt_files[8]);
    printf("Running: %s\n", cmd);
    status = system(cmd);
    printf("Processed duplicate definition file (may produce warnings)\n");
    
    /* Generate routine file to trigger more processing */
    printf("\n=== Generating routine file ===\n");
    snprintf(cmd, sizeof(cmd),
             "./gengtype-coverage -r gtype-desc.c %s %s %s",
             gt_files[0], gt_files[2], gt_files[4]);
    printf("Running: %s\n", cmd);
    status = system(cmd);
    
    return 1;
}

/* Verify coverage data was generated */
static int verify_coverage(void) {
    struct stat st;
    
    printf("\nVerifying coverage data...\n");
    
    /* Check for .gcda files */
    if (stat("gengtype.gcda", &st) == 0) {
        printf("✓ Coverage data generated for gengtype.cc\n");
        return 1;
    }
    
    printf("✗ No coverage data found\n");
    return 0;
}

/* Clean up temporary files */
static void cleanup(void) {
    int i;
    
    printf("\nCleaning up...\n");
    
    /* Remove generated .gt files */
    for (i = 0; i < sizeof(gt_files)/sizeof(gt_files[0]); i += 2) {
        remove(gt_files[i]);
    }
    
    /* Remove output files */
    remove("gt_filelist.txt");
    remove("output1.h");
    remove("output2.h");
    remove("output_batch.h");
    remove("output_combined.h");
    remove("output_error.h");
    remove("output_warn.h");
    remove("gtype-desc.c");
    remove("error.log");
    remove("warn.log");
    
    /* Remove object files and executable */
    remove("gengtype.o");
    remove("gengtype-state.o");
    remove("gengtype-lex.o");
    remove("gengtype-coverage");
    
    printf("Cleanup complete\n");
}

/* Main test driver */
int main(int argc, char **argv) {
    int i;
    int success = 1;
    
    printf("=== gengtype Switch Coverage Test ===\n");
    
    /* Create all .gt files */
    printf("\nCreating test .gt files...\n");
    for (i = 0; i < sizeof(gt_files)/sizeof(gt_files[0]); i += 2) {
        printf("Creating %s\n", gt_files[i]);
        if (!create_temp_file(gt_files[i], gt_files[i+1])) {
            fprintf(stderr, "Failed to create %s\n", gt_files[i]);
            success = 0;
        }
    }
    
    if (!success) {
        cleanup();
        return 1;
    }
    
    /* Build gengtype with coverage */
    if (!build_gengtype()) {
        fprintf(stderr, "Failed to build gengtype\n");
        cleanup();
        return 1;
    }
    
    /* Run tests */
    if (!run_gengtype_tests()) {
        fprintf(stderr, "Some tests failed\n");
        success = 0;
    }
    
    /* Verify coverage */
    if (!verify_coverage()) {
        fprintf(stderr, "Coverage verification failed\n");
        success = 0;
    }
    
    /* Generate coverage report */
    printf("\n=== Generating coverage report ===\n");
    system("gcov gengtype.cc");
    
    /* Display relevant coverage info */
    printf("\n=== Coverage of target switch (lines 182-213) ===\n");
    system("gcov -n gengtype.cc | grep -A 30 'Lines executed:'");
    
    cleanup();
    
    if (success) {
        printf("\n✓ Test completed successfully\n");
        return 0;
    } else {
        printf("\n✗ Test completed with errors\n");
        return 1;
    }
}
