/* driver.c - Test program to exercise gengtype type counting logic */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

/* Compile gengtype with coverage instrumentation */
#define COMPILE_GENGTYPE 1

/* Temporary file management */
typedef struct {
    char *filename;
    int fd;
} temp_file_t;

temp_file_t create_temp_file(const char *content, const char *suffix) {
    temp_file_t tf = {NULL, -1};
    char template[256];
    
    if (suffix)
        snprintf(template, sizeof(template), "/tmp/gengtype_test_XXXXXX%s", suffix);
    else
        snprintf(template, sizeof(template), "/tmp/gengtype_test_XXXXXX");
    
    tf.fd = mkstemps(template, suffix ? strlen(suffix) : 0);
    if (tf.fd < 0) {
        perror("mkstemps");
        return tf;
    }
    
    tf.filename = strdup(template);
    if (content) {
        write(tf.fd, content, strlen(content));
        fsync(tf.fd);
    }
    
    return tf;
}

void cleanup_temp_file(temp_file_t tf) {
    if (tf.fd >= 0) close(tf.fd);
    if (tf.filename) {
        unlink(tf.filename);
        free(tf.filename);
    }
}

/* GT file definitions covering all type categories */
const char *gt_file1 = 
"%{\n"
"/* Test file 1: Basic type definitions */\n"
"%}\n"
"\n"
"/* TYPE_UNDEFINED - forward declaration */\n"
"struct undefined_struct;\n"
"\n"
"/* TYPE_SCALAR - scalar typedef */\n"
"typedef int my_scalar;\n"
"typedef unsigned long scalar2;\n"
"\n"
"/* TYPE_STRING - string type */\n"
"struct string_struct {\n"
"  const char *name;  /* TYPE_STRING */\n"
"  char *data;\n"
"};\n"
"\n"
"/* TYPE_STRUCT - regular struct */\n"
"struct my_struct {\n"
"  int a;\n"
"  float b;\n"
"};\n"
"\n"
"/* TYPE_POINTER - pointer types */\n"
"typedef struct my_struct *my_ptr;\n"
"typedef my_scalar *scalar_ptr;\n"
"\n"
"/* TYPE_ARRAY - array types */\n"
"typedef int my_array[10];\n"
"typedef struct my_struct struct_array[5];\n";

const char *gt_file2 =
"%{\n"
"/* Test file 2: Advanced type definitions */\n"
"%}\n"
"\n"
"/* TYPE_USER_STRUCT - struct with user marking */\n"
"struct user_struct {\n"
"  int *p;\n"
"  void *data;\n"
"} GTY((user));\n"
"\n"
"/* TYPE_UNION - union type */\n"
"union my_union {\n"
"  int i;\n"
"  void *p;\n"
"  double d;\n"
"};\n"
"\n"
"/* TYPE_CALLBACK - callback type */\n"
"typedef void (*callback_fn)(void);\n"
"typedef int (*int_callback)(int, char*);\n"
"\n"
"/* TYPE_LANG_STRUCT - language-specific struct */\n"
"struct lang_struct {\n"
"  int data;\n"
"  void *lang_data;\n"
"} GTY ((lang));\n"
"\n"
"/* Complex nested type combining multiple categories */\n"
"struct complex_nested {\n"
"  union my_union *union_ptr;  /* TYPE_POINTER to TYPE_UNION */\n"
"  callback_fn handlers[5];     /* TYPE_ARRAY of TYPE_CALLBACK */\n"
"  struct user_struct user;     /* TYPE_USER_STRUCT */\n"
"  const char *description;     /* TYPE_STRING */\n"
"};\n"
"\n"
"/* Another scalar */\n"
"typedef short int16_t;\n";

const char *gt_file3 =
"%{\n"
"/* Test file 3: More complex combinations and edge cases */\n"
"%}\n"
"\n"
"/* Forward declarations (TYPE_UNDEFINED) */\n"
"struct forward1;\n"
"union forward2;\n"
"\n"
"/* Array of pointers to structs */\n"
"struct node {\n"
"  int value;\n"
"  struct node *children[10];  /* TYPE_ARRAY of TYPE_POINTER to TYPE_STRUCT */\n"
"};\n"
"\n"
"/* Union containing arrays */\n"
"union data_container {\n"
"  int numbers[100];           /* TYPE_ARRAY */\n"
"  char *strings[50];          /* TYPE_ARRAY of TYPE_POINTER to TYPE_STRING */\n"
"  void *pointers[20];         /* TYPE_ARRAY of TYPE_POINTER */\n"
"};\n"
"\n"
"/* Multiple callback types */\n"
"typedef void (*void_callback)(void);\n"
"typedef int (*comparator)(const void*, const void*);\n"
"\n"
"/* Mixed struct with all kinds */\n"
"struct kitchen_sink {\n"
"  int scalar_field;           /* TYPE_SCALAR */\n"
"  const char *string_field;   /* TYPE_STRING */\n"
"  struct node *struct_ptr;    /* TYPE_POINTER to TYPE_STRUCT */\n"
"  union data_container data;  /* TYPE_UNION */\n"
"  void_callback cb;           /* TYPE_CALLBACK */\n"
"  int array[5];               /* TYPE_ARRAY */\n"
"};\n"
"\n"
"/* Another lang struct */\n"
"struct lang_tree_node {\n"
"  struct lang_tree_node *left;\n"
"  struct lang_tree_node *right;\n"
"  int balance;\n"
"} GTY((lang));\n";

/* File with syntax error to test error paths */
const char *gt_file_error =
"%{\n"
"/* File with deliberate syntax error */\n"
"/* Missing closing %} to trigger error */\n"
"\n"
"struct bad_struct {\n"
"  int x;\n";

/* File with duplicate definition to test warnings */
const char *gt_file_duplicate =
"%{\n"
"/* File with duplicate type definition */\n"
"%}\n"
"\n"
"typedef int my_int;\n"
"typedef int my_int;  /* Duplicate */\n"
"\n"
"struct duplicate {\n"
"  int a;\n"
"};\n"
"\n"
"struct duplicate {   /* Duplicate struct */\n"
"  int b;\n"
"};\n";

/* Build and run gengtype with coverage */
int compile_gengtype_with_coverage() {
    printf("Compiling gengtype with coverage instrumentation...\n");
    
    /* Compilation command for gengtype */
    const char *compile_cmd = 
        "g++ -O0 -fprofile-arcs -ftest-coverage "
        "-DIN_GCC -DHAVE_CONFIG_H -I. -I../../include -I../../gcc "
        "-c gengtype.cc -o gengtype_coverage.o 2>&1";
    
    printf("Running: %s\n", compile_cmd);
    int status = system(compile_cmd);
    if (status != 0) {
        fprintf(stderr, "Failed to compile gengtype.cc\n");
        return -1;
    }
    
    /* Also compile gengtype-state.cc if needed */
    const char *compile_state_cmd = 
        "g++ -O0 -fprofile-arcs -ftest-coverage "
        "-DIN_GCC -DHAVE_CONFIG_H -I. -I../../include -I../../gcc "
        "-c gengtype-state.cc -o gengtype-state_coverage.o 2>&1";
    
    printf("Running: %s\n", compile_state_cmd);
    status = system(compile_state_cmd);
    if (status != 0) {
        fprintf(stderr, "Failed to compile gengtype-state.cc\n");
        return -1;
    }
    
    /* Link gengtype executable */
    const char *link_cmd = 
        "g++ -O0 -fprofile-arcs -ftest-coverage "
        "gengtype_coverage.o gengtype-state_coverage.o "
        "-lgcov -liberty -o gengtype_coverage 2>&1";
    
    printf("Running: %s\n", link_cmd);
    status = system(link_cmd);
    if (status != 0) {
        fprintf(stderr, "Failed to link gengtype\n");
        return -1;
    }
    
    return 0;
}

/* Run gengtype on a set of files */
int run_gengtype_on_files(const char **files, int count, const char *mode) {
    char cmd[4096];
    int status;
    int overall_status = 0;
    
    /* Pattern A: Process each file individually */
    if (strcmp(mode, "individual") == 0) {
        for (int i = 0; i < count; i++) {
            snprintf(cmd, sizeof(cmd), 
                    "./gengtype_coverage -g output%d.h %s 2>&1", 
                    i, files[i]);
            printf("Running: %s\n", cmd);
            status = system(cmd);
            if (status != 0) {
                printf("gengtype returned %d for file %s\n", status, files[i]);
                overall_status = 1;
            }
        }
    }
    /* Pattern B: Batch processing with -p flag */
    else if (strcmp(mode, "batch") == 0) {
        /* Create file list */
        temp_file_t filelist = create_temp_file(NULL, ".list");
        if (filelist.fd < 0) return -1;
        
        for (int i = 0; i < count; i++) {
            dprintf(filelist.fd, "%s\n", files[i]);
        }
        close(filelist.fd);
        
        snprintf(cmd, sizeof(cmd), 
                "./gengtype_coverage -p %s 2>&1", 
                filelist.filename);
        printf("Running: %s\n", cmd);
        status = system(cmd);
        
        cleanup_temp_file(filelist);
        if (status != 0) {
            printf("gengtype -p returned %d\n", status);
            overall_status = 1;
        }
    }
    /* Pattern C: Process all files together for header generation */
    else if (strcmp(mode, "combined") == 0) {
        snprintf(cmd, sizeof(cmd), "./gengtype_coverage -g combined_output.h");
        for (int i = 0; i < count; i++) {
            strcat(cmd, " ");
            strcat(cmd, files[i]);
        }
        strcat(cmd, " 2>&1");
        
        printf("Running: %s\n", cmd);
        status = system(cmd);
        if (status != 0) {
            printf("gengtype combined processing returned %d\n", status);
            overall_status = 1;
        }
    }
    
    return overall_status;
}

/* Main test driver */
int main(int argc, char *argv[]) {
    temp_file_t gt_files[6];
    const char *filenames[6];
    int num_files = 0;
    int result = 0;
    
    printf("=== Gengtype Type Counting Coverage Test ===\n\n");
    
    /* Create temporary GT files */
    printf("Creating test .gt files...\n");
    
    gt_files[0] = create_temp_file(gt_file1, ".gt");
    gt_files[1] = create_temp_file(gt_file2, ".gt");
    gt_files[2] = create_temp_file(gt_file3, ".gt");
    gt_files[3] = create_temp_file(gt_file_error, ".gt");
    gt_files[4] = create_temp_file(gt_file_duplicate, ".gt");
    
    /* Create an empty file for edge case */
    gt_files[5] = create_temp_file("%{\n%}\n", ".gt");
    
    for (int i = 0; i < 6; i++) {
        if (gt_files[i].filename) {
            filenames[num_files++] = gt_files[i].filename;
            printf("Created: %s\n", gt_files[i].filename);
        }
    }
    
    if (num_files == 0) {
        fprintf(stderr, "Failed to create any test files\n");
        return 1;
    }
    
    /* Compile gengtype with coverage */
    if (compile_gengtype_with_coverage() != 0) {
        fprintf(stderr, "Failed to compile gengtype\n");
        result = 1;
        goto cleanup;
    }
    
    /* Test Pattern A: Individual file processing */
    printf("\n--- Pattern A: Individual file processing ---\n");
    if (run_gengtype_on_files(filenames, num_files, "individual") != 0) {
        printf("Note: Some gengtype runs failed (expected for error cases)\n");
    }
    
    /* Test Pattern B: Batch processing */
    printf("\n--- Pattern B: Batch processing with -p flag ---\n");
    if (run_gengtype_on_files(filenames, num_files - 1, "batch") != 0) {  /* Skip error file */
        printf("Note: Batch processing had issues\n");
    }
    
    /* Test Pattern C: Combined processing for header generation */
    printf("\n--- Pattern C: Combined processing for header generation ---\n");
    if (run_gengtype_on_files(filenames, num_files - 1, "combined") != 0) {  /* Skip error file */
        printf("Note: Combined processing had issues\n");
    }
    
    /* Test Pattern D: Error and warning cases specifically */
    printf("\n--- Pattern D: Error and warning cases ---\n");
    {
        char cmd[1024];
        /* Test syntax error file */
        snprintf(cmd, sizeof(cmd), "./gengtype_coverage %s 2>&1", filenames[3]);
        printf("Running (expected to fail): %s\n", cmd);
        int status = system(cmd);
        printf("Syntax error test returned: %d\n", status);
        
        /* Test duplicate warning file */
        snprintf(cmd, sizeof(cmd), "./gengtype_coverage -g dup_output.h %s 2>&1", filenames[4]);
        printf("Running (may produce warnings): %s\n", cmd);
        status = system(cmd);
        printf("Duplicate test returned: %d\n", status);
    }
    
    /* Generate coverage report */
    printf("\n--- Generating coverage report ---\n");
    system("gcov gengtype_coverage.o 2>&1 | grep -A 20 'gengtype.cc'");
    
    /* Check if the specific switch was executed */
    printf("\n--- Checking specific switch coverage ---\n");
    FILE *gcov_file = fopen("gengtype.cc.gcov", "r");
    if (gcov_file) {
        char line[512];
        int in_target_block = 0;
        
        while (fgets(line, sizeof(line), gcov_file)) {
            if (strstr(line, "182:") || strstr(line, "182:")) {
                in_target_block = 1;
            }
            if (in_target_block) {
                printf("%s", line);
                if (strstr(line, "32:") || strstr(line, "213:")) {
                    break;
                }
            }
        }
        fclose(gcov_file);
    }
    
    printf("\n=== Test completed ===\n");
    printf("Coverage data written to gengtype.cc.gcov\n");
    printf("Check lines 182-213 for execution counts\n");

cleanup:
    /* Cleanup temporary files */
    printf("\nCleaning up temporary files...\n");
    for (int i = 0; i < 6; i++) {
        if (gt_files[i].filename) {
            cleanup_temp_file(gt_files[i]);
        }
    }
    
    /* Cleanup generated files */
    system("rm -f output*.h combined_output.h dup_output.h gengtype_coverage 2>/dev/null");
    system("rm -f *.gcda *.gcno *.o 2>/dev/null");
    
    return result;
}
