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

/* gengtype input files covering all type categories */
static const char *gt_files[] = {
    /* File 1: Basic types and structs */
    "types1.gt",
    "%%{\n"
    "/* Test file 1: Basic types */\n"
    "#include \"config.h\"\n"
    "#include \"system.h\"\n"
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
    "typedef my_scalar *scalar_ptr;\n"
    "\n"
    "/* TYPE_ARRAY */\n"
    "typedef int my_array[10];\n"
    "typedef struct my_struct struct_array[5];\n"
    "\n"
    "/* TYPE_STRING */\n"
    "struct string_struct {\n"
    "    const char *name;  /* string type */\n"
    "    char *buffer;\n"
    "};\n"
    "\n"
    "%%}\n",
    
    /* File 2: Unions, callbacks, and user structs */
    "types2.gt",
    "%%{\n"
    "/* Test file 2: Advanced types */\n"
    "#include \"config.h\"\n"
    "#include \"system.h\"\n"
    "\n"
    "/* TYPE_UNION */\n"
    "union my_union {\n"
    "    int i;\n"
    "    void *p;\n"
    "    double d;\n"
    "};\n"
    "\n"
    "/* TYPE_CALLBACK */\n"
    "typedef void (*callback_fn)(void);\n"
    "typedef int (*compare_fn)(const void *, const void *);\n"
    "\n"
    "/* TYPE_USER_STRUCT */\n"
    "struct user_struct {\n"
    "    int *p;\n"
    "    void *data;\n"
    "} GTY((user));\n"
    "\n"
    "/* Nested complex type */\n"
    "struct complex_struct {\n"
    "    union my_union u;\n"
    "    callback_fn handler;\n"
    "    struct user_struct *user_ptr;\n"
    "    int values[20];\n"
    "};\n"
    "\n"
    "/* Pointer to array of callbacks */\n"
    "typedef callback_fn (*callback_array_ptr)[5];\n"
    "\n"
    "%%}\n",
    
    /* File 3: Language structs and error cases */
    "types3.gt",
    "%%{\n"
    "/* Test file 3: Language-specific and edge cases */\n"
    "#include \"config.h\"\n"
    "#include \"system.h\"\n"
    "\n"
    "/* TYPE_LANG_STRUCT */\n"
    "struct lang_struct {\n"
    "    int data;\n"
    "    void *lang_data;\n"
    "} GTY ((lang));\n"
    "\n"
    "/* Another TYPE_USER_STRUCT with different layout */\n"
    "struct user_struct2 {\n"
    "    struct lang_struct *lang_ptr;\n"
    "    union my_union *union_ptr;\n"
    "} GTY((user));\n"
    "\n"
    "/* Complex nested type combining multiple categories */\n"
    "struct super_complex {\n"
    "    struct {\n"
    "        int count;\n"
    "        struct user_struct **users;\n"
    "    } header;\n"
    "    union {\n"
    "        callback_fn callbacks[10];\n"
    "        struct lang_struct langs[5];\n"
    "    } data;\n"
    "    const char *description;\n"
    "};\n"
    "\n"
    "/* Array of pointers to unions */\n"
    "typedef union my_union *union_ptr_array[8];\n"
    "\n"
    "%%}\n",
    
    /* File 4: Deliberate syntax error to test error paths */
    "error.gt",
    "%%{\n"
    "/* File with syntax error - missing closing %%} */\n"
    "#include \"config.h\"\n"
    "#include \"system.h\"\n"
    "\n"
    "struct error_struct {\n"
    "    int x;\n"
    "    int y;\n"
    "};\n"
    "\n"
    "/* Missing closing delimiter */\n",
    
    /* File 5: Duplicate definitions for warning testing */
    "duplicate.gt",
    "%%{\n"
    "/* File with duplicate definitions */\n"
    "#include \"config.h\"\n"
    "#include \"system.h\"\n"
    "\n"
    "struct duplicate_struct {\n"
    "    int a;\n"
    "};\n"
    "\n"
    "/* Duplicate definition */\n"
    "struct duplicate_struct {\n"
    "    int b;\n"
    "};\n"
    "\n"
    "%%}\n",
    
    NULL
};

/* Create temporary file with given content */
static char *create_temp_file(const char *content, const char *suffix) {
    char template[] = "/tmp/gengtype_test_XXXXXX";
    int fd = mkstemp(template);
    if (fd < 0) {
        perror("mkstemp");
        return NULL;
    }
    
    if (suffix) {
        char newname[256];
        snprintf(newname, sizeof(newname), "%s%s", template, suffix);
        close(fd);
        unlink(template);
        fd = open(newname, O_CREAT | O_WRONLY, 0600);
        strcpy(template, newname);
    }
    
    size_t len = strlen(content);
    ssize_t written = write(fd, content, len);
    close(fd);
    
    if (written != (ssize_t)len) {
        perror("write");
        unlink(template);
        return NULL;
    }
    
    return strdup(template);
}

/* Run gengtype with given arguments */
static int run_gengtype(const char *gengtype_exe, char *const argv[]) {
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return -1;
    }
    
    if (pid == 0) {
        /* Child process */
        execvp(gengtype_exe, argv);
        perror("execvp");
        exit(EXIT_FAILURE);
    }
    
    /* Parent process */
    int status;
    waitpid(pid, &status, 0);
    
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    return -1;
}

/* Build gengtype with coverage instrumentation */
static int build_gengtype() {
    printf("Building gengtype with coverage instrumentation...\n");
    
    /* Compile gengtype.cc */
    const char *compile_cmd[] = {
        "g++", "-O0", "-fprofile-arcs", "-ftest-coverage",
        "-DIN_GCC", "-DHAVE_CONFIG_H",
        "-I.", "-I../../include", "-I../../gcc",
        "-c", "gengtype.cc",
        "-o", "gengtype.o",
        NULL
    };
    
    if (run_gengtype("g++", (char *const *)compile_cmd) != 0) {
        fprintf(stderr, "Failed to compile gengtype.cc\n");
        return 0;
    }
    
    /* Compile gengtype-state.cc */
    const char *compile_state_cmd[] = {
        "g++", "-O0", "-fprofile-arcs", "-ftest-coverage",
        "-DIN_GCC", "-DHAVE_CONFIG_H",
        "-I.", "-I../../include", "-I../../gcc",
        "-c", "gengtype-state.cc",
        "-o", "gengtype-state.o",
        NULL
    };
    
    if (run_gengtype("g++", (char *const *)compile_state_cmd) != 0) {
        fprintf(stderr, "Failed to compile gengtype-state.cc\n");
        return 0;
    }
    
    /* Link gengtype executable */
    const char *link_cmd[] = {
        "g++", "-O0", "-fprofile-arcs", "-ftest-coverage",
        "gengtype.o", "gengtype-state.o",
        "-o", "gengtype-coverage",
        "-liberty", "-lgcov",
        NULL
    };
    
    if (run_gengtype("g++", (char *const *)link_cmd) != 0) {
        fprintf(stderr, "Failed to link gengtype\n");
        return 0;
    }
    
    printf("gengtype built successfully as 'gengtype-coverage'\n");
    return 1;
}

/* Test pattern A: Process each file individually */
static void test_pattern_a(const char *gengtype_exe, char **temp_files, int file_count) {
    printf("\n=== Pattern A: Processing files individually ===\n");
    
    for (int i = 0; i < file_count; i++) {
        printf("Processing %s...\n", temp_files[i]);
        
        char output_header[256];
        snprintf(output_header, sizeof(output_header), "output%d.h", i);
        
        const char *args[] = {
            gengtype_exe,
            "-g", output_header,
            temp_files[i],
            NULL
        };
        
        int result = run_gengtype(gengtype_exe, (char *const *)args);
        printf("  Result: %s (exit code: %d)\n", 
               result == 0 ? "SUCCESS" : "FAILED", result);
        
        /* Clean up output file */
        unlink(output_header);
    }
}

/* Test pattern B: Batch processing with -p flag */
static void test_pattern_b(const char *gengtype_exe, char **temp_files, int file_count) {
    printf("\n=== Pattern B: Batch processing with -p flag ===\n");
    
    /* Create file list */
    char *filelist = create_temp_file("", ".list");
    if (!filelist) {
        fprintf(stderr, "Failed to create file list\n");
        return;
    }
    
    FILE *fp = fopen(filelist, "w");
    if (!fp) {
        perror("fopen filelist");
        free(filelist);
        return;
    }
    
    for (int i = 0; i < file_count; i++) {
        fprintf(fp, "%s\n", temp_files[i]);
    }
    fclose(fp);
    
    const char *args[] = {
        gengtype_exe,
        "-p", filelist,
        NULL
    };
    
    int result = run_gengtype(gengtype_exe, (char *const *)args);
    printf("Batch processing result: %s (exit code: %d)\n",
           result == 0 ? "SUCCESS" : "FAILED", result);
    
    unlink(filelist);
    free(filelist);
}

/* Test pattern C: Generate both header and routine files */
static void test_pattern_c(const char *gengtype_exe, char **temp_files, int file_count) {
    printf("\n=== Pattern C: Generate header and routine files ===\n");
    
    /* Build argument list with all input files */
    char **args = malloc((file_count + 4) * sizeof(char *));
    if (!args) {
        perror("malloc");
        return;
    }
    
    int arg_idx = 0;
    args[arg_idx++] = (char *)gengtype_exe;
    args[arg_idx++] = "-g";
    args[arg_idx++] = "combined.h";
    args[arg_idx++] = "-r";
    args[arg_idx++] = "combined.c";
    
    for (int i = 0; i < file_count; i++) {
        args[arg_idx++] = temp_files[i];
    }
    args[arg_idx] = NULL;
    
    int result = run_gengtype(gengtype_exe, args);
    printf("Combined generation result: %s (exit code: %d)\n",
           result == 0 ? "SUCCESS" : "FAILED", result);
    
    /* Clean up output files */
    unlink("combined.h");
    unlink("combined.c");
    free(args);
}

/* Test pattern D: Error and warning cases */
static void test_pattern_d(const char *gengtype_exe, char **temp_files, int file_count) {
    printf("\n=== Pattern D: Error and warning testing ===\n");
    
    /* Test with syntax error file */
    printf("Testing syntax error file...\n");
    const char *error_args[] = {
        gengtype_exe,
        "-g", "error.h",
        temp_files[3],  /* error.gt */
        NULL
    };
    
    int error_result = run_gengtype(gengtype_exe, (char *const *)error_args);
    printf("  Syntax error test: %s (expected non-zero exit: %d)\n",
           error_result != 0 ? "PASS" : "FAIL", error_result);
    
    /* Test with duplicate definitions */
    printf("Testing duplicate definitions...\n");
    const char *dup_args[] = {
        gengtype_exe,
        "-g", "dup.h",
        temp_files[4],  /* duplicate.gt */
        NULL
    };
    
    int dup_result = run_gengtype(gengtype_exe, (char *const *)dup_args);
    printf("  Duplicate test: exit code %d\n", dup_result);
    
    /* Clean up */
    unlink("error.h");
    unlink("dup.h");
}

/* Main test driver */
int main(int argc, char *argv[]) {
    printf("=== gengtype Coverage Test Driver ===\n");
    
    /* Build gengtype with coverage */
    if (!build_gengtype()) {
        fprintf(stderr, "Failed to build gengtype\n");
        return EXIT_FAILURE;
    }
    
    /* Create temporary input files */
    int file_count = 0;
    char *temp_files[10];
    
    for (int i = 0; gt_files[i] != NULL; i += 2) {
        const char *filename = gt_files[i];
        const char *content = gt_files[i + 1];
        
        /* Replace %% with % in content */
        char *processed_content = strdup(content);
        if (!processed_content) {
            perror("strdup");
            continue;
        }
        
        /* gengtype expects %{ and %} not %%{ and %%} */
        char *p = processed_content;
        while ((p = strstr(p, "%%")) != NULL) {
            memmove(p, p + 1, strlen(p));  /* Remove one % */
        }
        
        char *temp_file = create_temp_file(processed_content, ".gt");
        free(processed_content);
        
        if (temp_file) {
            printf("Created temporary file: %s\n", temp_file);
            temp_files[file_count++] = temp_file;
        }
    }
    
    if (file_count == 0) {
        fprintf(stderr, "No input files created\n");
        return EXIT_FAILURE;
    }
    
    const char *gengtype_exe = "./gengtype-coverage";
    
    /* Execute all test patterns */
    test_pattern_a(gengtype_exe, temp_files, file_count);
    test_pattern_b(gengtype_exe, temp_files, file_count);
    test_pattern_c(gengtype_exe, temp_files, file_count);
    test_pattern_d(gengtype_exe, temp_files, file_count);
    
    /* Additional test: Process all valid files together */
    printf("\n=== Final comprehensive test ===\n");
    char **valid_args = malloc((4 + 3) * sizeof(char *));  /* 3 valid files */
    if (valid_args) {
        int idx = 0;
        valid_args[idx++] = (char *)gengtype_exe;
        valid_args[idx++] = "-g";
        valid_args[idx++] = "final.h";
        
        /* Add only valid files (skip error files) */
        for (int i = 0; i < 3 && i < file_count; i++) {
            valid_args[idx++] = temp_files[i];
        }
        valid_args[idx] = NULL;
        
        int final_result = run_gengtype(gengtype_exe, valid_args);
        printf("Final comprehensive test: %s (exit code: %d)\n",
               final_result == 0 ? "SUCCESS" : "FAILED", final_result);
        
        unlink("final.h");
        free(valid_args);
    }
    
    /* Clean up temporary files */
    printf("\nCleaning up temporary files...\n");
    for (int i = 0; i < file_count; i++) {
        unlink(temp_files[i]);
        free(temp_files[i]);
    }
    
    /* Generate coverage report */
    printf("\n=== Generating coverage report ===\n");
    system("gcov gengtype.cc gengtype-state.cc");
    
    /* Display coverage summary */
    FILE *gcov_file = fopen("gengtype.cc.gcov", "r");
    if (gcov_file) {
        char line[256];
        int target_lines[] = {182, 183, 184, 185, 186, 187, 188, 189, 190, 
                              191, 192, 193, 194, 195, 196, 197, 198, 199, 
                              200, 201, 202, 203, 204, 205, 206, 207, 208, 
                              209, 210, 211, 212, 213};
        int num_targets = sizeof(target_lines) / sizeof(target_lines[0]);
        
        printf("\nCoverage for target switch statement (lines 182-213):\n");
        printf("Line  Count  Content\n");
        printf("----  -----  -------\n");
        
        while (fgets(line, sizeof(line), gcov_file)) {
            int line_num;
            long count;
            char content[256];
            
            if (sscanf(line, "%ld:%d:%s", &count, &line_num, content) >= 2) {
                for (int i = 0; i < num_targets; i++) {
                    if (line_num == target_lines[i]) {
                        printf("%4d  %5ld  %s\n", line_num, count, 
                               line_num >= 182 && line_num <= 213 ? 
                               (count > 0 ? "EXECUTED" : "NOT EXECUTED") : "");
                        break;
                    }
                }
            }
        }
        fclose(gcov_file);
    }
    
    printf("\n=== Test completed ===\n");
    printf("Check gengtype.cc.gcov for detailed coverage information\n");
    
    return EXIT_SUCCESS;
}
