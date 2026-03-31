/* driver.c - Test driver for gengtype coverage */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

/* gengtype source files needed for compilation */
#define GT_SOURCE_FILES "gengtype.cc gengtype-state.cc gengtype-lex.cc gengtype-parse.cc"

/* Temporary file management */
typedef struct {
    char *filename;
    char *content;
} temp_file_t;

/* Create a temporary file with given content */
char *create_temp_file(const char *content, const char *suffix) {
    char template[256];
    snprintf(template, sizeof(template), "/tmp/gengtype_test_XXXXXX%s", suffix);
    int fd = mkstemps(template, strlen(suffix));
    if (fd < 0) {
        perror("mkstemps failed");
        return NULL;
    }
    
    FILE *f = fdopen(fd, "w");
    if (!f) {
        perror("fdopen failed");
        close(fd);
        return NULL;
    }
    
    fwrite(content, 1, strlen(content), f);
    fclose(fd);
    
    return strdup(template);
}

/* Build gengtype with coverage instrumentation */
int build_gengtype_with_coverage() {
    printf("Building gengtype with coverage instrumentation...\n");
    
    /* Compilation command with coverage flags */
    const char *compile_cmd = 
        "g++ -O0 -fprofile-arcs -ftest-coverage "
        "-DIN_GCC -DHAVE_CONFIG_H "
        "-I. -I../../include -I../../gcc "
        "-c gengtype.cc gengtype-state.cc gengtype-lex.cc gengtype-parse.cc "
        "2>&1";
    
    printf("Compiling: %s\n", compile_cmd);
    int status = system(compile_cmd);
    if (status != 0) {
        fprintf(stderr, "Compilation failed with status %d\n", status);
        return -1;
    }
    
    /* Linking command */
    const char *link_cmd = 
        "g++ -O0 -fprofile-arcs -ftest-coverage "
        "gengtype.o gengtype-state.o gengtype-lex.o gengtype-parse.o "
        "-lgcov -liberty -o gengtype_coverage "
        "2>&1";
    
    printf("Linking: %s\n", link_cmd);
    status = system(link_cmd);
    if (status != 0) {
        fprintf(stderr, "Linking failed with status %d\n", status);
        return -1;
    }
    
    printf("gengtype built successfully as 'gengtype_coverage'\n");
    return 0;
}

/* Run gengtype on input files */
int run_gengtype_on_files(const char **files, int count, const char *mode) {
    char cmd[4096];
    int status;
    
    /* Build command with all input files */
    snprintf(cmd, sizeof(cmd), "./gengtype_coverage %s ", mode);
    for (int i = 0; i < count; i++) {
        strncat(cmd, files[i], sizeof(cmd) - strlen(cmd) - 1);
        strncat(cmd, " ", sizeof(cmd) - strlen(cmd) - 1);
    }
    
    printf("Executing: %s\n", cmd);
    status = system(cmd);
    
    if (WIFEXITED(status)) {
        printf("gengtype exited with status %d\n", WEXITSTATUS(status));
    } else {
        printf("gengtype terminated abnormally\n");
    }
    
    return status;
}

/* Create file list for batch processing */
char *create_file_list(const char **files, int count) {
    char *list_file = create_temp_file("", ".list");
    if (!list_file) return NULL;
    
    FILE *f = fopen(list_file, "w");
    if (!f) {
        perror("Failed to open list file");
        free(list_file);
        return NULL;
    }
    
    for (int i = 0; i < count; i++) {
        fprintf(f, "%s\n", files[i]);
    }
    fclose(f);
    
    return list_file;
}

/* Main test driver */
int main() {
    int ret = 0;
    char *temp_files[10];
    int file_count = 0;
    
    /* Build gengtype with coverage */
    if (build_gengtype_with_coverage() != 0) {
        fprintf(stderr, "Failed to build gengtype\n");
        return 1;
    }
    
    /* ======================================================================
     * Create diverse .gt files to cover all type categories
     * ====================================================================== */
    
    /* File 1: Basic types and structs */
    const char *gt_file1 = 
        "%{\n"
        "/* Test file 1: Basic types */\n"
        "#include \"config.h\"\n"
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
        "  double b;\n"
        "};\n"
        "\n"
        "/* TYPE_POINTER: Pointer typedef */\n"
        "typedef struct my_struct *my_ptr;\n"
        "typedef my_scalar *scalar_ptr;\n"
        "\n"
        "/* TYPE_ARRAY: Array types */\n"
        "typedef int my_array[10];\n"
        "typedef struct my_struct struct_array[5];\n"
        "\n"
        "/* TYPE_CALLBACK: Callback function pointer */\n"
        "typedef void (*callback_fn)(void);\n"
        "typedef int (*callback_with_arg)(const char *);\n"
        "%}\n";
    
    /* File 2: Unions, user structs, and complex types */
    const char *gt_file2 = 
        "%{\n"
        "/* Test file 2: Unions and user structs */\n"
        "#include \"config.h\"\n"
        "%}\n"
        "\n"
        "/* TYPE_UNION: Union definition */\n"
        "union my_union {\n"
        "  int i;\n"
        "  void *p;\n"
        "  double d;\n"
        "};\n"
        "\n"
        "/* TYPE_USER_STRUCT: Struct with user marking */\n"
        "struct user_struct {\n"
        "  int *p;\n"
        "  void *data;\n"
        "} GTY((user));\n"
        "\n"
        "/* TYPE_LANG_STRUCT: Language-specific struct */\n"
        "struct lang_struct {\n"
        "  int data;\n"
        "  void *extra;\n"
        "} GTY((lang));\n"
        "\n"
        "/* Complex nested type: struct containing pointer to union of arrays */\n"
        "struct complex_type {\n"
        "  union nested_union {\n"
        "    int int_array[5];\n"
        "    char *ptr_array[3];\n"
        "  } u;\n"
        "  struct user_struct *user_ptr;\n"
        "  callback_fn callback;\n"
        "};\n"
        "\n"
        "/* Another user struct with callback */\n"
        "struct user_with_callback {\n"
        "  callback_fn notify;\n"
        "  int state;\n"
        "} GTY((user));\n"
        "%}\n";
    
    /* File 3: More complex types and error cases */
    const char *gt_file3 = 
        "%{\n"
        "/* Test file 3: Additional types and edge cases */\n"
        "#include \"config.h\"\n"
        "%}\n"
        "\n"
        "/* Multiple scalar types */\n"
        "typedef short small_scalar;\n"
        "typedef long long big_scalar;\n"
        "\n"
        "/* Pointer to array */\n"
        "typedef int (*array_ptr)[10];\n"
        "\n"
        "/* Struct with all kinds of members */\n"
        "struct kitchen_sink {\n"
        "  my_scalar scalar_field;          /* TYPE_SCALAR */\n"
        "  const char *string_field;        /* TYPE_STRING */\n"
        "  struct my_struct *struct_ptr;    /* TYPE_POINTER to TYPE_STRUCT */\n"
        "  union my_union union_field;      /* TYPE_UNION */\n"
        "  callback_fn action;              /* TYPE_CALLBACK */\n"
        "  int numbers[20];                 /* TYPE_ARRAY */\n"
        "};\n"
        "\n"
        "/* Chain of pointers */\n"
        "typedef struct chain_node {\n"
        "  int value;\n"
        "  struct chain_node *next;  /* Self-referential pointer */\n"
        "} chain_node_t;\n"
        "\n"
        "/* Mixed array types */\n"
        "typedef union my_union union_array[8];\n"
        "typedef callback_fn callback_array[4];\n"
        "%}\n";
    
    /* File 4: File with deliberate syntax error (missing %}) */
    const char *gt_file4_error = 
        "%{\n"
        "/* Test file 4: File with syntax error */\n"
        "#include \"config.h\"\n"
        "\n"
        "struct error_struct {\n"
        "  int missing_brace;\n"
        "};\n"
        "/* Missing closing %} to trigger error path */\n";
    
    /* File 5: Duplicate definitions for warning testing */
    const char *gt_file5_dup = 
        "%{\n"
        "/* Test file 5: Duplicate definitions */\n"
        "#include \"config.h\"\n"
        "%}\n"
        "\n"
        "/* Duplicate type definition */\n"
        "struct duplicate_struct {\n"
        "  int first;\n"
        "};\n"
        "\n"
        "/* Same struct defined again */\n"
        "struct duplicate_struct {\n"
        "  int second;\n"
        "};\n"
        "%}\n";
    
    /* Create temporary files */
    temp_files[file_count++] = create_temp_file(gt_file1, ".gt");
    temp_files[file_count++] = create_temp_file(gt_file2, ".gt");
    temp_files[file_count++] = create_temp_file(gt_file3, ".gt");
    temp_files[file_count++] = create_temp_file(gt_file4_error, ".gt");
    temp_files[file_count++] = create_temp_file(gt_file5_dup, ".gt");
    
    /* Check all files were created */
    for (int i = 0; i < file_count; i++) {
        if (!temp_files[i]) {
            fprintf(stderr, "Failed to create temp file %d\n", i);
            ret = 1;
            goto cleanup;
        }
        printf("Created temp file: %s\n", temp_files[i]);
    }
    
    /* ======================================================================
     * Execute gengtype in different modes to trigger type counting
     * ====================================================================== */
    
    printf("\n=== Pattern A: Multiple File Processing ===\n");
    /* Run gengtype on first 3 valid files to generate header */
    run_gengtype_on_files((const char **)temp_files, 3, "-g output_header.h");
    
    printf("\n=== Pattern B: Batch Processing with -p ===\n");
    /* Create file list and process with -p flag */
    char *list_file = create_file_list((const char **)temp_files, 3);
    if (list_file) {
        char cmd[512];
        snprintf(cmd, sizeof(cmd), "./gengtype_coverage -p %s -r output_routines.c", list_file);
        printf("Executing: %s\n", cmd);
        system(cmd);
        unlink(list_file);
        free(list_file);
    }
    
    printf("\n=== Pattern C: Header Generation with All Types ===\n");
    /* Process all files including error/warning ones */
    run_gengtype_on_files((const char **)temp_files, file_count, "-g all_output.h 2>&1");
    
    printf("\n=== Pattern D: Error and Warning Cases ===\n");
    /* Test error file alone */
    run_gengtype_on_files((const char **)&temp_files[3], 1, "-g error_output.h 2>&1");
    
    /* Test duplicate file alone */
    run_gengtype_on_files((const char **)&temp_files[4], 1, "-g dup_output.h 2>&1");
    
    /* ======================================================================
     * Additional test: Process with debug flag for more coverage
     * ====================================================================== */
    printf("\n=== Additional: Debug Mode Processing ===\n");
    /* Rebuild with debug flag if needed, or just run with verbose output */
    run_gengtype_on_files((const char **)temp_files, 3, "-v -g debug_output.h 2>&1");
    
    /* ======================================================================
     * Verify coverage data was generated
     * ====================================================================== */
    printf("\n=== Checking for coverage data ===\n");
    if (access("gengtype.gcda", F_OK) == 0) {
        printf("Coverage data file gengtype.gcda exists\n");
        
        /* Run gcov to see coverage */
        printf("\nRunning gcov to show coverage:\n");
        system("gcov gengtype.cc 2>&1 | head -50");
    } else {
        printf("Warning: gengtype.gcda not found - coverage may not be recorded\n");
    }
    
    /* Clean up output files */
    unlink("output_header.h");
    unlink("output_routines.c");
    unlink("all_output.h");
    unlink("error_output.h");
    unlink("dup_output.h");
    unlink("debug_output.h");
    
cleanup:
    /* Clean up temporary files */
    printf("\n=== Cleaning up temporary files ===\n");
    for (int i = 0; i < file_count; i++) {
        if (temp_files[i]) {
            unlink(temp_files[i]);
            free(temp_files[i]);
        }
    }
    
    /* Clean up object files */
    system("rm -f gengtype.o gengtype-state.o gengtype-lex.o gengtype-parse.o");
    system("rm -f gengtype_coverage");
    system("rm -f *.gcda *.gcno");
    
    if (ret == 0) {
        printf("\n=== Test completed successfully ===\n");
        printf("The switch statement in gengtype.cc lines 182-213 should now be covered.\n");
        printf("Run 'gcov gengtype.cc' to see detailed coverage information.\n");
    }
    
    return ret;
}
