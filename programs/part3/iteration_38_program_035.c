/* test_gengtype_coverage.c - Driver program to test gengtype type counting */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>

/* Compile gengtype with coverage flags */
#define COMPILE_GENGTYPE "g++ -O0 -fprofile-arcs -ftest-coverage -DIN_GCC -DHAVE_CONFIG_H -I. -I../../include -I../../gcc -c gengtype.cc gengtype-state.cc gengtype-lex.cc -lgcov -liberty 2>&1"
#define LINK_GENGTYPE "g++ -O0 -fprofile-arcs -ftest-coverage gengtype.o gengtype-state.o gengtype-lex.o -o gengtype_coverage -lgcov -liberty 2>&1"

/* GT file 1: Basic types and undefined */
const char *gt_file1 = 
"%{\n"
"/* Test file 1: Basic types and undefined */\n"
"%}\n"
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
"  const char *name;  /* string type */\n"
"  char *data;\n"
"};\n"
"\n"
"/* TYPE_POINTER */\n"
"typedef struct string_struct *string_ptr;\n"
"\n"
"/* TYPE_ARRAY */\n"
"typedef int my_array[10];\n"
"typedef struct string_struct struct_array[5];\n";

/* GT file 2: Structs, unions, and user structs */
const char *gt_file2 =
"%{\n"
"/* Test file 2: Structs, unions, and user structs */\n"
"%}\n"
"\n"
"/* TYPE_STRUCT */\n"
"struct my_struct {\n"
"  int a;\n"
"  double b;\n"
"  struct my_struct *next;\n"
"};\n"
"\n"
"/* TYPE_USER_STRUCT */\n"
"struct user_struct {\n"
"  int *p;\n"
"  void *data;\n"
"} GTY((user));\n"
"\n"
"/* TYPE_UNION */\n"
"union my_union {\n"
"  int i;\n"
"  void *p;\n"
"  double d;\n"
"};\n"
"\n"
"/* Complex nested type */\n"
"struct complex_struct {\n"
"  union my_union u;\n"
"  struct my_struct *struct_ptr;\n"
"  int array[20];\n"
"};\n"
"\n"
"/* Another pointer type */\n"
"typedef union my_union *union_ptr;\n";

/* GT file 3: Callback, lang struct, and more arrays */
const char *gt_file3 =
"%{\n"
"/* Test file 3: Callback, lang struct, and arrays */\n"
"%}\n"
"\n"
"/* TYPE_CALLBACK */\n"
"typedef void (*callback_fn)(void);\n"
"typedef int (*compare_fn)(const void *, const void *);\n"
"\n"
"/* TYPE_LANG_STRUCT */\n"
"struct lang_struct {\n"
"  int data;\n"
"  void *lang_data;\n"
"} GTY ((lang));\n"
"\n"
"/* More arrays */\n"
"typedef callback_fn callback_array[5];\n"
"typedef int multi_array[3][4][5];\n"
"\n"
"/* Struct with callback */\n"
"struct with_callback {\n"
"  callback_fn handler;\n"
"  int state;\n"
"};\n"
"\n"
"/* Pointer to lang struct */\n"
"typedef struct lang_struct *lang_ptr;\n";

/* GT file 4: With syntax error (for error path testing) */
const char *gt_file4 =
"%{\n"
"/* Test file 4: With deliberate syntax error */\n"
"/* Missing closing %} to trigger error */\n"
"\n"
"struct error_struct {\n"
"  int x;\n"
"};\n";

/* GT file 5: Duplicate definitions (for warning testing) */
const char *gt_file5 =
"%{\n"
"/* Test file 5: Duplicate type definitions */\n"
"%}\n"
"\n"
"/* Duplicate struct definition */\n"
"struct duplicate_struct {\n"
"  int a;\n"
"};\n"
"\n"
"struct duplicate_struct {\n"
"  int b;  /* This should trigger a warning */\n"
"};\n";

/* Create temporary file with given content */
char *create_temp_file(const char *content, const char *suffix) {
    char template[] = "/tmp/gengtype_test_XXXXXX";
    int fd = mkstemp(template);
    if (fd == -1) {
        perror("mkstemp");
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
        perror("fopen");
        free(filename);
        return NULL;
    }
    
    fwrite(content, 1, strlen(content), f);
    fclose(f);
    
    return filename;
}

/* Run command and check status */
int run_command(const char *cmd) {
    printf("Running: %s\n", cmd);
    int status = system(cmd);
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    return -1;
}

/* Main test driver */
int main(int argc, char *argv[]) {
    char *gt_files[5];
    char *filelist = NULL;
    int i, status;
    FILE *flist;
    
    printf("=== Gengtype Coverage Test ===\n\n");
    
    /* Step 1: Create temporary GT files */
    printf("1. Creating temporary GT files...\n");
    gt_files[0] = create_temp_file(gt_file1, ".gt");
    gt_files[1] = create_temp_file(gt_file2, ".gt");
    gt_files[2] = create_temp_file(gt_file3, ".gt");
    gt_files[3] = create_temp_file(gt_file4, ".gt");
    gt_files[4] = create_temp_file(gt_file5, ".gt");
    
    for (i = 0; i < 5; i++) {
        if (!gt_files[i]) {
            fprintf(stderr, "Failed to create temp file %d\n", i);
            return 1;
        }
        printf("  Created: %s\n", gt_files[i]);
    }
    
    /* Step 2: Create file list for batch processing */
    printf("\n2. Creating file list for batch processing...\n");
    filelist = create_temp_file("", ".list");
    if (!filelist) {
        fprintf(stderr, "Failed to create file list\n");
        return 1;
    }
    
    flist = fopen(filelist, "w");
    for (i = 0; i < 3; i++) {  /* Only first 3 valid files */
        fprintf(flist, "%s\n", gt_files[i]);
    }
    fclose(flist);
    printf("  File list: %s\n", filelist);
    
    /* Step 3: Compile gengtype with coverage */
    printf("\n3. Compiling gengtype with coverage instrumentation...\n");
    
    /* First check if we need to compile */
    struct stat st;
    if (stat("gengtype_coverage", &st) != 0) {
        printf("  Compiling gengtype...\n");
        
        /* Compile individual files */
        status = run_command("g++ -O0 -fprofile-arcs -ftest-coverage -DIN_GCC -DHAVE_CONFIG_H -I. -I../../include -I../../gcc -c gengtype.cc 2>&1");
        if (status != 0) {
            fprintf(stderr, "Failed to compile gengtype.cc\n");
            /* Try alternative include paths */
            status = run_command("g++ -O0 -fprofile-arcs -ftest-coverage -DIN_GCC -DHAVE_CONFIG_H -I. -c gengtype.cc 2>&1");
            if (status != 0) {
                fprintf(stderr, "Alternative compilation also failed\n");
            }
        }
        
        /* Link */
        status = run_command("g++ -O0 -fprofile-arcs -ftest-coverage gengtype.o -o gengtype_coverage -lgcov -liberty 2>&1");
        if (status != 0) {
            fprintf(stderr, "Failed to link gengtype\n");
            /* Try without libiberty for test */
            status = run_command("g++ -O0 -fprofile-arcs -ftest-coverage gengtype.o -o gengtype_coverage -lgcov 2>&1");
        }
    } else {
        printf("  gengtype_coverage already exists, skipping compilation\n");
    }
    
    /* Step 4: Run gengtype with various patterns */
    printf("\n4. Running gengtype with different patterns...\n");
    
    /* Pattern A: Process each file individually */
    printf("\n  Pattern A: Individual file processing\n");
    for (i = 0; i < 3; i++) {
        char cmd[1024];
        snprintf(cmd, sizeof(cmd), "./gengtype_coverage -g output%d.h %s 2>&1", i, gt_files[i]);
        printf("    Processing %s...\n", gt_files[i]);
        status = run_command(cmd);
        printf("    Exit status: %d\n", status);
        
        /* Clean up output */
        char output_file[64];
        snprintf(output_file, sizeof(output_file), "output%d.h", i);
        unlink(output_file);
    }
    
    /* Pattern B: Batch processing with -p */
    printf("\n  Pattern B: Batch processing with -p flag\n");
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "./gengtype_coverage -p %s -g batch_output.h 2>&1", filelist);
    printf("    Batch processing file list...\n");
    status = run_command(cmd);
    printf("    Exit status: %d\n", status);
    unlink("batch_output.h");
    
    /* Pattern C: Multiple files at once */
    printf("\n  Pattern C: Multiple files in one command\n");
    snprintf(cmd, sizeof(cmd), "./gengtype_coverage -g multi_output.h %s %s %s 2>&1", 
             gt_files[0], gt_files[1], gt_files[2]);
    printf("    Processing 3 files together...\n");
    status = run_command(cmd);
    printf("    Exit status: %d\n", status);
    unlink("multi_output.h");
    
    /* Pattern D: Error and warning cases */
    printf("\n  Pattern D: Error and warning cases\n");
    
    /* File with syntax error */
    printf("    Processing file with syntax error...\n");
    snprintf(cmd, sizeof(cmd), "./gengtype_coverage -g error_output.h %s 2>&1", gt_files[3]);
    status = run_command(cmd);
    printf("    Exit status: %d (expected non-zero)\n", status);
    
    /* File with duplicate definition */
    printf("    Processing file with duplicate definition...\n");
    snprintf(cmd, sizeof(cmd), "./gengtype_coverage -g warn_output.h %s 2>&1", gt_files[4]);
    status = run_command(cmd);
    printf("    Exit status: %d\n", status);
    
    unlink("error_output.h");
    unlink("warn_output.h");
    
    /* Step 5: Generate coverage data with -r flag (routine generation) */
    printf("\n5. Testing with routine generation (-r flag)...\n");
    snprintf(cmd, sizeof(cmd), "./gengtype_coverage -r routines.c %s %s 2>&1", 
             gt_files[0], gt_files[1]);
    printf("    Generating routines...\n");
    status = run_command(cmd);
    printf("    Exit status: %d\n", status);
    
    /* Also test combined -g and -r */
    printf("    Testing combined header and routine generation...\n");
    snprintf(cmd, sizeof(cmd), "./gengtype_coverage -g final.h -r final.c %s 2>&1", gt_files[2]);
    status = run_command(cmd);
    printf("    Exit status: %d\n", status);
    
    unlink("routines.c");
    unlink("final.h");
    unlink("final.c");
    
    /* Step 6: Cleanup */
    printf("\n6. Cleaning up temporary files...\n");
    for (i = 0; i < 5; i++) {
        unlink(gt_files[i]);
        free(gt_files[i]);
    }
    unlink(filelist);
    free(filelist);
    
    /* Clean up object files if they exist */
    unlink("gengtype.o");
    unlink("gengtype-state.o");
    unlink("gengtype-lex.o");
    
    printf("\n=== Test completed ===\n");
    printf("Coverage data should be in:\n");
    printf("  - gengtype.gcda\n");
    printf("  - gengtype.gcno\n");
    printf("\nRun 'gcov gengtype.cc' to see coverage results.\n");
    
    return 0;
}
