/* test_gengtype_coverage.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

/* Create temporary .gt files with various type definitions */
void create_gt_file(const char *filename, const char *content) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        perror("fopen");
        exit(1);
    }
    fprintf(f, "%s", content);
    fclose(f);
}

/* Build gengtype with coverage instrumentation */
int build_gengtype_with_coverage() {
    printf("Building gengtype with coverage instrumentation...\n");
    
    /* Compile gengtype.cc with coverage flags */
    const char *compile_cmd = 
        "g++ -O0 -fprofile-arcs -ftest-coverage -DIN_GCC "
        "-I. -I../../include -I../../gcc "
        "-c gengtype.cc -o gengtype.o 2>&1";
    
    printf("Compiling: %s\n", compile_cmd);
    if (system(compile_cmd) != 0) {
        fprintf(stderr, "Failed to compile gengtype.cc\n");
        return 0;
    }
    
    /* Compile gengtype-state.cc */
    const char *compile_state_cmd = 
        "g++ -O0 -fprofile-arcs -ftest-coverage -DIN_GCC "
        "-I. -I../../include -I../../gcc "
        "-c gengtype-state.cc -o gengtype-state.o 2>&1";
    
    printf("Compiling: %s\n", compile_state_cmd);
    if (system(compile_state_cmd) != 0) {
        fprintf(stderr, "Failed to compile gengtype-state.cc\n");
        return 0;
    }
    
    /* Link gengtype executable */
    const char *link_cmd = 
        "g++ -O0 -fprofile-arcs -ftest-coverage "
        "gengtype.o gengtype-state.o "
        "-lgcov -liberty -o gengtype_coverage 2>&1";
    
    printf("Linking: %s\n", link_cmd);
    if (system(link_cmd) != 0) {
        fprintf(stderr, "Failed to link gengtype\n");
        return 0;
    }
    
    return 1;
}

/* Run gengtype on multiple .gt files using different patterns */
void run_gengtype_tests() {
    printf("\n=== Creating test .gt files ===\n");
    
    /* File 1: Basic types and TYPE_UNDEFINED */
    const char *file1_content = 
        "%{\n"
        "/* Test file 1: Basic types with undefined forward declaration */\n"
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
        "  char *data;\n"
        "};\n"
        "\n"
        "/* TYPE_POINTER: Pointer types */\n"
        "typedef struct string_struct *string_ptr;\n"
        "typedef my_scalar *scalar_ptr;\n"
        "%}\n";
    
    /* File 2: Structs, unions, and arrays */
    const char *file2_content = 
        "%{\n"
        "/* Test file 2: Structs, unions, arrays */\n"
        "#include \"config.h\"\n"
        "#include \"system.h\"\n"
        "%}\n"
        "\n"
        "/* TYPE_STRUCT: Regular struct */\n"
        "struct my_struct {\n"
        "  int a;\n"
        "  double b;\n"
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
        "  struct my_struct *s;\n"
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
        "%}\n";
    
    /* File 3: Callbacks, lang structs, and more complex types */
    const char *file3_content = 
        "%{\n"
        "/* Test file 3: Callbacks and language-specific types */\n"
        "#include \"config.h\"\n"
        "#include \"system.h\"\n"
        "%}\n"
        "\n"
        "/* TYPE_CALLBACK: Callback function pointer */\n"
        "typedef void (*callback_fn)(void);\n"
        "typedef int (*compare_fn)(const void *, const void *);\n"
        "\n"
        "/* TYPE_LANG_STRUCT: Language-specific struct */\n"
        "struct lang_struct {\n"
        "  int data;\n"
        "  void *tree;\n"
        "} GTY ((lang));\n"
        "\n"
        "/* More complex nested types */\n"
        "struct container {\n"
        "  callback_fn handler;\n"
        "  struct lang_struct *lang_data;\n"
        "  union {\n"
        "    int tag;\n"
        "    void *ptr;\n"
        "  } variant;\n"
        "};\n"
        "\n"
        "/* Pointer to callback */\n"
        "typedef callback_fn *callback_ptr;\n"
        "%}\n";
    
    /* File 4: File with syntax error (for error path testing) */
    const char *file4_content = 
        "%{\n"
        "/* Test file 4: File with deliberate syntax error */\n"
        "#include \"config.h\"\n"
        "#include \"system.h\"\n"
        "/* Missing closing %} to trigger error */\n"
        "\n"
        "struct error_struct {\n"
        "  int x;\n";
    
    /* File 5: Duplicate definitions (for warning testing) */
    const char *file5_content = 
        "%{\n"
        "/* Test file 5: Duplicate type definitions */\n"
        "#include \"config.h\"\n"
        "#include \"system.h\"\n"
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
        "%}\n";
    
    /* Create the test files */
    create_gt_file("test1.gt", file1_content);
    create_gt_file("test2.gt", file2_content);
    create_gt_file("test3.gt", file3_content);
    create_gt_file("test4_error.gt", file4_content);
    create_gt_file("test5_dup.gt", file5_content);
    
    printf("Created 5 test .gt files\n");
    
    /* Pattern A: Process each file individually */
    printf("\n=== Pattern A: Processing files individually ===\n");
    const char *files[] = {"test1.gt", "test2.gt", "test3.gt", NULL};
    
    for (int i = 0; files[i]; i++) {
        char cmd[256];
        snprintf(cmd, sizeof(cmd), "./gengtype_coverage -g output%d.h %s 2>&1", 
                 i + 1, files[i]);
        printf("Running: %s\n", cmd);
        int ret = system(cmd);
        printf("Exit code: %d\n", WEXITSTATUS(ret));
    }
    
    /* Pattern B: Batch processing with -p flag */
    printf("\n=== Pattern B: Batch processing with -p ===\n");
    FILE *filelist = fopen("gt_filelist.txt", "w");
    if (filelist) {
        fprintf(filelist, "test1.gt\ntest2.gt\ntest3.gt\n");
        fclose(filelist);
        
        system("./gengtype_coverage -p gt_filelist.txt -g batch_output.h 2>&1");
    }
    
    /* Pattern C: Multiple files in one command */
    printf("\n=== Pattern C: Multiple files in one command ===\n");
    system("./gengtype_coverage -g combined.h test1.gt test2.gt test3.gt 2>&1");
    
    /* Pattern D: Error and warning cases */
    printf("\n=== Pattern D: Error and warning cases ===\n");
    printf("Testing syntax error (should fail):\n");
    system("./gengtype_coverage test4_error.gt 2>&1");
    
    printf("\nTesting duplicate definition (should warn):\n");
    system("./gengtype_coverage test5_dup.gt 2>&1");
    
    /* Additional test: Run with -r flag for routine generation */
    printf("\n=== Additional test: Generating routines ===\n");
    system("./gengtype_coverage -r gtype-desc.c test1.gt test2.gt 2>&1");
}

/* Clean up temporary files */
void cleanup() {
    printf("\n=== Cleaning up temporary files ===\n");
    
    /* Remove generated .gt files */
    remove("test1.gt");
    remove("test2.gt");
    remove("test3.gt");
    remove("test4_error.gt");
    remove("test5_dup.gt");
    
    /* Remove file list */
    remove("gt_filelist.txt");
    
    /* Remove output files */
    for (int i = 1; i <= 3; i++) {
        char filename[256];
        snprintf(filename, sizeof(filename), "output%d.h", i);
        remove(filename);
    }
    
    remove("batch_output.h");
    remove("combined.h");
    remove("gtype-desc.c");
    
    /* Remove coverage files */
    remove("gengtype.gcda");
    remove("gengtype.gcno");
    remove("gengtype-state.gcda");
    remove("gengtype-state.gcno");
    
    printf("Cleanup complete\n");
}

/* Main driver function */
int main() {
    printf("=== GCC gengtype Coverage Test Driver ===\n");
    
    /* Build gengtype with coverage instrumentation */
    if (!build_gengtype_with_coverage()) {
        fprintf(stderr, "Failed to build gengtype with coverage\n");
        return 1;
    }
    
    /* Run the tests */
    run_gengtype_tests();
    
    /* Generate coverage report */
    printf("\n=== Generating coverage report ===\n");
    system("gcov gengtype.cc 2>&1 | grep -A 20 'Lines executed:'");
    
    /* Specifically check for the switch statement coverage */
    printf("\n=== Checking switch statement coverage ===\n");
    system("gcov -b gengtype.cc 2>&1 | grep -B5 -A5 '182-213'");
    
    /* Clean up */
    cleanup();
    
    printf("\n=== Test completed successfully ===\n");
    return 0;
}
