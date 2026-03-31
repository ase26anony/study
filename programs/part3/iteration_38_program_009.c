/* test_gengtype_coverage.c - Driver program to test gengtype type counting */
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
    "/* TYPE_STRING: String type */\n"
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
    "/* TYPE_ARRAY: Array types */\n"
    "typedef int my_array[10];\n"
    "typedef struct my_struct struct_array[5];\n"
    "\n"
    "/* TYPE_CALLBACK: Callback function pointer */\n"
    "typedef void (*callback_fn)(void);\n"
    "typedef int (*callback_with_args)(int, char*);\n"
    "\n"
    "/* Nested complex type */\n"
    "struct complex_type {\n"
    "  my_array arr;          /* TYPE_ARRAY */\n"
    "  callback_fn cb;        /* TYPE_CALLBACK */\n"
    "  struct my_struct *next; /* TYPE_POINTER */\n"
    "};\n",

    /* File 2: Advanced types with GTY attributes */
    "%{\n"
    "/* Test file 2: Advanced types with GTY */\n"
    "#include \"config.h\"\n"
    "#include \"system.h\"\n"
    "%}\n"
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
    "/* TYPE_LANG_STRUCT: Language-specific struct */\n"
    "struct lang_struct {\n"
    "  int data;\n"
    "  void *extra;\n"
    "} GTY((lang));\n"
    "\n"
    "/* More TYPE_POINTER variations */\n"
    "typedef union my_union *union_ptr;\n"
    "typedef void (*void_func_ptr)(void);\n"
    "\n"
    "/* Complex nested structure */\n"
    "struct container {\n"
    "  union my_union u;              /* TYPE_UNION */\n"
    "  struct lang_struct lang;       /* TYPE_LANG_STRUCT */\n"
    "  struct user_struct *users;     /* TYPE_POINTER to TYPE_USER_STRUCT */\n"
    "  int (*compare)(int, int);      /* TYPE_CALLBACK */\n"
    "};\n"
    "\n"
    "/* Array of pointers to unions */\n"
    "union my_union *union_array[8];\n"
    "\n"
    "/* Forward declaration for undefined */\n"
    "struct another_undefined;\n",

    /* File 3: Mixed types with errors and warnings */
    "%{\n"
    "/* Test file 3: Mixed types with potential issues */\n"
    "#include \"config.h\"\n"
    "#include \"system.h\"\n"
    "%}\n"
    "\n"
    "/* Duplicate definition to trigger warning */\n"
    "struct my_struct {\n"
    "  int a;\n"
    "  float b;\n"
    "};\n"
    "\n"
    "/* More TYPE_STRING variations */\n"
    "struct string_container {\n"
    "  const char *static_str;\n"
    "  char *dynamic_str;\n"
    "  const char *const constant_str;\n"
    "};\n"
    "\n"
    "/* TYPE_ARRAY of arrays */\n"
    "typedef int matrix[10][10];\n"
    "typedef char string_array[20][50];\n"
    "\n"
    "/* Pointer to array */\n"
    "typedef int (*array_ptr)[10];\n"
    "\n"
    "/* Complex callback chain */\n"
    "typedef void (*init_func)(void);\n"
    "typedef void (*cleanup_func)(void*);\n"
    "struct callback_chain {\n"
    "  init_func init;\n"
    "  cleanup_func cleanup;\n"
    "  void *user_data;\n"
    "};\n"
    "\n"
    "/* Mixed struct with all types */\n"
    "struct mega_struct {\n"
    "  int scalar_field;              /* TYPE_SCALAR */\n"
    "  const char *string_field;      /* TYPE_STRING */\n"
    "  struct my_struct nested;       /* TYPE_STRUCT */\n"
    "  union my_union variant;        /* TYPE_UNION */\n"
    "  void *pointer_field;           /* TYPE_POINTER */\n"
    "  int array_field[5];            /* TYPE_ARRAY */\n"
    "  void (*callback_field)(void);  /* TYPE_CALLBACK */\n"
    "};\n"
    "\n"
    "/* File 4: Deliberate syntax error (missing %) - will trigger error path */\n",

    /* File 4: Syntax error case */
    "%{\n"
    "/* Test file 4: Syntax error - missing closing %}\n"
    "#include \"config.h\"\n"
    "#include \"system.h\"\n"
    "/* Missing closing %}\n"
    "\n"
    "struct error_struct {\n"
    "  int x;\n"
    "};\n"
};

static const char *gt_filenames[] = {
    "test_types1.gt",
    "test_types2.gt",
    "test_types3.gt",
    "test_error.gt"
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

/* Compile gengtype with coverage instrumentation */
static int compile_gengtype(void) {
    printf("Compiling gengtype with coverage instrumentation...\n");
    
    /* Compile command for gengtype */
    const char *compile_cmd = 
        "g++ -O0 -fprofile-arcs -ftest-coverage "
        "-DIN_GCC -DHAVE_CONFIG_H "
        "-I. -I../../include -I../../gcc "
        "-c gengtype.cc -o gengtype.o 2>&1";
    
    const char *link_cmd = 
        "g++ -O0 -fprofile-arcs -ftest-coverage "
        "gengtype.o -lgcov -liberty -o gengtype_coverage 2>&1";
    
    if (system(compile_cmd) != 0) {
        fprintf(stderr, "Failed to compile gengtype.cc\n");
        return 0;
    }
    
    if (system(link_cmd) != 0) {
        fprintf(stderr, "Failed to link gengtype\n");
        return 0;
    }
    
    printf("gengtype compiled successfully as 'gengtype_coverage'\n");
    return 1;
}

/* Run gengtype on a single file */
static int run_gengtype_single(const char *gt_file, const char *output_base) {
    char cmd[1024];
    int status;
    
    /* Pattern C: Generate header output to force full parsing */
    snprintf(cmd, sizeof(cmd), 
             "./gengtype_coverage -g %s.h %s 2>&1", 
             output_base, gt_file);
    
    printf("Running: %s\n", cmd);
    status = system(cmd);
    
    if (WIFEXITED(status)) {
        printf("gengtype exited with status %d for %s\n", 
               WEXITSTATUS(status), gt_file);
    }
    
    return 1;
}

/* Pattern B: Batch processing with file list */
static int run_gengtype_batch(const char **files, int count) {
    FILE *list = fopen("gt_filelist.txt", "w");
    char cmd[1024];
    int i;
    
    if (!list) {
        perror("fopen filelist");
        return 0;
    }
    
    for (i = 0; i < count; i++) {
        fprintf(list, "%s\n", files[i]);
    }
    fclose(list);
    
    /* Run gengtype with -p flag for batch processing */
    snprintf(cmd, sizeof(cmd), 
             "./gengtype_coverage -p gt_filelist.txt 2>&1");
    
    printf("Running batch: %s\n", cmd);
    system(cmd);
    
    return 1;
}

/* Pattern A: Multiple file processing with different flags */
static int run_gengtype_multiple_patterns(const char **files, int count) {
    int i;
    char output_name[256];
    
    for (i = 0; i < count; i++) {
        /* Different patterns for different files */
        if (i == 0) {
            /* Pattern C: Generate header */
            snprintf(output_name, sizeof(output_name), "output%d", i);
            run_gengtype_single(files[i], output_name);
        } else if (i == 1) {
            /* Pattern C variant: Generate routine file */
            char cmd[1024];
            snprintf(cmd, sizeof(cmd), 
                     "./gengtype_coverage -r output_routines.c %s 2>&1", 
                     files[i]);
            printf("Running: %s\n", cmd);
            system(cmd);
        } else if (i == 2) {
            /* Pattern C variant: Generate both header and routine */
            char cmd[1024];
            snprintf(cmd, sizeof(cmd), 
                     "./gengtype_coverage -g output_combined.h -r output_combined.c %s 2>&1", 
                     files[i]);
            printf("Running: %s\n", cmd);
            system(cmd);
        }
        /* File 3 (index 3) has syntax error - will be processed in batch */
    }
    
    return 1;
}

/* Clean up temporary files */
static void cleanup_files(const char **files, int count) {
    int i;
    for (i = 0; i < count; i++) {
        remove(files[i]);
    }
    remove("gt_filelist.txt");
    remove("output0.h");
    remove("output_routines.c");
    remove("output_combined.h");
    remove("output_combined.c");
    remove("gengtype_coverage");
    remove("gengtype.o");
}

/* Main test driver */
int main(int argc, char **argv) {
    int i;
    int num_files = sizeof(gt_files) / sizeof(gt_files[0]);
    
    printf("=== Gengtype Coverage Test Driver ===\n");
    printf("Creating %d test .gt files...\n", num_files);
    
    /* Create all .gt files */
    for (i = 0; i < num_files; i++) {
        if (!create_temp_file(gt_filenames[i], gt_files[i])) {
            fprintf(stderr, "Failed to create %s\n", gt_filenames[i]);
            return 1;
        }
        printf("Created %s\n", gt_filenames[i]);
    }
    
#if COMPILE_GENGTYPE
    /* Compile gengtype with coverage */
    if (!compile_gengtype()) {
        cleanup_files(gt_filenames, num_files);
        return 1;
    }
#endif
    
    printf("\n=== Running gengtype with various patterns ===\n");
    
    /* Pattern A: Process files individually with different flags */
    printf("\n--- Pattern A: Individual file processing ---\n");
    run_gengtype_multiple_patterns(gt_filenames, num_files - 1);
    
    /* Pattern B: Batch processing (includes error file) */
    printf("\n--- Pattern B: Batch processing with -p flag ---\n");
    run_gengtype_batch(gt_filenames, num_files);
    
    /* Pattern D: Error case handling */
    printf("\n--- Pattern D: Error case (syntax error) ---\n");
    /* The error file is already included in batch processing */
    /* Also run it individually to see error output */
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), 
             "./gengtype_coverage -g error_output.h %s 2>&1", 
             gt_filenames[3]);
    printf("Running error case: %s\n", cmd);
    system(cmd);
    
    /* Generate coverage data by processing all files again 
       with aggressive flags to ensure deep processing */
    printf("\n=== Aggressive type processing for coverage ===\n");
    for (i = 0; i < num_files - 1; i++) {  /* Skip error file */
        char aggressive_cmd[1024];
        /* Use -v for verbose output to ensure all code paths */
        snprintf(aggressive_cmd, sizeof(aggressive_cmd),
                 "./gengtype_coverage -v -g coverage%d.h -r coverage%d.c %s 2>&1",
                 i, i, gt_filenames[i]);
        printf("Aggressive run %d: %s\n", i, aggressive_cmd);
        system(aggressive_cmd);
        
        /* Clean up generated files */
        char cleanup_cmd[256];
        snprintf(cleanup_cmd, sizeof(cleanup_cmd), "rm -f coverage%d.h coverage%d.c", i, i);
        system(cleanup_cmd);
    }
    
    /* Verify coverage by checking if .gcda files exist */
    printf("\n=== Checking coverage data ===\n");
    if (system("ls -la *.gcda 2>/dev/null") != 0) {
        printf("No .gcda files found. Coverage may not be enabled.\n");
    } else {
        printf("Coverage data files (.gcda) generated.\n");
        /* Generate coverage report */
        system("gcov gengtype.cc 2>&1 | tail -20");
    }
    
    /* Cleanup */
    printf("\n=== Cleaning up temporary files ===\n");
    cleanup_files(gt_filenames, num_files);
    
    printf("\n=== Test completed ===\n");
    printf("The switch statement in gengtype.cc should have been executed\n");
    printf("for all type kinds through the various .gt input files.\n");
    
    return 0;
}
