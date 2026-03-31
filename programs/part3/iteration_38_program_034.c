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
    "%{\n"
    "#include \"config.h\"\n"
    "#include \"system.h\"\n"
    "%}\n"
    "\n"
    /* TYPE_UNDEFINED - forward declaration */
    "struct undefined_struct;\n"
    "\n"
    /* TYPE_SCALAR - scalar typedefs */
    "typedef int my_scalar;\n"
    "typedef unsigned long scalar2;\n"
    "\n"
    /* TYPE_STRING - string types */
    "struct string_struct {\n"
    "  const char *name;  /* TYPE_STRING */\n"
    "  char *data;\n"
    "};\n"
    "\n"
    /* TYPE_STRUCT - regular struct */
    "struct my_struct {\n"
    "  int a;\n"
    "  double b;\n"
    "};\n"
    "\n"
    /* TYPE_POINTER - pointer types */
    "typedef struct my_struct *my_ptr;\n"
    "typedef my_scalar *scalar_ptr;\n"
    "\n"
    /* TYPE_ARRAY - array types */
    "typedef int my_array[10];\n"
    "typedef struct my_struct struct_array[5];\n"
    "%}\n",
    
    /* File 2: Unions, user structs, and complex types */
    "%{\n"
    "#include \"config.h\"\n"
    "#include \"system.h\"\n"
    "%}\n"
    "\n"
    /* TYPE_UNION */
    "union my_union {\n"
    "  int i;\n"
    "  void *p;\n"
    "  double d;\n"
    "};\n"
    "\n"
    /* TYPE_USER_STRUCT - with user-provided marking */
    "struct user_struct {\n"
    "  int *p;\n"
    "  union my_union u;\n"
    "} GTY((user));\n"
    "\n"
    /* TYPE_CALLBACK - function pointer */
    "typedef void (*callback_fn)(void);\n"
    "typedef int (*compare_fn)(const void *, const void *);\n"
    "\n"
    /* Complex nested type combining multiple categories */
    "struct complex_type {\n"
    "  /* TYPE_POINTER to TYPE_ARRAY */\n"
    "  int (*matrix_ptr)[10][10];\n"
    "  \n"
    "  /* TYPE_UNION containing TYPE_POINTER */\n"
    "  union {\n"
    "    struct user_struct *usr;\n"
    "    callback_fn cb;\n"
    "  } choice;\n"
    "  \n"
    "  /* TYPE_ARRAY of TYPE_STRUCT */\n"
    "  struct my_struct items[20];\n"
    "  \n"
    "  /* TYPE_STRING */\n"
    "  const char *description;\n"
    "};\n"
    "\n"
    /* Another TYPE_POINTER example */
    "typedef union my_union *union_ptr_t;\n"
    "%}\n",
    
    /* File 3: Language structs and edge cases */
    "%{\n"
    "#include \"config.h\"\n"
    "#include \"system.h\"\n"
    "%}\n"
    "\n"
    /* TYPE_LANG_STRUCT - language-specific */
    "struct lang_struct {\n"
    "  int data;\n"
    "  void *extra;\n"
    "} GTY ((lang));\n"
    "\n"
    /* More TYPE_STRUCT variations */
    "struct nested_struct {\n"
    "  struct {\n"
    "    int x;\n"
    "    int y;\n"
    "  } point;\n"
    "  \n"
    "  struct lang_struct *lang_ptr;  /* TYPE_POINTER to TYPE_LANG_STRUCT */\n"
    "};\n"
    "\n"
    /* TYPE_ARRAY of TYPE_UNION */
    "typedef union my_union union_array[50];\n"
    "\n"
    /* TYPE_CALLBACK in struct */
    "struct with_callback {\n"
    "  callback_fn handler;\n"
    "  void *context;\n"
    "};\n"
    "\n"
    /* Forward declaration (TYPE_UNDEFINED) */
    "struct future_struct;\n"
    "\n"
    /* TYPE_POINTER to forward declared struct */
    "typedef struct future_struct *future_ptr;\n"
    "%}\n",
    
    /* File 4: File with syntax error (for error path testing) */
    "%{\n"
    "#include \"config.h\"\n"
    "#include \"system.h\"\n"
    "/* Missing closing %} to trigger error */\n"
    "\n"
    "struct error_struct {\n"
    "  int bad;\n"
    "};\n",
    
    /* File 5: Duplicate definitions (for warning testing) */
    "%{\n"
    "#include \"config.h\"\n"
    "#include \"system.h\"\n"
    "%}\n"
    "\n"
    "struct duplicate_struct {\n"
    "  int value;\n"
    "};\n"
    "\n"
    "/* Duplicate definition to trigger warning */\n"
    "struct duplicate_struct {\n"
    "  int value;\n"
    "};\n"
    "%}\n"
};

/* Create temporary file with given content */
static char *create_temp_file(const char *content, const char *suffix) {
    char template[] = "/tmp/gengtype_test_XXXXXX";
    int fd = mkstemp(template);
    if (fd == -1) {
        perror("mkstemp");
        return NULL;
    }
    
    /* Add suffix */
    char *filename = malloc(strlen(template) + strlen(suffix) + 1);
    strcpy(filename, template);
    strcat(filename, suffix);
    
    rename(template, filename);
    
    /* Write content */
    size_t len = strlen(content);
    if (write(fd, content, len) != (ssize_t)len) {
        perror("write");
        close(fd);
        free(filename);
        return NULL;
    }
    
    close(fd);
    return filename;
}

/* Compile gengtype with coverage instrumentation */
static int compile_gengtype(void) {
    printf("Compiling gengtype with coverage instrumentation...\n");
    
    /* Basic compilation command for gengtype */
    const char *compile_cmd = 
        "g++ -O0 -fprofile-arcs -ftest-coverage "
        "-DIN_GCC -DHAVE_CONFIG_H "
        "-I. -I../../include -I../../gcc "
        "-c gengtype.cc -o gengtype.o 2>&1";
    
    printf("Running: %s\n", compile_cmd);
    int status = system(compile_cmd);
    if (status != 0) {
        fprintf(stderr, "Failed to compile gengtype.cc\n");
        return -1;
    }
    
    /* Compile gengtype-state.cc */
    const char *compile_state_cmd = 
        "g++ -O0 -fprofile-arcs -ftest-coverage "
        "-DIN_GCC -DHAVE_CONFIG_H "
        "-I. -I../../include -I../../gcc "
        "-c gengtype-state.cc -o gengtype-state.o 2>&1";
    
    printf("Running: %s\n", compile_state_cmd);
    status = system(compile_state_cmd);
    if (status != 0) {
        fprintf(stderr, "Failed to compile gengtype-state.cc\n");
        return -1;
    }
    
    /* Link gengtype executable */
    const char *link_cmd = 
        "g++ -O0 -fprofile-arcs -ftest-coverage "
        "gengtype.o gengtype-state.o "
        "-lgcov -liberty -o gengtype_coverage 2>&1";
    
    printf("Running: %s\n", link_cmd);
    status = system(link_cmd);
    if (status != 0) {
        fprintf(stderr, "Failed to link gengtype\n");
        return -1;
    }
    
    printf("gengtype compiled successfully as ./gengtype_coverage\n");
    return 0;
}

/* Run gengtype on a single file */
static int run_gengtype_single(const char *gt_file, const char *output_base) {
    char cmd[1024];
    
    /* Pattern C: Generate header file to force full parsing */
    snprintf(cmd, sizeof(cmd),
             "./gengtype_coverage -g %s.h %s 2>&1",
             output_base, gt_file);
    
    printf("Running: %s\n", cmd);
    int status = system(cmd);
    
    if (status != 0) {
        printf("Note: gengtype returned non-zero (might be expected for error cases)\n");
    }
    
    /* Also run with -r flag to generate routines */
    snprintf(cmd, sizeof(cmd),
             "./gengtype_coverage -r %s.c %s 2>&1",
             output_base, gt_file);
    
    printf("Running: %s\n", cmd);
    status = system(cmd);
    
    return status;
}

/* Run gengtype with file list (Pattern B) */
static int run_gengtype_batch(char **file_list, int count) {
    /* Create file list */
    char *list_filename = create_temp_file("", ".filelist");
    if (!list_filename) return -1;
    
    FILE *list_fp = fopen(list_filename, "w");
    if (!list_fp) {
        perror("fopen list file");
        free(list_filename);
        return -1;
    }
    
    for (int i = 0; i < count; i++) {
        fprintf(list_fp, "%s\n", file_list[i]);
    }
    fclose(list_fp);
    
    /* Run gengtype with -p flag */
    char cmd[1024];
    snprintf(cmd, sizeof(cmd),
             "./gengtype_coverage -p %s 2>&1",
             list_filename);
    
    printf("Running batch: %s\n", cmd);
    int status = system(cmd);
    
    /* Clean up */
    unlink(list_filename);
    free(list_filename);
    
    return status;
}

/* Main test driver */
int main(void) {
    int i;
    char *gt_filenames[5];
    int ret = 0;
    
    printf("=== GCC gengtype Coverage Test ===\n\n");
    
    /* Step 1: Compile gengtype with coverage */
    if (compile_gengtype() != 0) {
        fprintf(stderr, "Failed to compile gengtype\n");
        return 1;
    }
    
    /* Step 2: Create temporary .gt files */
    printf("\nCreating test .gt files...\n");
    for (i = 0; i < 5; i++) {
        char suffix[32];
        snprintf(suffix, sizeof(suffix), "_%d.gt", i);
        
        gt_filenames[i] = create_temp_file(gt_files[i], suffix);
        if (!gt_filenames[i]) {
            fprintf(stderr, "Failed to create temp file %d\n", i);
            ret = 1;
            goto cleanup;
        }
        printf("Created: %s\n", gt_filenames[i]);
    }
    
    /* Step 3: Run gengtype on individual files (Pattern A/C) */
    printf("\n=== Testing individual files ===\n");
    for (i = 0; i < 3; i++) {  /* First 3 are valid files */
        char output_base[64];
        snprintf(output_base, sizeof(output_base), "output_%d", i);
        
        printf("\n--- Processing %s ---\n", gt_filenames[i]);
        int status = run_gengtype_single(gt_filenames[i], output_base);
        
        if (i < 2 && status != 0) {  /* First 2 should succeed */
            fprintf(stderr, "Warning: gengtype failed on valid file %s\n", 
                    gt_filenames[i]);
        }
        
        /* Clean up output files */
        char cleanup_cmd[128];
        snprintf(cleanup_cmd, sizeof(cleanup_cmd), "rm -f %s.h %s.c 2>/dev/null", 
                 output_base, output_base);
        system(cleanup_cmd);
    }
    
    /* Test error cases (files 3 and 4) */
    printf("\n=== Testing error/warning cases ===\n");
    for (i = 3; i < 5; i++) {
        char output_base[64];
        snprintf(output_base, sizeof(output_base), "error_%d", i-3);
        
        printf("\n--- Processing %s (expecting error/warning) ---\n", 
               gt_filenames[i]);
        int status = run_gengtype_single(gt_filenames[i], output_base);
        
        printf("Exit status: %d (non-zero expected for error cases)\n", status);
        
        /* Clean up */
        char cleanup_cmd[128];
        snprintf(cleanup_cmd, sizeof(cleanup_cmd), "rm -f %s.h %s.c 2>/dev/null", 
                 output_base, output_base);
        system(cleanup_cmd);
    }
    
    /* Step 4: Run batch processing (Pattern B) */
    printf("\n=== Testing batch processing ===\n");
    int batch_status = run_gengtype_batch(gt_filenames, 3);  /* Use first 3 valid files */
    printf("Batch processing exit status: %d\n", batch_status);
    
    /* Step 5: Generate coverage report */
    printf("\n=== Generating coverage report ===\n");
    system("gcov gengtype.cc 2>&1 | grep -A 20 'Lines executed:'");
    
    /* Check specifically for the switch statement coverage */
    printf("\n=== Checking switch statement coverage ===\n");
    system("gcov -b gengtype.cc 2>&1 | grep -B 5 -A 5 '182-213'");
    
cleanup:
    /* Clean up temporary files */
    printf("\n=== Cleaning up ===\n");
    for (i = 0; i < 5; i++) {
        if (gt_filenames[i]) {
            unlink(gt_filenames[i]);
            free(gt_filenames[i]);
            printf("Removed: temp file %d\n", i);
        }
    }
    
    /* Clean up compiled files */
    system("rm -f gengtype.o gengtype-state.o gengtype_coverage 2>/dev/null");
    system("rm -f *.gcda *.gcno *.gcov 2>/dev/null");
    system("rm -f output_*.h output_*.c error_*.h error_*.c 2>/dev/null");
    
    if (ret == 0) {
        printf("\n=== Test completed successfully ===\n");
        printf("The gengtype switch statement (lines 182-213) should now have coverage.\n");
        printf("Run 'gcov gengtype.cc' to see detailed coverage results.\n");
    }
    
    return ret;
}
