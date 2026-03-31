/* test_gengtype_coverage.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

/* Compile gengtype with coverage instrumentation */
#define COMPILE_GENGTYPE "g++ -O0 -fprofile-arcs -ftest-coverage -DIN_GCC -DHAVE_CONFIG_H -I. -I../../include -I../../gcc -c gengtype.cc gengtype-state.cc gengtype-lex.cc -lgcov -liberty && g++ -O0 -fprofile-arcs -ftest-coverage gengtype.o gengtype-state.o gengtype-lex.o -o gengtype_coverage -lgcov -liberty"

/* GT file 1: Basic types and structs */
const char *gt_file1_content = 
"%{\n"
"/* Test file 1: Basic type definitions */\n"
"%}\n"
"\n"
"/* TYPE_UNDEFINED: Forward declaration */\n"
"struct undefined_struct;\n"
"\n"
"/* TYPE_SCALAR: Scalar typedefs */\n"
"typedef int my_scalar;\n"
"typedef unsigned long my_ulong;\n"
"\n"
"/* TYPE_STRING: String type usage */\n"
"struct string_struct {\n"
"  const char *name;  /* TYPE_STRING */\n"
"  char *data;\n"
"};\n"
"\n"
"/* TYPE_STRUCT: Regular struct */\n"
"struct my_struct {\n"
"  int a;\n"
"  float b;\n"
"};\n"
"\n"
"/* TYPE_POINTER: Pointer typedef */\n"
"typedef struct my_struct *my_struct_ptr;\n"
"\n"
"/* TYPE_ARRAY: Array typedef */\n"
"typedef int my_int_array[10];\n"
"\n"
"/* Nested complex type: struct containing pointer to array */\n"
"struct complex_struct {\n"
"  my_int_array *array_ptr;  /* TYPE_POINTER to TYPE_ARRAY */\n"
"  struct string_struct str_member;\n"
"};\n"
"%}";

/* GT file 2: Unions, callbacks, and user structs */
const char *gt_file2_content =
"%{\n"
"/* Test file 2: Advanced type definitions */\n"
"%}\n"
"\n"
"/* TYPE_UNION: Union definition */\n"
"union my_union {\n"
"  int i;\n"
"  void *p;\n"
"  double d;\n"
"};\n"
"\n"
"/* TYPE_CALLBACK: Callback function pointer */\n"
"typedef void (*callback_fn)(void);\n"
"typedef int (*compare_fn)(const void *, const void *);\n"
"\n"
"/* TYPE_USER_STRUCT: Struct with user-provided marking */\n"
"struct user_struct {\n"
"  int *p;\n"
"  void *data;\n"
"} GTY((user));\n"
"\n"
"/* TYPE_LANG_STRUCT: Language-specific struct */\n"
"struct lang_struct {\n"
"  int data;\n"
"  void *tree_node;\n"
"} GTY((lang));\n"
"\n"
"/* Complex nested type: union containing struct with callback */\n"
"union complex_union {\n"
"  struct {\n"
"    callback_fn handler;\n"
"    int state;\n"
"  } callback_data;\n"
"  struct user_struct *user_ptr;\n"
"};\n"
"\n"
"/* Array of pointers to unions */\n"
"typedef union my_union *union_ptr_array[5];\n"
"%}";

/* GT file 3: Mixed types with errors and duplicates */
const char *gt_file3_content =
"%{\n"
"/* Test file 3: Mixed types with edge cases */\n"
"%}\n"
"\n"
"/* More scalar types */\n"
"typedef char byte;\n"
"typedef short int16;\n"
"\n"
"/* Another string struct */\n"
"struct another_string_struct {\n"
"  const char *title;\n"
"  char *buffer;\n"
"};\n"
"\n"
"/* Pointer to callback */\n"
"typedef callback_fn *callback_ptr;\n"
"\n"
"/* Multi-dimensional array */\n"
"typedef int matrix[3][3];\n"
"\n"
"/* Struct with all kinds of members */\n"
"struct kitchen_sink {\n"
"  my_scalar scalar_field;\n"
"  const char *string_field;\n"
"  union my_union union_field;\n"
"  callback_fn callback_field;\n"
"  int array_field[5];\n"
"  struct user_struct *user_struct_ptr;\n"
"  struct lang_struct lang_field;\n"
"};\n"
"\n"
"/* Duplicate definition to trigger warning */\n"
"struct my_struct {\n"
"  int a;\n"
"  float b;\n"
"};\n"
"%}";

/* GT file 4: Deliberate syntax error */
const char *gt_file4_content =
"%{\n"
"/* Test file 4: Syntax error case */\n"
"/* Missing closing %} to trigger error handling */\n"
"\n"
"struct error_struct {\n"
"  int x;\n"
"  int y;\n"
"};\n"
"/* No closing %} */";

/* Create temporary file with given content */
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

/* Run gengtype with given arguments */
int run_gengtype(const char *gengtype_path, const char **args, int arg_count) {
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork failed");
        return -1;
    }
    
    if (pid == 0) {
        /* Child process */
        char **argv = malloc((arg_count + 2) * sizeof(char *));
        argv[0] = (char *)gengtype_path;
        for (int i = 0; i < arg_count; i++) {
            argv[i + 1] = (char *)args[i];
        }
        argv[arg_count + 1] = NULL;
        
        execvp(gengtype_path, argv);
        perror("execvp failed");
        exit(EXIT_FAILURE);
    } else {
        /* Parent process */
        int status;
        waitpid(pid, &status, 0);
        return WEXITSTATUS(status);
    }
}

/* Create file list for batch processing */
char *create_file_list(char **files, int count) {
    char *listname = create_temp_file("", ".list");
    if (!listname) return NULL;
    
    FILE *f = fopen(listname, "w");
    if (!f) {
        free(listname);
        return NULL;
    }
    
    for (int i = 0; i < count; i++) {
        fprintf(f, "%s\n", files[i]);
    }
    fclose(f);
    
    return listname;
}

int main() {
    printf("=== GCC gengtype Coverage Test ===\n\n");
    
    /* Step 1: Compile gengtype with coverage instrumentation */
    printf("1. Compiling gengtype with coverage flags...\n");
    int compile_result = system(COMPILE_GENGTYPE);
    if (compile_result != 0) {
        fprintf(stderr, "Failed to compile gengtype with coverage\n");
        return 1;
    }
    printf("   gengtype compiled successfully as 'gengtype_coverage'\n\n");
    
    /* Step 2: Create temporary GT files */
    printf("2. Creating test GT files...\n");
    char *gt_files[4];
    
    gt_files[0] = create_temp_file(gt_file1_content, ".gt");
    gt_files[1] = create_temp_file(gt_file2_content, ".gt");
    gt_files[2] = create_temp_file(gt_file3_content, ".gt");
    gt_files[3] = create_temp_file(gt_file4_content, ".gt");
    
    for (int i = 0; i < 4; i++) {
        if (!gt_files[i]) {
            fprintf(stderr, "Failed to create GT file %d\n", i);
            return 1;
        }
        printf("   Created: %s\n", gt_files[i]);
    }
    printf("\n");
    
    /* Step 3: Run gengtype in various modes to trigger type counting */
    printf("3. Running gengtype to exercise type counting logic...\n\n");
    
    /* Pattern A: Process each file individually */
    printf("Pattern A: Individual file processing\n");
    printf("------------------------------------\n");
    for (int i = 0; i < 3; i++) {  /* Skip the error file for now */
        printf("Processing %s...\n", gt_files[i]);
        const char *args[] = {"-g", "output.h", gt_files[i], NULL};
        int result = run_gengtype("./gengtype_coverage", args, 3);
        printf("   Exit code: %d\n", result);
        
        /* Clean up output file */
        unlink("output.h");
    }
    printf("\n");
    
    /* Pattern B: Batch processing with -p flag */
    printf("Pattern B: Batch processing with -p flag\n");
    printf("----------------------------------------\n");
    char *filelist = create_file_list(gt_files, 3);  /* First 3 valid files */
    if (filelist) {
        printf("Processing file list: %s\n", filelist);
        const char *args[] = {"-p", filelist, NULL};
        int result = run_gengtype("./gengtype_coverage", args, 2);
        printf("   Exit code: %d\n", result);
        free(filelist);
    }
    printf("\n");
    
    /* Pattern C: Generate both header and routine files */
    printf("Pattern C: Generate header and routine files\n");
    printf("-------------------------------------------\n");
    printf("Processing all valid files together...\n");
    const char *args_c[] = {"-g", "types.h", "-r", "types.c", 
                           gt_files[0], gt_files[1], gt_files[2], NULL};
    int result_c = run_gengtype("./gengtype_coverage", args_c, 6);
    printf("   Exit code: %d\n", result_c);
    
    /* Check if output files were created */
    struct stat st;
    if (stat("types.h", &st) == 0) {
        printf("   Generated types.h (%ld bytes)\n", st.st_size);
    }
    if (stat("types.c", &st) == 0) {
        printf("   Generated types.c (%ld bytes)\n", st.st_size);
    }
    printf("\n");
    
    /* Pattern D: Error case processing */
    printf("Pattern D: Error case processing\n");
    printf("--------------------------------\n");
    printf("Processing file with syntax error: %s\n", gt_files[3]);
    const char *args_d[] = {gt_files[3], NULL};
    int result_d = run_gengtype("./gengtype_coverage", args_d, 1);
    printf("   Exit code: %d (expected non-zero for error)\n", result_d);
    printf("\n");
    
    /* Step 4: Run with debug flag for more verbose output */
    printf("4. Running with debug flag...\n");
    printf("-----------------------------\n");
    const char *args_debug[] = {"-d", "-g", "debug.h", 
                               gt_files[0], gt_files[1], NULL};
    int result_debug = run_gengtype("./gengtype_coverage", args_debug, 5);
    printf("   Exit code: %d\n", result_debug);
    
    /* Clean up debug output */
    unlink("debug.h");
    printf("\n");
    
    /* Step 5: Cleanup */
    printf("5. Cleaning up temporary files...\n");
    for (int i = 0; i < 4; i++) {
        unlink(gt_files[i]);
        free(gt_files[i]);
    }
    
    /* Clean up generated files */
    unlink("types.h");
    unlink("types.c");
    unlink("gengtype_coverage");
    unlink("gengtype.o");
    unlink("gengtype-state.o");
    unlink("gengtype-lex.o");
    
    /* Clean up coverage data files */
    unlink("gengtype.gcda");
    unlink("gengtype.gcno");
    unlink("gengtype-state.gcda");
    unlink("gengtype-state.gcno");
    unlink("gengtype-lex.gcda");
    unlink("gengtype-lex.gcno");
    
    printf("\n=== Test completed ===\n");
    printf("Coverage data generated in .gcda files\n");
    printf("Run 'gcov gengtype.cc' to see line coverage\n");
    
    return 0;
}
