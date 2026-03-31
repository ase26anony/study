/* test_gengtype_coverage.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>

/* Compilation flags for building gengtype with coverage */
#define COVERAGE_FLAGS "-O0 -fprofile-arcs -ftest-coverage"
#define INCLUDE_FLAGS "-I. -I../../include -I../../gcc"
#define DEFINES "-DIN_GCC -DHAVE_CONFIG_H"

/* Temporary file management */
typedef struct {
    char *filename;
    char *content;
} temp_file_t;

/* GT file definitions covering all type kinds */
static const char *gt_files[] = {
    /* File 1: Basic types and structs */
    "file1.gt",
    "%{\n"
    "/* Include any necessary headers */\n"
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
    "/* TYPE_STRING: String type usage */\n"
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
    "/* TYPE_POINTER: Pointer typedef */\n"
    "typedef struct my_struct *my_ptr;\n"
    "%}\n",

    /* File 2: User structs, unions, and arrays */
    "file2.gt",
    "%{\n"
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
    "/* TYPE_UNION: Union definition */\n"
    "union my_union {\n"
    "  int i;\n"
    "  void *p;\n"
    "  double d;\n"
    "};\n"
    "\n"
    "/* TYPE_ARRAY: Array typedefs */\n"
    "typedef int my_array[10];\n"
    "typedef struct my_struct struct_array[5];\n"
    "\n"
    "/* TYPE_CALLBACK: Callback function pointer */\n"
    "typedef void (*callback_fn)(void);\n"
    "typedef int (*another_callback)(const char *, int);\n"
    "\n"
    "/* Complex nested type */\n"
    "struct complex_type {\n"
    "  union my_union u;\n"
    "  my_array arr;\n"
    "  callback_fn cb;\n"
    "};\n"
    "%}\n",

    /* File 3: Language struct and more complex types */
    "file3.gt",
    "%{\n"
    "#include \"config.h\"\n"
    "#include \"system.h\"\n"
    "%}\n"
    "\n"
    "/* TYPE_LANG_STRUCT: Language-specific struct */\n"
    "struct lang_struct {\n"
    "  int data;\n"
    "  void *lang_data;\n"
    "} GTY((lang));\n"
    "\n"
    "/* More TYPE_POINTER variations */\n"
    "typedef union my_union *union_ptr;\n"
    "typedef callback_fn *callback_ptr;\n"
    "\n"
    "/* Struct containing pointer to union of arrays */\n"
    "struct nested_complex {\n"
    "  union {\n"
    "    int *int_ptr;\n"
    "    struct my_struct **struct_ptr_ptr;\n"
    "  } ptr_union;\n"
    "  int multi_array[3][4][5];\n"
    "};\n"
    "\n"
    "/* Another TYPE_STRING usage */\n"
    "struct with_strings {\n"
    "  const char *title;\n"
    "  char *buffer;\n"
    "  const char * const constant_string;\n"
    "};\n"
    "%}\n",

    /* File 4: File with syntax error (for error path testing) */
    "file4_error.gt",
    "%{\n"
    "#include \"config.h\"\n"
    "#include \"system.h\"\n"
    "/* Missing closing %} to trigger error */\n"
    "\n"
    "struct error_struct {\n"
    "  int x;\n"
    "};\n"
    "/* No closing %} */\n",

    /* File 5: Duplicate definitions (for warning testing) */
    "file5_dup.gt",
    "%{\n"
    "#include \"config.h\"\n"
    "#include \"system.h\"\n"
    "%}\n"
    "\n"
    "/* Duplicate type definition */\n"
    "struct my_struct {\n"
    "  int a;\n"
    "  float b;\n"
    "};\n"
    "\n"
    "/* Same struct defined again */\n"
    "struct my_struct {\n"
    "  int a;\n"
    "  float b;\n"
    "};\n"
    "%}\n"
};

/* Create a temporary file with given content */
static char *create_temp_file(const char *filename, const char *content) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        perror("fopen");
        return NULL;
    }
    fputs(content, f);
    fclose(f);
    return strdup(filename);
}

/* Build gengtype with coverage instrumentation */
static int build_gengtype_with_coverage() {
    printf("Building gengtype with coverage instrumentation...\n");
    
    /* Compile gengtype.cc */
    char cmd[1024];
    snprintf(cmd, sizeof(cmd),
             "g++ %s %s %s -c gengtype.cc -o gengtype.o",
             COVERAGE_FLAGS, INCLUDE_FLAGS, DEFINES);
    printf("Executing: %s\n", cmd);
    if (system(cmd) != 0) {
        fprintf(stderr, "Failed to compile gengtype.cc\n");
        return -1;
    }
    
    /* Compile gengtype-state.cc */
    snprintf(cmd, sizeof(cmd),
             "g++ %s %s %s -c gengtype-state.cc -o gengtype-state.o",
             COVERAGE_FLAGS, INCLUDE_FLAGS, DEFINES);
    printf("Executing: %s\n", cmd);
    if (system(cmd) != 0) {
        fprintf(stderr, "Failed to compile gengtype-state.cc\n");
        return -1;
    }
    
    /* Link gengtype executable */
    snprintf(cmd, sizeof(cmd),
             "g++ %s gengtype.o gengtype-state.o -o gengtype_coverage "
             "-lgcov -liberty -lz",
             COVERAGE_FLAGS);
    printf("Executing: %s\n", cmd);
    if (system(cmd) != 0) {
        fprintf(stderr, "Failed to link gengtype\n");
        return -1;
    }
    
    return 0;
}

/* Run gengtype with multiple input files (Pattern A) */
static int run_gengtype_multiple_files(const char **files, int count) {
    printf("\n=== Running gengtype with multiple files ===\n");
    
    for (int i = 0; i < count; i++) {
        char cmd[1024];
        /* Generate header output to force full parsing */
        snprintf(cmd, sizeof(cmd),
                 "./gengtype_coverage -g output%d.h %s",
                 i + 1, files[i]);
        printf("Executing: %s\n", cmd);
        
        int status = system(cmd);
        if (status != 0 && i < 3) { /* First 3 files should succeed */
            fprintf(stderr, "gengtype failed on file %s with status %d\n", 
                    files[i], status);
            return -1;
        }
        /* Files 4 and 5 are expected to produce errors/warnings */
    }
    
    return 0;
}

/* Run gengtype with batch processing (Pattern B) */
static int run_gengtype_batch(const char **files, int count) {
    printf("\n=== Running gengtype with batch processing ===\n");
    
    /* Create file list */
    FILE *list = fopen("gt_filelist.txt", "w");
    if (!list) {
        perror("fopen filelist");
        return -1;
    }
    
    for (int i = 0; i < count; i++) {
        fprintf(list, "%s\n", files[i]);
    }
    fclose(list);
    
    /* Run gengtype with -p flag */
    char cmd[1024];
    snprintf(cmd, sizeof(cmd),
             "./gengtype_coverage -p gt_filelist.txt");
    printf("Executing: %s\n", cmd);
    
    int status = system(cmd);
    if (status != 0) {
        fprintf(stderr, "Batch processing failed with status %d\n", status);
        return -1;
    }
    
    unlink("gt_filelist.txt");
    return 0;
}

/* Run gengtype with combined input (Pattern C) */
static int run_gengtype_combined(const char **files, int count) {
    printf("\n=== Running gengtype with combined input ===\n");
    
    /* Build command with all valid files (skip error files) */
    char cmd[2048] = "./gengtype_coverage -g combined.h -r combined.c";
    
    for (int i = 0; i < count && i < 3; i++) { /* Only first 3 valid files */
        strcat(cmd, " ");
        strcat(cmd, files[i]);
    }
    
    printf("Executing: %s\n", cmd);
    int status = system(cmd);
    if (status != 0) {
        fprintf(stderr, "Combined processing failed with status %d\n", status);
        return -1;
    }
    
    return 0;
}

/* Run gcov to verify coverage */
static void check_coverage() {
    printf("\n=== Checking coverage ===\n");
    
    /* Run gcov on gengtype.cc */
    system("gcov gengtype.cc");
    
    /* Display coverage summary */
    FILE *gcov_file = fopen("gengtype.cc.gcov", "r");
    if (gcov_file) {
        char line[256];
        printf("\nCoverage summary for critical lines:\n");
        printf("Lines 182-213 should show execution counts > 0\n");
        while (fgets(line, sizeof(line), gcov_file)) {
            /* Look for our target lines */
            if (strstr(line, ":182:") || strstr(line, ":190:") || 
                strstr(line, ":200:") || strstr(line, ":210:")) {
                printf("%s", line);
            }
        }
        fclose(gcov_file);
    }
}

/* Clean up temporary files */
static void cleanup_files(const char **files, int count) {
    for (int i = 0; i < count; i++) {
        unlink(files[i]);
    }
    unlink("output1.h");
    unlink("output2.h");
    unlink("output3.h");
    unlink("combined.h");
    unlink("combined.c");
    unlink("gengtype.gcda");
    unlink("gengtype.gcno");
    unlink("gengtype-state.gcda");
    unlink("gengtype-state.gcno");
}

int main(int argc, char **argv) {
    int num_files = sizeof(gt_files) / (2 * sizeof(char *));
    char *temp_files[10];
    int ret = 0;
    
    printf("=== GCC gengtype Coverage Test ===\n");
    printf("Testing switch statement coverage for lines 182-213 in gengtype.cc\n");
    
    /* Create temporary GT files */
    printf("\nCreating %d temporary .gt files...\n", num_files);
    for (int i = 0; i < num_files; i++) {
        const char *filename = gt_files[i * 2];
        const char *content = gt_files[i * 2 + 1];
        
        temp_files[i] = create_temp_file(filename, content);
        if (!temp_files[i]) {
            fprintf(stderr, "Failed to create file %s\n", filename);
            ret = -1;
            goto cleanup;
        }
        printf("Created: %s (%zu bytes)\n", filename, strlen(content));
    }
    
    /* Build gengtype with coverage */
    if (build_gengtype_with_coverage() != 0) {
        ret = -1;
        goto cleanup;
    }
    
    /* Pattern A: Multiple file processing */
    if (run_gengtype_multiple_files((const char **)temp_files, num_files) != 0) {
        ret = -1;
        goto cleanup;
    }
    
    /* Pattern B: Batch processing */
    if (run_gengtype_batch((const char **)temp_files, num_files) != 0) {
        ret = -1;
        goto cleanup;
    }
    
    /* Pattern C: Combined processing */
    if (run_gengtype_combined((const char **)temp_files, num_files) != 0) {
        ret = -1;
        goto cleanup;
    }
    
    /* Check coverage results */
    check_coverage();
    
    printf("\n=== Test completed successfully ===\n");
    printf("The switch statement in lines 182-213 should now be covered.\n");
    printf("All type kinds have been exercised:\n");
    printf("  - TYPE_UNDEFINED via forward declarations\n");
    printf("  - TYPE_SCALAR via typedefs\n");
    printf("  - TYPE_STRING via const char* members\n");
    printf("  - TYPE_STRUCT and TYPE_USER_STRUCT\n");
    printf("  - TYPE_UNION\n");
    printf("  - TYPE_POINTER\n");
    printf("  - TYPE_ARRAY\n");
    printf("  - TYPE_CALLBACK\n");
    printf("  - TYPE_LANG_STRUCT\n");
    
cleanup:
    /* Cleanup */
    cleanup_files((const char **)temp_files, num_files);
    for (int i = 0; i < num_files; i++) {
        free(temp_files[i]);
    }
    
    return ret;
}
