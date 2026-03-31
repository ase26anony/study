/* driver.c - Test driver for gengtype coverage of type counting switch */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>

/* Define required macros for gengtype compilation */
#ifndef IN_GCC
#define IN_GCC
#endif

#ifndef HAVE_CONFIG_H
#define HAVE_CONFIG_H
#endif

/* Forward declarations for gengtype functions we'll compile */
extern int main(int argc, char **argv);

/* Helper to write a string to a temporary file */
static char *write_temp_file(const char *prefix, const char *content) {
    char template[256];
    snprintf(template, sizeof(template), "/tmp/%s_XXXXXX", prefix);
    int fd = mkstemp(template);
    if (fd < 0) {
        perror("mkstemp");
        return NULL;
    }
    write(fd, content, strlen(content));
    close(fd);
    return strdup(template);
}

/* Compile gengtype with coverage instrumentation */
static int compile_gengtype_with_coverage() {
    const char *source_files[] = {
        "gengtype.cc",
        "gengtype-state.cc",
        "gengtype-lex.cc",
        "gengtype-parse.cc",
        "gengtype-c.cc",
        NULL
    };
    
    const char *compile_cmd = 
        "g++ -O0 -fprofile-arcs -ftest-coverage "
        "-DIN_GCC -DHAVE_CONFIG_H "
        "-I. -I../../include -I../../gcc "
        "-c gengtype.cc gengtype-state.cc gengtype-lex.cc "
        "gengtype-parse.cc gengtype-c.cc "
        "2>&1";
    
    printf("Compiling gengtype with: %s\n", compile_cmd);
    return system(compile_cmd);
}

/* Link gengtype executable */
static int link_gengtype() {
    const char *link_cmd = 
        "g++ -O0 -fprofile-arcs -ftest-coverage "
        "gengtype.o gengtype-state.o gengtype-lex.o "
        "gengtype-parse.o gengtype-c.o "
        "-o gengtype_coverage -liberty -lgcov "
        "2>&1";
    
    printf("Linking gengtype: %s\n", link_cmd);
    return system(link_cmd);
}

/* Generate diverse .gt files covering all type categories */
static char **generate_gt_files(int *count) {
    /* File 1: Basic types and undefined forward declaration */
    const char *gt1 = 
        "%{\n"
        "/* Test file 1: Basic types and undefined */\n"
        "#include \"config.h\"\n"
        "#include \"system.h\"\n"
        "%}\n"
        "\n"
        "/* TYPE_UNDEFINED: Forward declaration */\n"
        "struct undefined_struct;\n"
        "\n"
        "/* TYPE_SCALAR: Scalar typedefs */\n"
        "typedef int my_scalar;\n"
        "typedef unsigned long my_ulong;\n"
        "\n"
        "/* TYPE_STRING: String type */\n"
        "struct string_struct {\n"
        "  const char *name;  /* TYPE_STRING */\n"
        "  int id;\n"
        "};\n"
        "\n"
        "/* TYPE_POINTER: Pointer types */\n"
        "typedef struct string_struct *string_ptr;\n"
        "typedef my_scalar *scalar_ptr;\n"
        "%}\n";
    
    /* File 2: Structs, unions, and arrays */
    const char *gt2 =
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
        "  struct my_struct *next;  /* Nested pointer */\n"
        "};\n"
        "\n"
        "/* TYPE_USER_STRUCT: Struct with user marking */\n"
        "struct user_struct {\n"
        "  int *p;\n"
        "  char *data;\n"
        "} GTY((user));\n"
        "\n"
        "/* TYPE_UNION: Union type */\n"
        "union my_union {\n"
        "  int i;\n"
        "  void *p;\n"
        "  double d;\n"
        "};\n"
        "\n"
        "/* TYPE_ARRAY: Array types */\n"
        "typedef int my_array[10];\n"
        "typedef struct my_struct struct_array[5];\n"
        "\n"
        "/* Complex nested type: struct containing pointer to union of arrays */\n"
        "struct complex_nested {\n"
        "  union {\n"
        "    my_array arr;\n"
        "    struct_array sarr;\n"
        "  } data;\n"
        "  struct complex_nested **ptr_ptr;  /* Double pointer */\n"
        "};\n"
        "%}\n";
    
    /* File 3: Callbacks, lang structs, and more complex types */
    const char *gt3 =
        "%{\n"
        "/* Test file 3: Callbacks and language-specific types */\n"
        "#include \"config.h\"\n"
        "#include \"system.h\"\n"
        "%}\n"
        "\n"
        "/* TYPE_CALLBACK: Callback function type */\n"
        "typedef void (*callback_fn)(void);\n"
        "typedef int (*compare_fn)(const void *, const void *);\n"
        "\n"
        "/* TYPE_LANG_STRUCT: Language-specific struct */\n"
        "struct lang_struct {\n"
        "  int data;\n"
        "  void *extra;\n"
        "} GTY ((lang));\n"
        "\n"
        "/* More TYPE_USER_STRUCT variations */\n"
        "struct another_user_struct {\n"
        "  callback_fn handler;\n"
        "  struct lang_struct *lang_ptr;\n"
        "} GTY((user));\n"
        "\n"
        "/* Array of pointers to callbacks */\n"
        "typedef callback_fn callback_array[8];\n"
        "\n"
        "/* Struct mixing all categories */\n"
        "struct kitchen_sink {\n"
        "  my_scalar scalar_field;          /* TYPE_SCALAR */\n"
        "  const char *string_field;        /* TYPE_STRING */\n"
        "  struct my_struct *struct_ptr;    /* TYPE_POINTER to TYPE_STRUCT */\n"
        "  union my_union union_field;      /* TYPE_UNION */\n"
        "  my_array array_field;            /* TYPE_ARRAY */\n"
        "  callback_fn callback_field;      /* TYPE_CALLBACK */\n"
        "  struct lang_struct lang_field;   /* TYPE_LANG_STRUCT */\n"
        "};\n"
        "%}\n";
    
    /* File 4: File with syntax error to test error paths */
    const char *gt4 =
        "%{\n"
        "/* Test file 4: File with issues */\n"
        "#include \"config.h\"\n"
        "#include \"system.h\"\n"
        "%}\n"
        "\n"
        "/* Missing closing %} to trigger error */\n"
        "struct error_struct {\n"
        "  int x;\n";
    
    /* File 5: Duplicate definitions for warning testing */
    const char *gt5 =
        "%{\n"
        "/* Test file 5: Duplicate definitions */\n"
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
        "%}\n";
    
    static const char *contents[] = {gt1, gt2, gt3, gt4, gt5};
    static char *filenames[5];
    
    for (int i = 0; i < 5; i++) {
        char prefix[20];
        snprintf(prefix, sizeof(prefix), "gtype_test%d", i + 1);
        filenames[i] = write_temp_file(prefix, contents[i]);
        if (!filenames[i]) {
            fprintf(stderr, "Failed to create temp file %d\n", i + 1);
            for (int j = 0; j < i; j++) {
                unlink(filenames[j]);
                free(filenames[j]);
            }
            return NULL;
        }
        printf("Created temp file: %s\n", filenames[i]);
    }
    
    *count = 5;
    return filenames;
}

/* Run gengtype with various patterns to trigger type counting */
static int run_gengtype_patterns(char **filenames, int file_count) {
    int status;
    char cmd[1024];
    
    /* Pattern A: Process each file individually */
    printf("\n=== Pattern A: Individual file processing ===\n");
    for (int i = 0; i < file_count; i++) {
        snprintf(cmd, sizeof(cmd), 
                 "./gengtype_coverage -g output%d.h %s 2>&1", 
                 i + 1, filenames[i]);
        printf("Running: %s\n", cmd);
        status = system(cmd);
        printf("Exit status: %d\n", WEXITSTATUS(status));
        
        /* Clean up output files */
        char output_file[256];
        snprintf(output_file, sizeof(output_file), "output%d.h", i + 1);
        unlink(output_file);
    }
    
    /* Pattern B: Batch processing with -p flag */
    printf("\n=== Pattern B: Batch processing with -p ===\n");
    FILE *filelist = fopen("gt_filelist.txt", "w");
    if (filelist) {
        for (int i = 0; i < file_count; i++) {
            fprintf(filelist, "%s\n", filenames[i]);
        }
        fclose(filelist);
        
        snprintf(cmd, sizeof(cmd), 
                 "./gengtype_coverage -p gt_filelist.txt 2>&1");
        printf("Running: %s\n", cmd);
        status = system(cmd);
        printf("Exit status: %d\n", WEXITSTATUS(status));
        
        unlink("gt_filelist.txt");
    }
    
    /* Pattern C: Multiple files with header generation */
    printf("\n=== Pattern C: Multiple files, generate header ===\n");
    snprintf(cmd, sizeof(cmd), 
             "./gengtype_coverage -g combined.h %s %s %s 2>&1",
             filenames[0], filenames[1], filenames[2]);
    printf("Running: %s\n", cmd);
    status = system(cmd);
    printf("Exit status: %d\n", WEXITSTATUS(status));
    
    /* Also generate routines file */
    snprintf(cmd, sizeof(cmd), 
             "./gengtype_coverage -r combined.c %s %s %s 2>&1",
             filenames[0], filenames[1], filenames[2]);
    printf("Running: %s\n", cmd);
    status = system(cmd);
    printf("Exit status: %d\n", WEXITSTATUS(status));
    
    /* Clean up */
    unlink("combined.h");
    unlink("combined.c");
    
    /* Pattern D: Error cases */
    printf("\n=== Pattern D: Error and warning cases ===\n");
    snprintf(cmd, sizeof(cmd), 
             "./gengtype_coverage -g error.h %s 2>&1", filenames[3]);
    printf("Running (should error): %s\n", cmd);
    status = system(cmd);
    printf("Exit status: %d\n", WEXITSTATUS(status));
    
    snprintf(cmd, sizeof(cmd), 
             "./gengtype_coverage -g warning.h %s 2>&1", filenames[4]);
    printf("Running (should warn): %s\n", cmd);
    status = system(cmd);
    printf("Exit status: %d\n", WEXITSTATUS(status));
    
    unlink("error.h");
    unlink("warning.h");
    
    return 0;
}

/* Main test driver */
int main(int argc, char **argv) {
    printf("=== gengtype Type Counting Switch Coverage Test ===\n");
    
    /* Step 1: Compile gengtype with coverage */
    printf("\nStep 1: Compiling gengtype with coverage instrumentation\n");
    if (compile_gengtype_with_coverage() != 0) {
        fprintf(stderr, "Failed to compile gengtype sources\n");
        return 1;
    }
    
    if (link_gengtype() != 0) {
        fprintf(stderr, "Failed to link gengtype executable\n");
        return 1;
    }
    
    /* Step 2: Generate test .gt files */
    printf("\nStep 2: Generating test .gt files\n");
    int file_count;
    char **filenames = generate_gt_files(&file_count);
    if (!filenames) {
        fprintf(stderr, "Failed to generate test files\n");
        return 1;
    }
    
    /* Step 3: Run gengtype with various patterns */
    printf("\nStep 3: Running gengtype with different patterns\n");
    if (run_gengtype_patterns(filenames, file_count) != 0) {
        fprintf(stderr, "Failed to run gengtype patterns\n");
    }
    
    /* Step 4: Cleanup */
    printf("\nStep 4: Cleaning up\n");
    for (int i = 0; i < file_count; i++) {
        if (filenames[i]) {
            unlink(filenames[i]);
            free(filenames[i]);
        }
    }
    
    /* Step 5: Generate coverage report */
    printf("\nStep 5: Generating coverage report\n");
    system("gcov gengtype.cc gengtype-state.cc 2>&1 | grep -A 20 'gengtype.cc'");
    
    printf("\n=== Test Complete ===\n");
    printf("Check gengtype.c.gcov for line-by-line coverage\n");
    printf("The switch at lines 182-213 should show execution counts > 0\n");
    
    return 0;
}
