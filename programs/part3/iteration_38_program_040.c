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

/* Execute gengtype with given arguments */
int run_gengtype(const char *gengtype_exe, const char *arg1, const char *arg2, const char *arg3) {
    pid_t pid = fork();
    if (pid == 0) {
        /* Child process */
        if (arg3) {
            execl(gengtype_exe, gengtype_exe, arg1, arg2, arg3, NULL);
        } else if (arg2) {
            execl(gengtype_exe, gengtype_exe, arg1, arg2, NULL);
        } else if (arg1) {
            execl(gengtype_exe, gengtype_exe, arg1, NULL);
        } else {
            execl(gengtype_exe, gengtype_exe, NULL);
        }
        perror("execl");
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

int main(int argc, char *argv[]) {
    char tmpdir_template[] = "/tmp/gengtype_test_XXXXXX";
    char *tmpdir = mkdtemp(tmpdir_template);
    if (!tmpdir) {
        perror("mkdtemp");
        return 1;
    }
    
    printf("Created temporary directory: %s\n", tmpdir);
    
    /* File 1: Basic types with forward declarations and scalar types */
    char file1[256];
    snprintf(file1, sizeof(file1), "%s/types1.gt", tmpdir);
    const char *content1 = 
        "%{\n"
        "/* Test file 1: Basic types and forward declarations */\n"
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
        "/* TYPE_STRING: String type */\n"
        "struct string_struct {\n"
        "  const char *name;  /* TYPE_STRING */\n"
        "  int id;\n"
        "};\n"
        "\n"
        "/* TYPE_POINTER: Pointer types */\n"
        "typedef struct string_struct *string_ptr;\n"
        "typedef my_scalar *scalar_ptr;\n"
        "%}\n";
    
    /* File 2: Structs, unions, and arrays */
    char file2[256];
    snprintf(file2, sizeof(file2), "%s/types2.gt", tmpdir);
    const char *content2 =
        "%{\n"
        "/* Test file 2: Structs, unions, arrays */\n"
        "#include \"config.h\"\n"
        "#include \"system.h\"\n"
        "%}\n"
        "\n"
        "/* TYPE_STRUCT: Regular struct */\n"
        "struct my_struct {\n"
        "  int a;\n"
        "  double b;\n"
        "  struct my_struct *next;  /* Nested pointer */\n"
        "};\n"
        "\n"
        "/* TYPE_USER_STRUCT: Struct with user marking */\n"
        "struct user_struct {\n"
        "  int *p;\n"
        "  void *data;\n"
        "} GTY((user));\n"
        "\n"
        "/* TYPE_UNION: Union type */\n"
        "union my_union {\n"
        "  int i;\n"
        "  void *p;\n"
        "  double d;\n"
        "};\n"
        "\n"
        "/* TYPE_ARRAY: Array types */\n"
        "typedef int my_array[10];\n"
        "typedef struct my_struct struct_array[5];\n"
        "\n"
        "/* Complex nested type */\n"
        "struct complex_type {\n"
        "  union my_union u;\n"
        "  my_array arr;\n"
        "  struct user_struct *user_ptr;  /* Pointer to user struct */\n"
        "};\n"
        "%}\n";
    
    /* File 3: Callbacks, lang structs, and more complex types */
    char file3[256];
    snprintf(file3, sizeof(file3), "%s/types3.gt", tmpdir);
    const char *content3 =
        "%{\n"
        "/* Test file 3: Callbacks and language-specific types */\n"
        "#include \"config.h\"\n"
        "#include \"system.h\"\n"
        "%}\n"
        "\n"
        "/* TYPE_CALLBACK: Callback function type */\n"
        "typedef void (*callback_fn)(void);\n"
        "typedef int (*compare_fn)(const void *, const void *);\n"
        "\n"
        "/* TYPE_LANG_STRUCT: Language-specific struct */\n"
        "struct lang_struct {\n"
        "  int data;\n"
        "  void *lang_data;\n"
        "} GTY ((lang));\n"
        "\n"
        "/* More complex nested types */\n"
        "struct container {\n"
        "  callback_fn handler;          /* TYPE_CALLBACK */\n"
        "  struct lang_struct lang;      /* TYPE_LANG_STRUCT */\n"
        "  union {\n"
        "    int tag;\n"
        "    void *ptr;\n"
        "  } variant;\n"
        "  const char *description;      /* TYPE_STRING */\n"
        "};\n"
        "\n"
        "/* Array of pointers to callbacks */\n"
        "typedef callback_fn callback_table[8];\n"
        "\n"
        "/* Forward declaration for circular reference */\n"
        "struct circular;\n"
        "struct circular {\n"
        "  struct circular *next;\n"
        "  int value;\n"
        "};\n"
        "%}\n";
    
    /* File 4: File with syntax error (to test error paths) */
    char file4[256];
    snprintf(file4, sizeof(file4), "%s/error.gt", tmpdir);
    const char *content4 =
        "%{\n"
        "/* File with deliberate syntax error - missing closing %} */\n"
        "#include \"config.h\"\n"
        "struct bad_struct {\n"
        "  int x;\n";
        /* Missing %} to cause parse error */
    
    /* File 5: Duplicate definitions (to test warning paths) */
    char file5[256];
    snprintf(file5, sizeof(file5), "%s/duplicate.gt", tmpdir);
    const char *content5 =
        "%{\n"
        "#include \"config.h\"\n"
        "%}\n"
        "typedef int my_int;\n"
        "typedef int my_int;  /* Duplicate */\n"
        "struct dup { int a; };\n"
        "struct dup { int b; };  /* Duplicate struct */\n"
        "%}\n";
    
    /* Create all test files */
    create_gt_file(file1, content1);
    create_gt_file(file2, content2);
    create_gt_file(file3, content3);
    create_gt_file(file4, content4);
    create_gt_file(file5, content5);
    
    /* Create file list for batch processing */
    char filelist[256];
    snprintf(filelist, sizeof(filelist), "%s/filelist.txt", tmpdir);
    FILE *flist = fopen(filelist, "w");
    fprintf(flist, "%s\n%s\n%s\n", file1, file2, file3);
    fclose(flist);
    
    /* Build gengtype with coverage instrumentation */
    printf("Building gengtype with coverage instrumentation...\n");
    
    /* First, compile gengtype.cc with coverage flags */
    char compile_cmd[1024];
    snprintf(compile_cmd, sizeof(compile_cmd),
             "g++ -O0 -fprofile-arcs -ftest-coverage -DIN_GCC -DHAVE_CONFIG_H "
             "-I. -I../../include -I../../gcc -c gengtype.cc -o %s/gengtype.o",
             tmpdir);
    printf("Running: %s\n", compile_cmd);
    if (system(compile_cmd) != 0) {
        fprintf(stderr, "Failed to compile gengtype.cc\n");
        return 1;
    }
    
    /* Compile gengtype-state.cc */
    snprintf(compile_cmd, sizeof(compile_cmd),
             "g++ -O0 -fprofile-arcs -ftest-coverage -DIN_GCC -DHAVE_CONFIG_H "
             "-I. -I../../include -I../../gcc -c gengtype-state.cc -o %s/gengtype-state.o",
             tmpdir);
    printf("Running: %s\n", compile_cmd);
    if (system(compile_cmd) != 0) {
        fprintf(stderr, "Failed to compile gengtype-state.cc\n");
        return 1;
    }
    
    /* Link gengtype executable */
    char gengtype_exe[256];
    snprintf(gengtype_exe, sizeof(gengtype_exe), "%s/gengtype_test", tmpdir);
    snprintf(compile_cmd, sizeof(compile_cmd),
             "g++ -O0 -fprofile-arcs -ftest-coverage %s/gengtype.o %s/gengtype-state.o "
             "-lgcov -liberty -o %s",
             tmpdir, tmpdir, gengtype_exe);
    printf("Running: %s\n", compile_cmd);
    if (system(compile_cmd) != 0) {
        fprintf(stderr, "Failed to link gengtype\n");
        return 1;
    }
    
    printf("\n=== Running gengtype tests ===\n");
    
    /* Pattern A: Process each file individually */
    printf("\n1. Processing files individually:\n");
    printf("   Processing %s...\n", file1);
    run_gengtype(gengtype_exe, "-g", "/dev/null", file1);
    
    printf("   Processing %s...\n", file2);
    run_gengtype(gengtype_exe, "-g", "/dev/null", file2);
    
    printf("   Processing %s...\n", file3);
    run_gengtype(gengtype_exe, "-g", "/dev/null", file3);
    
    /* Pattern B: Batch processing with -p */
    printf("\n2. Batch processing with -p flag:\n");
    char output_header[256];
    snprintf(output_header, sizeof(output_header), "%s/output.h", tmpdir);
    run_gengtype(gengtype_exe, "-p", filelist, NULL);
    
    /* Pattern C: Generate header with multiple input files */
    printf("\n3. Generating header from multiple files:\n");
    run_gengtype(gengtype_exe, "-g", output_header, file1);
    run_gengtype(gengtype_exe, "-g", output_header, file2);
    run_gengtype(gengtype_exe, "-g", output_header, file3);
    
    /* Also test with all files at once */
    char output_header2[256];
    snprintf(output_header2, sizeof(output_header2), "%s/output2.h", tmpdir);
    run_gengtype(gengtype_exe, "-g", output_header2, file1, file2, file3);
    
    /* Pattern D: Test error and warning cases */
    printf("\n4. Testing error cases:\n");
    printf("   Processing file with syntax error...\n");
    run_gengtype(gengtype_exe, file4, NULL, NULL);
    
    printf("   Processing file with duplicate definitions...\n");
    run_gengtype(gengtype_exe, file5, NULL, NULL);
    
    /* Generate routine file as well */
    printf("\n5. Generating routine file:\n");
    char output_routine[256];
    snprintf(output_routine, sizeof(output_routine), "%s/output.c", tmpdir);
    run_gengtype(gengtype_exe, "-r", output_routine, file1);
    run_gengtype(gengtype_exe, "-r", output_routine, file2);
    run_gengtype(gengtype_exe, "-r", output_routine, file3);
    
    /* Test with debug flag to ensure more code paths */
    printf("\n6. Running with debug flag:\n");
    char gengtype_debug[256];
    snprintf(gengtype_debug, sizeof(gengtype_debug), "%s/gengtype_debug", tmpdir);
    
    /* Recompile with DEBUG_GENGTYPE */
    snprintf(compile_cmd, sizeof(compile_cmd),
             "g++ -O0 -fprofile-arcs -ftest-coverage -DIN_GCC -DHAVE_CONFIG_H -DDEBUG_GENGTYPE "
             "-I. -I../../include -I../../gcc gengtype.cc gengtype-state.cc "
             "-lgcov -liberty -o %s", gengtype_debug);
    system(compile_cmd);
    
    /* Run debug version */
    run_gengtype(gengtype_debug, "-g", "/dev/null", file1);
    run_gengtype(gengtype_debug, "-g", "/dev/null", file2);
    run_gengtype(gengtype_debug, "-g", "/dev/null", file3);
    
    /* Check if coverage files were generated */
    printf("\n=== Checking coverage data ===\n");
    char check_cmd[256];
    snprintf(check_cmd, sizeof(check_cmd), "ls -la %s/*.gcda 2>/dev/null | wc -l", tmpdir);
    system(check_cmd);
    
    /* Generate gcov report */
    printf("\nGenerating coverage report...\n");
    snprintf(check_cmd, sizeof(check_cmd), 
             "cd %s && gcov gengtype.cc 2>/dev/null | grep -A 20 'Lines executed'", tmpdir);
    system(check_cmd);
    
    printf("\n=== Test completed ===\n");
    printf("Temporary files in: %s\n", tmpdir);
    printf("You can examine coverage with: gcov -b %s/gengtype.cc\n", tmpdir);
    
    /* Cleanup (comment out for debugging) */
    /*
    snprintf(check_cmd, sizeof(check_cmd), "rm -rf %s", tmpdir);
    system(check_cmd);
    */
    
    return 0;
}
