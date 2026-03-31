/* driver.c - Test driver for gengtype coverage */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

/* Temporary file management */
typedef struct {
    char *filename;
    char *content;
} temp_file_t;

/* Create a temporary file with given content */
char *create_temp_file(const char *content, const char *suffix) {
    char template[256];
    snprintf(template, sizeof(template), "/tmp/gengtype_test_XXXXXX%s", suffix);
    int fd = mkstemps(template, strlen(suffix));
    if (fd < 0) {
        perror("mkstemps failed");
        return NULL;
    }
    
    FILE *f = fdopen(fd, "w");
    if (!f) {
        perror("fdopen failed");
        close(fd);
        return NULL;
    }
    
    fwrite(content, 1, strlen(content), f);
    fclose(fd);
    
    return strdup(template);
}

/* Clean up temporary files */
void cleanup_temp_files(char **files, int count) {
    for (int i = 0; i < count; i++) {
        if (files[i]) {
            unlink(files[i]);
            free(files[i]);
        }
    }
}

/* Execute gengtype with given arguments */
int run_gengtype(const char *gengtype_exe, char **args, int arg_count) {
    pid_t pid = fork();
    if (pid == 0) {
        /* Child process */
        char **argv = malloc((arg_count + 2) * sizeof(char *));
        argv[0] = (char *)gengtype_exe;
        for (int i = 0; i < arg_count; i++) {
            argv[i + 1] = args[i];
        }
        argv[arg_count + 1] = NULL;
        
        execvp(gengtype_exe, argv);
        perror("execvp failed");
        exit(1);
    } else if (pid > 0) {
        /* Parent process */
        int status;
        waitpid(pid, &status, 0);
        return WEXITSTATUS(status);
    } else {
        perror("fork failed");
        return -1;
    }
}

/* Compile gengtype with coverage instrumentation */
int compile_gengtype_with_coverage() {
    printf("Compiling gengtype with coverage instrumentation...\n");
    
    /* Compilation command for gengtype */
    const char *compile_cmd = 
        "g++ -O0 -fprofile-arcs -ftest-coverage "
        "-DIN_GCC -DHAVE_CONFIG_H "
        "-I. -I../../include -I../../gcc "
        "-c gengtype.cc -o gengtype.o 2>&1";
    
    printf("Running: %s\n", compile_cmd);
    return system(compile_cmd);
}

/* Link gengtype executable */
int link_gengtype() {
    printf("Linking gengtype executable...\n");
    
    const char *link_cmd = 
        "g++ -O0 -fprofile-arcs -ftest-coverage "
        "gengtype.o gengtype-state.o gengtype-lex.o "
        "-lgcov -liberty -o gengtype_coverage 2>&1";
    
    printf("Running: %s\n", link_cmd);
    return system(link_cmd);
}

/* Main test driver */
int main(int argc, char **argv) {
    printf("=== Gengtype Coverage Test Driver ===\n");
    
    /* Pattern A: Multiple .gt files with diverse type definitions */
    
    /* File 1: Basic types and scalar definitions */
    const char *gt_file1 = 
        "%{\n"
        "/* Test file 1: Basic scalar and struct types */\n"
        "#include \"config.h\"\n"
        "#include \"system.h\"\n"
        "%}\n"
        "\n"
        "/* TYPE_UNDEFINED: Forward declaration */\n"
        "struct undefined_struct;\n"
        "\n"
        "/* TYPE_SCALAR: Scalar typedefs */\n"
        "typedef int my_scalar;\n"
        "typedef unsigned long my_ulong;\n"
        "\n"
        "/* TYPE_STRUCT: Regular struct */\n"
        "struct my_struct {\n"
        "    int a;\n"
        "    double b;\n"
        "};\n"
        "\n"
        "/* TYPE_POINTER: Pointer typedef */\n"
        "typedef struct my_struct *my_ptr;\n"
        "\n"
        "/* TYPE_ARRAY: Array typedef */\n"
        "typedef int my_array[10];\n"
        "typedef struct my_struct struct_array[5];\n"
        "\n"
        "/* TYPE_STRING: String usage */\n"
        "struct string_struct {\n"
        "    const char *name;  /* TYPE_STRING */\n"
        "    char *data;\n"
        "};\n"
        "\n"
        "/* TYPE_CALLBACK: Function pointer */\n"
        "typedef void (*callback_fn)(int, void*);\n"
        "\n"
        "/* Nested complex type combining multiple categories */\n"
        "struct complex_type {\n"
        "    my_scalar scalar_field;\n"
        "    my_array array_field;\n"
        "    callback_fn callback_field;\n"
        "    struct string_struct *string_ptr;\n"
        "};\n";
    
    /* File 2: User structs, unions, and language structs */
    const char *gt_file2 = 
        "%{\n"
        "/* Test file 2: User structs, unions, and language-specific types */\n"
        "#include \"config.h\"\n"
        "#include \"system.h\"\n"
        "%}\n"
        "\n"
        "/* TYPE_USER_STRUCT: Struct with user-provided marking */\n"
        "struct user_struct {\n"
        "    int *p;\n"
        "    void *data;\n"
        "} GTY((user));\n"
        "\n"
        "/* TYPE_UNION: Union type */\n"
        "union my_union {\n"
        "    int i;\n"
        "    void *p;\n"
        "    double d;\n"
        "};\n"
        "\n"
        "/* TYPE_LANG_STRUCT: Language-specific struct */\n"
        "struct lang_struct {\n"
        "    int data;\n"
        "    void *lang_data;\n"
        "} GTY((lang));\n"
        "\n"
        "/* Complex nested type with union */\n"
        "struct container {\n"
        "    union my_union u;\n"
        "    struct lang_struct *lang_ptr;\n"
        "    struct user_struct user;\n"
        "};\n"
        "\n"
        "/* Pointer to union */\n"
        "typedef union my_union *union_ptr;\n"
        "\n"
        "/* Array of unions */\n"
        "typedef union my_union union_array[8];\n"
        "\n"
        "/* Callback that takes user struct */\n"
        "typedef void (*user_callback)(struct user_struct *);\n";
    
    /* File 3: More complex types and error cases */
    const char *gt_file3 = 
        "%{\n"
        "/* Test file 3: Complex nested types and edge cases */\n"
        "#include \"config.h\"\n"
        "#include \"system.h\"\n"
        "%}\n"
        "\n"
        "/* Multiple levels of indirection */\n"
        "struct level1 {\n"
        "    int a;\n"
        "};\n"
        "\n"
        "struct level2 {\n"
        "    struct level1 *l1;\n"
        "    int b;\n"
        "};\n"
        "\n"
        "struct level3 {\n"
        "    struct level2 **l2;\n"
        "    struct level1 l1_array[3];\n"
        "};\n"
        "\n"
        "/* Mixed array types */\n"
        "typedef struct level3 *ptr_array[4];\n"
        "typedef int (*func_ptr_array[2])(void);\n"
        "\n"
        "/* String arrays */\n"
        "struct string_container {\n"
        "    const char *strings[5];\n"
        "    char *dynamic_strings[3];\n"
        "};\n"
        "\n"
        "/* Complex callback signature */\n"
        "typedef int (*complex_callback)(struct level3 ***, const char *[], int);\n"
        "\n"
        "/* Self-referential structure */\n"
        "struct tree_node {\n"
        "    int value;\n"
        "    struct tree_node *left;\n"
        "    struct tree_node *right;\n"
        "    struct tree_node *parent;\n"
        "};\n";
    
    /* File 4: Deliberate syntax error (for error path testing) */
    const char *gt_file4 = 
        "%{\n"
        "/* Test file 4: File with syntax error (missing closing %) */\n"
        "#include \"config.h\"\n"
        "#include \"system.h\"\n"
        "/* Missing closing %}\n"
        "\n"
        "struct error_struct {\n"
        "    int will_not_parse;\n"
        "};\n";
    
    /* File 5: Duplicate definitions (for warning testing) */
    const char *gt_file5 = 
        "%{\n"
        "/* Test file 5: Duplicate type definitions */\n"
        "#include \"config.h\"\n"
        "#include \"system.h\"\n"
        "%}\n"
        "\n"
        "/* First definition */\n"
        "struct duplicate_struct {\n"
        "    int a;\n"
        "};\n"
        "\n"
        "/* Duplicate definition - should trigger warning */\n"
        "struct duplicate_struct {\n"
        "    int b;  /* Different but still duplicate */\n"
        "};\n"
        "\n"
        "/* Another scalar duplicate */\n"
        "typedef int my_int;\n"
        "typedef int my_int;  /* Duplicate */\n";
    
    /* Create temporary files */
    char *temp_files[10];
    int file_count = 0;
    
    printf("Creating temporary .gt files...\n");
    
    temp_files[file_count++] = create_temp_file(gt_file1, ".gt");
    temp_files[file_count++] = create_temp_file(gt_file2, ".gt");
    temp_files[file_count++] = create_temp_file(gt_file3, ".gt");
    temp_files[file_count++] = create_temp_file(gt_file4, ".gt");
    temp_files[file_count++] = create_temp_file(gt_file5, ".gt");
    
    /* Create file list for batch processing (Pattern B) */
    char filelist_content[1024] = "";
    for (int i = 0; i < 3; i++) {  /* Only first 3 valid files */
        strcat(filelist_content, temp_files[i]);
        strcat(filelist_content, "\n");
    }
    
    char *filelist = create_temp_file(filelist_content, ".list");
    temp_files[file_count++] = filelist;
    
    /* Create output files */
    char *output_header = create_temp_file("", ".h");
    char *output_routine = create_temp_file("", ".c");
    temp_files[file_count++] = output_header;
    temp_files[file_count++] = output_routine;
    
    /* First, compile and link gengtype with coverage */
    if (compile_gengtype_with_coverage() != 0) {
        fprintf(stderr, "Failed to compile gengtype\n");
        cleanup_temp_files(temp_files, file_count);
        return 1;
    }
    
    if (link_gengtype() != 0) {
        fprintf(stderr, "Failed to link gengtype\n");
        cleanup_temp_files(temp_files, file_count);
        return 1;
    }
    
    printf("\n=== Running gengtype with various patterns ===\n");
    
    /* Pattern A: Process each file individually */
    printf("\n--- Pattern A: Individual file processing ---\n");
    for (int i = 0; i < 3; i++) {
        printf("Processing %s...\n", temp_files[i]);
        char *args[] = {"-g", output_header, temp_files[i]};
        int status = run_gengtype("./gengtype_coverage", args, 3);
        printf("  Exit status: %d\n", status);
    }
    
    /* Pattern B: Batch processing with -p */
    printf("\n--- Pattern B: Batch processing with -p ---\n");
    char *batch_args[] = {"-p", filelist};
    printf("Batch processing files from %s...\n", filelist);
    int batch_status = run_gengtype("./gengtype_coverage", batch_args, 2);
    printf("  Exit status: %d\n", batch_status);
    
    /* Pattern C: Header generation with multiple files */
    printf("\n--- Pattern C: Header generation with all valid files ---\n");
    char *header_args[] = {"-g", output_header, temp_files[0], temp_files[1], temp_files[2]};
    printf("Generating header from 3 files...\n");
    int header_status = run_gengtype("./gengtype_coverage", header_args, 5);
    printf("  Exit status: %d\n", header_status);
    
    /* Pattern D: Error and warning cases */
    printf("\n--- Pattern D: Error and warning testing ---\n");
    
    /* Test syntax error file */
    printf("Testing syntax error file (%s)...\n", temp_files[3]);
    char *error_args[] = {"-g", output_header, temp_files[3]};
    int error_status = run_gengtype("./gengtype_coverage", error_args, 3);
    printf("  Exit status (expected non-zero): %d\n", error_status);
    
    /* Test duplicate definitions file */
    printf("Testing duplicate definitions file (%s)...\n", temp_files[4]);
    char *dup_args[] = {"-g", output_header, temp_files[4]};
    int dup_status = run_gengtype("./gengtype_coverage", dup_args, 3);
    printf("  Exit status: %d\n", dup_status);
    
    /* Generate routine file as well */
    printf("\n--- Generating routine file ---\n");
    char *routine_args[] = {"-r", output_routine, temp_files[0], temp_files[1], temp_files[2]};
    printf("Generating routine file...\n");
    int routine_status = run_gengtype("./gengtype_coverage", routine_args, 5);
    printf("  Exit status: %d\n", routine_status);
    
    /* Verify outputs were created */
    printf("\n=== Verifying outputs ===\n");
    FILE *f = fopen(output_header, "r");
    if (f) {
        fseek(f, 0, SEEK_END);
        long size = ftell(f);
        fclose(f);
        printf("Header file size: %ld bytes\n", size);
    } else {
        printf("Header file not created\n");
    }
    
    f = fopen(output_routine, "r");
    if (f) {
        fseek(f, 0, SEEK_END);
        long size = ftell(f);
        fclose(f);
        printf("Routine file size: %ld bytes\n", size);
    } else {
        printf("Routine file not created\n");
    }
    
    /* Clean up */
    printf("\n=== Cleaning up ===\n");
    cleanup_temp_files(temp_files, file_count);
    
    /* Remove coverage executable */
    unlink("gengtype_coverage");
    unlink("gengtype.o");
    
    printf("\n=== Test completed ===\n");
    printf("Coverage data should be in gengtype.gcda and related files\n");
    printf("Run 'gcov gengtype.cc' to see coverage results\n");
    
    return 0;
}
