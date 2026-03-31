/* test_gengtype_coverage.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

/* Function to create a temporary file with given content */
char* create_temp_file(const char* content, const char* suffix) {
    char template[] = "/tmp/gengtype_test_XXXXXX";
    int fd = mkstemp(template);
    if (fd == -1) {
        perror("mkstemp failed");
        return NULL;
    }
    
    if (suffix) {
        char newname[256];
        snprintf(newname, sizeof(newname), "%s%s", template, suffix);
        rename(template, newname);
        strcpy(template, newname);
    }
    
    FILE* f = fdopen(fd, "w");
    if (!f) {
        close(fd);
        return NULL;
    }
    
    fwrite(content, 1, strlen(content), f);
    fclose(f);
    
    return strdup(template);
}

/* Function to compile and run gengtype with coverage */
int compile_and_run_gengtype(const char** input_files, int num_files, 
                            const char* output_header, const char* output_routine) {
    int status;
    pid_t pid;
    
    /* First, compile gengtype with coverage instrumentation */
    printf("Compiling gengtype with coverage instrumentation...\n");
    
    pid = fork();
    if (pid == 0) {
        /* Child process: compile gengtype */
        execlp("g++", "g++", "-O0", "-fprofile-arcs", "-ftest-coverage",
               "-DIN_GCC", "-DHAVE_CONFIG_H", "-I.", "-I../../include", 
               "-I../../gcc", "-c", "gengtype.cc", "-o", "gengtype.o", NULL);
        perror("execlp failed for compilation");
        exit(1);
    } else if (pid > 0) {
        waitpid(pid, &status, 0);
        if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
            fprintf(stderr, "Failed to compile gengtype.cc\n");
            return -1;
        }
    } else {
        perror("fork failed");
        return -1;
    }
    
    /* Link gengtype executable */
    pid = fork();
    if (pid == 0) {
        execlp("g++", "g++", "-O0", "-fprofile-arcs", "-ftest-coverage",
               "gengtype.o", "gengtype-state.cc", "gengtype-lex.cc",
               "-o", "gengtype_coverage", "-liberty", "-lgcov", NULL);
        perror("execlp failed for linking");
        exit(1);
    } else if (pid > 0) {
        waitpid(pid, &status, 0);
        if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
            fprintf(stderr, "Failed to link gengtype\n");
            return -1;
        }
    }
    
    /* Now run gengtype with our test files */
    printf("Running gengtype with test files...\n");
    
    /* Build command line arguments */
    char** argv = malloc((num_files + 10) * sizeof(char*));
    int argc = 0;
    
    argv[argc++] = "./gengtype_coverage";
    argv[argc++] = "-g";  /* Generate header */
    argv[argc++] = output_header;
    argv[argc++] = "-r";  /* Generate routines */
    argv[argc++] = output_routine;
    
    /* Add all input files */
    for (int i = 0; i < num_files; i++) {
        argv[argc++] = (char*)input_files[i];
    }
    argv[argc] = NULL;
    
    /* Execute gengtype */
    pid = fork();
    if (pid == 0) {
        execv("./gengtype_coverage", argv);
        perror("execv failed");
        exit(1);
    } else if (pid > 0) {
        waitpid(pid, &status, 0);
        free(argv);
        if (!WIFEXITED(status)) {
            fprintf(stderr, "gengtype terminated abnormally\n");
            return -1;
        }
        return WEXITSTATUS(status);
    } else {
        free(argv);
        perror("fork failed");
        return -1;
    }
}

/* Function to create file list for batch processing */
char* create_file_list(const char** files, int count) {
    char* list_content = malloc(4096);
    list_content[0] = '\0';
    
    for (int i = 0; i < count; i++) {
        strcat(list_content, files[i]);
        strcat(list_content, "\n");
    }
    
    return list_content;
}

int main() {
    int ret = 0;
    char* temp_files[10];
    int file_count = 0;
    
    /* Define various .gt files covering all type categories */
    
    /* File 1: Basic types and structs */
    const char* gt_file1 = 
        "%{\n"
        "/* Test file 1: Basic types */\n"
        "typedef int my_scalar;                     /* TYPE_SCALAR */\n"
        "struct undefined_struct;                   /* TYPE_UNDEFINED (forward decl) */\n"
        "struct my_struct {                         /* TYPE_STRUCT */\n"
        "    int a;\n"
        "    double b;\n"
        "};\n"
        "union my_union {                           /* TYPE_UNION */\n"
        "    int i;\n"
        "    void *p;\n"
        "};\n"
        "typedef struct my_struct *my_ptr;          /* TYPE_POINTER */\n"
        "%}\n";
    
    /* File 2: Arrays, strings, and user structs */
    const char* gt_file2 = 
        "%{\n"
        "/* Test file 2: Arrays and strings */\n"
        "typedef int my_array[10];                  /* TYPE_ARRAY */\n"
        "struct string_struct {                     /* TYPE_STRING */\n"
        "    const char *name;                      /* string type */\n"
        "    char *data;\n"
        "};\n"
        "struct user_struct {                       /* TYPE_USER_STRUCT */\n"
        "    int *p;\n"
        "    struct my_struct *next;\n"
        "} GTY((user));\n"
        "typedef int multi_array[5][10];            /* Nested array */\n"
        "struct complex_struct {\n"
        "    union my_union u;\n"
        "    my_array arr;\n"
        "    struct string_struct *str_ptr;\n"
        "};\n"
        "%}\n";
    
    /* File 3: Callbacks, lang structs, and complex combinations */
    const char* gt_file3 = 
        "%{\n"
        "/* Test file 3: Callbacks and lang structs */\n"
        "typedef void (*callback_fn)(void);         /* TYPE_CALLBACK */\n"
        "typedef int (*int_callback)(int, char*);\n"
        "struct lang_struct {                       /* TYPE_LANG_STRUCT */\n"
        "    int data;\n"
        "    void *lang_data;\n"
        "} GTY ((lang));\n"
        "struct nested_types {\n"
        "    callback_fn handler;                   /* Callback in struct */\n"
        "    struct lang_struct *lang_ptr;          /* Pointer to lang struct */\n"
        "    union {\n"
        "        my_array arr;\n"
        "        struct user_struct *user;\n"
        "    } variant;\n"
        "};\n"
        "typedef struct nested_types** double_ptr;  /* Pointer to pointer */\n"
        "%}\n";
    
    /* File 4: File with syntax error (to test error paths) */
    const char* gt_file4 = 
        "%{\n"
        "/* Test file 4: File with deliberate issues */\n"
        "struct duplicate_struct {                  /* Duplicate definition */\n"
        "    int x;\n"
        "};\n"
        "struct duplicate_struct {                  /* Duplicate - should warn */\n"
        "    int y;\n"
        "};\n"
        "/* Missing closing %} to test error recovery */\n";
        /* Intentionally missing %} */
    
    /* File 5: More complex nested types */
    const char* gt_file5 = 
        "%{\n"
        "/* Test file 5: Deeply nested types */\n"
        "struct outer_struct {\n"
        "    struct {\n"
        "        int depth1;\n"
        "        struct {\n"
        "            char depth2;\n"
        "            union {\n"
        "                int i;\n"
        "                struct inner {\n"
        "                    callback_fn cb;\n"
        "                    my_array *arr_ptr;\n"
        "                } inner;\n"
        "            } u;\n"
        "        } nested;\n"
        "    } container;\n"
        "    struct lang_struct lang_member;\n"
        "    struct user_struct *user_chain[5];     /* Array of pointers to user structs */\n"
        "};\n"
        "typedef struct outer_struct* (*factory_fn)(int);  /* Callback returning pointer */\n"
        "%}\n";
    
    /* Create temporary files */
    temp_files[file_count++] = create_temp_file(gt_file1, ".gt");
    temp_files[file_count++] = create_temp_file(gt_file2, ".gt");
    temp_files[file_count++] = create_temp_file(gt_file3, ".gt");
    temp_files[file_count++] = create_temp_file(gt_file4, ".gt");
    temp_files[file_count++] = create_temp_file(gt_file5, ".gt");
    
    /* Create file list for batch processing */
    char* file_list_content = create_file_list((const char**)temp_files, file_count);
    temp_files[file_count++] = create_temp_file(file_list_content, ".list");
    free(file_list_content);
    
    /* Test 1: Process files individually */
    printf("\n=== Test 1: Processing files individually ===\n");
    for (int i = 0; i < 3; i++) {  /* Process first 3 valid files */
        const char* single_file[1] = { temp_files[i] };
        int result = compile_and_run_gengtype(single_file, 1, 
                                            "gt_output.h", "gt_output.c");
        if (result != 0) {
            printf("Warning: gengtype returned %d for file %d\n", result, i+1);
        }
    }
    
    /* Test 2: Process all files together */
    printf("\n=== Test 2: Processing all files together ===\n");
    int result = compile_and_run_gengtype((const char**)temp_files, 3, 
                                         "combined.h", "combined.c");
    if (result != 0) {
        printf("Warning: gengtype returned %d for combined processing\n", result);
    }
    
    /* Test 3: Batch processing with -p flag */
    printf("\n=== Test 3: Batch processing with -p flag ===\n");
    /* Create a simple test program that calls gengtype with -p */
    const char* batch_test = 
        "#include <stdio.h>\n"
        "#include <stdlib.h>\n"
        "#include <unistd.h>\n"
        "int main() {\n"
        "    char cmd[1024];\n"
        "    snprintf(cmd, sizeof(cmd), \"./gengtype_coverage -p %s -g batch.h -r batch.c\",\n"
        "             temp_files[file_count-1]);\n"
        "    return system(cmd);\n"
        "}\n";
    
    char* batch_prog = create_temp_file(batch_test, ".c");
    char compile_cmd[256];
    snprintf(compile_cmd, sizeof(compile_cmd), 
             "gcc -O0 %s -o batch_test", batch_prog);
    
    system(compile_cmd);
    system("./batch_test");
    
    /* Test 4: Error case processing */
    printf("\n=== Test 4: Testing error cases ===\n");
    const char* error_file[1] = { temp_files[3] };  /* File with syntax error */
    result = compile_and_run_gengtype(error_file, 1, "error.h", "error.c");
    printf("gengtype returned %d for error file (expected non-zero)\n", result);
    
    /* Cleanup */
    printf("\n=== Cleaning up ===\n");
    for (int i = 0; i < file_count; i++) {
        if (temp_files[i]) {
            unlink(temp_files[i]);
            free(temp_files[i]);
        }
    }
    if (batch_prog) {
        unlink(batch_prog);
        free(batch_prog);
    }
    unlink("gengtype_coverage");
    unlink("gengtype.o");
    unlink("gt_output.h");
    unlink("gt_output.c");
    unlink("combined.h");
    unlink("combined.c");
    unlink("batch.h");
    unlink("batch.c");
    unlink("error.h");
    unlink("error.c");
    unlink("batch_test");
    
    /* Clean up coverage files */
    system("rm -f *.gcda *.gcno gmon.out");
    
    printf("\nTest completed. Check coverage with:\n");
    printf("  gcov gengtype.cc\n");
    printf("  gcov -b gengtype.cc  # for branch coverage\n");
    
    return ret;
}
