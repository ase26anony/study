/* test_gengtype_coverage.c - Driver program to test gengtype switch coverage */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

/* Function to create a temporary .gt file with given content */
static char* create_temp_gt_file(const char* content, const char* suffix) {
    char template[] = "/tmp/gengtype_test_XXXXXX";
    int fd = mkstemp(template);
    if (fd == -1) {
        perror("mkstemp failed");
        return NULL;
    }
    
    char* filename = malloc(strlen(template) + strlen(suffix) + 1);
    strcpy(filename, template);
    strcat(filename, suffix);
    
    close(fd);
    unlink(template);
    
    FILE* f = fopen(filename, "w");
    if (!f) {
        perror("fopen failed");
        free(filename);
        return NULL;
    }
    
    fputs(content, f);
    fclose(f);
    return filename;
}

/* Function to compile and run gengtype with coverage */
static int compile_and_run_gengtype(const char** input_files, int num_files, 
                                   const char* output_header, const char* output_routine) {
    int status;
    pid_t pid;
    
    /* First, compile gengtype with coverage instrumentation */
    printf("Compiling gengtype with coverage instrumentation...\n");
    
    pid = fork();
    if (pid == 0) {
        /* Child process: compile gengtype */
        execlp("g++", "g++", "-O0", "-fprofile-arcs", "-ftest-coverage",
               "-DIN_GCC", "-DHAVE_CONFIG_H",
               "-I.", "-I../../include", "-I../../gcc",
               "-c", "gengtype.cc", "-o", "gengtype.o",
               NULL);
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
               "gengtype.o", "gengtype-state.o", "gengtype-lex.o", 
               "-o", "gengtype_coverage",
               "-liberty", "-lgcov",
               NULL);
        perror("execlp failed for linking");
        exit(1);
    } else if (pid > 0) {
        waitpid(pid, &status, 0);
        if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
            fprintf(stderr, "Failed to link gengtype\n");
            return -1;
        }
    }
    
    /* Build command line arguments for gengtype */
    char** argv = malloc((num_files + 10) * sizeof(char*));
    int argc = 0;
    
    argv[argc++] = "./gengtype_coverage";
    argv[argc++] = "-g";  /* Generate header */
    argv[argc++] = output_header;
    argv[argc++] = "-r";  /* Generate routine */
    argv[argc++] = output_routine;
    
    /* Add all input files */
    for (int i = 0; i < num_files; i++) {
        argv[argc++] = (char*)input_files[i];
    }
    argv[argc] = NULL;
    
    /* Run gengtype with the input files */
    printf("Running gengtype with %d input files...\n", num_files);
    
    pid = fork();
    if (pid == 0) {
        /* Child process: execute gengtype */
        execv("./gengtype_coverage", argv);
        perror("execv failed");
        exit(1);
    } else if (pid > 0) {
        waitpid(pid, &status, 0);
        free(argv);
        
        if (WIFEXITED(status)) {
            printf("gengtype exited with status %d\n", WEXITSTATUS(status));
            return WEXITSTATUS(status);
        } else {
            fprintf(stderr, "gengtype terminated abnormally\n");
            return -1;
        }
    } else {
        perror("fork failed");
        free(argv);
        return -1;
    }
}

/* Function to create file list for batch processing */
static char* create_file_list(const char** files, int num_files) {
    char template[] = "/tmp/gengtype_filelist_XXXXXX";
    int fd = mkstemp(template);
    if (fd == -1) {
        perror("mkstemp failed for file list");
        return NULL;
    }
    
    FILE* f = fdopen(fd, "w");
    if (!f) {
        perror("fdopen failed");
        close(fd);
        unlink(template);
        return NULL;
    }
    
    for (int i = 0; i < num_files; i++) {
        fprintf(f, "%s\n", files[i]);
    }
    
    fclose(f);
    return strdup(template);
}

/* Test gengtype with -p (batch processing) flag */
static int test_batch_processing(const char** files, int num_files) {
    char* filelist = create_file_list(files, num_files);
    if (!filelist) return -1;
    
    printf("Testing batch processing with file list: %s\n", filelist);
    
    pid_t pid = fork();
    if (pid == 0) {
        execl("./gengtype_coverage", "./gengtype_coverage", 
              "-p", filelist, NULL);
        perror("execl failed");
        exit(1);
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
        unlink(filelist);
        free(filelist);
        
        if (WIFEXITED(status)) {
            return WEXITSTATUS(status);
        }
        return -1;
    } else {
        perror("fork failed");
        free(filelist);
        return -1;
    }
}

int main(void) {
    /* Create multiple .gt files with diverse type definitions */
    
    /* File 1: Basic types and structs */
    const char* gt_content1 = 
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
        "typedef unsigned long scalar2;\n"
        "\n"
        "/* TYPE_STRUCT: Regular struct */\n"
        "struct my_struct {\n"
        "    int a;\n"
        "    double b;\n"
        "};\n"
        "\n"
        "/* TYPE_POINTER: Pointer types */\n"
        "typedef struct my_struct *my_ptr;\n"
        "typedef my_scalar *scalar_ptr;\n"
        "\n"
        "/* TYPE_ARRAY: Array types */\n"
        "typedef int my_array[10];\n"
        "typedef struct my_struct struct_array[5];\n"
        "%}\n";
    
    /* File 2: Advanced types including unions and callbacks */
    const char* gt_content2 =
        "%{\n"
        "/* Test file 2: Advanced types */\n"
        "#include \"config.h\"\n"
        "#include \"system.h\"\n"
        "%}\n"
        "\n"
        "/* TYPE_STRING: String type */\n"
        "struct string_struct {\n"
        "    const char *name;\n"
        "    char *buffer;\n"
        "};\n"
        "\n"
        "/* TYPE_UNION: Union type */\n"
        "union my_union {\n"
        "    int i;\n"
        "    void *p;\n"
        "    double d;\n"
        "};\n"
        "\n"
        "/* TYPE_CALLBACK: Callback function type */\n"
        "typedef void (*callback_fn)(void);\n"
        "typedef int (*compare_fn)(const void*, const void*);\n"
        "\n"
        "/* TYPE_USER_STRUCT: Struct with user marking */\n"
        "struct user_struct {\n"
        "    int *p;\n"
        "    void *data;\n"
        "} GTY((user));\n"
        "\n"
        "/* Complex nested type */\n"
        "struct complex_type {\n"
        "    union my_union u;\n"
        "    callback_fn handler;\n"
        "    struct string_struct *str_ptr;\n"
        "};\n"
        "%}\n";
    
    /* File 3: Language structs and edge cases */
    const char* gt_content3 =
        "%{\n"
        "/* Test file 3: Language structs and edge cases */\n"
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
        "/* Another language struct */\n"
        "struct cpp_struct {\n"
        "    int kind;\n"
        "    union {\n"
        "        int ival;\n"
        "        void *ptr;\n"
        "    } u;\n"
        "} GTY((lang));\n"
        "\n"
        "/* More complex nested types */\n"
        "struct nested_container {\n"
        "    struct lang_struct *langs[5];\n"
        "    union my_union (*union_array[10]);\n"
        "    callback_fn callbacks[3];\n"
        "};\n"
        "\n"
        "/* Pointer to array of pointers */\n"
        "typedef struct complex_type **double_ptr_array[8];\n"
        "%}\n";
    
    /* File 4: File with syntax error (to test error paths) */
    const char* gt_content4 =
        "%{\n"
        "/* Test file 4: File with syntax error */\n"
        "#include \"config.h\"\n"
        "#include \"system.h\"\n"
        "\n"
        "struct error_struct {\n"
        "    int missing_semicolon\n"  /* Missing semicolon */
        "};\n"
        "/* Missing closing %} */\n";
    
    /* File 5: Duplicate definitions (to test warning paths) */
    const char* gt_content5 =
        "%{\n"
        "/* Test file 5: Duplicate definitions */\n"
        "#include \"config.h\"\n"
        "#include \"system.h\"\n"
        "%}\n"
        "\n"
        "struct duplicate_struct {\n"
        "    int a;\n"
        "};\n"
        "\n"
        "/* Duplicate definition */\n"
        "struct duplicate_struct {\n"
        "    int b;\n"
        "};\n"
        "%}\n";
    
    /* Create temporary files */
    char* files[5];
    files[0] = create_temp_gt_file(gt_content1, ".gt");
    files[1] = create_temp_gt_file(gt_content2, ".gt");
    files[2] = create_temp_gt_file(gt_content3, ".gt");
    files[3] = create_temp_gt_file(gt_content4, "_error.gt");
    files[4] = create_temp_gt_file(gt_content5, "_dup.gt");
    
    /* Check all files were created */
    for (int i = 0; i < 5; i++) {
        if (!files[i]) {
            fprintf(stderr, "Failed to create temporary file %d\n", i);
            for (int j = 0; j < i; j++) {
                unlink(files[j]);
                free(files[j]);
            }
            return 1;
        }
        printf("Created temporary file: %s\n", files[i]);
    }
    
    /* Test 1: Normal processing with all valid files */
    printf("\n=== Test 1: Normal processing ===\n");
    int result = compile_and_run_gengtype(files, 3, "test_output.h", "test_output.c");
    
    /* Test 2: Batch processing with -p flag */
    printf("\n=== Test 2: Batch processing ===\n");
    test_batch_processing(files, 3);
    
    /* Test 3: Processing with error file */
    printf("\n=== Test 3: Processing with error file ===\n");
    const char* error_files[] = {files[0], files[3]};
    compile_and_run_gengtype(error_files, 2, "test_error.h", "test_error.c");
    
    /* Test 4: Processing with duplicate definitions */
    printf("\n=== Test 4: Processing with duplicate definitions ===\n");
    const char* dup_files[] = {files[0], files[4]};
    compile_and_run_gengtype(dup_files, 2, "test_dup.h", "test_dup.c");
    
    /* Cleanup */
    printf("\n=== Cleaning up ===\n");
    for (int i = 0; i < 5; i++) {
        unlink(files[i]);
        free(files[i]);
    }
    
    /* Remove generated files */
    unlink("test_output.h");
    unlink("test_output.c");
    unlink("test_error.h");
    unlink("test_error.c");
    unlink("test_dup.h");
    unlink("test_dup.c");
    unlink("gengtype.o");
    unlink("gengtype_coverage");
    
    /* Check for coverage data */
    printf("\n=== Coverage data generated ===\n");
    if (access("gengtype.gcda", F_OK) == 0) {
        printf("Coverage data file created: gengtype.gcda\n");
        printf("Run 'gcov gengtype.cc' to see line coverage\n");
    } else {
        printf("Warning: No coverage data generated\n");
    }
    
    return 0;
}
