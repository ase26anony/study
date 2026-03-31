/* test_gengtype_coverage.c - Driver program to exercise gengtype type counting */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>

/* gengtype source files needed for compilation */
#define GENGTYPE_SRC "gengtype.cc"
#define GENGTYPE_STATE_SRC "gengtype-state.cc"
#define GENGTYPE_HEADER "gengtype.h"

/* Temporary file management */
typedef struct {
    char *filename;
    char *content;
} temp_file_t;

/* Generated .gt files with diverse type definitions */
static const char *gt_files[] = {
    /* File 1: Basic types and structs */
    "test_types1.gt",
    "test_types2.gt", 
    "test_types3.gt",
    "test_error.gt",  /* File with deliberate error */
    "test_dup.gt"     /* File with duplicate definition */
};

static const char *gt_contents[] = {
    /* test_types1.gt - Basic type definitions */
    "%{\n"
    "/* Test file 1: Basic types */\n"
    "#include \"config.h\"\n"
    "#include \"system.h\"\n"
    "%}\n"
    "\n"
    "/* TYPE_UNDEFINED: Forward declaration */\n"
    "struct undefined_struct;\n"
    "\n"
    "/* TYPE_SCALAR: Scalar typedefs */\n"
    "typedef int my_scalar;\n"
    "typedef unsigned long scalar_t;\n"
    "\n"
    "/* TYPE_STRING: String type */\n"
    "struct string_struct {\n"
    "  const char *name;  /* TYPE_STRING */\n"
    "  int id;\n"
    "};\n"
    "\n"
    "/* TYPE_STRUCT: Regular struct */\n"
    "struct my_struct {\n"
    "  int a;\n"
    "  float b;\n"
    "  struct string_struct *str;\n"
    "};\n"
    "\n"
    "/* TYPE_POINTER: Pointer typedef */\n"
    "typedef struct my_struct *my_ptr;\n"
    "\n"
    "/* TYPE_ARRAY: Array types */\n"
    "typedef int my_array[10];\n"
    "typedef struct my_struct struct_array[5];\n"
    "\n"
    "/* TYPE_CALLBACK: Callback function pointer */\n"
    "typedef void (*callback_fn)(int, void*);\n"
    "typedef int (*compare_fn)(const void*, const void*);\n"
    "%}\n",

    /* test_types2.gt - Advanced types and unions */
    "%{\n"
    "/* Test file 2: Advanced types */\n"
    "#include \"config.h\"\n"
    "#include \"system.h\"\n"
    "%}\n"
    "\n"
    "/* TYPE_UNION: Union definition */\n"
    "union my_union {\n"
    "  int i;\n"
    "  void *p;\n"
    "  double d;\n"
    "  struct my_struct *s;\n"
    "};\n"
    "\n"
    "/* TYPE_USER_STRUCT: Struct with user marking */\n"
    "struct user_struct {\n"
    "  int *p;\n"
    "  union my_union u;\n"
    "} GTY((user));\n"
    "\n"
    "/* Complex nested type combining multiple categories */\n"
    "struct complex_type {\n"
    "  /* Contains pointer to union of arrays */\n"
    "  union data_union {\n"
    "    int int_array[20];      /* TYPE_ARRAY inside union */\n"
    "    void *ptr_array[10];    /* TYPE_ARRAY of TYPE_POINTER */\n"
    "    callback_fn callbacks[5]; /* TYPE_ARRAY of TYPE_CALLBACK */\n"
    "  } *data;  /* TYPE_POINTER to TYPE_UNION */\n"
    "  \n"
    "  const char *description;  /* TYPE_STRING */\n"
    "  \n"
    "  /* Nested struct with callback */\n"
    "  struct {\n"
    "    callback_fn handler;\n"
    "    void *context;\n"
    "  } callback_data;\n"
    "  \n"
    "  /* Array of pointers to user structs */\n"
    "  struct user_struct *users[8];\n"
    "};\n"
    "\n"
    "/* Another scalar */\n"
    "typedef volatile long atomic_scalar;\n"
    "%}\n",

    /* test_types3.gt - Language-specific and edge cases */
    "%{\n"
    "/* Test file 3: Language structs and edge cases */\n"
    "#include \"config.h\"\n"
    "#include \"system.h\"\n"
    "%}\n"
    "\n"
    "/* TYPE_LANG_STRUCT: Language-specific struct */\n"
    "struct lang_struct {\n"
    "  int data;\n"
    "  void *lang_data;\n"
    "} GTY ((lang));\n"
    "\n"
    "/* Another language struct with nested types */\n"
    "struct cpp_lang_struct {\n"
    "  struct lang_struct *base;\n"
    "  const char *name;\n"
    "  void (*destructor)(void*);  /* TYPE_CALLBACK */\n"
    "} GTY ((lang));\n"
    "\n"
    "/* Struct with all type categories */\n"
    "struct kitchen_sink {\n"
    "  /* TYPE_SCALAR */\n"
    "  int counter;\n"
    "  \n"
    "  /* TYPE_STRING */\n"
    "  const char *label;\n"
    "  \n"
    "  /* TYPE_POINTER */\n"
    "  struct lang_struct *lang_ptr;\n"
    "  \n"
    "  /* TYPE_UNION */\n"
    "  union {\n"
    "    int tag;\n"
    "    void *data;\n"
    "  } variant;\n"
    "  \n"
    "  /* TYPE_ARRAY */\n"
    "  callback_fn handlers[3];\n"
    "  \n"
    "  /* TYPE_STRUCT (anonymous) */\n"
    "  struct {\n"
    "    int x, y;\n"
    "  } point;\n"
    "};\n"
    "\n"
    "/* Forward declarations for undefined types */\n"
    "struct future_struct;\n"
    "union future_union;\n"
    "%}\n",

    /* test_error.gt - File with syntax error */
    "%{\n"
    "/* Test file with deliberate syntax error */\n"
    "#include \"config.h\"\n"
    "#include \"system.h\"\n"
    "\n"
    "struct error_struct {\n"
    "  int missing_semicolon\n"  /* Missing semicolon */
    "  float bad_field\n"
    "};\n"
    "/* Missing closing %} */\n",

    /* test_dup.gt - File with duplicate definition */
    "%{\n"
    "/* Test file with duplicate definition */\n"
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
    
    /* Append suffix if provided */
    if (suffix) {
        char *new_name = malloc(strlen(template) + strlen(suffix) + 1);
        strcpy(new_name, template);
        strcat(new_name, suffix);
        close(fd);
        unlink(template);
        fd = mkstemp(new_name);
        if (fd == -1) {
            free(new_name);
            perror("mkstemp with suffix");
            return NULL;
        }
        strcpy(template, new_name);
        free(new_name);
    }
    
    /* Write content */
    size_t len = strlen(content);
    if (write(fd, content, len) != (ssize_t)len) {
        perror("write");
        close(fd);
        unlink(template);
        return NULL;
    }
    
    close(fd);
    return strdup(template);
}

/* Compile gengtype with coverage instrumentation */
static int compile_gengtype(const char *source_dir) {
    char cmd[1024];
    int status;
    
    printf("Compiling gengtype with coverage instrumentation...\n");
    
    /* Compile gengtype.cc */
    snprintf(cmd, sizeof(cmd),
        "g++ -O0 -fprofile-arcs -ftest-coverage -DIN_GCC -DHAVE_CONFIG_H "
        "-I%s -I%s/../../include -I%s/../../gcc "
        "-c %s/%s -o gengtype_test.o",
        source_dir, source_dir, source_dir, source_dir, GENGTYPE_SRC);
    
    printf("Running: %s\n", cmd);
    status = system(cmd);
    if (status != 0) {
        fprintf(stderr, "Failed to compile gengtype.cc\n");
        return -1;
    }
    
    /* Compile gengtype-state.cc */
    snprintf(cmd, sizeof(cmd),
        "g++ -O0 -fprofile-arcs -ftest-coverage -DIN_GCC -DHAVE_CONFIG_H "
        "-I%s -I%s/../../include -I%s/../../gcc "
        "-c %s/%s -o gengtype_state_test.o",
        source_dir, source_dir, source_dir, source_dir, GENGTYPE_STATE_SRC);
    
    printf("Running: %s\n", cmd);
    status = system(cmd);
    if (status != 0) {
        fprintf(stderr, "Failed to compile gengtype-state.cc\n");
        return -1;
    }
    
    /* Link gengtype executable */
    snprintf(cmd, sizeof(cmd),
        "g++ -O0 -fprofile-arcs -ftest-coverage "
        "gengtype_test.o gengtype_state_test.o "
        "-liberty -lgcov -o gengtype_test_exec");
    
    printf("Running: %s\n", cmd);
    status = system(cmd);
    if (status != 0) {
        fprintf(stderr, "Failed to link gengtype executable\n");
        return -1;
    }
    
    return 0;
}

/* Run gengtype on a single file */
static int run_gengtype_single(const char *gt_file, const char *output_base) {
    char cmd[1024];
    int status;
    
    /* Pattern C: Generate header to force full parsing */
    snprintf(cmd, sizeof(cmd),
        "./gengtype_test_exec -g %s.h %s 2>&1",
        output_base, gt_file);
    
    printf("\nRunning gengtype on %s:\n", gt_file);
    printf("Command: %s\n", cmd);
    
    status = system(cmd);
    
    if (status != 0) {
        printf("gengtype exited with status %d (may be expected for error cases)\n",
               WEXITSTATUS(status));
    }
    
    /* Check if output was generated */
    char header_file[256];
    snprintf(header_file, sizeof(header_file), "%s.h", output_base);
    if (access(header_file, F_OK) == 0) {
        printf("Generated header file: %s\n", header_file);
        /* Clean up generated file */
        unlink(header_file);
    }
    
    return status;
}

/* Run gengtype with file list (Pattern B) */
static int run_gengtype_batch(char **gt_files, int count) {
    FILE *list_fp;
    char list_filename[] = "/tmp/gengtype_filelist_XXXXXX";
    char cmd[1024];
    int fd, status;
    int i;
    
    /* Create temporary file list */
    fd = mkstemp(list_filename);
    if (fd == -1) {
        perror("mkstemp for file list");
        return -1;
    }
    
    list_fp = fdopen(fd, "w");
    if (!list_fp) {
        perror("fdopen");
        close(fd);
        unlink(list_filename);
        return -1;
    }
    
    for (i = 0; i < count; i++) {
        fprintf(list_fp, "%s\n", gt_files[i]);
    }
    fclose(list_fp);
    
    /* Run gengtype with -p flag */
    snprintf(cmd, sizeof(cmd),
        "./gengtype_test_exec -p %s 2>&1",
        list_filename);
    
    printf("\nRunning gengtype in batch mode:\n");
    printf("Command: %s\n", cmd);
    
    status = system(cmd);
    
    /* Clean up file list */
    unlink(list_filename);
    
    return status;
}

/* Main test driver */
int main(int argc, char **argv) {
    char *temp_files[sizeof(gt_files)/sizeof(gt_files[0])];
    int i, status;
    int num_files = sizeof(gt_files)/sizeof(gt_files[0]);
    const char *source_dir = ".";
    
    if (argc > 1) {
        source_dir = argv[1];
    }
    
    printf("=== Gengtype Coverage Test ===\n");
    printf("Source directory: %s\n", source_dir);
    
    /* Step 1: Create temporary .gt files */
    printf("\n--- Creating test .gt files ---\n");
    for (i = 0; i < num_files; i++) {
        temp_files[i] = create_temp_file(gt_contents[i], ".gt");
        if (!temp_files[i]) {
            fprintf(stderr, "Failed to create temp file %d\n", i);
            for (int j = 0; j < i; j++) {
                unlink(temp_files[j]);
                free(temp_files[j]);
            }
            return 1;
        }
        printf("Created: %s\n", temp_files[i]);
    }
    
    /* Step 2: Compile gengtype with coverage */
    if (compile_gengtype(source_dir) != 0) {
        fprintf(stderr, "Failed to compile gengtype\n");
        goto cleanup;
    }
    
    /* Step 3: Run gengtype in various modes to exercise the switch */
    
    /* Pattern A: Process each file individually */
    printf("\n--- Pattern A: Individual file processing ---\n");
    for (i = 0; i < num_files; i++) {
        char output_base[256];
        snprintf(output_base, sizeof(output_base), "/tmp/gengtype_output_%d", i);
        run_gengtype_single(temp_files[i], output_base);
    }
    
    /* Pattern B: Batch processing with -p flag */
    printf("\n--- Pattern B: Batch processing ---\n");
    run_gengtype_batch(temp_files, num_files);
    
    /* Pattern C: Multiple files with header generation */
    printf("\n--- Pattern C: Multiple file header generation ---\n");
    char cmd[2048];
    snprintf(cmd, sizeof(cmd),
        "./gengtype_test_exec -g /tmp/combined.h %s %s %s 2>&1",
        temp_files[0], temp_files[1], temp_files[2]);
    
    printf("Command: %s\n", cmd);
    status = system(cmd);
    printf("Exit status: %d\n", WEXITSTATUS(status));
    
    /* Clean up generated header */
    unlink("/tmp/combined.h");
    
    /* Pattern D: Test with -r flag for routine generation */
    printf("\n--- Pattern D: Routine generation ---\n");
    snprintf(cmd, sizeof(cmd),
        "./gengtype_test_exec -r /tmp/routines.c %s %s 2>&1",
        temp_files[0], temp_files[1]);
    
    printf("Command: %s\n", cmd);
    status = system(cmd);
    printf("Exit status: %d\n", WEXITSTATUS(status));
    
    /* Clean up generated routines file */
    unlink("/tmp/routines.c");
    
    /* Step 4: Generate coverage report */
    printf("\n--- Generating coverage report ---\n");
    system("gcov gengtype_test.o 2>&1 | grep -A 20 'gengtype.cc'");
    
    /* Check if specific lines were covered */
    FILE *gcov_file = fopen("gengtype.cc.gcov", "r");
    if (gcov_file) {
        char line[256];
        int target_lines[] = {182, 185, 188, 191, 194, 197, 200, 203, 206, 209, 212};
        int num_targets = sizeof(target_lines)/sizeof(target_lines[0]);
        int covered_count = 0;
        
        printf("\nChecking coverage of target switch statement lines:\n");
        while (fgets(line, sizeof(line), gcov_file)) {
            for (i = 0; i < num_targets; i++) {
                char search[20];
                snprintf(search, sizeof(search), ":%d:", target_lines[i]);
                if (strstr(line, search)) {
                    printf("Line %d: %s", target_lines[i], line);
                    if (strstr(line, "#####") == NULL) {
                        covered_count++;
                    }
                    break;
                }
            }
        }
        fclose(gcov_file);
        
        printf("\nCoverage summary: %d/%d target lines executed\n", 
               covered_count, num_targets);
    }
    
cleanup:
    /* Step 5: Clean up temporary files */
    printf("\n--- Cleaning up ---\n");
    for (i = 0; i < num_files; i++) {
        if (temp_files[i]) {
            unlink(temp_files[i]);
            free(temp_files[i]);
            printf("Removed: %s\n", temp_files[i]);
        }
    }
    
    /* Clean up compiled objects */
    unlink("gengtype_test.o");
    unlink("gengtype_state_test.o");
    unlink("gengtype_test_exec");
    unlink("gengtype_test_exec.gcda");
    unlink("gengtype_test_exec.gcno");
    unlink("gengtype_test.o.gcda");
    unlink("gengtype_test.o.gcno");
    unlink("gengtype_state_test.o.gcda");
    unlink("gengtype_state_test.o.gcno");
    
    printf("\n=== Test completed ===\n");
    return 0;
}
