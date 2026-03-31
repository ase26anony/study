/* gengtype_coverage_test.c */
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
    const char *compile_cmd = 
        "g++ -O0 -fprofile-arcs -ftest-coverage -DIN_GCC "
        "-I. -I../../include -I../../gcc "
        "-c gengtype.cc -o gengtype.o 2>&1";
    
    const char *link_cmd = 
        "g++ -O0 -fprofile-arcs -ftest-coverage "
        "gengtype.o -lgcov -liberty -o gengtype_coverage 2>&1";
    
    printf("Compiling gengtype with coverage...\n");
    if (system(compile_cmd) != 0) {
        fprintf(stderr, "Failed to compile gengtype.cc\n");
        return 0;
    }
    
    printf("Linking gengtype...\n");
    if (system(link_cmd) != 0) {
        fprintf(stderr, "Failed to link gengtype\n");
        return 0;
    }
    
    return 1;
}

/* Run gengtype on a set of input files */
void run_gengtype_on_files(const char **files, int count, const char *mode) {
    char cmd[4096];
    int i;
    
    if (strcmp(mode, "batch") == 0) {
        /* Create file list for batch processing */
        FILE *list = fopen("gt_filelist.txt", "w");
        for (i = 0; i < count; i++) {
            fprintf(list, "%s\n", files[i]);
        }
        fclose(list);
        
        snprintf(cmd, sizeof(cmd), "./gengtype_coverage -p gt_filelist.txt 2>&1");
        printf("Running: %s\n", cmd);
        system(cmd);
        
        unlink("gt_filelist.txt");
    } 
    else if (strcmp(mode, "header") == 0) {
        /* Generate header file */
        snprintf(cmd, sizeof(cmd), "./gengtype_coverage -g gtype-dump.h ");
        for (i = 0; i < count; i++) {
            strcat(cmd, files[i]);
            strcat(cmd, " ");
        }
        strcat(cmd, "2>&1");
        printf("Running: %s\n", cmd);
        system(cmd);
    }
    else if (strcmp(mode, "routine") == 0) {
        /* Generate routine file */
        snprintf(cmd, sizeof(cmd), "./gengtype_coverage -r gtype-dump.c ");
        for (i = 0; i < count; i++) {
            strcat(cmd, files[i]);
            strcat(cmd, " ");
        }
        strcat(cmd, "2>&1");
        printf("Running: %s\n", cmd);
        system(cmd);
    }
}

int main() {
    /* Create multiple .gt files with diverse type definitions */
    
    /* File 1: Basic types and structs */
    const char *gt_file1_content = 
        "%{\n"
        "/* Test file 1: Basic types */\n"
        "%}\n"
        "\n"
        "/* TYPE_UNDEFINED: Forward declaration */\n"
        "struct undefined_struct;\n"
        "\n"
        "/* TYPE_SCALAR: Scalar typedef */\n"
        "typedef int my_scalar;\n"
        "typedef unsigned long size_type;\n"
        "\n"
        "/* TYPE_STRING: String type */\n"
        "struct string_struct {\n"
        "    const char *name;          /* TYPE_STRING */\n"
        "    char *buffer;\n"
        "};\n"
        "\n"
        "/* TYPE_STRUCT: Regular struct */\n"
        "struct my_struct {\n"
        "    int a;\n"
        "    float b;\n"
        "    struct string_struct *str;\n"
        "};\n"
        "\n"
        "/* TYPE_POINTER: Pointer typedef */\n"
        "typedef struct my_struct *my_ptr;\n"
        "typedef int *int_ptr;\n"
        "\n"
        "%}\n";
    
    /* File 2: Unions, arrays, and user structs */
    const char *gt_file2_content = 
        "%{\n"
        "/* Test file 2: Complex types */\n"
        "%}\n"
        "\n"
        "/* TYPE_UNION: Union definition */\n"
        "union my_union {\n"
        "    int i;\n"
        "    void *p;\n"
        "    double d;\n"
        "};\n"
        "\n"
        "/* TYPE_ARRAY: Array types */\n"
        "typedef int my_array[10];\n"
        "typedef union my_union union_array[5][5];\n"
        "\n"
        "/* TYPE_USER_STRUCT: Struct with user marking */\n"
        "struct user_struct {\n"
        "    int *p;\n"
        "    struct my_struct *next;\n"
        "} GTY((user));\n"
        "\n"
        "/* TYPE_CALLBACK: Callback function type */\n"
        "typedef void (*callback_fn)(void);\n"
        "typedef int (*compare_fn)(const void *, const void *);\n"
        "\n"
        "/* Nested complex type */\n"
        "struct container {\n"
        "    union my_union data;\n"
        "    my_array buffer;\n"
        "    callback_fn handler;\n"
        "};\n"
        "\n"
        "%}\n";
    
    /* File 3: Language structs and edge cases */
    const char *gt_file3_content = 
        "%{\n"
        "/* Test file 3: Language-specific and edge cases */\n"
        "%}\n"
        "\n"
        "/* TYPE_LANG_STRUCT: Language-specific struct */\n"
        "struct lang_struct {\n"
        "    int data;\n"
        "    void *lang_data;\n"
        "} GTY ((lang));\n"
        "\n"
        "/* More TYPE_POINTER variations */\n"
        "typedef callback_fn (*func_ptr)(int);\n"
        "\n"
        "/* Complex nested type combining multiple categories */\n"
        "struct super_complex {\n"
        "    struct lang_struct *lang;      /* TYPE_POINTER to TYPE_LANG_STRUCT */\n"
        "    union {\n"
        "        my_array arr;              /* TYPE_ARRAY */\n"
        "        callback_fn cb;            /* TYPE_CALLBACK */\n"
        "    } u;\n"
        "    const char *description;       /* TYPE_STRING */\n"
        "};\n"
        "\n"
        "/* Another TYPE_USER_STRUCT */\n"
        "struct another_user_struct {\n"
        "    struct super_complex *complex;\n"
        "    int count;\n"
        "} GTY((user));\n"
        "\n"
        "%}\n";
    
    /* File 4: With syntax error to test error paths */
    const char *gt_file4_content = 
        "%{\n"
        "/* Test file 4: Contains deliberate syntax error */\n"
        "%}\n"
        "\n"
        "struct error_struct {\n"
        "    int x;\n"
        "    /* Missing semicolon to cause error */\n"
        "    int y\n"
        "};\n"
        "\n"
        "/* Note: Missing closing %} to test error recovery */\n";
    
    /* File 5: Duplicate definitions for warning testing */
    const char *gt_file5_content = 
        "%{\n"
        "/* Test file 5: Duplicate type definitions */\n"
        "%}\n"
        "\n"
        "struct duplicate_struct {\n"
        "    int a;\n"
        "};\n"
        "\n"
        "/* Duplicate definition */\n"
        "struct duplicate_struct {\n"
        "    int b;\n"
        "};\n"
        "\n"
        "%}\n";
    
    /* Create temporary file names */
    const char *files[] = {
        "test_types1.gt",
        "test_types2.gt", 
        "test_types3.gt",
        "test_error.gt",
        "test_dup.gt"
    };
    
    /* Write the .gt files */
    create_gt_file(files[0], gt_file1_content);
    create_gt_file(files[1], gt_file2_content);
    create_gt_file(files[2], gt_file3_content);
    create_gt_file(files[3], gt_file4_content);
    create_gt_file(files[4], gt_file5_content);
    
    /* Build gengtype with coverage instrumentation */
    if (!build_gengtype_with_coverage()) {
        fprintf(stderr, "Failed to build gengtype with coverage\n");
        return 1;
    }
    
    printf("\n=== Running gengtype with various modes ===\n\n");
    
    /* Pattern A: Process each file individually */
    printf("--- Pattern A: Individual file processing ---\n");
    for (int i = 0; i < 3; i++) {  /* Only process first 3 valid files */
        char cmd[256];
        snprintf(cmd, sizeof(cmd), "./gengtype_coverage %s 2>&1", files[i]);
        printf("Running: %s\n", cmd);
        system(cmd);
    }
    
    /* Pattern B: Batch processing with -p flag */
    printf("\n--- Pattern B: Batch processing (-p) ---\n");
    run_gengtype_on_files(files, 3, "batch");
    
    /* Pattern C: Header generation */
    printf("\n--- Pattern C: Header generation (-g) ---\n");
    run_gengtype_on_files(files, 3, "header");
    
    /* Pattern C: Routine generation */
    printf("\n--- Pattern C: Routine generation (-r) ---\n");
    run_gengtype_on_files(files, 3, "routine");
    
    /* Pattern D: Error cases */
    printf("\n--- Pattern D: Error cases ---\n");
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "./gengtype_coverage %s 2>&1", files[3]);
    printf("Running (should show error): %s\n", cmd);
    system(cmd);
    
    snprintf(cmd, sizeof(cmd), "./gengtype_coverage %s 2>&1", files[4]);
    printf("\nRunning (should show warning): %s\n", cmd);
    system(cmd);
    
    /* Clean up temporary files */
    printf("\n=== Cleaning up ===\n");
    for (int i = 0; i < 5; i++) {
        unlink(files[i]);
    }
    unlink("gtype-dump.h");
    unlink("gtype-dump.c");
    unlink("gengtype.o");
    unlink("gengtype_coverage");
    
    /* Generate coverage report */
    printf("\n=== Generating coverage report ===\n");
    system("gcov gengtype.cc 2>&1 | tail -20");
    
    printf("\nTest completed. Check gengtype.c.gcov for line coverage.\n");
    printf("The switch statement at lines 182-213 should show execution counts > 0\n");
    
    return 0;
}
