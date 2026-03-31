/* test_gengtype_coverage.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>

/* Compile gengtype with coverage instrumentation */
#define COMPILE_GENGTYPE 1

/* Temporary file management */
typedef struct {
    char *filename;
    char *content;
} temp_file_t;

/* Generated .gt files covering all type categories */
static const char *gt_files[] = {
    /* File 1: Basic types and structs */
    "test_types1.gt",
    "test_types2.gt", 
    "test_types3.gt",
    "test_error.gt",  /* File with syntax error */
    "test_dup.gt"     /* File with duplicate definitions */
};

/* Content for each .gt file */
static const char *gt_contents[] = {
    /* test_types1.gt - Basic scalar, struct, union, pointer types */
    "%{\n"
    "/* Test file covering TYPE_SCALAR, TYPE_STRUCT, TYPE_UNION, TYPE_POINTER */\n"
    "#include \"config.h\"\n"
    "#include \"system.h\"\n"
    "%}\n"
    "\n"
    "/* TYPE_UNDEFINED - forward declaration */\n"
    "struct undefined_struct;\n"
    "\n"
    "/* TYPE_SCALAR */\n"
    "typedef int my_scalar;\n"
    "typedef unsigned long scalar2;\n"
    "\n"
    "/* TYPE_STRUCT */\n"
    "struct my_struct {\n"
    "    int a;\n"
    "    double b;\n"
    "};\n"
    "\n"
    "/* TYPE_UNION */\n"
    "union my_union {\n"
    "    int i;\n"
    "    void *p;\n"
    "    struct my_struct *s;\n"
    "};\n"
    "\n"
    "/* TYPE_POINTER */\n"
    "typedef struct my_struct *my_ptr;\n"
    "typedef union my_union *union_ptr;\n"
    "\n"
    "/* TYPE_ARRAY */\n"
    "typedef int my_array[10];\n"
    "typedef struct my_struct struct_array[5];\n"
    "\n"
    "/* TYPE_STRING */\n"
    "struct string_struct {\n"
    "    const char *name;  /* string type */\n"
    "    char *data;\n"
    "};\n"
    "\n"
    "/* TYPE_CALLBACK */\n"
    "typedef void (*callback_fn)(void);\n"
    "typedef int (*compare_fn)(const void *, const void *);\n"
    "%}\n",

    /* test_types2.gt - User structs, lang structs, nested types */
    "%{\n"
    "/* Test file covering TYPE_USER_STRUCT, TYPE_LANG_STRUCT, complex nested types */\n"
    "#include \"config.h\"\n"
    "#include \"system.h\"\n"
    "%}\n"
    "\n"
    "/* TYPE_USER_STRUCT - with user-provided marking routine */\n"
    "struct user_struct {\n"
    "    int *p;\n"
    "    void **data;\n"
    "} GTY((user));\n"
    "\n"
    "/* TYPE_LANG_STRUCT - language-specific struct */\n"
    "struct lang_struct {\n"
    "    int data;\n"
    "    void *lang_data;\n"
    "} GTY((lang));\n"
    "\n"
    "/* Complex nested type combining multiple categories */\n"
    "struct complex_nested {\n"
    "    /* Pointer to union containing arrays */\n"
    "    union {\n"
    "        int int_array[20];\n"
    "        struct user_struct *ptr_array[10];\n"
    "    } *union_ptr;\n"
    "    \n"
    "    /* Array of callback pointers */\n"
    "    void (*callbacks[5])(struct complex_nested *);\n"
    "    \n"
    "    /* String member */\n"
    "    const char *description;\n"
    "    \n"
    "    /* Nested struct with pointer */\n"
    "    struct {\n"
    "        struct lang_struct *lang;\n"
    "        my_scalar count;  /* Using scalar from first file */\n"
    "    } nested;\n"
    "};\n"
    "\n"
    "/* Another user struct with complex members */\n"
    "struct user_complex {\n"
    "    struct complex_nested **nested_pp;\n"
    "    union my_union u_array[3];\n"
    "} GTY((user));\n"
    "%}\n",

    /* test_types3.gt - More arrays, pointers, and callback variations */
    "%{\n"
    "/* Test file covering additional TYPE_ARRAY, TYPE_CALLBACK variations */\n"
    "#include \"config.h\"\n"
    "#include \"system.h\"\n"
    "%}\n"
    "\n"
    "/* Multi-dimensional arrays */\n"
    "typedef int matrix[10][10];\n"
    "typedef struct user_struct *ptr_matrix[5][5];\n"
    "\n"
    "/* Array of strings */\n"
    "typedef const char *string_array[50];\n"
    "\n"
    "/* Pointer to array */\n"
    "typedef int (*array_ptr)[10];\n"
    "\n"
    "/* Complex callback with parameters */\n"
    "typedef struct complex_nested *(*factory_fn)(int size, const char *name);\n"
    "\n"
    "/* Struct with all type kinds */\n"
    "struct kitchen_sink {\n"
    "    my_scalar scalar_field;          /* TYPE_SCALAR */\n"
    "    const char *string_field;        /* TYPE_STRING */\n"
    "    struct my_struct struct_field;   /* TYPE_STRUCT */\n"
    "    union my_union union_field;      /* TYPE_UNION */\n"
    "    struct user_struct *user_ptr;    /* TYPE_POINTER to TYPE_USER_STRUCT */\n"
    "    int array_field[20];             /* TYPE_ARRAY */\n"
    "    callback_fn callback_field;      /* TYPE_CALLBACK */\n"
    "    struct lang_struct lang_field;   /* TYPE_LANG_STRUCT */\n"
    "};\n"
    "\n"
    "/* Forward declaration for undefined struct */\n"
    "struct another_undefined;\n"
    "%}\n",

    /* test_error.gt - File with syntax error to test error paths */
    "%{\n"
    "/* This file has a syntax error - missing closing %}\n"
    "#include \"config.h\"\n"
    "#include \"system.h\"\n"
    "\n"
    "struct error_struct {\n"
    "    int x;\n"
    "    /* Missing closing brace and %}\n",

    /* test_dup.gt - File with duplicate definitions to test warning paths */
    "%{\n"
    "/* File with duplicate type definitions */\n"
    "#include \"config.h\"\n"
    "#include \"system.h\"\n"
    "%}\n"
    "\n"
    "/* Duplicate struct definition */\n"
    "struct dup_struct {\n"
    "    int a;\n"
    "};\n"
    "\n"
    "struct dup_struct {\n"
    "    int b;  /* This should trigger a warning */\n"
    "};\n"
    "\n"
    "/* Duplicate typedef */\n"
    "typedef int my_int;\n"
    "typedef int my_int;  /* Duplicate typedef */\n"
    "%}\n"
};

/* Create a temporary file with given content */
static char *create_temp_file(const char *content, const char *suffix) {
    char template[] = "/tmp/gengtype_test_XXXXXX";
    int fd = mkstemp(template);
    if (fd == -1) {
        perror("mkstemp");
        return NULL;
    }
    
    /* Write content */
    if (write(fd, content, strlen(content)) != (ssize_t)strlen(content)) {
        perror("write");
        close(fd);
        return NULL;
    }
    
    close(fd);
    
    /* Rename with proper suffix */
    char *new_name = malloc(strlen(template) + strlen(suffix) + 1);
    if (!new_name) {
        unlink(template);
        return NULL;
    }
    sprintf(new_name, "%s%s", template, suffix);
    
    if (rename(template, new_name) == -1) {
        perror("rename");
        free(new_name);
        unlink(template);
        return NULL;
    }
    
    return new_name;
}

/* Compile gengtype with coverage instrumentation */
static int compile_gengtype(void) {
    printf("Compiling gengtype with coverage instrumentation...\n");
    
    /* Compile gengtype.cc */
    const char *compile_cmd = 
        "g++ -O0 -fprofile-arcs -ftest-coverage "
        "-DIN_GCC -DHAVE_CONFIG_H "
        "-I. -I../../include -I../../gcc "
        "-c gengtype.cc -o gengtype_coverage.o 2>&1";
    
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
        "-c gengtype-state.cc -o gengtype_state_coverage.o 2>&1";
    
    printf("Running: %s\n", compile_state_cmd);
    if (system(compile_state_cmd) != 0) {
        fprintf(stderr, "Failed to compile gengtype-state.cc\n");
        return 0;
    }
    
    /* Link gengtype executable */
    const char *link_cmd = 
        "g++ -O0 -fprofile-arcs -ftest-coverage "
        "gengtype_coverage.o gengtype_state_coverage.o "
        "-liberty -lgcov -o gengtype_coverage 2>&1";
    
    printf("Running: %s\n", link_cmd);
    if (system(link_cmd) != 0) {
        fprintf(stderr, "Failed to link gengtype\n");
        return 0;
    }
    
    printf("Successfully compiled gengtype_coverage\n");
    return 1;
}

/* Run gengtype on a single file */
static int run_gengtype_single(const char *gt_file, const char *output_suffix) {
    char cmd[1024];
    int status;
    
    /* Pattern C: Generate header output to force full parsing */
    snprintf(cmd, sizeof(cmd), 
             "./gengtype_coverage -g output%s.h %s 2>&1", 
             output_suffix, gt_file);
    
    printf("\nRunning gengtype on %s:\n", gt_file);
    printf("Command: %s\n", cmd);
    
    status = system(cmd);
    
    if (WIFEXITED(status)) {
        printf("Exit status: %d\n", WEXITSTATUS(status));
    }
    
    /* Check if output was generated */
    char output_file[256];
    snprintf(output_file, sizeof(output_file), "output%s.h", output_suffix);
    struct stat st;
    if (stat(output_file, &st) == 0) {
        printf("Output file %s created (%ld bytes)\n", output_file, (long)st.st_size);
        /* Clean up output file */
        unlink(output_file);
    }
    
    return 1;
}

/* Run gengtype with file list (Pattern B) */
static int run_gengtype_batch(char **gt_files, int count) {
    FILE *list_file = fopen("gt_file_list.txt", "w");
    if (!list_file) {
        perror("fopen");
        return 0;
    }
    
    /* Write all .gt files to list */
    for (int i = 0; i < count; i++) {
        fprintf(list_file, "%s\n", gt_files[i]);
    }
    fclose(list_file);
    
    /* Run gengtype with -p flag */
    printf("\nRunning gengtype in batch mode with -p flag:\n");
    const char *cmd = "./gengtype_coverage -p gt_file_list.txt -g batch_output.h 2>&1";
    printf("Command: %s\n", cmd);
    
    int status = system(cmd);
    
    if (WIFEXITED(status)) {
        printf("Batch processing exit status: %d\n", WEXITSTATUS(status));
    }
    
    /* Clean up */
    unlink("gt_file_list.txt");
    unlink("batch_output.h");
    
    return 1;
}

/* Run gengtype with multiple files directly (Pattern C) */
static int run_gengtype_multiple(char **gt_files, int count) {
    char cmd[4096] = "./gengtype_coverage -g multi_output.h";
    
    /* Build command with all .gt files */
    for (int i = 0; i < count; i++) {
        strcat(cmd, " ");
        strcat(cmd, gt_files[i]);
    }
    strcat(cmd, " 2>&1");
    
    printf("\nRunning gengtype with multiple input files:\n");
    printf("Command: %s\n", cmd);
    
    int status = system(cmd);
    
    if (WIFEXITED(status)) {
        printf("Multiple files exit status: %d\n", WEXITSTATUS(status));
    }
    
    /* Clean up */
    unlink("multi_output.h");
    
    return 1;
}

/* Main test driver */
int main(void) {
    char *temp_files[sizeof(gt_files)/sizeof(gt_files[0])];
    int num_files = sizeof(gt_files)/sizeof(gt_files[0]);
    int i;
    
    printf("=== Gengtype Coverage Test ===\n");
    
    /* Step 1: Create temporary .gt files */
    printf("\n1. Creating temporary .gt files...\n");
    for (i = 0; i < num_files; i++) {
        temp_files[i] = create_temp_file(gt_contents[i], ".gt");
        if (!temp_files[i]) {
            fprintf(stderr, "Failed to create temp file %d\n", i);
            /* Clean up already created files */
            for (int j = 0; j < i; j++) {
                unlink(temp_files[j]);
                free(temp_files[j]);
            }
            return 1;
        }
        printf("  Created: %s\n", temp_files[i]);
    }
    
    /* Step 2: Compile gengtype with coverage */
    if (COMPILE_GENGTYPE) {
        if (!compile_gengtype()) {
            fprintf(stderr, "Failed to compile gengtype\n");
            goto cleanup;
        }
    } else {
        printf("Skipping gengtype compilation (assuming pre-built)\n");
    }
    
    /* Step 3: Run gengtype in various modes to trigger all code paths */
    
    /* Pattern A: Process each file individually */
    printf("\n3. Processing files individually...\n");
    for (i = 0; i < num_files; i++) {
        char suffix[16];
        snprintf(suffix, sizeof(suffix), "_%d", i);
        run_gengtype_single(temp_files[i], suffix);
    }
    
    /* Pattern B: Batch processing with -p flag */
    printf("\n4. Batch processing with -p flag...\n");
    run_gengtype_batch(temp_files, num_files);
    
    /* Pattern C: Multiple files at once */
    printf("\n5. Processing all files at once...\n");
    run_gengtype_multiple(temp_files, num_files);
    
    /* Pattern D: Test with error flags */
    printf("\n6. Testing with various gengtype flags...\n");
    
    /* Test with -r flag (generate routines) */
    printf("\n  Testing with -r flag (generate routines):\n");
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), 
             "./gengtype_coverage -r output_routines.c %s 2>&1",
             temp_files[0]);  /* Use first valid file */
    system(cmd);
    unlink("output_routines.c");
    
    /* Test with both -g and -r flags */
    printf("\n  Testing with both -g and -r flags:\n");
    snprintf(cmd, sizeof(cmd), 
             "./gengtype_coverage -g output_both.h -r output_both.c %s %s 2>&1",
             temp_files[0], temp_files[1]);
    system(cmd);
    unlink("output_both.h");
    unlink("output_both.c");
    
    /* Test with debug output */
    printf("\n  Testing with debug output (if supported):\n");
    snprintf(cmd, sizeof(cmd), 
             "./gengtype_coverage -g output_debug.h -d %s 2>&1 | head -20",
             temp_files[0]);
    system(cmd);
    unlink("output_debug.h");
    
    /* Step 4: Generate coverage report */
    printf("\n7. Generating coverage report...\n");
    
    /* Run gcov on gengtype to see coverage */
    system("gcov gengtype_coverage.o 2>&1 | grep -A 20 'gengtype.cc'");
    
    /* Also check the state file coverage */
    system("gcov gengtype_state_coverage.o 2>&1 | head -10");
    
    /* Look specifically at lines around the switch statement */
    printf("\n8. Checking specific switch statement coverage...\n");
    system("gcov -b gengtype_coverage.o 2>&1 | "
           "grep -B5 -A5 'lines 182-213' || "
           "echo 'Could not find exact line range; checking gengtype.cc.gcov'");
    
    /* Create a simple coverage summary */
    printf("\n9. Coverage summary files created:\n");
    system("ls -la *.gcda *.gcno *.gcov 2>/dev/null | head -10");
    
    printf("\n=== Test Complete ===\n");
    printf("The switch statement in gengtype.cc (lines 182-213) should now be covered.\n");
    printf("Check gengtype.cc.gcov for detailed line-by-line coverage.\n");
    
cleanup:
    /* Step 5: Clean up temporary files */
    printf("\nCleaning up temporary files...\n");
    for (i = 0; i < num_files; i++) {
        if (temp_files[i]) {
            unlink(temp_files[i]);
            free(temp_files[i]);
        }
    }
    
    /* Clean up coverage files if desired */
    printf("\nTo keep coverage data, don't delete *.gcda files\n");
    printf("To clean everything: rm -f *.gcda *.gcno *.gcov gengtype_coverage *.o\n");
    
    return 0;
}
