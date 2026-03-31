/* gengtype_coverage_test.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>

/* Structure to hold .gt file content and filename */
typedef struct {
    const char *filename;
    const char *content;
} gt_file;

/* Multiple .gt files with diverse type definitions */
static const gt_file gt_files[] = {
    {
        "types1.gt",
        "%{\n"
        "/* File 1: Basic type definitions */\n"
        "#include \"config.h\"\n"
        "#include \"system.h\"\n"
        "%}\n"
        "\n"
        "/* TYPE_UNDEFINED: Forward declaration */\n"
        "struct undefined_struct;\n"
        "\n"
        "/* TYPE_SCALAR: Scalar typedefs */\n"
        "typedef int my_scalar;\n"
        "typedef unsigned long scalar2;\n"
        "\n"
        "/* TYPE_STRING: String type usage */\n"
        "struct string_struct {\n"
        "    const char *name;          /* TYPE_STRING */\n"
        "    const char *description;\n"
        "};\n"
        "\n"
        "/* TYPE_STRUCT: Regular struct */\n"
        "struct my_struct {\n"
        "    int a;\n"
        "    float b;\n"
        "};\n"
        "\n"
        "/* TYPE_POINTER: Pointer typedef */\n"
        "typedef struct my_struct *my_ptr;\n"
        "\n"
        "/* TYPE_ARRAY: Array typedef */\n"
        "typedef int my_array[10];\n"
        "typedef struct my_struct struct_array[5];\n"
        "%}\n"
    },
    {
        "types2.gt",
        "%{\n"
        "/* File 2: Advanced and user-defined types */\n"
        "#include \"config.h\"\n"
        "#include \"system.h\"\n"
        "%}\n"
        "\n"
        "/* TYPE_USER_STRUCT: Struct with user marking */\n"
        "struct user_struct {\n"
        "    int *p;\n"
        "    void *data;\n"
        "} GTY((user));\n"
        "\n"
        "/* TYPE_UNION: Union definition */\n"
        "union my_union {\n"
        "    int i;\n"
        "    void *p;\n"
        "    double d;\n"
        "};\n"
        "\n"
        "/* TYPE_CALLBACK: Callback function type */\n"
        "typedef void (*callback_fn)(void);\n"
        "typedef int (*predicate_fn)(const char *);\n"
        "\n"
        "/* Complex nested type combining multiple categories */\n"
        "struct complex_type {\n"
        "    union my_union u;           /* TYPE_UNION */\n"
        "    callback_fn handler;        /* TYPE_CALLBACK */\n"
        "    struct user_struct *users;  /* TYPE_POINTER to TYPE_USER_STRUCT */\n"
        "    int matrix[3][4];          /* TYPE_ARRAY (multi-dimensional) */\n"
        "};\n"
        "\n"
        "/* Another undefined forward declaration */\n"
        "struct another_undefined;\n"
        "%}\n"
    },
    {
        "types3.gt",
        "%{\n"
        "/* File 3: Language-specific and edge cases */\n"
        "#include \"config.h\"\n"
        "#include \"system.h\"\n"
        "%}\n"
        "\n"
        "/* TYPE_LANG_STRUCT: Language-specific struct */\n"
        "struct lang_struct {\n"
        "    int data;\n"
        "    void *lang_data;\n"
        "} GTY ((lang));\n"
        "\n"
        "/* More string types */\n"
        "struct more_strings {\n"
        "    const char *title;\n"
        "    char *buffer;\n"
        "};\n"
        "\n"
        "/* Pointer to array */\n"
        "typedef int (*array_ptr)[10];\n"
        "\n"
        "/* Union containing struct */\n"
        "union union_with_struct {\n"
        "    struct {\n"
        "        int x;\n"
        "        int y;\n"
        "    } point;\n"
        "    long long bits;\n"
        "};\n"
        "\n"
        "/* Callback returning pointer */\n"
        "typedef struct lang_struct* (*lang_callback)(void);\n"
        "%}\n"
    },
    {
        "error_case.gt",
        "%{\n"
        "/* File with syntax error to test error paths */\n"
        "#include \"config.h\"\n"
        "#include \"system.h\"\n"
        "\n"
        "/* Missing closing %} to trigger error */\n"
        "struct error_struct {\n"
        "    int field;\n"
        "};\n"
        "/* No %} here - deliberate error */\n"
    },
    {
        "duplicate_types.gt",
        "%{\n"
        "/* File with duplicate definitions to test warnings */\n"
        "#include \"config.h\"\n"
        "#include \"system.h\"\n"
        "%}\n"
        "\n"
        "/* Duplicate type definition */\n"
        "struct my_struct {\n"
        "    int a;\n"
        "    float b;\n"
        "};\n"
        "\n"
        "/* Same type again */\n"
        "struct my_struct {\n"
        "    int a;\n"
        "    float b;\n"
        "};\n"
        "%}\n"
    }
};

/* Compilation command for gengtype with coverage */
static const char *gengtype_compile_cmd =
    "g++ -O0 -fprofile-arcs -ftest-coverage "
    "-DIN_GCC -DHAVE_CONFIG_H "
    "-I. -I../../include -I../../gcc "
    "-c gengtype.cc -o gengtype_coverage.o 2>&1";

static const char *gengtype_link_cmd =
    "g++ -O0 -fprofile-arcs -ftest-coverage "
    "gengtype_coverage.o "
    "-lgcov -liberty -o gengtype_coverage 2>&1";

/* Write a string to a temporary file */
static char *write_temp_file(const char *content, const char *prefix) {
    char template[256];
    snprintf(template, sizeof(template), "/tmp/%s_XXXXXX", prefix);
    
    int fd = mkstemp(template);
    if (fd == -1) {
        perror("mkstemp failed");
        return NULL;
    }
    
    FILE *f = fdopen(fd, "w");
    if (!f) {
        perror("fdopen failed");
        close(fd);
        return NULL;
    }
    
    fwrite(content, 1, strlen(content), f);
    fclose(f);
    
    return strdup(template);
}

/* Execute a command and return its output */
static int execute_command(const char *cmd, char **output) {
    FILE *pipe = popen(cmd, "r");
    if (!pipe) {
        return -1;
    }
    
    char buffer[4096];
    size_t total_size = 0;
    *output = malloc(1);
    **output = '\0';
    
    while (fgets(buffer, sizeof(buffer), pipe)) {
        size_t len = strlen(buffer);
        *output = realloc(*output, total_size + len + 1);
        strcpy(*output + total_size, buffer);
        total_size += len;
    }
    
    int status = pclose(pipe);
    return WEXITSTATUS(status);
}

/* Run gengtype on a single file */
static int run_gengtype_on_file(const char *filename, int generate_output) {
    char cmd[1024];
    char *output = NULL;
    int status;
    
    if (generate_output) {
        /* Generate header and routine files */
        snprintf(cmd, sizeof(cmd), 
                 "./gengtype_coverage -g gtype-header.h -r gtype-routines.c %s 2>&1",
                 filename);
    } else {
        /* Just parse the file */
        snprintf(cmd, sizeof(cmd), "./gengtype_coverage %s 2>&1", filename);
    }
    
    printf("Executing: %s\n", cmd);
    status = execute_command(cmd, &output);
    
    if (output) {
        printf("Output:\n%s\n", output);
        free(output);
    }
    
    return status;
}

/* Run gengtype with file list (-p option) */
static int run_gengtype_with_filelist(const char **files, int count) {
    char *list_filename = write_temp_file("", "gtlist");
    if (!list_filename) {
        return -1;
    }
    
    /* Write file list */
    FILE *list = fopen(list_filename, "w");
    if (!list) {
        free(list_filename);
        return -1;
    }
    
    for (int i = 0; i < count; i++) {
        fprintf(list, "%s\n", files[i]);
    }
    fclose(list);
    
    /* Run gengtype with -p option */
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), 
             "./gengtype_coverage -p %s 2>&1", list_filename);
    
    printf("Executing with file list: %s\n", cmd);
    char *output = NULL;
    int status = execute_command(cmd, &output);
    
    if (output) {
        printf("Output:\n%s\n", output);
        free(output);
    }
    
    unlink(list_filename);
    free(list_filename);
    return status;
}

/* Main test driver */
int main(int argc, char **argv) {
    printf("=== Gengtype Coverage Test ===\n\n");
    
    /* Step 1: Compile gengtype with coverage instrumentation */
    printf("1. Compiling gengtype with coverage...\n");
    char *compile_output = NULL;
    int compile_status = execute_command(gengtype_compile_cmd, &compile_output);
    
    if (compile_output) {
        printf("Compilation output:\n%s\n", compile_output);
        free(compile_output);
    }
    
    if (compile_status != 0) {
        printf("Compilation failed!\n");
        return 1;
    }
    
    /* Step 2: Link gengtype */
    printf("\n2. Linking gengtype...\n");
    char *link_output = NULL;
    int link_status = execute_command(gengtype_link_cmd, &link_output);
    
    if (link_output) {
        printf("Linking output:\n%s\n", link_output);
        free(link_output);
    }
    
    if (link_status != 0) {
        printf("Linking failed!\n");
        return 1;
    }
    
    /* Step 3: Create temporary .gt files */
    printf("\n3. Creating temporary .gt files...\n");
    char *temp_files[sizeof(gt_files)/sizeof(gt_files[0])];
    int num_files = sizeof(gt_files)/sizeof(gt_files[0]);
    
    for (int i = 0; i < num_files; i++) {
        temp_files[i] = write_temp_file(gt_files[i].content, gt_files[i].filename);
        if (temp_files[i]) {
            printf("  Created: %s\n", temp_files[i]);
        } else {
            printf("  Failed to create: %s\n", gt_files[i].filename);
            return 1;
        }
    }
    
    /* Step 4: Run gengtype in various modes to trigger coverage */
    printf("\n4. Running gengtype in different modes...\n");
    
    /* Pattern A: Process each file individually */
    printf("\n--- Pattern A: Individual file processing ---\n");
    for (int i = 0; i < num_files; i++) {
        printf("\nProcessing %s:\n", temp_files[i]);
        run_gengtype_on_file(temp_files[i], i < 3); /* Generate output for first 3 files */
    }
    
    /* Pattern B: Batch processing with -p flag */
    printf("\n--- Pattern B: Batch processing with -p flag ---\n");
    const char *valid_files[3] = {temp_files[0], temp_files[1], temp_files[2]};
    run_gengtype_with_filelist(valid_files, 3);
    
    /* Pattern C: Generate header with multiple input files */
    printf("\n--- Pattern C: Generate header with multiple files ---\n");
    char cmd[1024];
    snprintf(cmd, sizeof(cmd),
             "./gengtype_coverage -g combined.h -r combined.c %s %s %s 2>&1",
             temp_files[0], temp_files[1], temp_files[2]);
    
    printf("Executing: %s\n", cmd);
    char *combined_output = NULL;
    execute_command(cmd, &combined_output);
    if (combined_output) {
        printf("Output:\n%s\n", combined_output);
        free(combined_output);
    }
    
    /* Pattern D: Test with error cases */
    printf("\n--- Pattern D: Error and warning cases ---\n");
    printf("\nProcessing file with syntax error:\n");
    run_gengtype_on_file(temp_files[3], 0);
    
    printf("\nProcessing file with duplicate definitions:\n");
    run_gengtype_on_file(temp_files[4], 0);
    
    /* Step 5: Cleanup temporary files */
    printf("\n5. Cleaning up temporary files...\n");
    for (int i = 0; i < num_files; i++) {
        if (temp_files[i]) {
            unlink(temp_files[i]);
            free(temp_files[i]);
            printf("  Removed: %s\n", temp_files[i]);
        }
    }
    
    /* Remove generated output files */
    remove("gtype-header.h");
    remove("gtype-routines.c");
    remove("combined.h");
    remove("combined.c");
    remove("gengtype_coverage");
    remove("gengtype_coverage.o");
    
    /* Step 6: Generate coverage report */
    printf("\n6. Generating coverage report...\n");
    system("gcov gengtype_coverage.o 2>&1");
    
    /* Check if gengtype.gcda exists and show coverage */
    FILE *gcov_file = fopen("gengtype.gcda", "rb");
    if (gcov_file) {
        fclose(gcov_file);
        printf("\nCoverage data generated: gengtype.gcda\n");
        printf("Use 'gcov gengtype.cc' to see line-by-line coverage.\n");
    }
    
    /* Look for the specific switch statement coverage */
    printf("\n=== Checking for target switch statement coverage ===\n");
    system("gcov -b gengtype.cc 2>&1 | grep -A5 -B5 '182-213' || true");
    
    printf("\n=== Test completed ===\n");
    printf("The uncovered switch statement (lines 182-213) should now be executed.\n");
    printf("Check gengtype.c.gcov for detailed coverage information.\n");
    
    return 0;
}
