/* gengtype_coverage_test.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

/* Create temporary .gt files with various type definitions */
const char *gt_file1 = 
"%{\n"
"/* File 1: Basic types and structs */\n"
"#include \"config.h\"\n"
"%}\n"
"\n"
"/* TYPE_UNDEFINED: forward declaration */\n"
"struct undefined_struct;\n"
"\n"
"/* TYPE_SCALAR: scalar typedef */\n"
"typedef int my_scalar;\n"
"typedef unsigned long scalar2;\n"
"\n"
"/* TYPE_STRUCT: regular struct */\n"
"struct my_struct {\n"
"    int a;\n"
"    float b;\n"
"};\n"
"\n"
"/* TYPE_POINTER: pointer types */\n"
"typedef struct my_struct *my_ptr;\n"
"typedef my_scalar *scalar_ptr;\n"
"\n"
"/* TYPE_ARRAY: array types */\n"
"typedef int my_array[10];\n"
"typedef struct my_struct struct_array[5];\n"
"%}\n";

const char *gt_file2 = 
"%{\n"
"/* File 2: Unions, strings, and callbacks */\n"
"#include \"config.h\"\n"
"%}\n"
"\n"
"/* TYPE_UNION: union definition */\n"
"union my_union {\n"
"    int i;\n"
"    void *p;\n"
"    double d;\n"
"};\n"
"\n"
"/* TYPE_STRING: string type usage */\n"
"struct string_struct {\n"
"    const char *name;  /* string type */\n"
"    char *data;\n"
"};\n"
"\n"
"/* TYPE_CALLBACK: callback function pointer */\n"
"typedef void (*callback_fn)(void);\n"
"typedef int (*compare_fn)(const void *, const void *);\n"
"\n"
"/* TYPE_USER_STRUCT: struct with user marking */\n"
"struct user_struct {\n"
"    int *p;\n"
"    void *data;\n"
"} GTY((user));\n"
"\n"
"/* Nested complex type */\n"
"struct complex_nested {\n"
"    union my_union u;\n"
"    callback_fn handler;\n"
"    struct string_struct str_info;\n"
"};\n"
"%}\n";

const char *gt_file3 = 
"%{\n"
"/* File 3: Language structs and more complex types */\n"
"#include \"config.h\"\n"
"%}\n"
"\n"
"/* TYPE_LANG_STRUCT: language-specific struct */\n"
"struct lang_struct {\n"
"    int data;\n"
"    void *lang_data;\n"
"} GTY ((lang));\n"
"\n"
"/* More TYPE_ARRAY variations */\n"
"typedef union my_union union_array[20];\n"
"typedef callback_fn callback_array[5];\n"
"\n"
"/* Complex struct with all types */\n"
"struct mega_struct {\n"
"    /* scalar */\n"
"    int count;\n"
"    \n"
"    /* string */\n"
"    const char *description;\n"
"    \n"
"    /* pointer */\n"
"    struct lang_struct *lang_ptr;\n"
"    \n"
"    /* array */\n"
"    int values[100];\n"
"    \n"
"    /* union */\n"
"    union my_union storage;\n"
"    \n"
"    /* callback */\n"
"    callback_fn notify;\n"
"    \n"
"    /* nested struct */\n"
"    struct user_struct user_data;\n"
"};\n"
"\n"
"/* Pointer to array of pointers */\n"
"typedef struct mega_struct **complex_ptr_array[10];\n"
"%}\n";

/* File with error to test error paths */
const char *gt_file_error = 
"%{\n"
"/* File with syntax error - missing closing %}\n"
"#include \"config.h\"\n"
"\n"
"struct error_struct {\n"
"    int x;\n"
"    int y;\n"
"/* Deliberately missing %} */\n";

/* File with duplicate definition for warning test */
const char *gt_file_warning = 
"%{\n"
"#include \"config.h\"\n"
"%}\n"
"\n"
"/* Duplicate type definition */\n"
"struct duplicate_struct {\n"
"    int a;\n"
"};\n"
"\n"
"struct duplicate_struct {\n"
"    int b;\n"
"};\n"
"%}\n";

/* Create a temporary file with given content */
char *create_temp_file(const char *content, const char *suffix) {
    char template[] = "/tmp/gengtype_test_XXXXXX";
    int fd = mkstemp(template);
    if (fd == -1) {
        perror("mkstemp failed");
        return NULL;
    }
    
    /* Append suffix */
    char *filename = malloc(strlen(template) + strlen(suffix) + 1);
    strcpy(filename, template);
    strcat(filename, suffix);
    
    /* Rename to include suffix */
    close(fd);
    rename(template, filename);
    
    /* Write content */
    FILE *f = fopen(filename, "w");
    if (!f) {
        perror("fopen failed");
        free(filename);
        return NULL;
    }
    
    fwrite(content, 1, strlen(content), f);
    fclose(f);
    
    return filename;
}

/* Compile gengtype with coverage instrumentation */
int compile_gengtype_with_coverage() {
    printf("Compiling gengtype with coverage instrumentation...\n");
    
    /* Compile gengtype.cc */
    const char *compile_cmd = 
        "g++ -O0 -fprofile-arcs -ftest-coverage "
        "-DIN_GCC -DHAVE_CONFIG_H "
        "-I. -I../../include -I../../gcc "
        "-c gengtype.cc -o gengtype_coverage.o 2>&1";
    
    printf("Executing: %s\n", compile_cmd);
    int result = system(compile_cmd);
    if (result != 0) {
        fprintf(stderr, "Failed to compile gengtype.cc\n");
        return -1;
    }
    
    /* Compile gengtype-state.cc */
    const char *compile_state_cmd = 
        "g++ -O0 -fprofile-arcs -ftest-coverage "
        "-DIN_GCC -DHAVE_CONFIG_H "
        "-I. -I../../include -I../../gcc "
        "-c gengtype-state.cc -o gengtype_state_coverage.o 2>&1";
    
    printf("Executing: %s\n", compile_state_cmd);
    result = system(compile_state_cmd);
    if (result != 0) {
        fprintf(stderr, "Failed to compile gengtype-state.cc\n");
        return -1;
    }
    
    /* Link gengtype executable */
    const char *link_cmd = 
        "g++ -O0 -fprofile-arcs -ftest-coverage "
        "gengtype_coverage.o gengtype_state_coverage.o "
        "-lgcov -liberty -o gengtype_coverage 2>&1";
    
    printf("Executing: %s\n", link_cmd);
    result = system(link_cmd);
    if (result != 0) {
        fprintf(stderr, "Failed to link gengtype\n");
        return -1;
    }
    
    printf("Successfully compiled gengtype with coverage\n");
    return 0;
}

/* Run gengtype on a single file */
int run_gengtype_on_file(const char *filename, const char *mode) {
    char cmd[1024];
    
    if (strcmp(mode, "header") == 0) {
        /* Generate header file */
        snprintf(cmd, sizeof(cmd), 
                 "./gengtype_coverage -g %s_output.h %s 2>&1",
                 filename, filename);
    } else if (strcmp(mode, "routine") == 0) {
        /* Generate routine file */
        snprintf(cmd, sizeof(cmd),
                 "./gengtype_coverage -r %s_output.c %s 2>&1",
                 filename, filename);
    } else {
        /* Just parse */
        snprintf(cmd, sizeof(cmd),
                 "./gengtype_coverage %s 2>&1",
                 filename);
    }
    
    printf("Running: %s\n", cmd);
    int result = system(cmd);
    
    if (result != 0) {
        printf("gengtype returned %d for file %s\n", result, filename);
    }
    
    return result;
}

/* Run gengtype with file list (-p option) */
int run_gengtype_with_filelist(char **filenames, int count) {
    /* Create file list */
    char *listfile = create_temp_file("", ".list");
    FILE *f = fopen(listfile, "w");
    if (!f) {
        perror("Failed to create file list");
        free(listfile);
        return -1;
    }
    
    for (int i = 0; i < count; i++) {
        fprintf(f, "%s\n", filenames[i]);
    }
    fclose(f);
    
    /* Run gengtype with -p option */
    char cmd[1024];
    snprintf(cmd, sizeof(cmd),
             "./gengtype_coverage -p %s 2>&1",
             listfile);
    
    printf("Running with file list: %s\n", cmd);
    int result = system(cmd);
    
    /* Clean up */
    unlink(listfile);
    free(listfile);
    
    return result;
}

/* Run gengtype with batch processing of multiple files */
int run_gengtype_batch(char **filenames, int count) {
    char cmd[2048];
    snprintf(cmd, sizeof(cmd), "./gengtype_coverage -g batch_output.h");
    
    /* Add all filenames to command */
    for (int i = 0; i < count; i++) {
        strcat(cmd, " ");
        strcat(cmd, filenames[i]);
    }
    
    strcat(cmd, " 2>&1");
    
    printf("Running batch: %s\n", cmd);
    return system(cmd);
}

/* Main test driver */
int main() {
    printf("=== Gengtype Coverage Test ===\n\n");
    
    /* Step 1: Compile gengtype with coverage */
    if (compile_gengtype_with_coverage() != 0) {
        fprintf(stderr, "Failed to compile gengtype\n");
        return 1;
    }
    
    /* Step 2: Create temporary .gt files */
    printf("\nCreating test .gt files...\n");
    
    char *files[6];
    int file_count = 0;
    
    files[file_count++] = create_temp_file(gt_file1, ".gt");
    files[file_count++] = create_temp_file(gt_file2, ".gt");
    files[file_count++] = create_temp_file(gt_file3, ".gt");
    files[file_count++] = create_temp_file(gt_file_error, ".gt");
    files[file_count++] = create_temp_file(gt_file_warning, ".gt");
    
    /* Add one more file with mixed content */
    const char *gt_file_mixed = 
        "%{\n#include \"config.h\"\n%}\n"
        "struct mixed { int a; float b; void *c; };\n"
        "typedef struct mixed *mixed_ptr;\n"
        "union mixed_union { mixed_ptr p; int i; };\n"
        "%}\n";
    
    files[file_count++] = create_temp_file(gt_file_mixed, ".gt");
    
    /* Verify all files were created */
    for (int i = 0; i < file_count; i++) {
        if (!files[i]) {
            fprintf(stderr, "Failed to create file %d\n", i);
            return 1;
        }
        printf("Created: %s\n", files[i]);
    }
    
    /* Step 3: Run gengtype in various modes to trigger all code paths */
    printf("\n=== Running gengtype tests ===\n\n");
    
    /* Test 1: Process each file individually */
    printf("Test 1: Individual file processing\n");
    for (int i = 0; i < 3; i++) {  /* First 3 valid files */
        printf("\n--- Processing %s ---\n", files[i]);
        run_gengtype_on_file(files[i], "header");
        run_gengtype_on_file(files[i], "routine");
    }
    
    /* Test 2: Process error file (should fail) */
    printf("\n--- Processing error file (expected to fail) ---\n");
    run_gengtype_on_file(files[3], "header");
    
    /* Test 3: Process warning file */
    printf("\n--- Processing warning file ---\n");
    run_gengtype_on_file(files[4], "header");
    
    /* Test 4: Batch processing with -p option */
    printf("\nTest 2: Batch processing with -p option\n");
    run_gengtype_with_filelist(files, 3);  /* Use first 3 valid files */
    
    /* Test 5: Direct batch processing */
    printf("\nTest 3: Direct batch processing\n");
    run_gengtype_batch(files, 3);  /* Use first 3 valid files */
    
    /* Test 6: Process all files together */
    printf("\nTest 4: Process all files together\n");
    char all_cmd[4096] = "./gengtype_coverage -g all_output.h";
    for (int i = 0; i < file_count; i++) {
        strcat(all_cmd, " ");
        strcat(all_cmd, files[i]);
    }
    strcat(all_cmd, " 2>&1");
    printf("Running: %s\n", all_cmd);
    system(all_cmd);
    
    /* Step 4: Generate coverage report */
    printf("\n=== Generating coverage report ===\n");
    system("gcov gengtype_coverage.o 2>&1 | grep -A 20 'gengtype.cc'");
    
    /* Step 5: Clean up */
    printf("\n=== Cleaning up ===\n");
    for (int i = 0; i < file_count; i++) {
        unlink(files[i]);
        free(files[i]);
    }
    
    /* Clean up generated files */
    system("rm -f *.gcda *.gcno *.gcov gengtype_coverage *.o *_output.h *_output.c 2>/dev/null");
    
    printf("\n=== Test completed ===\n");
    printf("Check gengtype.c.gcov for line-by-line coverage\n");
    printf("Specifically verify lines 182-213 were executed\n");
    
    return 0;
}
