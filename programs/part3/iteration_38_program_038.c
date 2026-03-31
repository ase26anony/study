/* test_gengtype_coverage.c - Driver program to test gengtype type counting */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>

/* Temporary file management */
typedef struct {
    char *filename;
    char *content;
} temp_file_t;

/* Generated .gt file contents covering all type categories */
static const char *gt_files[] = {
    /* File 1: Basic types and structs */
    "%{\n"
    "/* Test file 1: Basic types */\n"
    "#include \"config.h\"\n"
    "#include \"system.h\"\n"
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
    "  const char *name;  /* string type */\n"
    "  char *data;\n"
    "};\n"
    "\n"
    "/* TYPE_STRUCT */\n"
    "struct my_struct {\n"
    "  int a;\n"
    "  double b;\n"
    "};\n"
    "\n"
    "/* TYPE_POINTER */\n"
    "typedef struct my_struct *struct_ptr;\n"
    "typedef int *int_ptr;\n"
    "\n"
    "/* TYPE_ARRAY */\n"
    "typedef int my_array[10];\n"
    "typedef struct my_struct struct_array[5];\n"
    "\n"
    "/* TYPE_CALLBACK */\n"
    "typedef void (*callback_fn)(void);\n"
    "typedef int (*compare_fn)(const void *, const void *);\n",

    /* File 2: Unions, user structs, and nested types */
    "%{\n"
    "/* Test file 2: Advanced types */\n"
    "#include \"config.h\"\n"
    "#include \"system.h\"\n"
    "%}\n"
    "\n"
    "/* TYPE_UNION */\n"
    "union my_union {\n"
    "  int i;\n"
    "  void *p;\n"
    "  double d;\n"
    "};\n"
    "\n"
    "/* TYPE_USER_STRUCT */\n"
    "struct user_struct {\n"
    "  int *p;\n"
    "  union my_union u;\n"
    "} GTY((user));\n"
    "\n"
    "/* Complex nested type combining multiple categories */\n"
    "struct complex_struct {\n"
    "  /* Pointer to union containing array */\n"
    "  union my_union *union_ptr;\n"
    "  \n"
    "  /* Array of pointers to callbacks */\n"
    "  void (*callbacks[5])(void);\n"
    "  \n"
    "  /* String member */\n"
    "  const char *description;\n"
    "  \n"
    "  /* Nested struct */\n"
    "  struct inner_struct {\n"
    "    int x;\n"
    "    my_array arr;  /* TYPE_ARRAY */\n"
    "  } inner;\n"
    "};\n"
    "\n"
    "/* Another pointer type to ensure coverage */\n"
    "typedef union my_union *union_ptr_t;\n",

    /* File 3: Language structs and edge cases */
    "%{\n"
    "/* Test file 3: Language-specific and edge cases */\n"
    "#include \"config.h\"\n"
    "#include \"system.h\"\n"
    "%}\n"
    "\n"
    "/* TYPE_LANG_STRUCT */\n"
    "struct lang_struct {\n"
    "  int data;\n"
    "  void *lang_data;\n"
    "} GTY((lang));\n"
    "\n"
    "/* Another user struct with different layout */\n"
    "struct user_struct2 {\n"
    "  struct lang_struct *lang_ptr;\n"
    "  callback_fn handler;\n"
    "} GTY((user));\n"
    "\n"
    "/* Complex type with all categories */\n"
    "struct mega_type {\n"
    "  /* Scalar */\n"
    "  my_scalar count;\n"
    "  \n"
    "  /* String */\n"
    "  const char *name;\n"
    "  \n"
    "  /* Struct */\n"
    "  struct my_struct s;\n"
    "  \n"
    "  /* Union */\n"
    "  union my_union u;\n"
    "  \n"
    "  /* Pointer */\n"
    "  struct complex_struct *complex;\n"
    "  \n"
    "  /* Array */\n"
    "  int values[20];\n"
    "  \n"
    "  /* Callback */\n"
    "  callback_fn notify;\n"
    "  \n"
    "  /* Nested lang struct */\n"
    "  struct lang_struct lang;\n"
    "};\n"
    "\n"
    "/* Forward declaration for undefined */\n"
    "struct another_undefined;\n",

    /* File 4: Error case - missing %} */
    "%{\n"
    "/* Test file 4: Error case - missing closing delimiter */\n"
    "#include \"config.h\"\n"
    "#include \"system.h\"\n"
    "/* Deliberately missing %} to trigger error path */\n"
    "\n"
    "struct error_struct {\n"
    "  int will_not_parse;\n"
    "};\n",

    /* File 5: Duplicate definitions for warning */
    "%{\n"
    "/* Test file 5: Duplicate definitions */\n"
    "#include \"config.h\"\n"
    "#include \"system.h\"\n"
    "%}\n"
    "\n"
    "/* Duplicate definition to trigger warning */\n"
    "struct my_struct {\n"
    "  int a;\n"
    "  double b;\n"
    "};\n"
    "\n"
    "/* Same struct again */\n"
    "struct my_struct {\n"
    "  int a;\n"
    "  double b;\n"
    "};\n"
};

static const int num_gt_files = sizeof(gt_files) / sizeof(gt_files[0]);

/* Create a temporary file with given content */
static char *create_temp_file(const char *content, const char *suffix) {
    char template[] = "/tmp/gengtype_test_XXXXXX";
    int fd = mkstemp(template);
    if (fd == -1) {
        perror("mkstemp");
        return NULL;
    }
    
    if (suffix) {
        char newname[256];
        snprintf(newname, sizeof(newname), "%s%s", template, suffix);
        rename(template, newname);
        strcpy(template, newname);
    }
    
    FILE *f = fdopen(fd, "w");
    if (!f) {
        close(fd);
        return NULL;
    }
    
    fwrite(content, 1, strlen(content), f);
    fclose(f);
    
    return strdup(template);
}

/* Build gengtype with coverage instrumentation */
static int build_gengtype(const char *source_dir) {
    char cmd[1024];
    int status;
    
    printf("Building gengtype with coverage instrumentation...\n");
    
    /* Compile gengtype.cc */
    snprintf(cmd, sizeof(cmd),
             "g++ -O0 -fprofile-arcs -ftest-coverage -DIN_GCC -DHAVE_CONFIG_H "
             "-I%s -I%s/../include -I%s/../gcc -c %s/gengtype.cc -o gengtype.o",
             source_dir, source_dir, source_dir, source_dir);
    printf("Running: %s\n", cmd);
    status = system(cmd);
    if (status != 0) {
        fprintf(stderr, "Failed to compile gengtype.cc\n");
        return -1;
    }
    
    /* Compile gengtype-state.cc */
    snprintf(cmd, sizeof(cmd),
             "g++ -O0 -fprofile-arcs -ftest-coverage -DIN_GCC -DHAVE_CONFIG_H "
             "-I%s -I%s/../include -I%s/../gcc -c %s/gengtype-state.cc -o gengtype-state.o",
             source_dir, source_dir, source_dir, source_dir);
    printf("Running: %s\n", cmd);
    status = system(cmd);
    if (status != 0) {
        fprintf(stderr, "Failed to compile gengtype-state.cc\n");
        return -1;
    }
    
    /* Link gengtype executable */
    snprintf(cmd, sizeof(cmd),
             "g++ -O0 -fprofile-arcs -ftest-coverage gengtype.o gengtype-state.o "
             "-lgcov -liberty -o gengtype_coverage");
    printf("Running: %s\n", cmd);
    status = system(cmd);
    if (status != 0) {
        fprintf(stderr, "Failed to link gengtype\n");
        return -1;
    }
    
    return 0;
}

/* Run gengtype with various patterns to trigger type counting */
static int run_gengtype_tests(char **temp_files, int num_files) {
    int i, status;
    char cmd[2048];
    FILE *list_file;
    
    printf("\n=== Running gengtype tests ===\n");
    
    /* Pattern A: Process each file individually */
    printf("\n--- Pattern A: Individual file processing ---\n");
    for (i = 0; i < num_files; i++) {
        snprintf(cmd, sizeof(cmd), "./gengtype_coverage -g output%d.h %s", i, temp_files[i]);
        printf("Running: %s\n", cmd);
        status = system(cmd);
        printf("Exit status: %d\n", WEXITSTATUS(status));
        
        /* Clean up output file if created */
        snprintf(cmd, sizeof(cmd), "rm -f output%d.h", i);
        system(cmd);
    }
    
    /* Pattern B: Batch processing with -p flag */
    printf("\n--- Pattern B: Batch processing with -p ---\n");
    list_file = fopen("filelist.txt", "w");
    if (list_file) {
        for (i = 0; i < num_files; i++) {
            fprintf(list_file, "%s\n", temp_files[i]);
        }
        fclose(list_file);
        
        status = system("./gengtype_coverage -p filelist.txt");
        printf("Batch processing exit status: %d\n", WEXITSTATUS(status));
        unlink("filelist.txt");
    }
    
    /* Pattern C: Multiple files with header generation */
    printf("\n--- Pattern C: Multiple files, generate header ---\n");
    snprintf(cmd, sizeof(cmd), "./gengtype_coverage -g combined.h %s %s %s",
             temp_files[0], temp_files[1], temp_files[2]);
    printf("Running: %s\n", cmd);
    status = system(cmd);
    printf("Exit status: %d\n", WEXITSTATUS(status));
    
    /* Pattern D: Generate routine file */
    printf("\n--- Pattern D: Generate routine file ---\n");
    snprintf(cmd, sizeof(cmd), "./gengtype_coverage -r combined.c %s %s",
             temp_files[0], temp_files[1]);
    printf("Running: %s\n", cmd);
    status = system(cmd);
    printf("Exit status: %d\n", WEXITSTATUS(status));
    
    /* Clean up */
    unlink("combined.h");
    unlink("combined.c");
    
    return 0;
}

/* Generate coverage report */
static void generate_coverage_report(void) {
    printf("\n=== Generating coverage report ===\n");
    
    /* Run gcov on the instrumented files */
    system("gcov gengtype.cc");
    system("gcov gengtype-state.cc");
    
    /* Display coverage summary */
    printf("\n--- Coverage summary for gengtype.cc ---\n");
    FILE *gcov_file = fopen("gengtype.cc.gcov", "r");
    if (gcov_file) {
        char line[256];
        int uncovered_lines = 0;
        int total_lines = 0;
        
        while (fgets(line, sizeof(line), gcov_file)) {
            if (strstr(line, "182:") || strstr(line, "213:")) {
                printf("Switch statement lines: %s", line);
            }
            if (strstr(line, ":    0:")) {
                uncovered_lines++;
            }
            total_lines++;
        }
        fclose(gcov_file);
        
        if (total_lines > 0) {
            float coverage = 100.0 * (1.0 - (float)uncovered_lines / total_lines);
            printf("\nEstimated coverage: %.2f%%\n", coverage);
        }
    }
}

int main(int argc, char **argv) {
    char *temp_files[10];
    int i;
    const char *source_dir = ".";
    
    if (argc > 1) {
        source_dir = argv[1];
    }
    
    printf("GCC gengtype Coverage Test Driver\n");
    printf("Source directory: %s\n", source_dir);
    
    /* Create temporary .gt files */
    printf("\nCreating %d test .gt files...\n", num_gt_files);
    for (i = 0; i < num_gt_files; i++) {
        temp_files[i] = create_temp_file(gt_files[i], ".gt");
        if (!temp_files[i]) {
            fprintf(stderr, "Failed to create temp file %d\n", i);
            return 1;
        }
        printf("Created: %s\n", temp_files[i]);
    }
    
    /* Build gengtype with coverage */
    if (build_gengtype(source_dir) != 0) {
        fprintf(stderr, "Failed to build gengtype\n");
        return 1;
    }
    
    /* Run tests with various patterns */
    run_gengtype_tests(temp_files, num_gt_files);
    
    /* Generate coverage report */
    generate_coverage_report();
    
    /* Cleanup */
    printf("\nCleaning up temporary files...\n");
    for (i = 0; i < num_gt_files; i++) {
        if (temp_files[i]) {
            unlink(temp_files[i]);
            free(temp_files[i]);
        }
    }
    
    /* Cleanup compiled files */
    unlink("gengtype.o");
    unlink("gengtype-state.o");
    unlink("gengtype_coverage");
    unlink("gengtype.gcda");
    unlink("gengtype.gcno");
    unlink("gengtype-state.gcda");
    unlink("gengtype-state.gcno");
    unlink("gengtype.cc.gcov");
    unlink("gengtype-state.cc.gcov");
    
    printf("\nTest completed successfully!\n");
    return 0;
}
