/* driver.c - Test driver for gengtype coverage of type counting switch */
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

/* Execute gengtype with given arguments */
int run_gengtype(const char *gengtype_exe, char *const argv[]) {
    pid_t pid = fork();
    if (pid == 0) {
        /* Child process */
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

/* Clean up temporary files */
void cleanup_files(char **files, int count) {
    for (int i = 0; i < count; i++) {
        if (files[i]) {
            unlink(files[i]);
            free(files[i]);
        }
    }
}

/* Build gengtype with coverage instrumentation */
int build_gengtype_with_coverage() {
    printf("Building gengtype with coverage instrumentation...\n");
    
    /* Compile gengtype.cc with coverage flags */
    const char *compile_cmd = "g++ -O0 -fprofile-arcs -ftest-coverage "
                              "-DIN_GCC -DHAVE_CONFIG_H "
                              "-I. -I../../include -I../../gcc "
                              "-c gengtype.cc -o gengtype.o";
    
    printf("Compiling: %s\n", compile_cmd);
    if (system(compile_cmd) != 0) {
        fprintf(stderr, "Failed to compile gengtype.cc\n");
        return 0;
    }
    
    /* Compile gengtype-state.cc */
    const char *compile_state_cmd = "g++ -O0 -fprofile-arcs -ftest-coverage "
                                    "-DIN_GCC -DHAVE_CONFIG_H "
                                    "-I. -I../../include -I../../gcc "
                                    "-c gengtype-state.cc -o gengtype-state.o";
    
    printf("Compiling: %s\n", compile_state_cmd);
    if (system(compile_state_cmd) != 0) {
        fprintf(stderr, "Failed to compile gengtype-state.cc\n");
        return 0;
    }
    
    /* Link gengtype executable */
    const char *link_cmd = "g++ -O0 -fprofile-arcs -ftest-coverage "
                           "gengtype.o gengtype-state.o "
                           "-lgcov -liberty -o gengtype_coverage";
    
    printf("Linking: %s\n", link_cmd);
    if (system(link_cmd) != 0) {
        fprintf(stderr, "Failed to link gengtype\n");
        return 0;
    }
    
    printf("gengtype built successfully as 'gengtype_coverage'\n");
    return 1;
}

/* Main test driver */
int main(int argc, char *argv[]) {
    printf("=== Gengtype Type Counting Switch Coverage Test ===\n");
    
    /* Build gengtype with coverage */
    if (!build_gengtype_with_coverage()) {
        return 1;
    }
    
    /* Define multiple .gt files with diverse type definitions */
    
    /* File 1: Basic types and structs */
    const char *gt_file1 = 
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
        "/* TYPE_STRUCT */\n"
        "struct my_struct {\n"
        "    int a;\n"
        "    double b;\n"
        "};\n"
        "\n"
        "/* TYPE_POINTER */\n"
        "typedef struct my_struct *my_ptr;\n"
        "typedef int *int_ptr;\n"
        "\n"
        "/* TYPE_ARRAY */\n"
        "typedef int my_array[10];\n"
        "typedef struct my_struct struct_array[5];\n"
        "\n"
        "/* TYPE_STRING */\n"
        "struct string_struct {\n"
        "    const char *name;  /* string type */\n"
        "    char *data;\n"
        "};\n"
        "\n"
        "/* TYPE_UNION */\n"
        "union my_union {\n"
        "    int i;\n"
        "    void *p;\n"
        "    struct my_struct *s;\n"
        "};\n"
        "\n"
        "/* TYPE_CALLBACK */\n"
        "typedef void (*callback_fn)(void);\n"
        "typedef int (*another_callback)(char *, int);\n"
        "\n"
        "%{\n"
        "/* End of file 1 */\n"
        "%}\n";
    
    /* File 2: User structs, lang structs, and complex types */
    const char *gt_file2 =
        "%{\n"
        "/* Test file 2: Advanced types */\n"
        "%}\n"
        "\n"
        "/* TYPE_USER_STRUCT */\n"
        "struct user_struct {\n"
        "    int *p;\n"
        "    void **data;\n"
        "} GTY((user));\n"
        "\n"
        "/* Another user struct */\n"
        "struct complex_user_struct {\n"
        "    struct user_struct *next;\n"
        "    int count;\n"
        "    void (*callback)(void);\n"
        "} GTY((user));\n"
        "\n"
        "/* TYPE_LANG_STRUCT */\n"
        "struct lang_struct {\n"
        "    int data;\n"
        "    void *extra;\n"
        "} GTY ((lang));\n"
        "\n"
        "/* Complex nested types */\n"
        "struct container {\n"
        "    /* Pointer to union containing arrays */\n"
        "    union my_union *union_ptr;\n"
        "    \n"
        "    /* Array of pointers to callbacks */\n"
        "    callback_fn callbacks[5];\n"
        "    \n"
        "    /* Nested struct with string */\n"
        "    struct {\n"
        "        const char *id;\n"
        "        int value;\n"
        "    } nested;\n"
        "    \n"
        "    /* Multi-dimensional array */\n"
        "    int matrix[3][3];\n"
        "};\n"
        "\n"
        "/* More pointer types */\n"
        "typedef union my_union **double_ptr_to_union;\n"
        "typedef int (*func_ptr_array[10])(void);\n"
        "\n"
        "%{\n"
        "/* End of file 2 */\n"
        "%}\n";
    
    /* File 3: Error cases and edge cases */
    const char *gt_file3 =
        "%{\n"
        "/* Test file 3: Edge cases and potential errors */\n"
        "%}\n"
        "\n"
        "/* Duplicate type definition to test warnings */\n"
        "struct my_struct;  /* Already defined in file1 */\n"
        "\n"
        "/* More scalar types */\n"
        "typedef char byte;\n"
        "typedef float real;\n"
        "\n"
        "/* Complex pointer chain */\n"
        "typedef struct container ****quad_ptr;\n"
        "\n"
        "/* Mixed array types */\n"
        "typedef union my_union mixed_array[2][2];\n"
        "\n"
        "/* Struct with all type kinds */\n"
        "struct mega_struct {\n"
        "    /* SCALAR */\n"
        "    my_scalar scalar_field;\n"
        "    \n"
        "    /* STRING */\n"
        "    const char *string_field;\n"
        "    \n"
        "    /* POINTER */\n"
        "    struct container *ptr_field;\n"
        "    \n"
        "    /* ARRAY */\n"
        "    callback_fn callback_array[3];\n"
        "    \n"
        "    /* UNION */\n"
        "    union my_union union_field;\n"
        "    \n"
        "    /* Nested STRUCT */\n"
        "    struct {\n"
        "        int x, y;\n"
        "    } point;\n"
        "};\n"
        "\n"
        "/* File intentionally missing closing %} to test error handling */\n";
        /* Note: Missing %} to trigger error path */
    
    /* File 4: Valid file to ensure we have at least one fully valid input */
    const char *gt_file4 =
        "%{\n"
        "/* Test file 4: Additional valid types */\n"
        "%}\n"
        "\n"
        "/* More TYPE_CALLBACK variations */\n"
        "typedef void (*void_callback)(void);\n"
        "typedef int (*int_callback)(int, char*);\n"
        "typedef struct my_struct* (*struct_callback)(void);\n"
        "\n"
        "/* Pointer to array */\n"
        "typedef int (*ptr_to_array)[10];\n"
        "\n"
        "/* Array of pointers */\n"
        "typedef void* ptr_array[20];\n"
        "\n"
        "/* Const pointer to const */\n"
        "typedef const int * const const_ptr_to_const;\n"
        "\n"
        "/* Volatile types */\n"
        "typedef volatile int volatile_int;\n"
        "\n"
        "%{\n"
        "/* End of file 4 */\n"
        "%}\n";
    
    /* Create temporary .gt files */
    char *files[5];
    int file_count = 0;
    
    files[file_count++] = create_temp_file(gt_file1, ".gt");
    files[file_count++] = create_temp_file(gt_file2, ".gt");
    files[file_count++] = create_temp_file(gt_file3, ".gt");
    files[file_count++] = create_temp_file(gt_file4, ".gt");
    
    /* Create file list for batch processing */
    char *filelist = create_temp_file("", ".list");
    FILE *fl = fopen(filelist, "w");
    if (fl) {
        for (int i = 0; i < file_count; i++) {
            fprintf(fl, "%s\n", files[i]);
        }
        fclose(fl);
    }
    
    printf("\nCreated %d test .gt files:\n", file_count);
    for (int i = 0; i < file_count; i++) {
        printf("  %d: %s\n", i + 1, files[i]);
    }
    printf("File list: %s\n", filelist);
    
    /* Test Pattern A: Process each file individually */
    printf("\n=== Pattern A: Processing files individually ===\n");
    for (int i = 0; i < file_count; i++) {
        printf("\nProcessing %s:\n", files[i]);
        
        /* Test with header generation flag */
        char output_header[256];
        snprintf(output_header, sizeof(output_header), "/tmp/output_%d.h", i);
        
        char *args1[] = {
            "./gengtype_coverage",
            "-g", output_header,
            files[i],
            NULL
        };
        
        int result = run_gengtype("./gengtype_coverage", args1);
        printf("  gengtype -g %s %s -> exit code: %d\n", 
               output_header, files[i], result);
        
        /* Also test with routine generation */
        char output_routine[256];
        snprintf(output_routine, sizeof(output_routine), "/tmp/routine_%d.c", i);
        
        char *args2[] = {
            "./gengtype_coverage",
            "-r", output_routine,
            files[i],
            NULL
        };
        
        result = run_gengtype("./gengtype_coverage", args2);
        printf("  gengtype -r %s %s -> exit code: %d\n",
               output_routine, files[i], result);
    }
    
    /* Test Pattern B: Batch processing with -p */
    printf("\n=== Pattern B: Batch processing with -p ===\n");
    char *batch_args[] = {
        "./gengtype_coverage",
        "-p",
        filelist,
        NULL
    };
    
    int batch_result = run_gengtype("./gengtype_coverage", batch_args);
    printf("gengtype -p %s -> exit code: %d\n", filelist, batch_result);
    
    /* Test Pattern C: Multiple files with header generation */
    printf("\n=== Pattern C: Multiple files with header generation ===\n");
    char *multi_args[] = {
        "./gengtype_coverage",
        "-g", "/tmp/combined.h",
        files[0], files[1], files[3],  /* Skip file 3 (has error) */
        NULL
    };
    
    int multi_result = run_gengtype("./gengtype_coverage", multi_args);
    printf("gengtype -g /tmp/combined.h %s %s %s -> exit code: %d\n",
           files[0], files[1], files[3], multi_result);
    
    /* Test Pattern D: Error case handling */
    printf("\n=== Pattern D: Error case handling ===\n");
    char *error_args[] = {
        "./gengtype_coverage",
        files[2],  /* File with missing %} */
        NULL
    };
    
    int error_result = run_gengtype("./gengtype_coverage", error_args);
    printf("gengtype %s (file with error) -> exit code: %d\n",
           files[2], error_result);
    
    /* Generate coverage data by running gcov */
    printf("\n=== Generating coverage data ===\n");
    system("gcov gengtype.cc gengtype-state.cc");
    
    /* Display coverage summary */
    printf("\n=== Coverage Summary ===\n");
    system("cat gengtype.cc.gcov | head -50");
    
    /* Cleanup */
    printf("\n=== Cleaning up ===\n");
    cleanup_files(files, file_count);
    if (filelist) {
        unlink(filelist);
        free(filelist);
    }
    
    /* Remove generated output files */
    for (int i = 0; i < file_count; i++) {
        char header[256], routine[256];
        snprintf(header, sizeof(header), "/tmp/output_%d.h", i);
        snprintf(routine, sizeof(routine), "/tmp/routine_%d.c", i);
        unlink(header);
        unlink(routine);
    }
    unlink("/tmp/combined.h");
    
    /* Remove coverage build artifacts */
    unlink("gengtype.o");
    unlink("gengtype-state.o");
    unlink("gengtype_coverage");
    unlink("gengtype.gcda");
    unlink("gengtype.gcno");
    unlink("gengtype-state.gcda");
    unlink("gengtype-state.gcno");
    
    printf("\nTest completed successfully!\n");
    printf("Check gengtype.cc.gcov for detailed coverage of lines 182-213\n");
    
    return 0;
}
