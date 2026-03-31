/* test_gengtype_state.c - Test program to exercise gengtype state generation */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

/* Function to create a temporary directory */
static char *create_temp_dir(void) {
    char template[] = "/tmp/gengtype_test_XXXXXX";
    char *dir = mkdtemp(template);
    if (!dir) {
        perror("Failed to create temporary directory");
        return NULL;
    }
    return strdup(dir);
}

/* Function to write test header file with all type categories */
static int write_test_header(const char *dir) {
    char path[1024];
    snprintf(path, sizeof(path), "%s/test_state_types.h", dir);
    
    FILE *f = fopen(path, "w");
    if (!f) {
        perror("Failed to open header file");
        return 0;
    }
    
    fprintf(f, "/* Test header with all GTY type categories */\n");
    fprintf(f, "#ifndef TEST_STATE_TYPES_H\n");
    fprintf(f, "#define TEST_STATE_TYPES_H\n\n");
    
    /* TYPE_UNDEFINED - forward declaration */
    fprintf(f, "/* TYPE_UNDEFINED */\n");
    fprintf(f, "struct undefined_type;\n\n");
    
    /* TYPE_SCALAR */
    fprintf(f, "/* TYPE_SCALAR */\n");
    fprintf(f, "typedef unsigned int my_scalar GTY((skip));\n");
    fprintf(f, "typedef long long my_scalar2 GTY((skip));\n\n");
    
    /* TYPE_STRING */
    fprintf(f, "/* TYPE_STRING */\n");
    fprintf(f, "typedef const char *my_string GTY((string));\n");
    fprintf(f, "typedef const char * const my_const_string GTY((string));\n\n");
    
    /* TYPE_STRUCT */
    fprintf(f, "/* TYPE_STRUCT */\n");
    fprintf(f, "struct my_struct GTY(()) {\n");
    fprintf(f, "  my_scalar field1;\n");
    fprintf(f, "  my_string field2;\n");
    fprintf(f, "  int extra_field GTY((skip));\n");
    fprintf(f, "};\n\n");
    
    /* TYPE_USER_STRUCT */
    fprintf(f, "/* TYPE_USER_STRUCT */\n");
    fprintf(f, "struct my_user_struct GTY((user)) {\n");
    fprintf(f, "  int user_field1;\n");
    fprintf(f, "  char *user_field2;\n");
    fprintf(f, "};\n\n");
    
    /* TYPE_UNION */
    fprintf(f, "/* TYPE_UNION */\n");
    fprintf(f, "union my_union GTY(()) {\n");
    fprintf(f, "  my_scalar as_scalar;\n");
    fprintf(f, "  my_string as_string;\n");
    fprintf(f, "  struct my_struct *as_ptr;\n");
    fprintf(f, "};\n\n");
    
    /* TYPE_POINTER */
    fprintf(f, "/* TYPE_POINTER */\n");
    fprintf(f, "typedef struct my_struct *my_ptr GTY((skip));\n");
    fprintf(f, "typedef union my_union *union_ptr GTY((skip));\n\n");
    
    /* TYPE_ARRAY */
    fprintf(f, "/* TYPE_ARRAY */\n");
    fprintf(f, "typedef int my_array[10] GTY((skip));\n");
    fprintf(f, "typedef struct my_struct *struct_ptr_array[5] GTY((skip));\n\n");
    
    /* TYPE_CALLBACK */
    fprintf(f, "/* TYPE_CALLBACK */\n");
    fprintf(f, "typedef void (*my_callback)(int) GTY((skip));\n");
    fprintf(f, "typedef int (*another_callback)(const char*) GTY((skip));\n\n");
    
    /* TYPE_LANG_STRUCT - wrapped in GCC macro */
    fprintf(f, "/* TYPE_LANG_STRUCT */\n");
    fprintf(f, "#ifdef GCC\n");
    fprintf(f, "struct lang_struct GTY(()) {\n");
    fprintf(f, "  int lang_field;\n");
    fprintf(f, "  void *lang_data;\n");
    fprintf(f, "};\n");
    fprintf(f, "#endif\n\n");
    
    /* Additional complex nested types */
    fprintf(f, "/* Nested structure for more coverage */\n");
    fprintf(f, "struct nested_struct GTY(()) {\n");
    fprintf(f, "  struct my_struct *inner_ptr;\n");
    fprintf(f, "  union my_union inner_union;\n");
    fprintf(f, "  my_array inner_array;\n");
    fprintf(f, "};\n\n");
    
    fprintf(f, "#endif /* TEST_STATE_TYPES_H */\n");
    
    fclose(f);
    return 1;
}

/* Function to write test root source file */
static int write_test_root(const char *dir) {
    char path[1024];
    snprintf(path, sizeof(path), "%s/test_state_root.c", dir);
    
    FILE *f = fopen(path, "w");
    if (!f) {
        perror("Failed to open root file");
        return 0;
    }
    
    fprintf(f, "/* Test root file with GTY root marker */\n");
    fprintf(f, "#include \"test_state_types.h\"\n\n");
    
    /* Root structure containing fields of various types */
    fprintf(f, "/* Root structure - TYPE_STRUCT with root marker */\n");
    fprintf(f, "struct root_struct GTY((root)) {\n");
    fprintf(f, "  my_scalar scalar_field;\n");
    fprintf(f, "  my_string string_field;\n");
    fprintf(f, "  struct my_struct *struct_ptr_field;\n");
    fprintf(f, "  union my_union union_field;\n");
    fprintf(f, "  my_array array_field;\n");
    fprintf(f, "  my_callback callback_field;\n");
    fprintf(f, "  struct nested_struct nested_field;\n");
    fprintf(f, "  struct undefined_type *undefined_ptr GTY((skip));\n");
    fprintf(f, "};\n\n");
    
    /* Global root variable */
    fprintf(f, "/* Global root variable */\n");
    fprintf(f, "struct root_struct global_root GTY((root));\n\n");
    
    /* Additional root variables for different types */
    fprintf(f, "/* Additional roots for coverage */\n");
    fprintf(f, "struct my_struct another_root GTY((root));\n");
    fprintf(f, "union my_union union_root GTY((root));\n");
    
    fclose(f);
    return 1;
}

/* Function to write input list file */
static int write_input_list(const char *dir) {
    char path[1024];
    snprintf(path, sizeof(path), "%s/input.list", dir);
    
    FILE *f = fopen(path, "w");
    if (!f) {
        perror("Failed to open input list");
        return 0;
    }
    
    fprintf(f, "%s/test_state_types.h\n", dir);
    fprintf(f, "%s/test_state_root.c\n", dir);
    
    fclose(f);
    return 1;
}

/* Function to execute gengtype and check output */
static int run_gengtype(const char *dir, const char *gengtype_path) {
    char cmd[2048];
    char state_output[1024];
    
    snprintf(state_output, sizeof(state_output), "%s/output_state", dir);
    
    /* Build the command to run gengtype */
    if (gengtype_path && gengtype_path[0]) {
        snprintf(cmd, sizeof(cmd), "%s -r %s/input.list -s %s",
                gengtype_path, dir, state_output);
    } else {
        /* Try to find gengtype in PATH */
        snprintf(cmd, sizeof(cmd), "gengtype -r %s/input.list -s %s",
                dir, state_output);
    }
    
    printf("Executing: %s\n", cmd);
    
    int ret = system(cmd);
    if (ret != 0) {
        fprintf(stderr, "gengtype execution failed with code %d\n", ret);
        return 0;
    }
    
    /* Check if state file was created and is non-empty */
    struct stat st;
    snprintf(state_output, sizeof(state_output), "%s/output_state", dir);
    
    if (stat(state_output, &st) != 0) {
        perror("Failed to stat output state file");
        return 0;
    }
    
    if (st.st_size == 0) {
        fprintf(stderr, "Output state file is empty\n");
        return 0;
    }
    
    printf("Successfully generated state file (%ld bytes)\n", st.st_size);
    
    /* Print first few lines of output for verification */
    printf("\nFirst 20 lines of generated state file:\n");
    char print_cmd[1024];
    snprintf(print_cmd, sizeof(print_cmd), "head -20 %s/output_state", dir);
    system(print_cmd);
    
    return 1;
}

/* Main function */
int main(int argc, char *argv[]) {
    char *temp_dir = NULL;
    const char *gengtype_path = NULL;
    
    /* Parse command line arguments */
    if (argc > 1) {
        gengtype_path = argv[1];
    }
    
    printf("=== Gengtype State Generation Test ===\n");
    
    /* Create temporary directory */
    temp_dir = create_temp_dir();
    if (!temp_dir) {
        return 1;
    }
    
    printf("Created temporary directory: %s\n", temp_dir);
    
    /* Write test files */
    if (!write_test_header(temp_dir) ||
        !write_test_root(temp_dir) ||
        !write_input_list(temp_dir)) {
        free(temp_dir);
        return 1;
    }
    
    printf("Created test files in %s\n", temp_dir);
    
    /* Run gengtype */
    int success = run_gengtype(temp_dir, gengtype_path);
    
    /* Cleanup */
    if (success) {
        printf("\nTest completed successfully!\n");
        printf("All type categories should have been processed:\n");
        printf("  - TYPE_SCALAR\n");
        printf("  - TYPE_STRING\n");
        printf("  - TYPE_STRUCT\n");
        printf("  - TYPE_USER_STRUCT\n");
        printf("  - TYPE_UNION\n");
        printf("  - TYPE_POINTER\n");
        printf("  - TYPE_ARRAY\n");
        printf("  - TYPE_CALLBACK\n");
        printf("  - TYPE_LANG_STRUCT (if GCC defined)\n");
        printf("  - TYPE_UNDEFINED\n");
    }
    
    /* Optionally clean up temp directory */
    char cleanup_cmd[1024];
    snprintf(cleanup_cmd, sizeof(cleanup_cmd), "rm -rf %s", temp_dir);
    printf("\nTo clean up, run: %s\n", cleanup_cmd);
    
    free(temp_dir);
    return success ? 0 : 1;
}
