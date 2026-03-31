/* test_gengtype_coverage.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

/* Create temporary .gt files with various type definitions */
void create_gt_file(const char *filename, const char *content) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        perror("fopen");
        exit(1);
    }
    fprintf(f, "%s", content);
    fclose(f);
}

/* Execute gengtype with coverage instrumentation */
int run_gengtype(const char *gengtype_exe, const char **args, int argc) {
    pid_t pid = fork();
    if (pid == 0) {
        /* Child process */
        char **argv = malloc((argc + 2) * sizeof(char *));
        argv[0] = (char *)gengtype_exe;
        for (int i = 0; i < argc; i++) {
            argv[i + 1] = (char *)args[i];
        }
        argv[argc + 1] = NULL;
        
        execvp(gengtype_exe, argv);
        perror("execvp");
        exit(1);
    } else if (pid > 0) {
        /* Parent process */
        int status;
        waitpid(pid, &status, 0);
        return WEXITSTATUS(status);
    } else {
        perror("fork");
        return -1;
    }
}

int main(int argc, char **argv) {
    char tmpdir_template[] = "/tmp/gengtype_test_XXXXXX";
    char *tmpdir = mkdtemp(tmpdir_template);
    if (!tmpdir) {
        perror("mkdtemp");
        return 1;
    }
    
    printf("Created temporary directory: %s\n", tmpdir);
    
    /* File 1: Basic types and structs */
    char file1_path[256];
    snprintf(file1_path, sizeof(file1_path), "%s/types1.gt", tmpdir);
    const char *file1_content = 
        "%{\n"
        "/* Test file 1: Basic types */\n"
        "#include \"config.h\"\n"
        "#include \"system.h\"\n"
        "%}\n"
        "\n"
        "/* TYPE_UNDEFINED: Forward declaration */\n"
        "struct undefined_struct;\n"
        "\n"
        "/* TYPE_SCALAR: Scalar typedef */\n"
        "typedef int my_scalar;\n"
        "typedef unsigned long another_scalar;\n"
        "\n"
        "/* TYPE_STRING: String type */\n"
        "struct string_struct {\n"
        "  const char *name;  /* TYPE_STRING */\n"
        "  char *data;\n"
        "};\n"
        "\n"
        "/* TYPE_STRUCT: Regular struct */\n"
        "struct my_struct {\n"
        "  int a;\n"
        "  double b;\n"
        "};\n"
        "\n"
        "/* TYPE_POINTER: Pointer types */\n"
        "typedef struct my_struct *my_ptr;\n"
        "typedef my_scalar *scalar_ptr;\n"
        "\n"
        "/* TYPE_ARRAY: Array types */\n"
        "typedef int my_array[10];\n"
        "typedef struct my_struct struct_array[5];\n"
        "\n"
        "/* TYPE_CALLBACK: Callback function type */\n"
        "typedef void (*callback_fn)(void);\n"
        "typedef int (*compare_fn)(const void *, const void *);\n";
    
    create_gt_file(file1_path, file1_content);
    
    /* File 2: Unions, user structs, and complex types */
    char file2_path[256];
    snprintf(file2_path, sizeof(file2_path), "%s/types2.gt", tmpdir);
    const char *file2_content = 
        "%{\n"
        "/* Test file 2: Advanced types */\n"
        "#include \"config.h\"\n"
        "#include \"system.h\"\n"
        "%}\n"
        "\n"
        "/* TYPE_UNION: Union type */\n"
        "union my_union {\n"
        "  int i;\n"
        "  void *p;\n"
        "  double d;\n"
        "};\n"
        "\n"
        "/* TYPE_USER_STRUCT: Struct with user marking */\n"
        "struct user_struct {\n"
        "  int *p;\n"
        "  void *data;\n"
        "} GTY((user));\n"
        "\n"
        "/* TYPE_LANG_STRUCT: Language-specific struct */\n"
        "struct lang_struct {\n"
        "  int data;\n"
        "  void *extra;\n"
        "} GTY((lang));\n"
        "\n"
        "/* Complex nested types */\n"
        "struct complex_struct {\n"
        "  union my_union u;           /* TYPE_UNION */\n"
        "  struct my_struct *next;     /* TYPE_POINTER to TYPE_STRUCT */\n"
        "  callback_fn handler;        /* TYPE_CALLBACK */\n"
        "  int values[20];             /* TYPE_ARRAY of TYPE_SCALAR */\n"
        "  const char *description;    /* TYPE_STRING */\n"
        "};\n"
        "\n"
        "/* Array of pointers to unions */\n"
        "typedef union my_union *union_ptr_array[8];\n"
        "\n"
        "/* Callback returning pointer to struct */\n"
        "typedef struct complex_struct *(*factory_fn)(int);\n";
    
    create_gt_file(file2_path, file2_content);
    
    /* File 3: More variations and edge cases */
    char file3_path[256];
    snprintf(file3_path, sizeof(file3_path), "%s/types3.gt", tmpdir);
    const char *file3_content = 
        "%{\n"
        "/* Test file 3: Edge cases and duplicates */\n"
        "#include \"config.h\"\n"
        "#include \"system.h\"\n"
        "%}\n"
        "\n"
        "/* Another undefined struct */\n"
        "struct another_undefined;\n"
        "\n"
        "/* More scalar types */\n"
        "typedef char byte;\n"
        "typedef float real;\n"
        "\n"
        "/* String in union */\n"
        "union string_union {\n"
        "  const char *str;\n"
        "  int id;\n"
        "};\n"
        "\n"
        "/* Struct with array of callbacks */\n"
        "struct callback_container {\n"
        "  callback_fn handlers[5];\n"
        "  void (*fallback)(int, char *);\n"
        "};\n"
        "\n"
        "/* Pointer to array */\n"
        "typedef int (*array_ptr)[10];\n"
        "\n"
        "/* Multi-dimensional array */\n"
        "typedef int matrix[3][4];\n"
        "\n"
        "/* Another lang struct with different options */\n"
        "struct lang_struct2 {\n"
        "  matrix m;\n"
        "  byte *buffer;\n"
        "} GTY((lang, desc(\"language_specific_2\")));\n"
        "\n"
        "/* User struct with nested types */\n"
        "struct nested_user_struct {\n"
        "  struct user_struct *user;\n"
        "  struct lang_struct *lang;\n"
        "  union my_union data;\n"
        "} GTY((user));\n";
    
    create_gt_file(file3_path, file3_content);
    
    /* File 4: With deliberate syntax error for error path testing */
    char file4_path[256];
    snprintf(file4_path, sizeof(file4_path), "%s/error.gt", tmpdir);
    const char *file4_content = 
        "%{\n"
        "/* Test file with syntax error */\n"
        "#include \"config.h\"\n"
        "#include \"system.h\"\n"
        "/* Missing closing %}\n"
        "\n"
        "struct error_struct {\n"
        "  int x;\n"
        "};\n";
    
    create_gt_file(file4_path, file4_content);
    
    /* File 5: Duplicate definitions for warning testing */
    char file5_path[256];
    snprintf(file5_path, sizeof(file5_path), "%s/duplicate.gt", tmpdir);
    const char *file5_content = 
        "%{\n"
        "/* Test file with duplicate definitions */\n"
        "#include \"config.h\"\n"
        "#include \"system.h\"\n"
        "%}\n"
        "\n"
        "struct duplicate_struct {\n"
        "  int a;\n"
        "};\n"
        "\n"
        "/* Duplicate definition */\n"
        "struct duplicate_struct {\n"
        "  int b;\n"
        "};\n";
    
    create_gt_file(file5_path, file5_content);
    
    /* Create file list for batch processing */
    char filelist_path[256];
    snprintf(filelist_path, sizeof(filelist_path), "%s/filelist.txt", tmpdir);
    FILE *flist = fopen(filelist_path, "w");
    if (flist) {
        fprintf(flist, "%s\n", file1_path);
        fprintf(flist, "%s\n", file2_path);
        fprintf(flist, "%s\n", file3_path);
        fclose(flist);
    }
    
    /* Build gengtype with coverage instrumentation */
    printf("Building gengtype with coverage...\n");
    
    /* Compilation commands would go here in a real test harness */
    /* For this example, we assume gengtype is already built */
    
    /* Pattern A: Process files individually */
    printf("\n=== Pattern A: Individual file processing ===\n");
    const char *gengtype_exe = "./gengtype";  /* Assumes gengtype is in current dir */
    
    /* Process each valid file */
    const char *files[] = {file1_path, file2_path, file3_path};
    for (int i = 0; i < 3; i++) {
        printf("Processing %s...\n", files[i]);
        char output_header[256];
        snprintf(output_header, sizeof(output_header), "%s/output%d.h", tmpdir, i);
        
        const char *args[] = {"-g", output_header, files[i], NULL};
        int result = run_gengtype(gengtype_exe, args, 3);
        printf("  Result: %d\n", result);
    }
    
    /* Pattern B: Batch processing with -p */
    printf("\n=== Pattern B: Batch processing ===\n");
    char batch_output[256];
    snprintf(batch_output, sizeof(batch_output), "%s/batch_output.h", tmpdir);
    
    const char *batch_args[] = {"-g", batch_output, "-p", filelist_path, NULL};
    int batch_result = run_gengtype(gengtype_exe, batch_args, 4);
    printf("Batch processing result: %d\n", batch_result);
    
    /* Pattern C: Generate both header and routine files */
    printf("\n=== Pattern C: Full generation ===\n");
    char header_out[256], routine_out[256];
    snprintf(header_out, sizeof(header_out), "%s/full_header.h", tmpdir);
    snprintf(routine_out, sizeof(routine_out), "%s/full_routine.c", tmpdir);
    
    const char *full_args[] = {
        "-g", header_out,
        "-r", routine_out,
        file1_path, file2_path, file3_path,
        NULL
    };
    int full_result = run_gengtype(gengtype_exe, full_args, 7);
    printf("Full generation result: %d\n", full_result);
    
    /* Pattern D: Error and warning cases */
    printf("\n=== Pattern D: Error cases ===\n");
    
    /* Syntax error */
    printf("Testing syntax error file...\n");
    const char *error_args[] = {"-g", "/tmp/dummy.h", file4_path, NULL};
    int error_result = run_gengtype(gengtype_exe, error_args, 3);
    printf("  Syntax error processing result: %d\n", error_result);
    
    /* Warning case (duplicate) */
    printf("Testing duplicate definitions...\n");
    const char *dup_args[] = {"-g", "/tmp/dummy2.h", file5_path, NULL};
    int dup_result = run_gengtype(gengtype_exe, dup_args, 3);
    printf("  Duplicate processing result: %d\n", dup_result);
    
    /* Cleanup */
    printf("\nCleaning up temporary files...\n");
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "rm -rf %s", tmpdir);
    system(cmd);
    
    printf("\nTest completed. Coverage data should be in .gcda files.\n");
    printf("Run 'gcov gengtype.cc' to see coverage results.\n");
    
    return 0;
}
