/* test_gengtype_coverage.c - Driver program to test gengtype switch coverage */

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
    
    printf("Executing: %s\n", compile_cmd);
    if (system(compile_cmd) != 0) {
        fprintf(stderr, "Failed to compile gengtype.cc\n");
        return 0;
    }
    
    /* Compile gengtype-state.cc */
    const char *compile_state_cmd = 
        "g++ -O0 -fprofile-arcs -ftest-coverage -DIN_GCC "
        "-I. -I../../include -I../../gcc "
        "-c gengtype-state.cc -o gengtype-state.o 2>&1";
    
    printf("Executing: %s\n", compile_state_cmd);
    if (system(compile_state_cmd) != 0) {
        fprintf(stderr, "Failed to compile gengtype-state.cc\n");
        return 0;
    }
    
    /* Link gengtype executable */
    const char *link_cmd = 
        "g++ -O0 -fprofile-arcs -ftest-coverage "
        "gengtype.o gengtype-state.o "
        "-lgcov -liberty -o gengtype_coverage 2>&1";
    
    printf("Executing: %s\n", link_cmd);
    if (system(link_cmd) != 0) {
        fprintf(stderr, "Failed to link gengtype\n");
        return 0;
    }
    
    return 1;
}

/* Run gengtype on a set of input files */
int run_gengtype_on_files(const char **files, int count, const char *mode) {
    char cmd[4096];
    int status;
    
    /* Pattern C: Generate header with multiple input files */
    if (strcmp(mode, "header") == 0) {
        snprintf(cmd, sizeof(cmd), "./gengtype_coverage -g output.h");
        for (int i = 0; i < count; i++) {
            strncat(cmd, " ", sizeof(cmd) - strlen(cmd) - 1);
            strncat(cmd, files[i], sizeof(cmd) - strlen(cmd) - 1);
        }
        
        printf("Executing: %s\n", cmd);
        status = system(cmd);
        if (status != 0) {
            fprintf(stderr, "gengtype failed with status %d\n", status);
            return 0;
        }
        
        /* Verify output was created */
        if (access("output.h", F_OK) != 0) {
            fprintf(stderr, "Output header not created\n");
            return 0;
        }
    }
    
    return 1;
}

/* Pattern B: Batch processing with -p flag */
int run_gengtype_batch(const char **files, int count) {
    FILE *list = fopen("filelist.txt", "w");
    if (!list) {
        perror("fopen filelist.txt");
        return 0;
    }
    
    for (int i = 0; i < count; i++) {
        fprintf(list, "%s\n", files[i]);
    }
    fclose(list);
    
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "./gengtype_coverage -p filelist.txt 2>&1");
    
    printf("Executing: %s\n", cmd);
    int status = system(cmd);
    
    unlink("filelist.txt");
    
    return (status == 0);
}

/* Pattern D: Test error cases */
int test_error_cases() {
    /* Create a file with syntax error */
    const char *error_content = 
        "%{\n"
        "struct bad_struct {\n"
        "  int x;\n"
        "  /* Missing closing brace and %}\n";
    
    create_gt_file("error.gt", error_content);
    
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "./gengtype_coverage error.gt 2>&1");
    
    printf("Testing error case: %s\n", cmd);
    int status = system(cmd);
    
    unlink("error.gt");
    
    /* gengtype should fail on syntax error */
    return (status != 0);
}

int main() {
    /* Create multiple .gt files with diverse type definitions */
    
    /* File 1: Basic types covering most categories */
    const char *file1_content = 
        "%{\n"
        "/* TYPE_UNDEFINED: Forward declaration */\n"
        "struct undefined_struct;\n"
        "\n"
        "/* TYPE_SCALAR: Scalar typedef */\n"
        "typedef int my_scalar;\n"
        "typedef unsigned long size_type;\n"
        "\n"
        "/* TYPE_STRUCT: Regular struct */\n"
        "struct my_struct {\n"
        "  int a;\n"
        "  double b;\n"
        "};\n"
        "\n"
        "/* TYPE_POINTER: Pointer types */\n"
        "typedef struct my_struct *my_ptr;\n"
        "typedef int *int_ptr;\n"
        "\n"
        "/* TYPE_ARRAY: Array types */\n"
        "typedef int my_array[10];\n"
        "typedef char name_array[32];\n"
        "%}\n";
    
    /* File 2: Advanced types including unions, callbacks, and user structs */
    const char *file2_content = 
        "%{\n"
        "/* TYPE_UNION: Union definition */\n"
        "union my_union {\n"
        "  int i;\n"
        "  void *p;\n"
        "  double d;\n"
        "};\n"
        "\n"
        "/* TYPE_CALLBACK: Callback function type */\n"
        "typedef void (*callback_fn)(void);\n"
        "typedef int (*compare_fn)(const void *, const void *);\n"
        "\n"
        "/* TYPE_USER_STRUCT: Struct with user marking */\n"
        "struct user_struct {\n"
        "  int *p;\n"
        "  void *data;\n"
        "} GTY((user));\n"
        "\n"
        "/* TYPE_STRING: String usage */\n"
        "struct string_struct {\n"
        "  const char *name;\n"
        "  char *buffer;\n"
        "};\n"
        "\n"
        "/* Complex nested type */\n"
        "struct complex_struct {\n"
        "  union my_union u;\n"
        "  callback_fn handler;\n"
        "  struct string_info {\n"
        "    const char *str;\n"
        "    int length;\n"
        "  } info;\n"
        "};\n"
        "%}\n";
    
    /* File 3: Language structs and more complex combinations */
    const char *file3_content = 
        "%{\n"
        "/* TYPE_LANG_STRUCT: Language-specific struct */\n"
        "struct lang_struct {\n"
        "  int data;\n"
        "  void *lang_data;\n"
        "} GTY ((lang));\n"
        "\n"
        "/* More TYPE_ARRAY variations */\n"
        "typedef struct my_struct struct_array[5];\n"
        "typedef union my_union union_array[8];\n"
        "\n"
        "/* Pointer to array */\n"
        "typedef int (*array_ptr)[10];\n"
        "\n"
        "/* Struct containing array of pointers */\n"
        "struct container {\n"
        "  void *items[20];\n"
        "  callback_fn callbacks[5];\n"
        "  struct lang_struct *lang_objs;\n"
        "};\n"
        "\n"
        "/* Forward declaration for undefined */\n"
        "struct another_undefined;\n"
        "\n"
        "/* Mixed type definitions */\n"
        "typedef enum { RED, GREEN, BLUE } color;\n"
        "struct color_info {\n"
        "  color c;\n"
        "  const char *name;\n"
        "};\n"
        "%}\n";
    
    /* File 4: Duplicate definitions for warning testing */
    const char *file4_content = 
        "%{\n"
        "/* Duplicate struct definition to trigger warnings */\n"
        "struct duplicate_struct {\n"
        "  int x;\n"
        "};\n"
        "\n"
        "struct duplicate_struct {\n"
        "  int y;\n"
        "};\n"
        "\n"
        "/* More scalar types */\n"
        "typedef short small_int;\n"
        "typedef long long big_int;\n"
        "%}\n";
    
    /* Create the files */
    create_gt_file("types1.gt", file1_content);
    create_gt_file("types2.gt", file2_content);
    create_gt_file("types3.gt", file3_content);
    create_gt_file("types4.gt", file4_content);
    
    const char *files[] = {"types1.gt", "types2.gt", "types3.gt", "types4.gt"};
    int file_count = 4;
    
    /* Build gengtype with coverage */
    if (!build_gengtype_with_coverage()) {
        fprintf(stderr, "Failed to build gengtype\n");
        return 1;
    }
    
    /* Pattern A: Process each file individually */
    printf("\n=== Pattern A: Processing files individually ===\n");
    for (int i = 0; i < file_count; i++) {
        char cmd[256];
        snprintf(cmd, sizeof(cmd), "./gengtype_coverage %s 2>&1", files[i]);
        printf("Processing %s: %s\n", files[i], cmd);
        
        int status = system(cmd);
        if (status != 0) {
            printf("Note: File %s may have warnings (expected)\n", files[i]);
        }
    }
    
    /* Pattern B: Batch processing */
    printf("\n=== Pattern B: Batch processing with -p flag ===\n");
    if (!run_gengtype_batch(files, file_count)) {
        fprintf(stderr, "Batch processing failed\n");
    }
    
    /* Pattern C: Generate header output */
    printf("\n=== Pattern C: Generate header with all files ===\n");
    if (!run_gengtype_on_files(files, file_count, "header")) {
        fprintf(stderr, "Header generation failed\n");
    }
    
    /* Pattern D: Test error cases */
    printf("\n=== Pattern D: Testing error cases ===\n");
    if (!test_error_cases()) {
        printf("Error case test completed (failure expected)\n");
    }
    
    /* Additional test: Run with debug flag */
    printf("\n=== Additional: Run with debug flag ===\n");
    char debug_cmd[1024];
    snprintf(debug_cmd, sizeof(debug_cmd), 
             "./gengtype_coverage -g debug_output.h types1.gt types2.gt 2>&1");
    system(debug_cmd);
    
    /* Cleanup temporary files */
    for (int i = 0; i < file_count; i++) {
        unlink(files[i]);
    }
    unlink("output.h");
    unlink("debug_output.h");
    
    /* Generate coverage report */
    printf("\n=== Generating coverage report ===\n");
    system("gcov gengtype.cc gengtype-state.cc 2>&1");
    
    /* Display gengtype.cc coverage */
    printf("\n=== Coverage for gengtype.cc ===\n");
    system("cat gengtype.cc.gcov | head -50 2>&1");
    
    printf("\nTest completed. Check gengtype.cc.gcov for line coverage.\n");
    printf("Specifically look for coverage of lines 182-213 in the switch statement.\n");
    
    return 0;
}
