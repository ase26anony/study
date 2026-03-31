/* test_gengtype_coverage.c - Driver program to exercise gengtype type counting */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
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
int build_gengtype() {
    printf("Building gengtype with coverage instrumentation...\n");
    
    /* Compile gengtype.cc with coverage flags */
    const char *compile_cmd = 
        "g++ -O0 -fprofile-arcs -ftest-coverage -DIN_GCC "
        "-I. -I../../include -I../../gcc "
        "-c gengtype.cc -o gengtype.o 2>&1";
    
    if (system(compile_cmd) != 0) {
        fprintf(stderr, "Failed to compile gengtype.cc\n");
        return 0;
    }
    
    /* Compile gengtype-state.cc */
    const char *compile_state_cmd = 
        "g++ -O0 -fprofile-arcs -ftest-coverage -DIN_GCC "
        "-I. -I../../include -I../../gcc "
        "-c gengtype-state.cc -o gengtype-state.o 2>&1";
    
    if (system(compile_state_cmd) != 0) {
        fprintf(stderr, "Failed to compile gengtype-state.cc\n");
        return 0;
    }
    
    /* Link gengtype executable */
    const char *link_cmd = 
        "g++ -O0 -fprofile-arcs -ftest-coverage "
        "gengtype.o gengtype-state.o -lgcov -liberty -o gengtype_coverage 2>&1";
    
    if (system(link_cmd) != 0) {
        fprintf(stderr, "Failed to link gengtype\n");
        return 0;
    }
    
    return 1;
}

/* Run gengtype on a set of files */
int run_gengtype(const char **files, int count, const char *mode) {
    char cmd[4096];
    int ret = 0;
    
    if (strcmp(mode, "batch") == 0) {
        /* Create file list for batch processing */
        FILE *list = fopen("gt_filelist.txt", "w");
        if (!list) {
            perror("fopen filelist");
            return 0;
        }
        for (int i = 0; i < count; i++) {
            fprintf(list, "%s\n", files[i]);
        }
        fclose(list);
        
        /* Run with -p flag for batch processing */
        snprintf(cmd, sizeof(cmd), "./gengtype_coverage -p gt_filelist.txt 2>&1");
        ret = system(cmd);
        
        remove("gt_filelist.txt");
    } 
    else if (strcmp(mode, "header") == 0) {
        /* Generate header file */
        snprintf(cmd, sizeof(cmd), "./gengtype_coverage -g gtype-dump.h ");
        for (int i = 0; i < count; i++) {
            strcat(cmd, files[i]);
            strcat(cmd, " ");
        }
        strcat(cmd, "2>&1");
        ret = system(cmd);
        
        if (access("gtype-dump.h", F_OK) == 0) {
            printf("Generated header file: gtype-dump.h\n");
        }
    }
    else if (strcmp(mode, "routine") == 0) {
        /* Generate routine file */
        snprintf(cmd, sizeof(cmd), "./gengtype_coverage -r gtype-dump.c ");
        for (int i = 0; i < count; i++) {
            strcat(cmd, files[i]);
            strcat(cmd, " ");
        }
        strcat(cmd, "2>&1");
        ret = system(cmd);
        
        if (access("gtype-dump.c", F_OK) == 0) {
            printf("Generated routine file: gtype-dump.c\n");
        }
    }
    
    return (ret == 0);
}

int main() {
    /* GT file 1: Basic types and structs */
    const char *gt1_content = 
        "%{\n"
        "/* Test file 1: Basic types */\n"
        "#include \"config.h\"\n"
        "%}\n"
        "\n"
        "/* TYPE_UNDEFINED: Forward declaration */\n"
        "struct undefined_struct;\n"
        "\n"
        "/* TYPE_SCALAR: Scalar typedefs */\n"
        "typedef int my_scalar;\n"
        "typedef unsigned long scalar_ulong;\n"
        "\n"
        "/* TYPE_STRING: String type */\n"
        "struct string_struct {\n"
        "  const char *name;  /* TYPE_STRING */\n"
        "  int id;\n"
        "};\n"
        "\n"
        "/* TYPE_STRUCT: Regular struct */\n"
        "struct my_struct {\n"
        "  int a;\n"
        "  float b;\n"
        "};\n"
        "\n"
        "/* TYPE_POINTER: Pointer types */\n"
        "typedef struct my_struct *my_ptr;\n"
        "typedef int *int_ptr;\n"
        "\n"
        "/* TYPE_ARRAY: Array types */\n"
        "typedef int my_array[10];\n"
        "typedef struct my_struct struct_array[5];\n"
        "%}\n";
    
    /* GT file 2: Advanced types and unions */
    const char *gt2_content = 
        "%{\n"
        "/* Test file 2: Advanced types */\n"
        "#include \"config.h\"\n"
        "%}\n"
        "\n"
        "/* TYPE_UNION: Union type */\n"
        "union my_union {\n"
        "  int i;\n"
        "  void *p;\n"
        "  double d;\n"
        "};\n"
        "\n"
        "/* TYPE_USER_STRUCT: Struct with user marking */\n"
        "struct user_struct {\n"
        "  int *p;\n"
        "  char *data;\n"
        "} GTY((user));\n"
        "\n"
        "/* TYPE_CALLBACK: Callback function pointer */\n"
        "typedef void (*callback_fn)(void);\n"
        "typedef int (*compare_fn)(const void *, const void *);\n"
        "\n"
        "/* TYPE_LANG_STRUCT: Language-specific struct */\n"
        "struct lang_struct {\n"
        "  int data;\n"
        "  void *extra;\n"
        "} GTY ((lang));\n"
        "\n"
        "/* Complex nested type: struct containing pointer to union of arrays */\n"
        "struct complex_nested {\n"
        "  union my_union *uptr;          /* TYPE_POINTER to TYPE_UNION */\n"
        "  struct user_struct *user_ptr;  /* TYPE_POINTER to TYPE_USER_STRUCT */\n"
        "  callback_fn handler;           /* TYPE_CALLBACK */\n"
        "  int matrix[3][4];              /* TYPE_ARRAY (multidimensional) */\n"
        "};\n"
        "%}\n";
    
    /* GT file 3: Mixed types with errors and warnings */
    const char *gt3_content = 
        "%{\n"
        "/* Test file 3: Mixed types with potential issues */\n"
        "#include \"config.h\"\n"
        "%}\n"
        "\n"
        "/* Duplicate definition to trigger warning */\n"
        "struct my_struct {\n"
        "  int a;\n"
        "  float b;\n"
        "};\n"
        "\n"
        "/* More scalar types */\n"
        "typedef short small_scalar;\n"
        "typedef long double big_scalar;\n"
        "\n"
        "/* String array */\n"
        "struct string_array {\n"
        "  const char *names[5];  /* TYPE_ARRAY of TYPE_STRING */\n"
        "};\n"
        "\n"
        "/* Pointer to callback */\n"
        "typedef callback_fn *callback_ptr;\n"
        "\n"
        "/* Union with array */\n"
        "union union_with_array {\n"
        "  int numbers[10];\n"
        "  char chars[20];\n"
        "};\n"
        "\n"
        "/* Struct with all type kinds */\n"
        "struct kitchen_sink {\n"
        "  my_scalar scalar_field;        /* TYPE_SCALAR */\n"
        "  const char *string_field;      /* TYPE_STRING */\n"
        "  struct my_struct *struct_ptr;  /* TYPE_POINTER to TYPE_STRUCT */\n"
        "  union my_union union_field;    /* TYPE_UNION */\n"
        "  int array_field[8];            /* TYPE_ARRAY */\n"
        "  callback_fn callback_field;    /* TYPE_CALLBACK */\n"
        "};\n"
        "%}\n";
    
    /* GT file 4: File with syntax error (missing %}) */
    const char *gt4_content = 
        "%{\n"
        "/* Test file 4: File with syntax error */\n"
        "#include \"config.h\"\n"
        "%}\n"
        "\n"
        "struct error_struct {\n"
        "  int missing_brace;\n"
        "};\n"
        "/* Missing closing %} */\n";
    
    /* Create temporary files */
    const char *filenames[] = {
        "test_types1.gt",
        "test_types2.gt", 
        "test_types3.gt",
        "test_error.gt"
    };
    
    create_gt_file(filenames[0], gt1_content);
    create_gt_file(filenames[1], gt2_content);
    create_gt_file(filenames[2], gt3_content);
    create_gt_file(filenames[3], gt4_content);
    
    printf("Created %d test .gt files\n", 4);
    
    /* Build gengtype with coverage */
    if (!build_gengtype()) {
        fprintf(stderr, "Failed to build gengtype\n");
        cleanup_files(filenames, 4);
        return 1;
    }
    
    /* Test Pattern A: Process files individually */
    printf("\n=== Pattern A: Processing files individually ===\n");
    for (int i = 0; i < 3; i++) {  /* Skip error file for now */
        char cmd[256];
        snprintf(cmd, sizeof(cmd), "./gengtype_coverage %s 2>&1", filenames[i]);
        printf("Running: %s\n", cmd);
        if (system(cmd) != 0) {
            printf("Note: gengtype returned non-zero (expected for some cases)\n");
        }
    }
    
    /* Test Pattern B: Batch processing with -p flag */
    printf("\n=== Pattern B: Batch processing (-p flag) ===\n");
    const char *valid_files[] = {filenames[0], filenames[1], filenames[2]};
    if (!run_gengtype(valid_files, 3, "batch")) {
        printf("Batch processing completed (errors may be expected)\n");
    }
    
    /* Test Pattern C: Header generation */
    printf("\n=== Pattern C: Header generation (-g flag) ===\n");
    if (run_gengtype(valid_files, 3, "header")) {
        printf("Header generation successful\n");
    } else {
        printf("Header generation had issues (may be expected)\n");
    }
    
    /* Test Pattern D: Routine generation */
    printf("\n=== Pattern D: Routine generation (-r flag) ===\n");
    if (run_gengtype(valid_files, 3, "routine")) {
        printf("Routine generation successful\n");
    } else {
        printf("Routine generation had issues (may be expected)\n");
    }
    
    /* Test error file separately */
    printf("\n=== Testing error file ===\n");
    char error_cmd[256];
    snprintf(error_cmd, sizeof(error_cmd), "./gengtype_coverage %s 2>&1", filenames[3]);
    printf("Running error file (expected to fail): %s\n", error_cmd);
    int error_result = system(error_cmd);
    printf("Error file returned: %d\n", error_result);
    
    /* Cleanup */
    printf("\n=== Cleaning up ===\n");
    for (int i = 0; i < 4; i++) {
        remove(filenames[i]);
        printf("Removed: %s\n", filenames[i]);
    }
    
    remove("gtype-dump.h");
    remove("gtype-dump.c");
    remove("gengtype_coverage");
    remove("gengtype.o");
    remove("gengtype-state.o");
    
    /* Remove coverage data files */
    remove("gengtype.gcda");
    remove("gengtype.gcno");
    remove("gengtype-state.gcda");
    remove("gengtype-state.gcno");
    
    printf("\n=== Test completed ===\n");
    printf("To check coverage, run:\n");
    printf("  gcov gengtype.cc\n");
    printf("  gcov gengtype-state.cc\n");
    
    return 0;
}

/* Helper function for cleanup */
void cleanup_files(const char **files, int count) {
    for (int i = 0; i < count; i++) {
        remove(files[i]);
    }
}
