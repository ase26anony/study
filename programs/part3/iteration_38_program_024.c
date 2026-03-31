/* gengtype_coverage_test.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>

/* Compile gengtype with coverage flags */
#define GENGTYPE_SOURCE "gengtype.cc"
#define GENGTYPE_STATE_SOURCE "gengtype-state.cc"
#define GENGTYPE_EXECUTABLE "gengtype_coverage"

/* Create diverse .gt files to cover all type categories */
const char *gt_files[] = {
    /* File 1: Basic types and structs */
    "test1.gt",
    "test2.gt", 
    "test3.gt",
    "test4.gt",
    "test5.gt"
};

const char *gt_contents[] = {
    /* test1.gt - Basic types and forward declarations */
    "%{\n"
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
    "/* TYPE_STRING: String types */\n"
    "struct string_struct {\n"
    "  const char *name;  /* TYPE_STRING */\n"
    "  char *buffer;\n"
    "};\n"
    "\n"
    "/* TYPE_STRUCT: Regular struct */\n"
    "struct my_struct {\n"
    "  int a;\n"
    "  double b;\n"
    "};\n"
    "\n"
    "/* TYPE_POINTER: Pointer typedef */\n"
    "typedef struct my_struct *my_struct_ptr;\n"
    "%}\n",
    
    /* test2.gt - User structs, unions, and arrays */
    "%{\n"
    "#include \"config.h\"\n"
    "#include \"system.h\"\n"
    "%}\n"
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
    "/* TYPE_CALLBACK: Callback function pointer */\n"
    "typedef void (*callback_fn)(void);\n"
    "typedef int (*compare_fn)(const void *, const void *);\n"
    "\n"
    "/* Complex nested type */\n"
    "struct complex_struct {\n"
    "  union my_union u;\n"
    "  my_array arr;\n"
    "  callback_fn handler;\n"
    "};\n"
    "%}\n",
    
    /* test3.gt - Language structs and more pointers */
    "%{\n"
    "#include \"config.h\"\n"
    "#include \"system.h\"\n"
    "%}\n"
    "\n"
    "/* TYPE_LANG_STRUCT: Language-specific struct */\n"
    "struct lang_struct {\n"
    "  int data;\n"
    "  void *extra;\n"
    "} GTY((lang));\n"
    "\n"
    "/* More pointer types */\n"
    "typedef union my_union *union_ptr;\n"
    "typedef callback_fn *callback_ptr;\n"
    "\n"
    "/* Array of pointers */\n"
    "typedef void *ptr_array[20];\n"
    "\n"
    "/* Struct with multiple pointer types */\n"
    "struct pointer_heavy {\n"
    "  int **double_ptr;\n"
    "  struct my_struct *next;\n"
    "  const char *const *string_ptrs;\n"
    "};\n"
    "%}\n",
    
    /* test4.gt - Mixed types with nesting */
    "%{\n"
    "#include \"config.h\"\n"
    "#include \"system.h\"\n"
    "%}\n"
    "\n"
    "/* Forward declarations (TYPE_UNDEFINED) */\n"
    "struct forward1;\n"
    "struct forward2;\n"
    "\n"
    "/* Scalar chain */\n"
    "typedef short small_scalar;\n"
    "typedef small_scalar smaller;\n"
    "\n"
    "/* String in union */\n"
    "union string_union {\n"
    "  const char *str;\n"
    "  char *modifiable_str;\n"
    "};\n"
    "\n"
    "/* Struct containing array of callbacks */\n"
    "struct callback_container {\n"
    "  callback_fn handlers[5];\n"
    "  int count;\n"
    "};\n"
    "\n"
    "/* User struct with complex members */\n"
    "struct complex_user_struct {\n"
    "  struct lang_struct *lang;\n"
    "  union my_union data;\n"
    "  my_array numbers;\n"
    "} GTY((user));\n"
    "%}\n",
    
    /* test5.gt - File with intentional error for error path testing */
    "%{\n"
    "#include \"config.h\"\n"
    "#include \"system.h\"\n"
    "\n"
    "/* Missing closing %} to trigger error */\n"
    "struct error_test {\n"
    "  int should_fail;\n"
    "};\n"
    "/* No closing %} */\n"
};

/* Build gengtype with coverage instrumentation */
int build_gengtype_with_coverage() {
    char compile_cmd[1024];
    int status;
    
    printf("Building gengtype with coverage instrumentation...\n");
    
    /* Compile gengtype.cc */
    snprintf(compile_cmd, sizeof(compile_cmd),
             "g++ -O0 -fprofile-arcs -ftest-coverage "
             "-DIN_GCC -DHAVE_CONFIG_H "
             "-I. -I../../include -I../../gcc "
             "-c %s -o gengtype.o",
             GENGTYPE_SOURCE);
    
    status = system(compile_cmd);
    if (status != 0) {
        fprintf(stderr, "Failed to compile gengtype.cc\n");
        return -1;
    }
    
    /* Compile gengtype-state.cc */
    snprintf(compile_cmd, sizeof(compile_cmd),
             "g++ -O0 -fprofile-arcs -ftest-coverage "
             "-DIN_GCC -DHAVE_CONFIG_H "
             "-I. -I../../include -I../../gcc "
             "-c %s -o gengtype-state.o",
             GENGTYPE_STATE_SOURCE);
    
    status = system(compile_cmd);
    if (status != 0) {
        fprintf(stderr, "Failed to compile gengtype-state.cc\n");
        return -1;
    }
    
    /* Link the executable */
    snprintf(compile_cmd, sizeof(compile_cmd),
             "g++ -O0 -fprofile-arcs -ftest-coverage "
             "gengtype.o gengtype-state.o "
             "-lgcov -liberty -o %s",
             GENGTYPE_EXECUTABLE);
    
    status = system(compile_cmd);
    if (status != 0) {
        fprintf(stderr, "Failed to link gengtype executable\n");
        return -1;
    }
    
    printf("Successfully built %s\n", GENGTYPE_EXECUTABLE);
    return 0;
}

/* Create temporary .gt files */
int create_gt_files() {
    int num_files = sizeof(gt_files) / sizeof(gt_files[0]);
    
    for (int i = 0; i < num_files; i++) {
        FILE *f = fopen(gt_files[i], "w");
        if (!f) {
            perror("Failed to create .gt file");
            return -1;
        }
        
        fwrite(gt_contents[i], 1, strlen(gt_contents[i]), f);
        fclose(f);
        
        printf("Created %s\n", gt_files[i]);
    }
    
    return 0;
}

/* Run gengtype with various patterns to trigger type counting */
int run_gengtype_patterns() {
    char cmd[1024];
    int status;
    FILE *filelist;
    
    printf("\n=== Running gengtype patterns ===\n");
    
    /* Pattern A: Process each file individually */
    printf("\nPattern A: Processing files individually\n");
    for (int i = 0; i < 4; i++) {  /* Skip the error file for now */
        snprintf(cmd, sizeof(cmd), "./%s -g output%d.h %s", 
                GENGTYPE_EXECUTABLE, i, gt_files[i]);
        printf("Running: %s\n", cmd);
        status = system(cmd);
        printf("Exit status: %d\n", WEXITSTATUS(status));
    }
    
    /* Pattern B: Batch processing with -p flag */
    printf("\nPattern B: Batch processing with -p\n");
    filelist = fopen("filelist.txt", "w");
    if (filelist) {
        for (int i = 0; i < 4; i++) {
            fprintf(filelist, "%s\n", gt_files[i]);
        }
        fclose(filelist);
        
        snprintf(cmd, sizeof(cmd), "./%s -p filelist.txt", GENGTYPE_EXECUTABLE);
        printf("Running: %s\n", cmd);
        status = system(cmd);
        printf("Exit status: %d\n", WEXITSTATUS(status));
    }
    
    /* Pattern C: Multiple files with header generation */
    printf("\nPattern C: Multiple files with header generation\n");
    snprintf(cmd, sizeof(cmd), "./%s -g combined.h test1.gt test2.gt test3.gt", 
            GENGTYPE_EXECUTABLE);
    printf("Running: %s\n", cmd);
    status = system(cmd);
    printf("Exit status: %d\n", WEXITSTATUS(status));
    
    /* Pattern D: Error case */
    printf("\nPattern D: Error case (syntax error)\n");
    snprintf(cmd, sizeof(cmd), "./%s test5.gt", GENGTYPE_EXECUTABLE);
    printf("Running: %s\n", cmd);
    status = system(cmd);
    printf("Exit status: %d\n", WEXITSTATUS(status));
    
    /* Additional pattern: Generate routine file */
    printf("\nAdditional: Generate routine file\n");
    snprintf(cmd, sizeof(cmd), "./%s -r routines.c test1.gt test2.gt", 
            GENGTYPE_EXECUTABLE);
    printf("Running: %s\n", cmd);
    status = system(cmd);
    printf("Exit status: %d\n", WEXITSTATUS(status));
    
    return 0;
}

/* Clean up temporary files */
void cleanup() {
    int num_files = sizeof(gt_files) / sizeof(gt_files[0]);
    
    for (int i = 0; i < num_files; i++) {
        remove(gt_files[i]);
    }
    
    remove("filelist.txt");
    remove("output0.h");
    remove("output1.h");
    remove("output2.h");
    remove("output3.h");
    remove("combined.h");
    remove("routines.c");
    remove("gengtype.o");
    remove("gengtype-state.o");
    remove(GENGTYPE_EXECUTABLE);
    
    /* Remove coverage data files */
    remove("gengtype.gcda");
    remove("gengtype.gcno");
    remove("gengtype-state.gcda");
    remove("gengtype-state.gcno");
}

/* Main test driver */
int main() {
    printf("=== Gengtype Coverage Test ===\n");
    
    /* Step 1: Build gengtype with coverage */
    if (build_gengtype_with_coverage() != 0) {
        fprintf(stderr, "Failed to build gengtype\n");
        return 1;
    }
    
    /* Step 2: Create test .gt files */
    if (create_gt_files() != 0) {
        fprintf(stderr, "Failed to create .gt files\n");
        cleanup();
        return 1;
    }
    
    /* Step 3: Run gengtype with various patterns */
    if (run_gengtype_patterns() != 0) {
        fprintf(stderr, "Failed to run gengtype patterns\n");
        cleanup();
        return 1;
    }
    
    /* Step 4: Generate coverage report */
    printf("\n=== Generating Coverage Report ===\n");
    system("gcov gengtype.cc");
    
    /* Display relevant coverage information */
    FILE *gcov_output = fopen("gengtype.cc.gcov", "r");
    if (gcov_output) {
        char line[256];
        int in_target_block = 0;
        
        printf("\nCoverage for target switch statement (lines 182-213):\n");
        printf("=====================================================\n");
        
        while (fgets(line, sizeof(line), gcov_output)) {
            if (strstr(line, ":182:")) {
                in_target_block = 1;
            }
            if (in_target_block) {
                printf("%s", line);
                if (strstr(line, ":213:")) {
                    break;
                }
            }
        }
        fclose(gcov_output);
    }
    
    /* Step 5: Cleanup */
    cleanup();
    
    printf("\n=== Test Complete ===\n");
    printf("Check gengtype.cc.gcov for detailed coverage information\n");
    
    return 0;
}
