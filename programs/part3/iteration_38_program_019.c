/* test_gengtype_coverage.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>

/* Compile gengtype with coverage instrumentation */
#define COMPILE_GENGTYPE "g++ -O0 -fprofile-arcs -ftest-coverage -DIN_GCC -DHAVE_CONFIG_H -I. -I../../include -I../../gcc -c gengtype.cc -o gengtype.o && " \
                         "g++ -O0 -fprofile-arcs -ftest-coverage -DIN_GCC -DHAVE_CONFIG_H -I. -I../../include -I../../gcc -c gengtype-state.cc -o gengtype-state.o && " \
                         "g++ -O0 -fprofile-arcs -ftest-coverage gengtype.o gengtype-state.o -lgcov -liberty -o gengtype_coverage"

/* Test files with various type definitions */
const char *gt_file1 = 
"%{\n"
"/* File 1: Basic type definitions */\n"
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
"typedef struct my_struct struct_array[5];\n";

const char *gt_file2 =
"%{\n"
"/* File 2: Complex and user-defined types */\n"
"%}\n"
"\n"
"/* TYPE_USER_STRUCT with user-provided marking routine */\n"
"struct user_struct {\n"
"  int *p;\n"
"  void *data;\n"
"} GTY((user));\n"
"\n"
"/* TYPE_UNION */\n"
"union my_union {\n"
"  int i;\n"
"  void *p;\n"
"  double d;\n"
"};\n"
"\n"
"/* TYPE_CALLBACK */\n"
"typedef void (*callback_fn)(void);\n"
"typedef int (*compare_fn)(const void *, const void *);\n"
"\n"
"/* TYPE_LANG_STRUCT */\n"
"struct lang_struct {\n"
"  int data;\n"
"  void *extra;\n"
"} GTY ((lang));\n"
"\n"
"/* Complex nested types */\n"
"struct complex_struct {\n"
"  union my_union u;          /* union member */\n"
"  callback_fn handler;       /* callback */\n"
"  struct lang_struct *lang;  /* pointer to lang struct */\n"
"  int array[20];            /* array */\n"
"};\n"
"\n"
"/* Another scalar */\n"
"typedef float float_scalar;\n";

const char *gt_file3 =
"%{\n"
"/* File 3: More combinations and edge cases */\n"
"%}\n"
"\n"
"/* Forward declarations (TYPE_UNDEFINED) */\n"
"struct forward1;\n"
"union forward_union;\n"
"\n"
"/* String-heavy struct */\n"
"struct string_container {\n"
"  const char *title;\n"
"  char *buffer;\n"
"  const char *description;\n"
"};\n"
"\n"
"/* Array of pointers */\n"
"typedef void *ptr_array[50];\n"
"\n"
"/* Pointer to array */\n"
"typedef int (*array_ptr)[10];\n"
"\n"
"/* Union containing struct */\n"
"union complex_union {\n"
"  struct {\n"
"    int x;\n"
"    int y;\n"
"  } point;\n"
"  float values[4];\n"
"};\n"
"\n"
"/* Multiple callbacks */\n"
"typedef void (*start_fn)(void);\n"
"typedef void (*finish_fn)(int);\n"
"\n"
"/* Mixed struct with all types */\n"
"struct mega_struct {\n"
"  int id;                    /* scalar */\n"
"  const char *name;          /* string */\n"
"  void *data;                /* pointer */\n"
"  int scores[100];           /* array */\n"
"  union my_union value;      /* union */\n"
"  callback_fn notify;        /* callback */\n"
"};\n";

/* File with syntax error to test error paths */
const char *gt_file_error =
"%{\n"
"/* File with deliberate error - missing closing %}\n"
"/* This should trigger error handling but still process some types */\n"
"\n"
"struct error_struct {\n"
"  int x;\n"
"};\n"
"/* Missing %} here */\n";

/* File with duplicate definition to test warnings */
const char *gt_file_warning =
"%{\n"
"/* File with duplicate type definition */\n"
"%}\n"
"\n"
"struct duplicate {\n"
"  int a;\n"
"};\n"
"\n"
"/* Duplicate definition */\n"
"struct duplicate {\n"
"  int b;\n"
"};\n";

/* Create temporary file with given content */
char *create_temp_file(const char *content, const char *suffix) {
    char template[] = "/tmp/gengtype_test_XXXXXX";
    int fd = mkstemp(template);
    if (fd == -1) {
        perror("mkstemp");
        return NULL;
    }
    
    char *filename = strdup(template);
    strcat(filename, suffix);
    
    /* Rename to add suffix */
    rename(template, filename);
    
    FILE *f = fdopen(fd, "w");
    if (!f) {
        perror("fdopen");
        free(filename);
        return NULL;
    }
    
    fwrite(content, 1, strlen(content), f);
    fclose(f);
    
    return filename;
}

/* Run gengtype with given arguments */
int run_gengtype(const char *gengtype_path, const char **args, int arg_count) {
    pid_t pid = fork();
    if (pid == 0) {
        /* Child process */
        char **argv = malloc((arg_count + 2) * sizeof(char *));
        argv[0] = (char *)gengtype_path;
        for (int i = 0; i < arg_count; i++) {
            argv[i + 1] = (char *)args[i];
        }
        argv[arg_count + 1] = NULL;
        
        execv(gengtype_path, argv);
        perror("execv");
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

/* Create file list for batch processing */
char *create_file_list(char **files, int count) {
    char *listname = strdup("/tmp/gengtype_filelist.txt");
    FILE *f = fopen(listname, "w");
    if (!f) {
        perror("fopen");
        free(listname);
        return NULL;
    }
    
    for (int i = 0; i < count; i++) {
        fprintf(f, "%s\n", files[i]);
    }
    fclose(f);
    
    return listname;
}

int main() {
    printf("=== GCC gengtype Coverage Test ===\n\n");
    
    /* Step 1: Compile gengtype with coverage instrumentation */
    printf("1. Compiling gengtype with coverage flags...\n");
    int compile_result = system(COMPILE_GENGTYPE);
    if (compile_result != 0) {
        fprintf(stderr, "Failed to compile gengtype\n");
        return 1;
    }
    printf("   Compilation successful\n\n");
    
    /* Step 2: Create temporary .gt files */
    printf("2. Creating test .gt files...\n");
    char *files[6];
    
    files[0] = create_temp_file(gt_file1, ".gt");
    files[1] = create_temp_file(gt_file2, ".gt");
    files[2] = create_temp_file(gt_file3, ".gt");
    files[3] = create_temp_file(gt_file_error, "_error.gt");
    files[4] = create_temp_file(gt_file_warning, "_warning.gt");
    
    for (int i = 0; i < 5; i++) {
        if (!files[i]) {
            fprintf(stderr, "Failed to create temp file %d\n", i);
            return 1;
        }
        printf("   Created: %s\n", files[i]);
    }
    printf("\n");
    
    /* Step 3: Test Pattern A - Process each file individually */
    printf("3. Pattern A: Processing files individually...\n");
    for (int i = 0; i < 3; i++) {  /* Only process valid files */
        printf("   Processing %s...\n", files[i]);
        const char *args[] = {"-g", "output.h", files[i]};
        int result = run_gengtype("./gengtype_coverage", args, 3);
        printf("   Exit code: %d\n", result);
        
        /* Clean up output file */
        unlink("output.h");
    }
    printf("\n");
    
    /* Step 4: Test Pattern B - Batch processing with -p */
    printf("4. Pattern B: Batch processing with -p flag...\n");
    char *filelist = create_file_list(files, 3);  /* List only valid files */
    if (filelist) {
        const char *args[] = {"-p", filelist};
        int result = run_gengtype("./gengtype_coverage", args, 2);
        printf("   Batch processing exit code: %d\n", result);
        unlink(filelist);
        free(filelist);
    }
    printf("\n");
    
    /* Step 5: Test Pattern C - Generate both header and routine files */
    printf("5. Pattern C: Generating header and routine files...\n");
    const char *args_c[] = {"-g", "types.h", "-r", "types.c", files[0], files[1], files[2]};
    int result_c = run_gengtype("./gengtype_coverage", args_c, 7);
    printf("   Generation exit code: %d\n", result_c);
    
    /* Verify output files were created */
    struct stat st;
    if (stat("types.h", &st) == 0) {
        printf("   Output file 'types.h' created (%ld bytes)\n", st.st_size);
    }
    if (stat("types.c", &st) == 0) {
        printf("   Output file 'types.c' created (%ld bytes)\n", st.st_size);
    }
    printf("\n");
    
    /* Step 6: Test Pattern D - Error and warning cases */
    printf("6. Pattern D: Testing error and warning paths...\n");
    
    /* Test error file */
    printf("   Processing file with syntax error...\n");
    const char *args_err[] = {files[3]};
    int result_err = run_gengtype("./gengtype_coverage", args_err, 1);
    printf("   Error file exit code: %d (expected non-zero)\n", result_err);
    
    /* Test warning file */
    printf("   Processing file with duplicate definition...\n");
    const char *args_warn[] = {files[4]};
    int result_warn = run_gengtype("./gengtype_coverage", args_warn, 1);
    printf("   Warning file exit code: %d\n", result_warn);
    printf("\n");
    
    /* Step 7: Clean up */
    printf("7. Cleaning up temporary files...\n");
    for (int i = 0; i < 5; i++) {
        unlink(files[i]);
        free(files[i]);
    }
    
    unlink("types.h");
    unlink("types.c");
    unlink("gengtype_coverage");
    unlink("gengtype.o");
    unlink("gengtype-state.o");
    
    /* Clean up coverage files */
    unlink("gengtype.gcda");
    unlink("gengtype.gcno");
    unlink("gengtype-state.gcda");
    unlink("gengtype-state.gcno");
    
    printf("\n=== Test completed ===\n");
    
    /* Step 8: Generate coverage report */
    printf("\n8. Generating coverage report...\n");
    system("gcov gengtype.cc");
    
    /* Check if our target lines were covered */
    FILE *gcov_output = fopen("gengtype.cc.gcov", "r");
    if (gcov_output) {
        char line[256];
        int target_start = 182;
        int target_end = 213;
        
        printf("\nCoverage for target lines %d-%d:\n", target_start, target_end);
        while (fgets(line, sizeof(line), gcov_output)) {
            int line_num;
            int count;
            if (sscanf(line, "%d:%d", &count, &line_num) == 2) {
                if (line_num >= target_start && line_num <= target_end) {
                    printf("Line %d: executed %d time(s)\n", line_num, count);
                }
            }
        }
        fclose(gcov_output);
    }
    
    return 0;
}
