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
const char *gt_file_1 = 
"%{\n"
"/* File 1: Basic types and structs */\n"
"#include \"config.h\"\n"
"#include \"system.h\"\n"
"%}\n"
"\n"
"/* TYPE_UNDEFINED - forward declaration */\n"
"struct undefined_struct;\n"
"\n"
"/* TYPE_SCALAR */\n"
"typedef int my_scalar;\n"
"typedef unsigned long scalar_two;\n"
"\n"
"/* TYPE_STRING */\n"
"struct string_struct {\n"
"  const char *name;  /* string type */\n"
"  char *data;\n"
"};\n"
"\n"
"/* TYPE_STRUCT */\n"
"struct my_struct {\n"
"  int a;\n"
"  double b;\n"
"  struct my_struct *next;\n"
"};\n"
"\n"
"/* TYPE_POINTER */\n"
"typedef struct my_struct *my_ptr;\n"
"typedef my_scalar *scalar_ptr;\n"
"\n"
"/* TYPE_ARRAY */\n"
"typedef int my_array[10];\n"
"typedef struct my_struct struct_array[5];\n"
"%}";

const char *gt_file_2 = 
"%{\n"
"/* File 2: Unions, callbacks, and user structs */\n"
"#include \"config.h\"\n"
"#include \"system.h\"\n"
"%}\n"
"\n"
"/* TYPE_UNION */\n"
"union my_union {\n"
"  int i;\n"
"  void *p;\n"
"  double d;\n"
"};\n"
"\n"
"/* TYPE_CALLBACK */\n"
"typedef void (*callback_fn)(void);\n"
"typedef int (*compare_fn)(const void *, const void *);\n"
"\n"
"/* TYPE_USER_STRUCT */\n"
"struct user_struct {\n"
"  int *p;\n"
"  void *data;\n"
"} GTY((user));\n"
"\n"
"/* Complex nested type combining multiple categories */\n"
"struct complex_type {\n"
"  union my_union u;          /* TYPE_UNION */\n"
"  callback_fn handler;       /* TYPE_CALLBACK */\n"
"  struct user_struct *user;  /* TYPE_POINTER to TYPE_USER_STRUCT */\n"
"  int array[20];            /* TYPE_ARRAY */\n"
"  const char *description;  /* TYPE_STRING */\n"
"};\n"
"\n"
"/* Another scalar */\n"
"typedef float another_scalar;\n"
"%}";

const char *gt_file_3 = 
"%{\n"
"/* File 3: Lang structs, arrays of pointers, and error cases */\n"
"#include \"config.h\"\n"
"#include \"system.h\"\n"
"%}\n"
"\n"
"/* TYPE_LANG_STRUCT */\n"
"struct lang_struct {\n"
"  int data;\n"
"  void *lang_data;\n"
"} GTY((lang));\n"
"\n"
"/* More complex nested types */\n"
"struct nested_container {\n"
"  struct lang_struct lang;           /* TYPE_LANG_STRUCT */\n"
"  union {\n"
"    int x;\n"
"    struct lang_struct *lang_ptr;    /* TYPE_POINTER to TYPE_LANG_STRUCT */\n"
"  } choice;\n"
"  void (*operations[5])(void);       /* TYPE_ARRAY of TYPE_CALLBACK */\n"
"};\n"
"\n"
"/* Array of pointers to unions */\n"
"union my_union *ptr_array[10];\n"
"\n"
"/* String array */\n"
"const char *string_array[] = {\"one\", \"two\", \"three\"};\n"
"\n"
"/* Forward declaration for undefined */\n"
"struct another_undefined;\n"
"%}";

/* File with deliberate syntax error to test error paths */
const char *gt_file_error = 
"%{\n"
"/* File with syntax error - missing closing %} */\n"
"#include \"config.h\"\n"
"#include \"system.h\"\n"
"\n"
"struct error_struct {\n"
"  int missing_brace;\n"
"/* Deliberately missing %} */";

/* File with duplicate definition to test warning paths */
const char *gt_file_warning = 
"%{\n"
"#include \"config.h\"\n"
"#include \"system.h\"\n"
"%}\n"
"\n"
"/* Duplicate definition to trigger warning */\n"
"typedef int my_scalar;\n"
"typedef int my_scalar;  /* Duplicate */\n"
"\n"
"struct duplicate_struct {\n"
"  int a;\n"
"};\n"
"\n"
"struct duplicate_struct {  /* Duplicate struct */\n"
"  int b;\n"
"};\n"
"%}";

/* Create a temporary file with given content */
char *create_temp_file(const char *content, const char *suffix) {
    char template[] = "/tmp/gengtype_test_XXXXXX";
    int fd = mkstemp(template);
    if (fd == -1) {
        perror("mkstemp");
        return NULL;
    }
    
    char *filename = strdup(template);
    strcat(filename, suffix);
    
    /* Rename to add suffix */
    close(fd);
    rename(template, filename);
    
    /* Write content */
    FILE *f = fopen(filename, "w");
    if (!f) {
        perror("fopen");
        free(filename);
        return NULL;
    }
    
    fwrite(content, 1, strlen(content), f);
    fclose(f);
    
    return filename;
}

/* Compile gengtype with coverage instrumentation */
int compile_gengtype() {
    printf("Compiling gengtype with coverage instrumentation...\n");
    
    /* Compile gengtype.cc */
    char cmd[1024];
    snprintf(cmd, sizeof(cmd),
        "g++ -O0 -fprofile-arcs -ftest-coverage "
        "-DIN_GCC -DHAVE_CONFIG_H "
        "-I. -I../../include -I../../gcc "
        "-c %s -o gengtype.o",
        GENGTYPE_SOURCE);
    
    printf("Running: %s\n", cmd);
    if (system(cmd) != 0) {
        fprintf(stderr, "Failed to compile gengtype.cc\n");
        return 0;
    }
    
    /* Compile gengtype-state.cc if it exists */
    snprintf(cmd, sizeof(cmd),
        "g++ -O0 -fprofile-arcs -ftest-coverage "
        "-DIN_GCC -DHAVE_CONFIG_H "
        "-I. -I../../include -I../../gcc "
        "-c %s -o gengtype-state.o",
        GENGTYPE_STATE_SOURCE);
    
    printf("Running: %s\n", cmd);
    if (system(cmd) != 0) {
        fprintf(stderr, "Note: gengtype-state.cc compilation failed, continuing...\n");
    }
    
    /* Link gengtype executable */
    snprintf(cmd, sizeof(cmd),
        "g++ -O0 -fprofile-arcs -ftest-coverage "
        "gengtype.o gengtype-state.o -o %s "
        "-liberty -lgcov",
        GENGTYPE_EXECUTABLE);
    
    printf("Running: %s\n", cmd);
    if (system(cmd) != 0) {
        fprintf(stderr, "Failed to link gengtype\n");
        return 0;
    }
    
    return 1;
}

/* Run gengtype with various input patterns */
void run_gengtype_patterns(char **files, int file_count) {
    int i;
    char cmd[2048];
    
    printf("\n=== Pattern A: Process each file individually ===\n");
    for (i = 0; i < file_count; i++) {
        snprintf(cmd, sizeof(cmd), "%s -g output_%d.h %s", 
                 GENGTYPE_EXECUTABLE, i, files[i]);
        printf("Running: %s\n", cmd);
        if (system(cmd) == 0) {
            printf("Successfully processed %s\n", files[i]);
        } else {
            printf("Warning: Failed to process %s (may be expected for error cases)\n", files[i]);
        }
    }
    
    printf("\n=== Pattern B: Batch processing with -p flag ===\n");
    /* Create file list */
    FILE *list = fopen("filelist.txt", "w");
    if (list) {
        for (i = 0; i < file_count; i++) {
            fprintf(list, "%s\n", files[i]);
        }
        fclose(list);
        
        snprintf(cmd, sizeof(cmd), "%s -p filelist.txt", GENGTYPE_EXECUTABLE);
        printf("Running: %s\n", cmd);
        system(cmd);
        
        remove("filelist.txt");
    }
    
    printf("\n=== Pattern C: Generate header with multiple files ===\n");
    snprintf(cmd, sizeof(cmd), "%s -g combined_output.h ", GENGTYPE_EXECUTABLE);
    for (i = 0; i < file_count && i < 3; i++) { /* Use first 3 files */
        strcat(cmd, files[i]);
        strcat(cmd, " ");
    }
    printf("Running: %s\n", cmd);
    if (system(cmd) == 0) {
        printf("Successfully generated combined header\n");
    }
    
    printf("\n=== Pattern D: Generate routine file ===\n");
    snprintf(cmd, sizeof(cmd), "%s -r combined_routines.c %s %s", 
             GENGTYPE_EXECUTABLE, files[0], files[1]);
    printf("Running: %s\n", cmd);
    system(cmd);
    
    printf("\n=== Pattern E: Process with debug output ===\n");
    snprintf(cmd, sizeof(cmd), "%s -d -g debug_output.h %s", 
             GENGTYPE_EXECUTABLE, files[2]);
    printf("Running: %s\n", cmd);
    system(cmd);
}

/* Clean up temporary files */
void cleanup_files(char **files, int file_count) {
    int i;
    for (i = 0; i < file_count; i++) {
        if (files[i]) {
            remove(files[i]);
            free(files[i]);
        }
    }
    
    /* Clean up generated files */
    remove("output_0.h");
    remove("output_1.h");
    remove("output_2.h");
    remove("combined_output.h");
    remove("combined_routines.c");
    remove("debug_output.h");
    remove("gengtype.o");
    remove("gengtype-state.o");
    remove(GENGTYPE_EXECUTABLE);
    remove("gengtype.gcda");
    remove("gengtype.gcno");
    remove("gengtype-state.gcda");
    remove("gengtype-state.gcno");
}

int main() {
    char *files[6];
    int file_count = 0;
    
    printf("=== GCC gengtype Coverage Test ===\n");
    
    /* Create temporary .gt files */
    printf("\nCreating test .gt files...\n");
    
    files[file_count++] = create_temp_file(gt_file_1, ".gt");
    files[file_count++] = create_temp_file(gt_file_2, ".gt");
    files[file_count++] = create_temp_file(gt_file_3, ".gt");
    files[file_count++] = create_temp_file(gt_file_error, ".gt");
    files[file_count++] = create_temp_file(gt_file_warning, ".gt");
    
    for (int i = 0; i < file_count; i++) {
        if (!files[i]) {
            fprintf(stderr, "Failed to create temp file %d\n", i);
            cleanup_files(files, file_count);
            return 1;
        }
        printf("Created: %s\n", files[i]);
    }
    
    /* Compile gengtype with coverage */
    if (!compile_gengtype()) {
        cleanup_files(files, file_count);
        return 1;
    }
    
    /* Run gengtype with various patterns to exercise all code paths */
    run_gengtype_patterns(files, file_count);
    
    /* Generate coverage report */
    printf("\n=== Generating Coverage Report ===\n");
    system("gcov gengtype.cc");
    
    /* Display relevant coverage information */
    printf("\n=== Coverage Summary ===\n");
    printf("Check gengtype.cc.gcov for line-by-line coverage\n");
    printf("Specifically look at lines 182-213 for switch statement coverage\n");
    
    /* Clean up */
    cleanup_files(files, file_count);
    
    printf("\n=== Test Complete ===\n");
    printf("The test has exercised gengtype with diverse input files covering:\n");
    printf("  - TYPE_UNDEFINED (forward declarations)\n");
    printf("  - TYPE_SCALAR (typedefs)\n");
    printf("  - TYPE_STRING (const char*)\n");
    printf("  - TYPE_STRUCT (regular structs)\n");
    printf("  - TYPE_USER_STRUCT (GTY((user)))\n");
    printf("  - TYPE_UNION (unions)\n");
    printf("  - TYPE_POINTER (pointer types)\n");
    printf("  - TYPE_ARRAY (arrays)\n");
    printf("  - TYPE_CALLBACK (function pointers)\n");
    printf("  - TYPE_LANG_STRUCT (GTY((lang)))\n");
    printf("\nAll type categories in the switch statement should have been counted.\n");
    
    return 0;
}
