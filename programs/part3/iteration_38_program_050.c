/* test_gengtype_coverage.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

/* Compile gengtype with coverage flags */
#define GENGTYPE_SOURCE "gengtype.cc"
#define GENGTYPE_STATE_SOURCE "gengtype-state.cc"
#define GENGTYPE_EXECUTABLE "./gengtype_coverage"

/* Create temporary .gt files with various type definitions */
const char *gt_files[] = {
    /* File 1: Basic types and structs */
    "file1.gt",
    /* File 2: Advanced types with unions and arrays */
    "file2.gt", 
    /* File 3: Language-specific and callback types */
    "file3.gt",
    /* File 4: File with syntax error (for error path testing) */
    "file4.gt",
    /* File 5: Duplicate definitions (for warning path testing) */
    "file5.gt"
};

const char *gt_contents[] = {
    /* file1.gt - Basic types covering most categories */
    "%{\n"
    "/* Test file 1: Basic type definitions */\n"
    "#include \"config.h\"\n"
    "#include \"system.h\"\n"
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
    "  const char *name;  /* TYPE_STRING */\n"
    "  char *buffer;\n"
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
    "  struct my_struct *next;\n"
    "} GTY((user));\n"
    "\n"
    "/* TYPE_POINTER: Pointer typedef */\n"
    "typedef struct my_struct *my_ptr;\n"
    "typedef void *generic_ptr;\n"
    "\n"
    "/* TYPE_ARRAY: Array typedef */\n"
    "typedef int my_array[10];\n"
    "typedef struct my_struct struct_array[5];\n"
    "\n"
    "/* Nested complex type */\n"
    "struct complex_nested {\n"
    "  my_array data;\n"
    "  my_ptr *ptr_array;\n"
    "  struct string_struct str_field;\n"
    "};\n"
    "%}\n",
    
    /* file2.gt - Unions, arrays, and more complex types */
    "%{\n"
    "/* Test file 2: Unions and advanced types */\n"
    "#include \"config.h\"\n"
    "#include \"system.h\"\n"
    "%}\n"
    "\n"
    "/* TYPE_UNION: Union definition */\n"
    "union my_union {\n"
    "  int i;\n"
    "  void *p;\n"
    "  double d;\n"
    "};\n"
    "\n"
    "/* Another scalar */\n"
    "typedef short small_scalar;\n"
    "\n"
    "/* Struct containing union */\n"
    "struct union_container {\n"
    "  union my_union data;\n"
    "  int tag;\n"
    "};\n"
    "\n"
    "/* Array of unions */\n"
    "typedef union my_union union_array[20];\n"
    "\n"
    "/* Pointer to array */\n"
    "typedef int (*array_ptr)[10];\n"
    "\n"
    "/* Multi-dimensional array */\n"
    "typedef int matrix[5][5];\n"
    "\n"
    "/* Complex nested type combining multiple categories */\n"
    "struct deeply_nested {\n"
    "  union {\n"
    "    struct union_container *cont_ptr;  /* TYPE_POINTER to TYPE_STRUCT */\n"
    "    union_array *ua_ptr;               /* TYPE_POINTER to TYPE_ARRAY of TYPE_UNION */\n"
    "  } u;\n"
    "  matrix grid;                         /* TYPE_ARRAY (2D) */\n"
    "  const char *description;             /* TYPE_STRING */\n"
    "};\n"
    "\n"
    "/* Forward declaration for circular reference */\n"
    "struct forward_decl;\n"
    "\n"
    "struct circular_ref {\n"
    "  struct forward_decl *next;\n"
    "};\n"
    "\n"
    "struct forward_decl {\n"
    "  struct circular_ref *prev;\n"
    "  int value;\n"
    "};\n"
    "%}\n",
    
    /* file3.gt - Callback and language-specific types */
    "%{\n"
    "/* Test file 3: Callbacks and language structs */\n"
    "#include \"config.h\"\n"
    "#include \"system.h\"\n"
    "%}\n"
    "\n"
    "/* TYPE_CALLBACK: Callback function pointer */\n"
    "typedef void (*callback_fn)(void);\n"
    "typedef int (*compare_fn)(const void *, const void *);\n"
    "\n"
    "/* Struct with callback member */\n"
    "struct callback_container {\n"
    "  callback_fn handler;\n"
    "  void *user_data;\n"
    "};\n"
    "\n"
    "/* TYPE_LANG_STRUCT: Language-specific struct */\n"
    "struct lang_struct {\n"
    "  int data;\n"
    "  void *lang_specific;\n"
    "} GTY ((lang));\n"
    "\n"
    "/* Another language struct with different options */\n"
    "struct lang_struct2 {\n"
    "  long id;\n"
    "  struct lang_struct *next;\n"
    "} GTY ((lang, desc (\"0\"), chain_next (\"%h.next\")));\n"
    "\n"
    "/* Mixed types */\n"
    "struct mixed_bag {\n"
    "  callback_fn callbacks[5];           /* TYPE_ARRAY of TYPE_CALLBACK */\n"
    "  struct lang_struct *lang_items;     /* TYPE_POINTER to TYPE_LANG_STRUCT */\n"
    "  union my_union variant;             /* TYPE_UNION from file2 */\n"
    "  const char *names[3];               /* TYPE_ARRAY of TYPE_STRING */\n"
    "};\n"
    "\n"
    "/* Template-like pattern */\n"
    "#define DEFINE_GTY_STRUCT(name, field) \\\n"
    "  struct name { \\\n"
    "    field data; \\\n"
    "    struct name *next; \\\n"
    "  }\n"
    "\n"
    "DEFINE_GTY_STRUCT(gty_list_int, int);\n"
    "DEFINE_GTY_STRUCT(gty_list_ptr, void *);\n"
    "%}\n",
    
    /* file4.gt - File with syntax error (missing closing %) */
    "%{\n"
    "/* Test file 4: Syntax error case */\n"
    "#include \"config.h\"\n"
    "#include \"system.h\"\n"
    "\n"
    "struct error_struct {\n"
    "  int missing_semicolon\n"  /* Missing semicolon */
    "};\n"
    "\n"
    "/* Missing closing %}\n",
    
    /* file5.gt - Duplicate definitions for warning testing */
    "%{\n"
    "/* Test file 5: Duplicate definitions */\n"
    "#include \"config.h\"\n"
    "#include \"system.h\"\n"
    "%}\n"
    "\n"
    "/* Duplicate scalar */\n"
    "typedef int my_scalar;  /* Duplicate from file1 */\n"
    "\n"
    "/* Duplicate struct */\n"
    "struct my_struct {      /* Duplicate from file1 */\n"
    "  int a;\n"
    "  float b;\n"
    "};\n"
    "\n"
    "/* New unique type */\n"
    "struct unique_type {\n"
    "  double special;\n"
    "};\n"
    "%}\n"
};

/* Create a file listing all .gt files for batch processing */
const char *filelist_content = 
    "file1.gt\n"
    "file2.gt\n"
    "file3.gt\n"
    "file5.gt\n";  /* Skip file4.gt (syntax error) for batch processing */

/* Function to create temporary files */
int create_temp_file(const char *filename, const char *content) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        perror("fopen");
        return 0;
    }
    fputs(content, f);
    fclose(f);
    return 1;
}

/* Function to compile gengtype with coverage */
int compile_gengtype_with_coverage() {
    printf("Compiling gengtype with coverage instrumentation...\n");
    
    /* Compile gengtype.cc */
    char cmd[1024];
    snprintf(cmd, sizeof(cmd),
        "g++ -O0 -fprofile-arcs -ftest-coverage "
        "-DIN_GCC -DHAVE_CONFIG_H "
        "-I. -I../../include -I../../gcc -I../.. "
        "-c " GENGTYPE_SOURCE " -o gengtype.o");
    
    printf("Running: %s\n", cmd);
    if (system(cmd) != 0) {
        fprintf(stderr, "Failed to compile gengtype.cc\n");
        return 0;
    }
    
    /* Compile gengtype-state.cc if it exists */
    snprintf(cmd, sizeof(cmd),
        "g++ -O0 -fprofile-arcs -ftest-coverage "
        "-DIN_GCC -DHAVE_CONFIG_H "
        "-I. -I../../include -I../../gcc -I../.. "
        "-c " GENGTYPE_STATE_SOURCE " -o gengtype-state.o 2>/dev/null || true");
    
    printf("Running: %s\n", cmd);
    system(cmd);  /* Optional, continue even if it fails */
    
    /* Link gengtype executable */
    snprintf(cmd, sizeof(cmd),
        "g++ -O0 -fprofile-arcs -ftest-coverage "
        "gengtype.o gengtype-state.o -o " GENGTYPE_EXECUTABLE " "
        "-liberty -lgcov 2>&1");
    
    printf("Running: %s\n", cmd);
    if (system(cmd) != 0) {
        /* Try linking without gengtype-state.o */
        snprintf(cmd, sizeof(cmd),
            "g++ -O0 -fprofile-arcs -ftest-coverage "
            "gengtype.o -o " GENGTYPE_EXECUTABLE " "
            "-liberty -lgcov");
        
        printf("Retrying without gengtype-state.o: %s\n", cmd);
        if (system(cmd) != 0) {
            fprintf(stderr, "Failed to link gengtype executable\n");
            return 0;
        }
    }
    
    printf("Successfully compiled " GENGTYPE_EXECUTABLE "\n");
    return 1;
}

/* Function to run gengtype with various options */
void run_gengtype_test(const char *test_name, const char *args) {
    printf("\n=== Running test: %s ===\n", test_name);
    printf("Command: " GENGTYPE_EXECUTABLE " %s\n", args);
    
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), GENGTYPE_EXECUTABLE " %s", args);
    
    int status = system(cmd);
    if (status != 0) {
        printf("Test exited with status %d (this may be expected for error tests)\n", 
               WEXITSTATUS(status));
    }
}

/* Main test driver */
int main(int argc, char *argv[]) {
    printf("=== GCC gengtype Coverage Test ===\n");
    printf("Target: Cover switch statement for type counting (lines 182-213 in gengtype.cc)\n\n");
    
    /* Step 1: Create all .gt files */
    printf("Creating test .gt files...\n");
    for (int i = 0; i < sizeof(gt_files)/sizeof(gt_files[0]); i++) {
        if (!create_temp_file(gt_files[i], gt_contents[i])) {
            fprintf(stderr, "Failed to create %s\n", gt_files[i]);
            return 1;
        }
        printf("  Created %s\n", gt_files[i]);
    }
    
    /* Create file list for batch processing */
    if (!create_temp_file("gt_filelist.txt", filelist_content)) {
        fprintf(stderr, "Failed to create file list\n");
        return 1;
    }
    printf("  Created gt_filelist.txt\n");
    
    /* Step 2: Compile gengtype with coverage */
    if (!compile_gengtype_with_coverage()) {
        fprintf(stderr, "Failed to compile gengtype\n");
        return 1;
    }
    
    /* Step 3: Run various gengtype tests to exercise the switch statement */
    
    /* Test 1: Process individual files */
    run_gengtype_test("Individual file processing (file1)", "file1.gt");
    run_gengtype_test("Individual file processing (file2)", "file2.gt");
    run_gengtype_test("Individual file processing (file3)", "file3.gt");
    
    /* Test 2: Process multiple files at once */
    run_gengtype_test("Multiple file processing", "file1.gt file2.gt file3.gt");
    
    /* Test 3: Batch processing with -p flag */
    run_gengtype_test("Batch processing with -p", "-p gt_filelist.txt");
    
    /* Test 4: Generate header file (forces full parsing) */
    run_gengtype_test("Generate header file", "-g gtype-test.h file1.gt file2.gt file3.gt");
    
    /* Test 5: Generate routine file */
    run_gengtype_test("Generate routine file", "-r gtype-test.c file1.gt file2.gt file3.gt");
    
    /* Test 6: Process with debug output */
    run_gengtype_test("With debug output", "-d file1.gt");
    
    /* Test 7: Error case - file with syntax error */
    run_gengtype_test("Error case (syntax error)", "file4.gt");
    
    /* Test 8: Warning case - duplicate definitions */
    run_gengtype_test("Warning case (duplicates)", "file5.gt");
    
    /* Test 9: Combined operation with both -g and -r */
    run_gengtype_test("Generate both header and routine", 
                     "-g gtype-combined.h -r gtype-combined.c file1.gt file2.gt file3.gt");
    
    /* Test 10: Process all files including duplicates */
    run_gengtype_test("All files together", 
                     "file1.gt file2.gt file3.gt file5.gt");
    
    /* Step 4: Verify coverage by checking if .gcda files were created */
    printf("\n=== Checking coverage files ===\n");
    if (system("ls -la *.gcda 2>/dev/null || echo 'No .gcda files found'") != 0) {
        printf("Warning: No coverage data files found\n");
    }
    
    /* Step 5: Generate coverage report */
    printf("\n=== Generating coverage report ===\n");
    system("gcov " GENGTYPE_SOURCE " 2>/dev/null | grep -A 20 'Lines executed:' || "
           "echo 'Could not generate gcov report'");
    
    /* Step 6: Specifically check the target switch statement */
    printf("\n=== Checking target lines (182-213) ===\n");
    system("gcov -b " GENGTYPE_SOURCE " 2>/dev/null | "
           "sed -n '/^ *182:/,/^ *214:/p' || "
           "echo 'Could not extract line coverage for target section'");
    
    /* Step 7: Cleanup */
    printf("\n=== Cleaning up ===\n");
    for (int i = 0; i < sizeof(gt_files)/sizeof(gt_files[0]); i++) {
        remove(gt_files[i]);
        printf("  Removed %s\n", gt_files[i]);
    }
    remove("gt_filelist.txt");
    remove("gtype-test.h");
    remove("gtype-test.c");
    remove("gtype-combined.h");
    remove("gtype-combined.c");
    remove("gengtype.o");
    remove("gengtype-state.o");
    remove(GENGTYPE_EXECUTABLE);
    
    printf("\n=== Test completed ===\n");
    printf("Coverage data should be in gengtype.gcda and gengtype.gcno\n");
    printf("Use 'gcov gengtype.cc' to see detailed line coverage\n");
    
    return 0;
}
