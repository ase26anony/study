/* driver.c - Test driver for gengtype coverage of type counting switch */
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

/* GT file definitions covering all type categories */
static const char *gt_files[] = {
    /* File 1: Basic types and structs */
    "test_types1.gt",
    "%%{\n"
    "/* Test file 1: Basic types */\n"
    "\n"
    "/* TYPE_UNDEFINED - forward declaration */\n"
    "struct undefined_struct;\n"
    "\n"
    "/* TYPE_SCALAR */\n"
    "typedef int my_scalar;\n"
    "typedef unsigned long scalar2;\n"
    "\n"
    "/* TYPE_STRING */\n"
    "struct string_struct {\n"
    "  const char *name;  /* TYPE_STRING */\n"
    "  char *data;\n"
    "};\n"
    "\n"
    "/* TYPE_STRUCT */\n"
    "struct my_struct {\n"
    "  int a;\n"
    "  float b;\n"
    "};\n"
    "\n"
    "/* TYPE_POINTER */\n"
    "typedef struct my_struct *struct_ptr;\n"
    "typedef int *int_ptr;\n"
    "\n"
    "/* TYPE_ARRAY */\n"
    "typedef int my_array[10];\n"
    "typedef struct my_struct struct_array[5];\n"
    "\n"
    "%%}\n",

    /* File 2: Unions, callbacks, and user structs */
    "test_types2.gt",
    "%%{\n"
    "/* Test file 2: Advanced types */\n"
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
    "/* TYPE_LANG_STRUCT */\n"
    "struct lang_struct {\n"
    "  int lang_data;\n"
    "  void *lang_ptr;\n"
    "} GTY((lang));\n"
    "\n"
    "/* Complex nested type combining multiple categories */\n"
    "struct complex_type {\n"
    "  union my_union u;           /* TYPE_UNION */\n"
    "  callback_fn handler;        /* TYPE_CALLBACK */\n"
    "  struct user_struct *usr;    /* TYPE_POINTER to TYPE_USER_STRUCT */\n"
    "  int array[20];              /* TYPE_ARRAY */\n"
    "  const char *description;    /* TYPE_STRING */\n"
    "};\n"
    "\n"
    "/* Another pointer type */\n"
    "typedef union my_union *union_ptr;\n"
    "\n"
    "%%}\n",

    /* File 3: More complex nested structures and error cases */
    "test_types3.gt",
    "%%{\n"
    "/* Test file 3: Complex nested types */\n"
    "\n"
    "/* Forward declarations (TYPE_UNDEFINED) */\n"
    "struct forward1;\n"
    "struct forward2;\n"
    "\n"
    "/* Struct containing pointer to array of callbacks */\n"
    "struct nested_callback {\n"
    "  void (*callbacks[10])(void);  /* TYPE_ARRAY of TYPE_CALLBACK */\n"
    "  int count;\n"
    "};\n"
    "\n"
    "/* Union containing struct with string */\n"
    "union complex_union {\n"
    "  struct {\n"
    "    const char *str;\n"
    "    int len;\n"
    "  } string_part;\n"
    "  struct {\n"
    "    int *numbers;\n"
    "    int size;\n"
    "  } array_part;\n"
    "};\n"
    "\n"
    "/* Multiple scalar types */\n"
    "typedef char byte;\n"
    "typedef short int16;\n"
    "typedef long long int64;\n"
    "\n"
    "/* Pointer to pointer */\n"
    "typedef int **double_ptr;\n"
    "\n"
    "/* Array of pointers to structs */\n"
    "struct node {\n"
    "  int value;\n"
    "  struct node *children[8];\n"
    "};\n"
    "\n"
    "/* Mixed types in one struct */\n"
    "struct mixed {\n"
    "  byte b;                    /* TYPE_SCALAR */\n"
    "  const char *name;          /* TYPE_STRING */\n"
    "  struct node *root;         /* TYPE_POINTER */\n"
    "  union complex_union data;  /* TYPE_UNION */\n"
    "  void (*cleanup)(void);     /* TYPE_CALLBACK */\n"
    "};\n"
    "\n"
    "%%}\n",

    /* File 4: File with syntax error to test error paths */
    "test_error.gt",
    "%%{\n"
    "/* This file has a syntax error - missing closing %%} */\n"
    "\n"
    "struct error_struct {\n"
    "  int x;\n"
    "  int y;\n"
    "};\n"
    "\n"
    "/* Missing closing delimiter */\n",

    /* File 5: Duplicate definitions for warning testing */
    "test_dup.gt",
    "%%{\n"
    "/* File with duplicate definitions */\n"
    "\n"
    "struct duplicate {\n"
    "  int a;\n"
    "};\n"
    "\n"
    "/* Duplicate definition */\n"
    "struct duplicate {\n"
    "  int b;\n"
    "};\n"
    "\n"
    "%%}\n",

    NULL, NULL  /* Terminator */
};

/* Create temporary file with given content */
static char *create_temp_file(const char *filename, const char *content) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        perror("fopen");
        return NULL;
    }
    
    /* Replace %% with % for actual gengtype syntax */
    const char *src = content;
    while (*src) {
        if (src[0] == '%' && src[1] == '%') {
            fputc('%', f);
            src += 2;
        } else {
            fputc(*src, f);
            src++;
        }
    }
    
    fclose(f);
    return strdup(filename);
}

/* Build gengtype with coverage instrumentation */
static int build_gengtype(void) {
    printf("Building gengtype with coverage instrumentation...\n");
    
    /* Compile gengtype.cc */
    const char *compile_cmd = 
        "g++ -O0 -fprofile-arcs -ftest-coverage "
        "-DIN_GCC -DHAVE_CONFIG_H "
        "-I. -I../../include -I../../gcc "
        "-c gengtype.cc -o gengtype_coverage.o 2>&1";
    
    printf("Compiling: %s\n", compile_cmd);
    if (system(compile_cmd) != 0) {
        fprintf(stderr, "Failed to compile gengtype.cc\n");
        return 0;
    }
    
    /* Compile gengtype-state.cc */
    const char *compile_state_cmd = 
        "g++ -O0 -fprofile-arcs -ftest-coverage "
        "-DIN_GCC -DHAVE_CONFIG_H "
        "-I. -I../../include -I../../gcc "
        "-c gengtype-state.cc -o gengtype-state_coverage.o 2>&1";
    
    printf("Compiling: %s\n", compile_state_cmd);
    if (system(compile_state_cmd) != 0) {
        fprintf(stderr, "Failed to compile gengtype-state.cc\n");
        return 0;
    }
    
    /* Link gengtype executable */
    const char *link_cmd = 
        "g++ -O0 -fprofile-arcs -ftest-coverage "
        "gengtype_coverage.o gengtype-state_coverage.o "
        "-lgcov -liberty -o gengtype_coverage 2>&1";
    
    printf("Linking: %s\n", link_cmd);
    if (system(link_cmd) != 0) {
        fprintf(stderr, "Failed to link gengtype\n");
        return 0;
    }
    
    printf("gengtype built successfully as 'gengtype_coverage'\n");
    return 1;
}

/* Run gengtype on a single file */
static int run_gengtype_single(const char *gt_file, const char *output_base) {
    char cmd[1024];
    int status;
    
    /* Pattern A: Process single file with header generation */
    snprintf(cmd, sizeof(cmd), 
             "./gengtype_coverage -g %s.h %s 2>&1",
             output_base, gt_file);
    
    printf("Running: %s\n", cmd);
    status = system(cmd);
    
    if (WIFEXITED(status)) {
        printf("Exit status: %d\n", WEXITSTATUS(status));
    }
    
    /* Check if output was generated */
    char header_file[256];
    snprintf(header_file, sizeof(header_file), "%s.h", output_base);
    struct stat st;
    if (stat(header_file, &st) == 0 && st.st_size > 0) {
        printf("Generated header file: %s (%ld bytes)\n", header_file, st.st_size);
        return 1;
    }
    
    return 0;
}

/* Run gengtype with file list (-p option) */
static int run_gengtype_batch(const char **files, int count, const char *list_file) {
    FILE *list = fopen(list_file, "w");
    if (!list) {
        perror("fopen list file");
        return 0;
    }
    
    /* Write all file names to list */
    for (int i = 0; i < count; i++) {
        fprintf(list, "%s\n", files[i]);
    }
    fclose(list);
    
    /* Pattern B: Batch processing with -p */
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), 
             "./gengtype_coverage -p %s -g batch_output.h 2>&1",
             list_file);
    
    printf("Running batch: %s\n", cmd);
    int status = system(cmd);
    
    if (WIFEXITED(status)) {
        printf("Batch exit status: %d\n", WEXITSTATUS(status));
    }
    
    /* Check output */
    struct stat st;
    if (stat("batch_output.h", &st) == 0 && st.st_size > 0) {
        printf("Generated batch header: batch_output.h (%ld bytes)\n", st.st_size);
        return 1;
    }
    
    return 0;
}

/* Run gengtype with multiple input files directly */
static int run_gengtype_multiple(const char **files, int count) {
    /* Pattern C: Multiple files with header generation */
    char cmd[2048];
    snprintf(cmd, sizeof(cmd), "./gengtype_coverage -g multi_output.h");
    
    /* Add all files to command */
    char *pos = cmd + strlen(cmd);
    for (int i = 0; i < count; i++) {
        snprintf(pos, sizeof(cmd) - (pos - cmd), " %s", files[i]);
        pos += strlen(pos);
    }
    
    strcat(cmd, " 2>&1");
    
    printf("Running multiple: %s\n", cmd);
    int status = system(cmd);
    
    if (WIFEXITED(status)) {
        printf("Multiple files exit status: %d\n", WEXITSTATUS(status));
    }
    
    /* Check output */
    struct stat st;
    if (stat("multi_output.h", &st) == 0 && st.st_size > 0) {
        printf("Generated multi header: multi_output.h (%ld bytes)\n", st.st_size);
        return 1;
    }
    
    return 0;
}

/* Run gengtype with error cases */
static int run_gengtype_errors(const char *error_file, const char *dup_file) {
    printf("\n=== Testing error cases ===\n");
    
    /* Pattern D: Error case */
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "./gengtype_coverage %s 2>&1", error_file);
    printf("Running error case: %s\n", cmd);
    int status = system(cmd);
    printf("Error case exit status: %d (expected non-zero)\n", WEXITSTATUS(status));
    
    /* Warning case (duplicate definition) */
    snprintf(cmd, sizeof(cmd), "./gengtype_coverage %s 2>&1", dup_file);
    printf("Running warning case: %s\n", cmd);
    status = system(cmd);
    printf("Warning case exit status: %d\n", WEXITSTATUS(status));
    
    return 1;
}

/* Generate coverage report */
static void generate_coverage_report(void) {
    printf("\n=== Generating coverage report ===\n");
    
    /* Run gcov on gengtype.cc */
    system("gcov gengtype_coverage.gcda 2>&1");
    
    /* Check for gengtype.cc.gcov */
    FILE *gcov_file = fopen("gengtype.cc.gcov", "r");
    if (gcov_file) {
        char line[256];
        int in_switch = 0;
        int switch_line = 0;
        
        printf("\nCoverage data for switch statement (lines 182-213):\n");
        printf("Line  Count\n");
        printf("----  -----\n");
        
        while (fgets(line, sizeof(line), gcov_file)) {
            int line_num, count;
            char source[256];
            
            if (sscanf(line, "%d:%d:%s", &count, &line_num, source) >= 2) {
                if (line_num >= 182 && line_num <= 213) {
                    printf("%4d: %4d\n", line_num, count);
                }
            }
        }
        fclose(gcov_file);
    } else {
        printf("No gcov file generated\n");
    }
}

/* Clean up temporary files */
static void cleanup_files(char **file_list, int count, const char *list_file) {
    for (int i = 0; i < count; i++) {
        if (file_list[i]) {
            unlink(file_list[i]);
            free(file_list[i]);
        }
    }
    
    if (list_file) {
        unlink(list_file);
    }
    
    /* Clean up generated files */
    unlink("output.h");
    unlink("batch_output.h");
    unlink("multi_output.h");
    unlink("file_list.txt");
    
    /* Clean up coverage files */
    unlink("gengtype_coverage");
    unlink("gengtype_coverage.o");
    unlink("gengtype-state_coverage.o");
    unlink("gengtype_coverage.gcda");
    unlink("gengtype_coverage.gcno");
    unlink("gengtype.cc.gcov");
}

int main(void) {
    char *temp_files[10];
    int file_count = 0;
    int success = 0;
    
    printf("=== gengtype Switch Coverage Test ===\n\n");
    
    /* Create temporary .gt files */
    for (int i = 0; gt_files[i] != NULL; i += 2) {
        temp_files[file_count] = create_temp_file(gt_files[i], gt_files[i + 1]);
        if (temp_files[file_count]) {
            printf("Created: %s\n", temp_files[file_count]);
            file_count++;
        }
    }
    
    if (file_count < 3) {
        fprintf(stderr, "Failed to create enough test files\n");
        cleanup_files(temp_files, file_count, NULL);
        return 1;
    }
    
    /* Build gengtype with coverage */
    if (!build_gengtype()) {
        cleanup_files(temp_files, file_count, NULL);
        return 1;
    }
    
    /* Test Pattern A: Single file processing */
    printf("\n=== Pattern A: Single File Processing ===\n");
    for (int i = 0; i < 3; i++) {  /* Test first 3 valid files */
        char output_name[64];
        snprintf(output_name, sizeof(output_name), "output%d", i);
        if (run_gengtype_single(temp_files[i], output_name)) {
            success++;
        }
    }
    
    /* Test Pattern B: Batch processing */
    printf("\n=== Pattern B: Batch Processing ===\n");
    if (run_gengtype_batch(temp_files, 3, "file_list.txt")) {
        success++;
    }
    
    /* Test Pattern C: Multiple file processing */
    printf("\n=== Pattern C: Multiple File Processing ===\n");
    if (run_gengtype_multiple(temp_files, 3)) {
        success++;
    }
    
    /* Test Pattern D: Error and warning cases */
    if (file_count >= 5) {
        run_gengtype_errors(temp_files[3], temp_files[4]);
    }
    
    /* Generate coverage report */
    generate_coverage_report();
    
    /* Clean up */
    cleanup_files(temp_files, file_count, "file_list.txt");
    
    printf("\n=== Test Complete ===\n");
    printf("Successful executions: %d\n", success);
    
    return (success >= 3) ? 0 : 1;
}
