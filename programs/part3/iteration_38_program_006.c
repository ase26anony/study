/* test_gengtype_coverage.c - Driver program to test gengtype type counting */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>

/* Compile gengtype with coverage instrumentation */
#define COMPILE_GENGTYPE 1

/* Temporary file management */
typedef struct {
    char *filename;
    char *content;
} temp_file_t;

/* Generated .gt files covering all type categories */
static const char *gt_files[] = {
    /* File 1: Basic types and undefined */
    "file1.gt",
    "%{\n"
    "#include \"config.h\"\n"
    "#include \"system.h\"\n"
    "%}\n"
    "\n"
    /* TYPE_UNDEFINED - forward declaration */
    "struct undefined_struct;\n"
    "\n"
    /* TYPE_SCALAR - scalar typedefs */
    "typedef int my_scalar;\n"
    "typedef unsigned long scalar2;\n"
    "\n"
    /* TYPE_STRING - string type usage */
    "struct string_struct {\n"
    "  const char *name;  /* TYPE_STRING */\n"
    "  char *data;\n"
    "};\n"
    "\n"
    /* TYPE_STRUCT - regular struct */
    "struct my_struct {\n"
    "  int a;\n"
    "  float b;\n"
    "};\n"
    "\n"
    /* TYPE_POINTER - pointer typedef */
    "typedef struct my_struct *my_ptr;\n"
    "%}",

    /* File 2: User structs, unions, and arrays */
    "file2.gt",
    "%{\n"
    "#include \"config.h\"\n"
    "#include \"system.h\"\n"
    "%}\n"
    "\n"
    /* TYPE_USER_STRUCT - struct with user marking */
    "struct user_struct {\n"
    "  int *p;\n"
    "  void *data;\n"
    "} GTY((user));\n"
    "\n"
    /* TYPE_UNION - union definition */
    "union my_union {\n"
    "  int i;\n"
    "  void *p;\n"
    "  double d;\n"
    "};\n"
    "\n"
    /* TYPE_ARRAY - array typedefs */
    "typedef int my_array[10];\n"
    "typedef struct my_struct struct_array[5];\n"
    "\n"
    /* TYPE_CALLBACK - callback function pointer */
    "typedef void (*callback_fn)(void);\n"
    "typedef int (*callback_with_args)(int, char*);\n"
    "\n"
    /* Complex nested type combining multiple categories */
    "struct complex_nested {\n"
    "  union my_union *uptr;      /* pointer to union */\n"
    "  my_array arr;              /* array type */\n"
    "  callback_fn handler;       /* callback */\n"
    "  struct user_struct *user;  /* pointer to user struct */\n"
    "};\n"
    "%}",

    /* File 3: Language struct and more complex types */
    "file3.gt",
    "%{\n"
    "#include \"config.h\"\n"
    "#include \"system.h\"\n"
    "%}\n"
    "\n"
    /* TYPE_LANG_STRUCT - language-specific struct */
    "struct lang_struct {\n"
    "  int data;\n"
    "  void *lang_data;\n"
    "} GTY ((lang));\n"
    "\n"
    /* More pointer variations */
    "typedef union my_union **double_ptr;\n"
    "typedef int (*func_ptr_array[5])(void);\n"
    "\n"
    /* Struct with all type kinds */
    "struct kitchen_sink {\n"
    "  my_scalar scalar_field;          /* TYPE_SCALAR */\n"
    "  const char *string_field;        /* TYPE_STRING */\n"
    "  struct my_struct regular;        /* TYPE_STRUCT */\n"
    "  struct user_struct *user_ptr;    /* TYPE_USER_STRUCT via pointer */\n"
    "  union my_union union_field;      /* TYPE_UNION */\n"
    "  int *int_ptr;                    /* TYPE_POINTER */\n"
    "  int array_field[20];             /* TYPE_ARRAY */\n"
    "  callback_fn cb;                  /* TYPE_CALLBACK */\n"
    "  struct lang_struct lang;         /* TYPE_LANG_STRUCT */\n"
    "};\n"
    "\n"
    /* Forward declaration for another undefined */
    "struct another_undefined;\n"
    "%}",

    /* File 4: With syntax error to test error paths */
    "file4.gt",
    "%{\n"
    "#include \"config.h\"\n"
    "#include \"system.h\"\n"
    "/* Missing closing %} to trigger error */\n"
    "\n"
    "struct error_struct {\n"
    "  int x;\n"
    "};\n"
    /* Deliberately missing %} */,

    /* File 5: Duplicate definitions for warning testing */
    "file5.gt",
    "%{\n"
    "#include \"config.h\"\n"
    "#include \"system.h\"\n"
    "%}\n"
    "\n"
    "struct duplicate_struct {\n"
    "  int a;\n"
    "};\n"
    "\n"
    /* Duplicate definition */
    "struct duplicate_struct {\n"
    "  int b;\n"
    "};\n"
    "%}",
    
    NULL
};

/* Create temporary file with given content */
static char *create_temp_file(const char *filename, const char *content) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        perror("fopen");
        return NULL;
    }
    fwrite(content, 1, strlen(content), f);
    fclose(f);
    return strdup(filename);
}

/* Compile gengtype with coverage instrumentation */
static int compile_gengtype(void) {
    const char *source_files[] = {
        "gengtype.cc",
        "gengtype-state.cc",
        "gengtype-lex.cc",
        "gengtype-parse.cc",
        NULL
    };
    
    const char *compile_cmd = 
        "g++ -O0 -fprofile-arcs -ftest-coverage "
        "-DIN_GCC -DHAVE_CONFIG_H "
        "-I. -I../../include -I../../gcc "
        "-c gengtype.cc gengtype-state.cc gengtype-lex.cc gengtype-parse.cc "
        "&& g++ -O0 -fprofile-arcs -ftest-coverage "
        "-o gengtype_coverage gengtype.o gengtype-state.o gengtype-lex.o gengtype-parse.o "
        "-liberty -lgcov";
    
    printf("Compiling gengtype with coverage...\n");
    return system(compile_cmd);
}

/* Run gengtype with various patterns to trigger type counting */
static int run_gengtype_patterns(const char **temp_files, int file_count) {
    int status;
    pid_t pid;
    
    /* Pattern A: Process each file individually */
    printf("\n=== Pattern A: Individual file processing ===\n");
    for (int i = 0; i < file_count; i++) {
        char cmd[1024];
        snprintf(cmd, sizeof(cmd), 
                 "./gengtype_coverage -g output%d.h %s 2>&1",
                 i, temp_files[i]);
        
        printf("Running: %s\n", cmd);
        status = system(cmd);
        if (status != 0) {
            printf("Command failed with status %d (expected for error cases)\n", 
                   WEXITSTATUS(status));
        }
    }
    
    /* Pattern B: Batch processing with -p flag */
    printf("\n=== Pattern B: Batch processing with -p ===\n");
    FILE *filelist = fopen("gt_filelist.txt", "w");
    if (filelist) {
        for (int i = 0; i < file_count; i++) {
            fprintf(filelist, "%s\n", temp_files[i]);
        }
        fclose(filelist);
        
        status = system("./gengtype_coverage -p gt_filelist.txt -g batch_output.h 2>&1");
        printf("Batch processing completed with status %d\n", WEXITSTATUS(status));
    }
    
    /* Pattern C: Multiple files in one command */
    printf("\n=== Pattern C: Multiple files in one command ===\n");
    char cmd[2048] = "./gengtype_coverage -g combined_output.h ";
    for (int i = 0; i < file_count && i < 3; i++) { /* Use first 3 valid files */
        strcat(cmd, temp_files[i]);
        strcat(cmd, " ");
    }
    strcat(cmd, "2>&1");
    
    printf("Running: %s\n", cmd);
    status = system(cmd);
    printf("Multi-file processing completed with status %d\n", WEXITSTATUS(status));
    
    /* Pattern D: Generate routines file */
    printf("\n=== Pattern D: Routine generation ===\n");
    status = system("./gengtype_coverage -r gtype-desc.cc -g gtype-desc.h file1.gt file2.gt 2>&1");
    printf("Routine generation completed with status %d\n", WEXITSTATUS(status));
    
    return 0;
}

/* Add debug prints to gengtype source to verify execution */
static void patch_gengtype_for_debug(void) {
    /* This would patch the actual gengtype.cc source file */
    /* For now, we'll rely on coverage instrumentation */
    printf("Note: To add debug prints, patch gengtype.cc around line 180-220\n");
    printf("to add fprintf(stderr, \"Counting type: %%d\\n\", token);\n");
}

/* Main test driver */
int main(int argc, char **argv) {
    int file_count = 0;
    char *temp_files[10];
    
    printf("=== GCC gengtype Type Counting Coverage Test ===\n");
    
    /* Create temporary .gt files */
    printf("\nCreating test .gt files...\n");
    for (int i = 0; gt_files[i] != NULL; i += 2) {
        temp_files[file_count] = create_temp_file(
            gt_files[i], 
            gt_files[i + 1]
        );
        if (temp_files[file_count]) {
            printf("Created: %s\n", temp_files[file_count]);
            file_count++;
        }
    }
    
    if (file_count == 0) {
        fprintf(stderr, "Failed to create test files\n");
        return 1;
    }
    
#if COMPILE_GENGTYPE
    /* Compile gengtype with coverage */
    if (compile_gengtype() != 0) {
        fprintf(stderr, "Failed to compile gengtype\n");
        
        /* Cleanup */
        for (int i = 0; i < file_count; i++) {
            unlink(temp_files[i]);
            free(temp_files[i]);
        }
        return 1;
    }
#endif
    
    /* Optional: Patch gengtype source for debug output */
    if (argc > 1 && strcmp(argv[1], "--debug") == 0) {
        patch_gengtype_for_debug();
    }
    
    /* Run gengtype with various patterns */
    run_gengtype_patterns((const char **)temp_files, file_count);
    
    /* Generate coverage report */
    printf("\n=== Generating Coverage Report ===\n");
    system("gcov gengtype.cc gengtype-state.cc 2>&1 | grep -A 20 'gengtype.cc'");
    
    /* Check specific switch coverage */
    printf("\n=== Checking Switch Statement Coverage ===\n");
    system("grep -n 'nb_.*++' gengtype.cc.gcov | head -20");
    
    /* Cleanup */
    printf("\n=== Cleaning up ===\n");
    for (int i = 0; i < file_count; i++) {
        unlink(temp_files[i]);
        free(temp_files[i]);
    }
    
    unlink("gt_filelist.txt");
    unlink("gengtype_coverage");
    
    /* Remove generated output files */
    for (int i = 0; i < 10; i++) {
        char fname[50];
        snprintf(fname, sizeof(fname), "output%d.h", i);
        unlink(fname);
    }
    unlink("batch_output.h");
    unlink("combined_output.h");
    unlink("gtype-desc.h");
    unlink("gtype-desc.cc");
    
    /* Remove coverage data files */
    unlink("*.gcda");
    unlink("*.gcno");
    unlink("*.gcov");
    
    printf("\n=== Test completed ===\n");
    printf("Check gengtype.cc.gcov for line-by-line coverage\n");
    printf("Lines 182-213 should show execution counts > 0\n");
    
    return 0;
}
