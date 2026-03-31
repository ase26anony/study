/* test_gengtype_coverage.c - Driver program to test gengtype type counting */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>

/* gengtype source files needed for compilation */
#define GENGTYPE_SRC "gengtype.cc"
#define GENGTYPE_STATE_SRC "gengtype-state.cc"
#define GENGTYPE_HEADER_SRC "gengtype.h"

/* Temporary file management */
typedef struct {
    char *filename;
    char *content;
} temp_file_t;

/* Generated .gt files with diverse type definitions */
static const char *gt_files[] = {
    /* File 1: Basic types and structs */
    "test_types1.gt",
    /* File 2: Advanced types with callbacks and arrays */
    "test_types2.gt", 
    /* File 3: Language-specific and user structs */
    "test_types3.gt",
    /* File 4: With syntax error to test error paths */
    "test_error.gt",
    NULL
};

/* Content for each .gt file */
static const char *gt_contents[] = {
    /* File 1: Basic types covering most categories */
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
    "  const char *name;          /* TYPE_STRING */\n"
    "  char *data;\n"
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
    "typedef my_scalar *scalar_ptr;\n"
    "\n"
    "/* TYPE_ARRAY: Array types */\n"
    "typedef int my_array[10];\n"
    "typedef struct my_struct struct_array[5];\n"
    "\n"
    "/* TYPE_UNION: Union type */\n"
    "union my_union {\n"
    "  int i;\n"
    "  void *p;\n"
    "  my_array arr;\n"
    "};\n"
    "\n"
    "/* Nested complex type */\n"
    "struct complex_struct {\n"
    "  union my_union u;\n"
    "  my_ptr p;\n"
    "  my_array a;\n"
    "};\n"
    "%}",
    
    /* File 2: Advanced types with callbacks and user structs */
    "%{\n"
    "/* Test file 2: Advanced type definitions */\n"
    "#include \"config.h\"\n"
    "#include \"system.h\"\n"
    "%}\n"
    "\n"
    "/* TYPE_CALLBACK: Callback function type */\n"
    "typedef void (*callback_fn)(void);\n"
    "typedef int (*compare_fn)(const void *, const void *);\n"
    "\n"
    "/* TYPE_USER_STRUCT: Struct with user-provided marking */\n"
    "struct user_struct {\n"
    "  int *p;\n"
    "  void *data;\n"
    "} GTY((user));\n"
    "\n"
    "/* Another user struct with callback */\n"
    "struct user_with_callback {\n"
    "  callback_fn handler;\n"
    "  void *context;\n"
    "} GTY((user));\n"
    "\n"
    "/* Complex nested types */\n"
    "struct container {\n"
    "  /* Pointer to union containing array */\n"
    "  union my_union *union_ptr;\n"
    "  \n"
    "  /* Array of pointers */\n"
    "  struct user_struct *user_ptrs[5];\n"
    "  \n"
    "  /* Callback function pointer */\n"
    "  compare_fn comparator;\n"
    "  \n"
    "  /* Multi-dimensional array */\n"
    "  int matrix[3][3];\n"
    "};\n"
    "\n"
    "/* Pointer to array of callbacks */\n"
    "typedef callback_fn (*callback_array_ptr)[10];\n"
    "\n"
    "/* Union with struct and pointer */\n"
    "union complex_union {\n"
    "  struct container c;\n"
    "  callback_array_ptr cap;\n"
    "  struct user_struct **usp;\n"
    "};\n"
    "%}",
    
    /* File 3: Language-specific structs and edge cases */
    "%{\n"
    "/* Test file 3: Language-specific and edge cases */\n"
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
    "/* Another language struct with nested types */\n"
    "struct lang_container GTY((lang)) {\n"
    "  struct lang_struct *items;\n"
    "  int count;\n"
    "  callback_fn cleanup;\n"
    "};\n"
    "\n"
    "/* Mixed user and language structs */\n"
    "struct mixed_bag {\n"
    "  struct user_struct user;   /* TYPE_USER_STRUCT */\n"
    "  struct lang_struct lang;   /* TYPE_LANG_STRUCT */\n"
    "  struct my_struct regular;  /* TYPE_STRUCT */\n"
    "  union my_union u;          /* TYPE_UNION */\n"
    "};\n"
    "\n"
    "/* More pointer variations */\n"
    "typedef struct lang_struct **lang_ptr_ptr;\n"
    "typedef void (*lang_callback)(struct lang_struct *);\n"
    "\n"
    "/* Array of language structs */\n"
    "struct lang_struct lang_array[7] GTY((lang));\n"
    "\n"
    "/* Complex type combining everything */\n"
    "struct ultimate_type {\n"
    "  /* All major type categories */\n"
    "  my_scalar scalar;          /* TYPE_SCALAR */\n"
    "  const char *str;           /* TYPE_STRING */\n"
    "  struct my_struct s;        /* TYPE_STRUCT */\n"
    "  struct user_struct us;     /* TYPE_USER_STRUCT */\n"
    "  union my_union u;          /* TYPE_UNION */\n"
    "  struct my_struct *ptr;     /* TYPE_POINTER */\n"
    "  int arr[5];                /* TYPE_ARRAY */\n"
    "  callback_fn cb;            /* TYPE_CALLBACK */\n"
    "  struct lang_struct ls;     /* TYPE_LANG_STRUCT */\n"
    "};\n"
    "%}",
    
    /* File 4: File with syntax error to test error handling */
    "%{\n"
    "/* Test file with syntax error - missing closing %}\n"
    "#include \"config.h\"\n"
    "#include \"system.h\"\n"
    "\n"
    "struct error_struct {\n"
    "  int x;\n"
    "  /* Missing closing brace and %}\n",
    
    NULL
};

/* Create a temporary file with given content */
static char *create_temp_file(const char *content, const char *suffix) {
    char template[256];
    int fd;
    FILE *f;
    
    snprintf(template, sizeof(template), "/tmp/gengtype_test_XXXXXX%s", suffix);
    fd = mkstemps(template, strlen(suffix));
    if (fd < 0) {
        perror("mkstemps failed");
        return NULL;
    }
    
    f = fdopen(fd, "w");
    if (!f) {
        close(fd);
        return NULL;
    }
    
    fwrite(content, 1, strlen(content), f);
    fclose(f);
    
    return strdup(template);
}

/* Compile gengtype with coverage instrumentation */
static int compile_gengtype(void) {
    char cmd[1024];
    int status;
    
    printf("Compiling gengtype with coverage instrumentation...\n");
    
    /* Compile gengtype.cc */
    snprintf(cmd, sizeof(cmd),
        "g++ -O0 -fprofile-arcs -ftest-coverage "
        "-DIN_GCC -DHAVE_CONFIG_H "
        "-I. -I../../include -I../../gcc "
        "-c " GENGTYPE_SRC " -o gengtype.o");
    
    printf("Running: %s\n", cmd);
    status = system(cmd);
    if (status != 0) {
        fprintf(stderr, "Failed to compile gengtype.cc\n");
        return -1;
    }
    
    /* Compile gengtype-state.cc */
    snprintf(cmd, sizeof(cmd),
        "g++ -O0 -fprofile-arcs -ftest-coverage "
        "-DIN_GCC -DHAVE_CONFIG_H "
        "-I. -I../../include -I../../gcc "
        "-c gengtype-state.cc -o gengtype-state.o");
    
    printf("Running: %s\n", cmd);
    status = system(cmd);
    if (status != 0) {
        fprintf(stderr, "Failed to compile gengtype-state.cc\n");
        return -1;
    }
    
    /* Link gengtype executable */
    snprintf(cmd, sizeof(cmd),
        "g++ -O0 -fprofile-arcs -ftest-coverage "
        "gengtype.o gengtype-state.o "
        "-lgcov -liberty -o gengtype_test");
    
    printf("Running: %s\n", cmd);
    status = system(cmd);
    if (status != 0) {
        fprintf(stderr, "Failed to link gengtype\n");
        return -1;
    }
    
    return 0;
}

/* Run gengtype on a single file */
static int run_gengtype_on_file(const char *filename, const char *mode) {
    char cmd[1024];
    int status;
    
    if (strcmp(mode, "header") == 0) {
        /* Generate header file */
        char output_header[256];
        snprintf(output_header, sizeof(output_header), "%s.h", filename);
        
        snprintf(cmd, sizeof(cmd),
            "./gengtype_test -g %s %s",
            output_header, filename);
    } else if (strcmp(mode, "routine") == 0) {
        /* Generate routine file */
        char output_routine[256];
        snprintf(output_routine, sizeof(output_routine), "%s.c", filename);
        
        snprintf(cmd, sizeof(cmd),
            "./gengtype_test -r %s %s",
            output_routine, filename);
    } else if (strcmp(mode, "batch") == 0) {
        /* Batch process using -p flag */
        snprintf(cmd, sizeof(cmd),
            "./gengtype_test -p %s",
            filename);
    } else {
        /* Default: parse only */
        snprintf(cmd, sizeof(cmd),
            "./gengtype_test %s",
            filename);
    }
    
    printf("Running: %s\n", cmd);
    status = system(cmd);
    
    if (status != 0) {
        printf("Note: gengtype exited with status %d (expected for error tests)\n",
               WEXITSTATUS(status));
    }
    
    return status;
}

/* Create file list for batch processing */
static char *create_file_list(char **files, int count) {
    char template[256];
    int fd;
    FILE *f;
    int i;
    
    snprintf(template, sizeof(template), "/tmp/gengtype_filelist_XXXXXX.txt");
    fd = mkstemp(template);
    if (fd < 0) {
        perror("mkstemp failed");
        return NULL;
    }
    
    f = fdopen(fd, "w");
    if (!f) {
        close(fd);
        return NULL;
    }
    
    for (i = 0; i < count; i++) {
        fprintf(f, "%s\n", files[i]);
    }
    
    fclose(f);
    return strdup(template);
}

/* Main test driver */
int main(int argc, char **argv) {
    char **temp_files = NULL;
    char *file_list = NULL;
    int i, num_files = 0;
    int ret = 0;
    
    printf("=== Gengtype Coverage Test Driver ===\n\n");
    
    /* Count number of .gt files */
    while (gt_files[num_files] != NULL) {
        num_files++;
    }
    
    /* Allocate array for temporary file names */
    temp_files = malloc(num_files * sizeof(char *));
    if (!temp_files) {
        perror("malloc failed");
        return 1;
    }
    
    /* Create temporary .gt files */
    printf("Creating %d test .gt files...\n", num_files);
    for (i = 0; i < num_files; i++) {
        temp_files[i] = create_temp_file(gt_contents[i], ".gt");
        if (!temp_files[i]) {
            fprintf(stderr, "Failed to create temp file %s\n", gt_files[i]);
            ret = 1;
            goto cleanup;
        }
        printf("  Created: %s\n", temp_files[i]);
    }
    
    /* Compile gengtype with coverage */
    if (compile_gengtype() != 0) {
        fprintf(stderr, "Failed to compile gengtype\n");
        ret = 1;
        goto cleanup;
    }
    
    printf("\n=== Testing Pattern A: Individual File Processing ===\n");
    /* Process each file individually to ensure all types are encountered */
    for (i = 0; i < num_files; i++) {
        printf("\n--- Processing %s ---\n", temp_files[i]);
        
        /* Test 1: Parse only */
        printf("\nTest 1: Basic parsing\n");
        run_gengtype_on_file(temp_files[i], "parse");
        
        /* Test 2: Generate header (forces full type processing) */
        if (i < num_files - 1) { /* Skip error file for header generation */
            printf("\nTest 2: Header generation\n");
            run_gengtype_on_file(temp_files[i], "header");
        }
        
        /* Test 3: Generate routine file */
        if (i < num_files - 1) { /* Skip error file */
            printf("\nTest 3: Routine generation\n");
            run_gengtype_on_file(temp_files[i], "routine");
        }
    }
    
    printf("\n=== Testing Pattern B: Batch Processing ===\n");
    /* Create file list for batch processing */
    file_list = create_file_list(temp_files, num_files - 1); /* Exclude error file */
    if (file_list) {
        printf("\nBatch processing with file list: %s\n", file_list);
        run_gengtype_on_file(file_list, "batch");
    }
    
    printf("\n=== Testing Pattern C: Multiple Input Files ===\n");
    /* Process all valid files together */
    if (num_files >= 3) {
        char cmd[1024];
        snprintf(cmd, sizeof(cmd),
            "./gengtype_test -g output.h %s %s %s",
            temp_files[0], temp_files[1], temp_files[2]);
        
        printf("Running: %s\n", cmd);
        system(cmd);
    }
    
    printf("\n=== Testing Pattern D: Error Cases ===\n");
    /* Process the file with syntax error */
    printf("\nProcessing file with syntax error (should fail):\n");
    run_gengtype_on_file(temp_files[num_files - 1], "parse");
    
    printf("\n=== Coverage Analysis ===\n");
    /* Generate gcov report */
    system("gcov gengtype.cc");
    
    /* Display coverage for the target lines */
    printf("\nChecking coverage for target switch statement (lines 182-213):\n");
    system("gcov -n gengtype.cc | grep -A 30 '^182'");
    
    printf("\n=== Test Complete ===\n");
    
cleanup:
    /* Cleanup temporary files */
    printf("\nCleaning up temporary files...\n");
    for (i = 0; i < num_files; i++) {
        if (temp_files[i]) {
            unlink(temp_files[i]);
            free(temp_files[i]);
        }
    }
    
    if (file_list) {
        unlink(file_list);
        free(file_list);
    }
    
    /* Cleanup generated files */
    unlink("gengtype.o");
    unlink("gengtype-state.o");
    unlink("gengtype_test");
    unlink("output.h");
    unlink("output.c");
    
    free(temp_files);
    
    return ret;
}
