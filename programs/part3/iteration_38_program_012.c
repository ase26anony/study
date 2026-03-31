/* driver.c - Test driver for gengtype coverage of type counting switch */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>

/* Compile gengtype with coverage instrumentation */
#define COMPILE_GENGTYPE 1

/* Temporary file management */
typedef struct {
    char *filename;
    char *content;
} temp_file_t;

/* Generated .gt files with diverse type definitions */
static const char *gt_files[] = {
    /* File 1: Basic types and structs */
    "test_types1.gt",
    /* File 2: Complex nested types and callbacks */
    "test_types2.gt", 
    /* File 3: Language-specific and user types with errors */
    "test_types3.gt"
};

/* Content for each .gt file */
static const char *gt_contents[] = {
    /* File 1: Basic types covering most categories */
    "%{\n"
    "/* Test file 1: Basic type definitions */\n"
    "#include \"config.h\"\n"
    "%}\n\n"
    "/* TYPE_UNDEFINED: Forward declaration */\n"
    "struct undefined_struct;\n\n"
    "/* TYPE_SCALAR: Scalar typedef */\n"
    "typedef int my_scalar;\n"
    "typedef unsigned long scalar2;\n\n"
    "/* TYPE_STRING: String type usage */\n"
    "struct string_struct {\n"
    "  const char *name;  /* TYPE_STRING */\n"
    "  int id;\n"
    "};\n\n"
    "/* TYPE_STRUCT: Regular struct */\n"
    "struct my_struct {\n"
    "  int a;\n"
    "  double b;\n"
    "};\n\n"
    "/* TYPE_UNION: Union definition */\n"
    "union my_union {\n"
    "  int i;\n"
    "  void *p;\n"
    "  struct my_struct *s;\n"
    "};\n\n"
    "/* TYPE_POINTER: Pointer typedef */\n"
    "typedef struct my_struct *my_ptr;\n"
    "typedef union my_union *union_ptr;\n\n"
    "/* TYPE_ARRAY: Array typedef */\n"
    "typedef int my_array[10];\n"
    "typedef struct my_struct struct_array[5];\n",
    
    /* File 2: Complex nested types and callbacks */
    "%{\n"
    "/* Test file 2: Complex nested types */\n"
    "#include \"config.h\"\n"
    "%}\n\n"
    "/* TYPE_USER_STRUCT: Struct with user marking */\n"
    "struct user_struct {\n"
    "  int *p;\n"
    "  void *data;\n"
    "} GTY((user));\n\n"
    "/* TYPE_CALLBACK: Callback function pointer */\n"
    "typedef void (*callback_fn)(void);\n"
    "typedef int (*compare_fn)(const void *, const void *);\n\n"
    "/* Complex nested struct with multiple type kinds */\n"
    "struct complex_nested {\n"
    "  /* TYPE_POINTER to TYPE_ARRAY */\n"
    "  int (*matrix_ptr)[10][10];\n"
    "  \n"
    "  /* TYPE_UNION containing TYPE_POINTER */\n"
    "  union {\n"
    "    struct user_struct *user;\n"
    "    callback_fn cb;\n"
    "  } u;\n"
    "  \n"
    "  /* TYPE_ARRAY of TYPE_STRUCT */\n"
    "  struct string_struct strings[5];\n"
    "  \n"
    "  /* TYPE_POINTER to TYPE_CALLBACK */\n"
    "  callback_fn *callbacks;\n"
    "  \n"
    "  /* TYPE_STRING field */\n"
    "  const char *description;\n"
    "};\n\n"
    "/* Another TYPE_STRUCT with scalar array */\n"
    "struct with_arrays {\n"
    "  my_array data;          /* TYPE_ARRAY */\n"
    "  int *dynamic_array;     /* TYPE_POINTER (will be array) */\n"
    "  size_t count;\n"
    "};\n",
    
    /* File 3: Language-specific types and edge cases */
    "%{\n"
    "/* Test file 3: Language structs and edge cases */\n"
    "#include \"config.h\"\n"
    "%}\n\n"
    "/* TYPE_LANG_STRUCT: Language-specific struct */\n"
    "struct lang_struct {\n"
    "  int data;\n"
    "  void *lang_data;\n"
    "} GTY ((lang));\n\n"
    "/* Another TYPE_LANG_STRUCT with nested types */\n"
    "struct lang_tree_node {\n"
    "  struct lang_tree_node *left;\n"
    "  struct lang_tree_node *right;\n"
    "  int value;\n"
    "  const char *name;\n"
    "} GTY ((lang));\n\n"
    "/* Mixed TYPE_USER_STRUCT with callback */\n"
    "struct user_with_callback {\n"
    "  callback_fn notify;\n"
    "  void *user_data;\n"
    "} GTY((user));\n\n"
    "/* Forward declaration (TYPE_UNDEFINED) */\n"
    "struct future_struct;\n\n"
    "/* Pointer to undefined struct (still TYPE_POINTER) */\n"
    "typedef struct future_struct *future_ptr;\n\n"
    "/* Complex type combining everything */\n"
    "struct mega_type {\n"
    "  struct lang_struct *lang;      /* TYPE_POINTER to TYPE_LANG_STRUCT */\n"
    "  union my_union data;           /* TYPE_UNION */\n"
    "  callback_fn handlers[5];       /* TYPE_ARRAY of TYPE_CALLBACK */\n"
    "  struct user_struct user;       /* TYPE_USER_STRUCT */\n"
    "  const char *tags[10];          /* TYPE_ARRAY of TYPE_STRING */\n"
    "};\n\n"
    "/* Deliberate duplicate to test warning path */\n"
    "struct my_struct;\n"  /* Forward declaration of already defined struct */
};

/* Create a temporary file with given content */
static char *create_temp_file(const char *content, const char *suffix) {
    char template[] = "/tmp/gengtype_test_XXXXXX";
    int fd = mkstemp(template);
    if (fd == -1) {
        perror("mkstemp");
        return NULL;
    }
    
    /* Append suffix if provided */
    char *filename;
    if (suffix) {
        filename = malloc(strlen(template) + strlen(suffix) + 1);
        strcpy(filename, template);
        strcat(filename, suffix);
        rename(template, filename);
    } else {
        filename = strdup(template);
    }
    
    /* Write content */
    if (content) {
        write(fd, content, strlen(content));
    }
    close(fd);
    
    return filename;
}

/* Compile gengtype with coverage instrumentation */
static int compile_gengtype(void) {
    printf("Compiling gengtype with coverage instrumentation...\n");
    
    /* Compile command for gengtype */
    const char *compile_cmd = 
        "g++ -O0 -fprofile-arcs -ftest-coverage "
        "-DIN_GCC -DHAVE_CONFIG_H "
        "-I. -I../../include -I../../gcc "
        "-c gengtype.cc -o gengtype_coverage.o 2>&1";
    
    printf("Running: %s\n", compile_cmd);
    int status = system(compile_cmd);
    if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
        fprintf(stderr, "Failed to compile gengtype.cc\n");
        return -1;
    }
    
    /* Also compile gengtype-state.cc if it exists */
    const char *state_compile = 
        "g++ -O0 -fprofile-arcs -ftest-coverage "
        "-DIN_GCC -DHAVE_CONFIG_H "
        "-I. -I../../include -I../../gcc "
        "-c gengtype-state.cc -o gengtype-state_coverage.o 2>&1";
    
    system(state_compile);
    
    /* Link gengtype executable */
    const char *link_cmd = 
        "g++ -O0 -fprofile-arcs -ftest-coverage "
        "gengtype_coverage.o gengtype-state_coverage.o "
        "-lgcov -liberty -o gengtype_coverage 2>&1";
    
    printf("Running: %s\n", link_cmd);
    status = system(link_cmd);
    if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
        fprintf(stderr, "Failed to link gengtype\n");
        return -1;
    }
    
    printf("gengtype compiled successfully as ./gengtype_coverage\n");
    return 0;
}

/* Run gengtype on a single file with specific flags */
static int run_gengtype_on_file(const char *filename, const char *flags) {
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "./gengtype_coverage %s %s 2>&1", flags, filename);
    
    printf("Running: %s\n", cmd);
    int status = system(cmd);
    
    if (WIFEXITED(status)) {
        int exit_status = WEXITSTATUS(status);
        printf("gengtype exited with status %d\n", exit_status);
        return exit_status;
    }
    
    return -1;
}

/* Run gengtype with file list (-p option) */
static int run_gengtype_with_filelist(const char **files, int count) {
    /* Create file list */
    char *listfile = create_temp_file(NULL, ".list");
    if (!listfile) return -1;
    
    FILE *fp = fopen(listfile, "w");
    if (!fp) {
        free(listfile);
        return -1;
    }
    
    for (int i = 0; i < count; i++) {
        fprintf(fp, "%s\n", files[i]);
    }
    fclose(fp);
    
    /* Run gengtype with -p option */
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "./gengtype_coverage -p %s 2>&1", listfile);
    
    printf("Running (batch): %s\n", cmd);
    int status = system(cmd);
    
    unlink(listfile);
    free(listfile);
    
    return WIFEXITED(status) ? WEXITSTATUS(status) : -1;
}

/* Run gengtype to generate header file */
static int run_gengtype_generate_header(const char **files, int count) {
    char cmd[2048] = "./gengtype_coverage -g gtype_test.h";
    
    /* Add all files to command */
    for (int i = 0; i < count; i++) {
        strcat(cmd, " ");
        strcat(cmd, files[i]);
    }
    
    strcat(cmd, " 2>&1");
    
    printf("Running (header gen): %s\n", cmd);
    int status = system(cmd);
    
    /* Check if header was created */
    if (access("gtype_test.h", F_OK) == 0) {
        printf("Generated header file: gtype_test.h\n");
        unlink("gtype_test.h");
    }
    
    return WIFEXITED(status) ? WEXITSTATUS(status) : -1;
}

/* Create a file with syntax error to test error paths */
static int test_error_path(void) {
    const char *error_content = 
        "%{\n"
        "#include \"config.h\"\n"
        "/* Missing closing %%} */\n"
        "struct bad_struct {\n"
        "  int x;\n";
    
    char *error_file = create_temp_file(error_content, ".gt");
    if (!error_file) return -1;
    
    printf("\n=== Testing error path ===\n");
    int status = run_gengtype_on_file(error_file, "");
    
    unlink(error_file);
    free(error_file);
    
    return status;
}

/* Main test driver */
int main(int argc, char **argv) {
    printf("=== gengtype Type Counting Switch Coverage Test ===\n\n");
    
    /* Step 1: Compile gengtype with coverage */
    if (COMPILE_GENGTYPE) {
        if (compile_gengtype() != 0) {
            fprintf(stderr, "Failed to compile gengtype. Aborting.\n");
            return 1;
        }
    } else {
        printf("Using pre-compiled gengtype_coverage binary\n");
        if (access("./gengtype_coverage", X_OK) != 0) {
            fprintf(stderr, "gengtype_coverage not found. Please compile first.\n");
            return 1;
        }
    }
    
    /* Step 2: Create temporary .gt files */
    printf("\n=== Creating test .gt files ===\n");
    char *temp_files[sizeof(gt_files)/sizeof(gt_files[0])];
    
    for (size_t i = 0; i < sizeof(gt_files)/sizeof(gt_files[0]); i++) {
        temp_files[i] = create_temp_file(gt_contents[i], ".gt");
        if (!temp_files[i]) {
            fprintf(stderr, "Failed to create temp file %zu\n", i);
            return 1;
        }
        printf("Created: %s\n", temp_files[i]);
    }
    
    /* Step 3: Run gengtype in various modes to exercise the switch */
    printf("\n=== Running gengtype tests ===\n");
    
    /* Test 1: Process each file individually */
    printf("\n--- Test 1: Individual file processing ---\n");
    for (size_t i = 0; i < sizeof(temp_files)/sizeof(temp_files[0]); i++) {
        printf("\nProcessing %s:\n", temp_files[i]);
        run_gengtype_on_file(temp_files[i], "");
    }
    
    /* Test 2: Batch processing with -p flag */
    printf("\n--- Test 2: Batch processing (-p flag) ---\n");
    run_gengtype_with_filelist((const char **)temp_files, 
                              sizeof(temp_files)/sizeof(temp_files[0]));
    
    /* Test 3: Generate header file */
    printf("\n--- Test 3: Header generation (-g flag) ---\n");
    run_gengtype_generate_header((const char **)temp_files,
                                sizeof(temp_files)/sizeof(temp_files[0]));
    
    /* Test 4: Generate routine file */
    printf("\n--- Test 4: Routine generation (-r flag) ---\n");
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), 
             "./gengtype_coverage -r gtype_test.c %s %s 2>&1",
             temp_files[0], temp_files[1]);
    printf("Running: %s\n", cmd);
    system(cmd);
    
    if (access("gtype_test.c", F_OK) == 0) {
        printf("Generated routine file: gtype_test.c\n");
        unlink("gtype_test.c");
    }
    
    /* Test 5: Error path testing */
    printf("\n--- Test 5: Error path testing ---\n");
    test_error_path();
    
    /* Test 6: Combined processing with all flags */
    printf("\n--- Test 6: Combined processing ---\n");
    snprintf(cmd, sizeof(cmd),
             "./gengtype_coverage -g combined.h -r combined.c %s %s %s 2>&1",
             temp_files[0], temp_files[1], temp_files[2]);
    printf("Running: %s\n", cmd);
    system(cmd);
    
    /* Clean up generated files */
    if (access("combined.h", F_OK) == 0) unlink("combined.h");
    if (access("combined.c", F_OK) == 0) unlink("combined.c");
    
    /* Step 4: Cleanup temporary files */
    printf("\n=== Cleaning up ===\n");
    for (size_t i = 0; i < sizeof(temp_files)/sizeof(temp_files[0]); i++) {
        if (temp_files[i]) {
            unlink(temp_files[i]);
            free(temp_files[i]);
            printf("Removed: %s\n", temp_files[i]);
        }
    }
    
    /* Step 5: Generate coverage report */
    printf("\n=== Generating coverage report ===\n");
    system("gcov gengtype_coverage.o 2>&1 | grep -A 20 'gengtype.cc'");
    
    /* Specifically check for the switch statement lines */
    printf("\n=== Checking coverage of target lines (182-213) ===\n");
    system("gcov -b gengtype_coverage.o 2>&1 | "
           "sed -n '/^Lines executed:/p;/^Branch/,/^$/p'");
    
    /* Look for the specific switch block */
    printf("\n=== Detailed coverage of switch statement ===\n");
    system("gcov -c gengtype_coverage.o 2>&1 | "
           "grep -E '^(182|19[0-9]|20[0-9]):'");
    
    printf("\n=== Test completed ===\n");
    printf("Check gengtype.cc.gcov for detailed line-by-line coverage\n");
    
    return 0;
}
