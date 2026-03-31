/* test_gengtype_coverage.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

/* Compile gengtype with coverage flags */
#define GENGTYPE_SOURCE "gengtype.cc"
#define GENGTYPE_STATE_SOURCE "gengtype-state.cc"
#define GENGTYPE_EXECUTABLE "gengtype_coverage"

/* Create temporary .gt files with various type definitions */
const char *gt_files[] = {
    /* File 1: Basic types and structs */
    "test_types1.gt",
    /* File 2: Complex nested types and unions */
    "test_types2.gt", 
    /* File 3: Language structs, callbacks, and edge cases */
    "test_types3.gt",
    /* File 4: File with syntax error (for error path testing) */
    "test_error.gt",
    /* File 5: Duplicate definitions (for warning path testing) */
    "test_duplicate.gt"
};

/* Content for each .gt file */
const char *gt_contents[] = {
    /* test_types1.gt - Basic type definitions */
    "%{\n"
    "/* Test file 1: Basic types */\n"
    "#include \"config.h\"\n"
    "#include \"system.h\"\n"
    "%}\n"
    "\n"
    "/* TYPE_UNDEFINED: Forward declaration */\n"
    "struct undefined_struct;\n"
    "\n"
    "/* TYPE_SCALAR: Scalar typedef */\n"
    "typedef int my_scalar;\n"
    "typedef unsigned long another_scalar;\n"
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
    "typedef my_scalar *scalar_ptr;\n"
    "\n"
    "/* TYPE_ARRAY: Array types */\n"
    "typedef int my_array[10];\n"
    "typedef struct my_struct struct_array[5];\n"
    "%}\n",

    /* test_types2.gt - Complex nested types */
    "%{\n"
    "/* Test file 2: Complex nested types */\n"
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
    "/* Nested struct with union */\n"
    "struct container {\n"
    "  union my_union data;\n"
    "  struct container *next;\n"
    "};\n"
    "\n"
    "/* Complex pointer to array of structs containing unions */\n"
    "struct complex_type {\n"
    "  union my_union items[10];  /* TYPE_ARRAY inside union */\n"
    "  struct container *containers;  /* TYPE_POINTER */\n"
    "};\n"
    "\n"
    "/* Another user struct with complex nesting */\n"
    "struct nested_user_struct {\n"
    "  struct complex_type *complex;  /* TYPE_POINTER */\n"
    "  union my_union current;        /* TYPE_UNION */\n"
    "} GTY((user));\n"
    "\n"
    "/* Array of pointers */\n"
    "typedef struct container *ptr_array[20];\n"
    "%}\n",

    /* test_types3.gt - Language structs and callbacks */
    "%{\n"
    "/* Test file 3: Language structs and callbacks */\n"
    "#include \"config.h\"\n"
    "#include \"system.h\"\n"
    "%}\n"
    "\n"
    "/* TYPE_CALLBACK: Callback function type */\n"
    "typedef void (*callback_fn)(void);\n"
    "typedef int (*compare_fn)(const void *, const void *);\n"
    "\n"
    "/* Struct with callback member */\n"
    "struct callback_struct {\n"
    "  callback_fn handler;\n"
    "  compare_fn comparator;\n"
    "  void *user_data;\n"
    "};\n"
    "\n"
    "/* TYPE_LANG_STRUCT: Language-specific struct */\n"
    "struct lang_struct {\n"
    "  int data;\n"
    "  void *lang_specific;\n"
    "} GTY ((lang));\n"
    "\n"
    "/* Another lang struct with callback */\n"
    "struct lang_callback_struct {\n"
    "  callback_fn lang_handler;\n"
    "  struct lang_struct *lang_data;\n"
    "} GTY ((lang));\n"
    "\n"
    "/* Mixed types for comprehensive coverage */\n"
    "struct comprehensive {\n"
    "  /* TYPE_SCALAR */\n"
    "  int count;\n"
    "  \n"
    "  /* TYPE_STRING */\n"
    "  const char *description;\n"
    "  \n"
    "  /* TYPE_POINTER */\n"
    "  struct lang_struct *lang_ptr;\n"
    "  \n"
    "  /* TYPE_ARRAY */\n"
    "  callback_fn handlers[5];\n"
    "  \n"
    "  /* TYPE_UNION */\n"
    "  union {\n"
    "    int int_val;\n"
    "    callback_fn cb_val;\n"
    "  } choice;\n"
    "};\n"
    "%}\n",

    /* test_error.gt - File with syntax error */
    "%{\n"
    "/* Test file with syntax error - missing closing %}\n"
    "#include \"config.h\"\n"
    "#include \"system.h\"\n"
    "\n"
    "struct error_struct {\n"
    "  int missing_semicolon\n"
    "  /* Missing closing %} to trigger error path */\n",

    /* test_duplicate.gt - Duplicate definitions for warnings */
    "%{\n"
    "/* Test file with duplicate definitions */\n"
    "#include \"config.h\"\n"
    "#include \"system.h\"\n"
    "%}\n"
    "\n"
    "/* First definition */\n"
    "struct duplicate_struct {\n"
    "  int a;\n"
    "};\n"
    "\n"
    "/* Duplicate definition - should trigger warning */\n"
    "struct duplicate_struct {\n"
    "  int b;\n"
    "};\n"
    "\n"
    "typedef int my_int;\n"
    "typedef int my_int;  /* Duplicate typedef */\n"
    "%}\n"
};

/* File listing all .gt files for batch processing */
const char *filelist_content = 
    "test_types1.gt\n"
    "test_types2.gt\n"
    "test_types3.gt\n"
    "test_duplicate.gt\n";

/* Create a temporary file with given content */
char *create_temp_file(const char *content, const char *template) {
    char *filename = strdup(template);
    int fd = mkstemp(filename);
    if (fd == -1) {
        free(filename);
        return NULL;
    }
    
    write(fd, content, strlen(content));
    close(fd);
    return filename;
}

/* Compile gengtype with coverage instrumentation */
int compile_gengtype_with_coverage() {
    printf("Compiling gengtype with coverage instrumentation...\n");
    
    /* Compilation command for gengtype */
    const char *compile_cmd = 
        "g++ -O0 -fprofile-arcs -ftest-coverage "
        "-DIN_GCC -DHAVE_CONFIG_H "
        "-I. -I../../include -I../../gcc "
        "-c " GENGTYPE_SOURCE " -o gengtype.o 2>&1";
    
    printf("Running: %s\n", compile_cmd);
    if (system(compile_cmd) != 0) {
        fprintf(stderr, "Failed to compile gengtype.cc\n");
        return 0;
    }
    
    /* Compile gengtype-state.cc */
    const char *compile_state_cmd = 
        "g++ -O0 -fprofile-arcs -ftest-coverage "
        "-DIN_GCC -DHAVE_CONFIG_H "
        "-I. -I../../include -I../../gcc "
        "-c " GENGTYPE_STATE_SOURCE " -o gengtype-state.o 2>&1";
    
    printf("Running: %s\n", compile_state_cmd);
    if (system(compile_state_cmd) != 0) {
        fprintf(stderr, "Failed to compile gengtype-state.cc\n");
        return 0;
    }
    
    /* Link the executable */
    const char *link_cmd = 
        "g++ -O0 -fprofile-arcs -ftest-coverage "
        "gengtype.o gengtype-state.o "
        "-lgcov -liberty -o " GENGTYPE_EXECUTABLE " 2>&1";
    
    printf("Running: %s\n", link_cmd);
    if (system(link_cmd) != 0) {
        fprintf(stderr, "Failed to link gengtype executable\n");
        return 0;
    }
    
    printf("Successfully compiled " GENGTYPE_EXECUTABLE "\n");
    return 1;
}

/* Run gengtype with various patterns to trigger the switch statement */
void run_gengtype_patterns(char **temp_files, int file_count) {
    int i;
    char cmd[1024];
    
    printf("\n=== Pattern A: Process each file individually ===\n");
    for (i = 0; i < file_count; i++) {
        if (strstr(temp_files[i], "test_error.gt")) {
            /* Error file - test error handling path */
            snprintf(cmd, sizeof(cmd), 
                    "./" GENGTYPE_EXECUTABLE " -g output%d.h %s 2>&1",
                    i, temp_files[i]);
        } else {
            /* Normal files */
            snprintf(cmd, sizeof(cmd), 
                    "./" GENGTYPE_EXECUTABLE " -g output%d.h %s 2>&1",
                    i, temp_files[i]);
        }
        
        printf("Running: %s\n", cmd);
        int result = system(cmd);
        printf("Exit code: %d\n", WEXITSTATUS(result));
    }
    
    printf("\n=== Pattern B: Batch processing with -p flag ===\n");
    /* Create file list */
    char *filelist = create_temp_file(filelist_content, "gt_filelistXXXXXX");
    if (filelist) {
        snprintf(cmd, sizeof(cmd), 
                "./" GENGTYPE_EXECUTABLE " -p %s 2>&1", filelist);
        printf("Running: %s\n", cmd);
        system(cmd);
        unlink(filelist);
        free(filelist);
    }
    
    printf("\n=== Pattern C: Multiple files with header generation ===\n");
    /* Process first 3 files together */
    snprintf(cmd, sizeof(cmd), 
            "./" GENGTYPE_EXECUTABLE " -g combined.h %s %s %s 2>&1",
            temp_files[0], temp_files[1], temp_files[2]);
    printf("Running: %s\n", cmd);
    system(cmd);
    
    printf("\n=== Pattern D: With debug output ===\n");
    /* Run with debug flag if supported */
    snprintf(cmd, sizeof(cmd), 
            "./" GENGTYPE_EXECUTABLE " -d -g debug.h %s 2>&1",
            temp_files[0]);
    printf("Running: %s\n", cmd);
    system(cmd);
}

/* Generate coverage report */
void generate_coverage_report() {
    printf("\n=== Generating coverage report ===\n");
    
    /* Run gcov on gengtype */
    system("gcov " GENGTYPE_SOURCE " 2>&1");
    
    /* Check if the specific lines were covered */
    printf("\n=== Checking coverage of target lines (182-213) ===\n");
    system("cat gengtype.cc.gcov | grep -A 40 -B 5 '^       182:'");
    
    /* Create summary */
    printf("\n=== Coverage summary ===\n");
    system("gcov -b " GENGTYPE_SOURCE " 2>&1 | tail -20");
}

/* Clean up temporary files */
void cleanup_temp_files(char **temp_files, int file_count) {
    int i;
    for (i = 0; i < file_count; i++) {
        if (temp_files[i]) {
            unlink(temp_files[i]);
            free(temp_files[i]);
        }
    }
    
    /* Clean up generated files */
    unlink("output0.h");
    unlink("output1.h");
    unlink("output2.h");
    unlink("output3.h");
    unlink("output4.h");
    unlink("combined.h");
    unlink("debug.h");
    unlink("gengtype.o");
    unlink("gengtype-state.o");
    unlink(GENGTYPE_EXECUTABLE);
    unlink("gengtype.gcda");
    unlink("gengtype.gcno");
    unlink("gengtype-state.gcda");
    unlink("gengtype-state.gcno");
}

int main(int argc, char *argv[]) {
    int i;
    int file_count = sizeof(gt_files) / sizeof(gt_files[0]);
    char *temp_files[file_count];
    
    printf("=== Starting gengtype coverage test ===\n");
    
    /* Create temporary .gt files */
    printf("Creating temporary .gt files...\n");
    for (i = 0; i < file_count; i++) {
        temp_files[i] = create_temp_file(gt_contents[i], "temp_XXXXXX.gt");
        if (!temp_files[i]) {
            fprintf(stderr, "Failed to create temporary file %d\n", i);
            return 1;
        }
        printf("Created: %s\n", temp_files[i]);
    }
    
    /* Compile gengtype with coverage */
    if (!compile_gengtype_with_coverage()) {
        cleanup_temp_files(temp_files, file_count);
        return 1;
    }
    
    /* Run gengtype with various patterns */
    run_gengtype_patterns(temp_files, file_count);
    
    /* Generate coverage report */
    generate_coverage_report();
    
    /* Clean up */
    cleanup_temp_files(temp_files, file_count);
    
    printf("\n=== Test completed ===\n");
    printf("The switch statement in lines 182-213 should now be covered.\n");
    printf("Check gengtype.cc.gcov for detailed line coverage.\n");
    
    return 0;
}
