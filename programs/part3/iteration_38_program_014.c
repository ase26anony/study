/* driver.c - Test program to exercise gengtype type counting logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

/* gengtype source files needed for compilation */
#define GT_SOURCE_FILES "gengtype.cc gengtype-state.cc gengtype-lex.cc"

/* Create temporary .gt file with given content */
static char* create_temp_gt_file(const char* content) {
    char template[] = "/tmp/gt_test_XXXXXX";
    int fd = mkstemp(template);
    if (fd == -1) {
        perror("mkstemp failed");
        return NULL;
    }
    
    write(fd, content, strlen(content));
    close(fd);
    
    return strdup(template);
}

/* Build gengtype with coverage instrumentation */
static int build_gengtype_with_coverage() {
    printf("Building gengtype with coverage instrumentation...\n");
    
    /* Compile each source file with coverage flags */
    const char* compile_cmd = 
        "g++ -O0 -fprofile-arcs -ftest-coverage "
        "-DIN_GCC -DHAVE_CONFIG_H "
        "-I. -I../../include -I../../gcc "
        "-c gengtype.cc -o gengtype.o 2>&1";
    
    if (system(compile_cmd) != 0) {
        fprintf(stderr, "Failed to compile gengtype.cc\n");
        return 0;
    }
    
    /* Compile other required files similarly */
    const char* compile_state_cmd = 
        "g++ -O0 -fprofile-arcs -ftest-coverage "
        "-DIN_GCC -DHAVE_CONFIG_H "
        "-I. -I../../include -I../../gcc "
        "-c gengtype-state.cc -o gengtype-state.o 2>&1";
    
    if (system(compile_state_cmd) != 0) {
        fprintf(stderr, "Failed to compile gengtype-state.cc\n");
        return 0;
    }
    
    /* Link everything together */
    const char* link_cmd = 
        "g++ -O0 -fprofile-arcs -ftest-coverage "
        "gengtype.o gengtype-state.o "
        "-lgcov -liberty -o gengtype_coverage 2>&1";
    
    if (system(link_cmd) != 0) {
        fprintf(stderr, "Failed to link gengtype\n");
        return 0;
    }
    
    return 1;
}

/* Run gengtype on a single file */
static int run_gengtype_on_file(const char* filename, const char* mode) {
    char cmd[1024];
    int status;
    
    if (strcmp(mode, "header") == 0) {
        snprintf(cmd, sizeof(cmd), 
                 "./gengtype_coverage -g /tmp/output.h %s 2>&1", filename);
    } else if (strcmp(mode, "routine") == 0) {
        snprintf(cmd, sizeof(cmd), 
                 "./gengtype_coverage -r /tmp/output.c %s 2>&1", filename);
    } else {
        snprintf(cmd, sizeof(cmd), 
                 "./gengtype_coverage %s 2>&1", filename);
    }
    
    printf("Running: %s\n", cmd);
    status = system(cmd);
    
    if (WIFEXITED(status)) {
        printf("Exit status: %d\n", WEXITSTATUS(status));
        return WEXITSTATUS(status) == 0;
    }
    
    return 0;
}

/* Run gengtype with file list */
static int run_gengtype_with_filelist(const char** files, int count) {
    char cmd[2048];
    int status;
    FILE* listfile;
    
    /* Create file list */
    listfile = fopen("/tmp/gt_filelist.txt", "w");
    if (!listfile) {
        perror("Failed to create file list");
        return 0;
    }
    
    for (int i = 0; i < count; i++) {
        fprintf(listfile, "%s\n", files[i]);
    }
    fclose(listfile);
    
    snprintf(cmd, sizeof(cmd),
             "./gengtype_coverage -p /tmp/gt_filelist.txt 2>&1");
    
    printf("Running batch: %s\n", cmd);
    status = system(cmd);
    
    unlink("/tmp/gt_filelist.txt");
    
    return WIFEXITED(status) && WEXITSTATUS(status) == 0;
}

int main() {
    char* temp_files[10];
    int file_count = 0;
    int success = 1;
    
    /* Define various .gt files covering all type categories */
    
    /* File 1: Basic types and structs */
    const char* gt_file1 = 
        "%{\n"
        "/* Test file 1: Basic types */\n"
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
        "    const char *name;  /* string type */\n"
        "    char *data;\n"
        "};\n"
        "\n"
        "/* TYPE_STRUCT */\n"
        "struct my_struct {\n"
        "    int a;\n"
        "    double b;\n"
        "};\n"
        "\n"
        "/* TYPE_POINTER */\n"
        "typedef struct my_struct *my_ptr;\n"
        "typedef void (*void_func_ptr)(void);\n"
        "%}";
    
    /* File 2: User structs, unions, and arrays */
    const char* gt_file2 = 
        "%{\n"
        "/* Test file 2: Complex types */\n"
        "#include <stdlib.h>\n"
        "%}\n"
        "\n"
        "/* TYPE_USER_STRUCT with custom marking */\n"
        "struct user_struct {\n"
        "    int *p;\n"
        "    void *data;\n"
        "} GTY((user));\n"
        "\n"
        "/* TYPE_UNION */\n"
        "union my_union {\n"
        "    int i;\n"
        "    void *p;\n"
        "    double d;\n"
        "};\n"
        "\n"
        "/* TYPE_ARRAY */\n"
        "typedef int my_array[10];\n"
        "typedef struct my_struct struct_array[5];\n"
        "\n"
        "/* Nested complex type */\n"
        "struct container {\n"
        "    union my_union *uptr;  /* pointer to union */\n"
        "    my_array arr;          /* array */\n"
        "    struct user_struct *user;  /* pointer to user struct */\n"
        "};\n"
        "\n"
        "/* Another scalar */\n"
        "typedef float float_scalar;\n"
        "%}";
    
    /* File 3: Callbacks, lang structs, and error cases */
    const char* gt_file3 = 
        "%{\n"
        "/* Test file 3: Special types */\n"
        "%}\n"
        "\n"
        "/* TYPE_CALLBACK */\n"
        "typedef void (*callback_fn)(void);\n"
        "typedef int (*compare_fn)(const void *, const void *);\n"
        "\n"
        "/* TYPE_LANG_STRUCT */\n"
        "struct lang_struct {\n"
        "    int data;\n"
        "    void *extra;\n"
        "} GTY ((lang));\n"
        "\n"
        "/* More string types */\n"
        "struct lang_info {\n"
        "    const char *lang_name;\n"
        "    callback_fn init_func;\n"
        "};\n"
        "\n"
        "/* Complex nested type combining multiple categories */\n"
        "struct super_complex {\n"
        "    union {\n"
        "        struct lang_struct *lang;\n"
        "        callback_fn cb;\n"
        "    } u;\n"
        "    my_array matrix[5][5];  /* 2D array of arrays */\n"
        "    struct undefined_struct *undef_ptr;  /* pointer to undefined */\n"
        "};\n"
        "%}";
    
    /* File 4: File with syntax error (to test error paths) */
    const char* gt_file4 = 
        "%{\n"
        "/* Test file 4: Deliberate syntax error */\n"
        "struct error_struct {\n"
        "    int missing_semicolon\n"  /* Missing semicolon */
        "};\n"
        "\n"
        "/* Missing closing %} */\n";
    
    /* File 5: Duplicate definitions (to test warning paths) */
    const char* gt_file5 = 
        "%{\n"
        "/* Test file 5: Duplicate types */\n"
        "%}\n"
        "\n"
        "struct duplicate_struct {\n"
        "    int x;\n"
        "};\n"
        "\n"
        "/* Duplicate definition */\n"
        "struct duplicate_struct {\n"
        "    int y;\n"
        "};\n"
        "\n"
        "/* Valid scalar */\n"
        "typedef short short_scalar;\n"
        "%}";
    
    /* Build gengtype with coverage */
    if (!build_gengtype_with_coverage()) {
        fprintf(stderr, "Failed to build gengtype\n");
        return 1;
    }
    
    /* Create temporary .gt files */
    temp_files[file_count++] = create_temp_gt_file(gt_file1);
    temp_files[file_count++] = create_temp_gt_file(gt_file2);
    temp_files[file_count++] = create_temp_gt_file(gt_file3);
    temp_files[file_count++] = create_temp_gt_file(gt_file4);
    temp_files[file_count++] = create_temp_gt_file(gt_file5);
    
    /* Test 1: Process each file individually for header generation */
    printf("\n=== Test 1: Individual file processing ===\n");
    for (int i = 0; i < 3; i++) {  /* First 3 valid files */
        printf("\nProcessing file %d...\n", i+1);
        if (!run_gengtype_on_file(temp_files[i], "header")) {
            printf("Warning: File %d processing had issues\n", i+1);
        }
    }
    
    /* Test 2: Batch processing with -p flag */
    printf("\n=== Test 2: Batch processing ===\n");
    if (!run_gengtype_with_filelist((const char**)temp_files, 3)) {
        printf("Batch processing had issues\n");
        success = 0;
    }
    
    /* Test 3: Process with routine generation */
    printf("\n=== Test 3: Routine generation ===\n");
    for (int i = 0; i < 3; i++) {
        printf("\nGenerating routines for file %d...\n", i+1);
        if (!run_gengtype_on_file(temp_files[i], "routine")) {
            printf("Routine generation for file %d had issues\n", i+1);
        }
    }
    
    /* Test 4: Process files with errors (should still trigger parsing) */
    printf("\n=== Test 4: Error case processing ===\n");
    printf("Processing file with syntax error...\n");
    run_gengtype_on_file(temp_files[3], "header");
    
    printf("Processing file with duplicate definition...\n");
    run_gengtype_on_file(temp_files[4], "header");
    
    /* Test 5: Process all files together */
    printf("\n=== Test 5: Combined processing ===\n");
    {
        char cmd[4096] = "./gengtype_coverage -g /tmp/combined.h ";
        for (int i = 0; i < 3; i++) {
            strcat(cmd, temp_files[i]);
            strcat(cmd, " ");
        }
        strcat(cmd, "2>&1");
        
        printf("Running: %s\n", cmd);
        system(cmd);
    }
    
    /* Cleanup temporary files */
    printf("\n=== Cleaning up ===\n");
    for (int i = 0; i < file_count; i++) {
        if (temp_files[i]) {
            unlink(temp_files[i]);
            free(temp_files[i]);
        }
    }
    
    /* Cleanup generated files */
    unlink("/tmp/output.h");
    unlink("/tmp/output.c");
    unlink("/tmp/combined.h");
    
    /* Display coverage hint */
    printf("\n=== Coverage Collection ===\n");
    printf("To collect coverage data:\n");
    printf("1. Run: gcov gengtype.cc\n");
    printf("2. Check gengtype.cc.gcov for line execution counts\n");
    printf("3. Look for the switch statement at lines 182-213\n");
    printf("4. All case statements should have positive counts\n");
    
    /* Check if gengtype binary was created and run */
    if (access("gengtype_coverage", F_OK) == 0) {
        printf("\nGengtype executable exists and was run.\n");
        printf("Coverage data should be in gengtype.gcda\n");
    } else {
        printf("\nERROR: gengtype_coverage executable not found!\n");
        success = 0;
    }
    
    return success ? 0 : 1;
}
