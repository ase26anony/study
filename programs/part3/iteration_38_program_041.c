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

/* Generated .gt file contents */
static const char *gt_files[] = {
    /* File 1: Basic types and undefined */
    "%{\n"
    "/* Test file 1: Basic types */\n"
    "#include \"config.h\"\n"
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
    "    const char *name;  /* string type */\n"
    "    char *buffer;\n"
    "};\n"
    "\n"
    "/* TYPE_STRUCT */\n"
    "struct my_struct {\n"
    "    int a;\n"
    "    double b;\n"
    "};\n"
    "\n"
    "/* TYPE_POINTER */\n"
    "typedef struct my_struct *struct_ptr;\n"
    "typedef int *int_ptr;\n"
    "%}\n",
    
    /* File 2: Complex types and user structs */
    "%{\n"
    "/* Test file 2: Complex types */\n"
    "#include \"config.h\"\n"
    "%}\n"
    "\n"
    "/* TYPE_USER_STRUCT with custom marking */\n"
    "struct user_struct {\n"
    "    int *p;\n"
    "    void *data;\n"
    "} GTY((user));\n"
    "\n"
    "/* TYPE_UNION */\n"
    "union my_union {\n"
    "    int i;\n"
    "    void *p;\n"
    "    double d;\n"
    "};\n"
    "\n"
    "/* TYPE_ARRAY */\n"
    "typedef int my_array[10];\n"
    "typedef struct my_struct struct_array[5];\n"
    "\n"
    "/* Nested complex type */\n"
    "struct complex_nested {\n"
    "    union my_union *ptr_to_union;  /* TYPE_POINTER to TYPE_UNION */\n"
    "    my_array arrays[3];           /* Array of arrays */\n"
    "    struct user_struct *user;     /* Pointer to user struct */\n"
    "};\n"
    "%}\n",
    
    /* File 3: Callbacks, lang structs, and edge cases */
    "%{\n"
    "/* Test file 3: Advanced types */\n"
    "#include \"config.h\"\n"
    "%}\n"
    "\n"
    "/* TYPE_CALLBACK */\n"
    "typedef void (*callback_fn)(void);\n"
    "typedef int (*compare_fn)(const void *, const void *);\n"
    "\n"
    "/* TYPE_LANG_STRUCT */\n"
    "struct lang_struct {\n"
    "    int data;\n"
    "    void *lang_data;\n"
    "} GTY ((lang));\n"
    "\n"
    "/* More TYPE_STRING variations */\n"
    "struct more_strings {\n"
    "    const char *const_string;\n"
    "    char *modifiable_string;\n"
    "};\n"
    "\n"
    "/* Combined types */\n"
    "struct all_in_one {\n"
    "    callback_fn handler;          /* TYPE_CALLBACK */\n"
    "    struct lang_struct *lang_ptr; /* TYPE_POINTER to TYPE_LANG_STRUCT */\n"
    "    union my_union variant;       /* TYPE_UNION */\n"
    "    int numbers[20];              /* TYPE_ARRAY */\n"
    "};\n"
    "\n"
    "/* Duplicate definition to test warnings */\n"
    "struct my_struct {\n"
    "    int a;\n"
    "    double b;\n"
    "};\n"
    "%}\n",
    
    /* File 4: With syntax error to test error paths */
    "%{\n"
    "/* Test file 4: Intentionally malformed */\n"
    "#include \"config.h\"\n"
    "/* Missing closing %} to trigger error */\n"
    "\n"
    "struct error_test {\n"
    "    int x;\n"
    "};\n"
    "/* No closing tag */\n"
};

/* Compilation command for gengtype with coverage */
static const char *gengtype_compile_cmd =
    "g++ -O0 -fprofile-arcs -ftest-coverage "
    "-DIN_GCC -DHAVE_CONFIG_H "
    "-I. -I../../include -I../../gcc "
    "-c gengtype.cc -o gengtype.o 2>&1";

static const char *gengtype_link_cmd =
    "g++ -O0 -fprofile-arcs -ftest-coverage "
    "gengtype.o gengtype-state.o gengtype-lex.o "
    "-lgcov -liberty -o gengtype_coverage 2>&1";

/* Create temporary file with given content */
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

/* Execute command and capture output */
static int execute_command(const char *cmd, char *output, size_t output_size) {
    FILE *fp = popen(cmd, "r");
    if (!fp) {
        return -1;
    }
    
    if (output) {
        output[0] = '\0';
        while (fgets(output + strlen(output), 
                    output_size - strlen(output), fp) != NULL) {
            /* Continue reading */
        }
    }
    
    int status = pclose(fp);
    return WEXITSTATUS(status);
}

/* Run gengtype with various patterns to trigger type counting */
static int run_gengtype_tests(const char *gengtype_exe, char **temp_files, int file_count) {
    int ret = 0;
    char cmd[1024];
    char output[4096];
    
    printf("=== Running gengtype tests ===\n");
    
    /* Pattern A: Process each file individually */
    printf("\n--- Pattern A: Individual file processing ---\n");
    for (int i = 0; i < file_count; i++) {
        snprintf(cmd, sizeof(cmd), "%s -g output%d.h %s 2>&1", 
                gengtype_exe, i, temp_files[i]);
        printf("Running: %s\n", cmd);
        if (execute_command(cmd, output, sizeof(output)) != 0) {
            printf("Output: %s\n", output);
            if (i != file_count - 1) { /* Last file has syntax error */
                ret = 1;
            }
        }
    }
    
    /* Pattern B: Batch processing with -p flag */
    printf("\n--- Pattern B: Batch processing with -p ---\n");
    char *filelist = create_temp_file("", ".list");
    if (filelist) {
        FILE *f = fopen(filelist, "w");
        if (f) {
            for (int i = 0; i < file_count - 1; i++) { /* Skip error file */
                fprintf(f, "%s\n", temp_files[i]);
            }
            fclose(f);
            
            snprintf(cmd, sizeof(cmd), "%s -p %s 2>&1", gengtype_exe, filelist);
            printf("Running: %s\n", cmd);
            execute_command(cmd, output, sizeof(output));
            printf("Output: %s\n", output);
            
            unlink(filelist);
            free(filelist);
        }
    }
    
    /* Pattern C: Generate both header and routine files */
    printf("\n--- Pattern C: Full generation (-g and -r) ---\n");
    snprintf(cmd, sizeof(cmd), "%s -g combined.h -r combined.c %s %s 2>&1",
            gengtype_exe, temp_files[0], temp_files[1]);
    printf("Running: %s\n", cmd);
    execute_command(cmd, output, sizeof(output));
    printf("Output: %s\n", output);
    
    /* Pattern D: Test with debug flag for more verbose output */
    printf("\n--- Pattern D: With debug output ---\n");
    snprintf(cmd, sizeof(cmd), "%s -g debug.h -d %s 2>&1",
            gengtype_exe, temp_files[0]);
    printf("Running: %s\n", cmd);
    execute_command(cmd, output, sizeof(output));
    printf("Output (first 500 chars): %.500s\n", output);
    
    return ret;
}

/* Add debug prints to gengtype source (simulated) */
static void patch_gengtype_for_debug(void) {
    /* In a real scenario, we would patch the source file.
       For this test, we'll rely on coverage instrumentation. */
    printf("Note: To add debug prints, modify gengtype.cc around line 180:\n");
    printf("  Add: fprintf(stderr, \"Counting type: %%d\\n\", (int)t);\n");
}

/* Main test driver */
int main(int argc, char **argv) {
    int ret = 0;
    char **temp_files = NULL;
    int file_count = sizeof(gt_files) / sizeof(gt_files[0]);
    
    printf("=== GCC gengtype Coverage Test ===\n");
    printf("Testing switch statement for type counting (lines 182-213)\n\n");
    
    /* Step 1: Create temporary .gt files */
    printf("--- Creating test .gt files ---\n");
    temp_files = malloc(file_count * sizeof(char *));
    if (!temp_files) {
        perror("malloc");
        return 1;
    }
    
    for (int i = 0; i < file_count; i++) {
        const char *suffix = (i == file_count - 1) ? ".gt" : ".gt";
        temp_files[i] = create_temp_file(gt_files[i], suffix);
        if (!temp_files[i]) {
            fprintf(stderr, "Failed to create temp file %d\n", i);
            ret = 1;
            goto cleanup;
        }
        printf("Created: %s (%zu bytes)\n", temp_files[i], strlen(gt_files[i]));
    }
    
    /* Step 2: Compile gengtype with coverage instrumentation */
    printf("\n--- Compiling gengtype with coverage ---\n");
    char compile_output[4096];
    
    printf("Compiling...\n");
    if (execute_command(gengtype_compile_cmd, compile_output, sizeof(compile_output)) != 0) {
        fprintf(stderr, "Compilation failed:\n%s\n", compile_output);
        printf("Trying alternative compilation...\n");
        
        /* Try simpler compilation if the first fails */
        const char *simple_compile = 
            "g++ -O0 -fprofile-arcs -ftest-coverage "
            "-DIN_GCC "
            "-I. "
            "-c gengtype.cc -o gengtype.o 2>&1";
        if (execute_command(simple_compile, compile_output, sizeof(compile_output)) != 0) {
            fprintf(stderr, "Alternative compilation also failed:\n%s\n", compile_output);
            ret = 1;
            goto cleanup;
        }
    }
    
    printf("Linking...\n");
    if (execute_command(gengtype_link_cmd, compile_output, sizeof(compile_output)) != 0) {
        fprintf(stderr, "Linking failed:\n%s\n", compile_output);
        printf("Trying alternative linking...\n");
        
        /* Try simpler linking */
        const char *simple_link = 
            "g++ -O0 -fprofile-arcs -ftest-coverage "
            "gengtype.o -lgcov -liberty -o gengtype_coverage 2>&1";
        if (execute_command(simple_link, compile_output, sizeof(compile_output)) != 0) {
            fprintf(stderr, "Alternative linking also failed:\n%s\n", compile_output);
            ret = 1;
            goto cleanup;
        }
    }
    
    /* Step 3: Run tests with various patterns */
    if (access("gengtype_coverage", X_OK) == 0) {
        patch_gengtype_for_debug();
        ret = run_gengtype_tests("./gengtype_coverage", temp_files, file_count);
        
        /* Step 4: Generate coverage report */
        printf("\n--- Generating coverage report ---\n");
        if (access("gengtype.gcda", F_OK) == 0) {
            printf("Coverage data generated: gengtype.gcda\n");
            printf("To view coverage: gcov gengtype.cc\n");
            
            /* Run gcov to show coverage */
            system("gcov gengtype.cc 2>&1 | grep -A 20 'Lines executed:'");
        } else {
            printf("Warning: No coverage data generated\n");
        }
    } else {
        fprintf(stderr, "gengtype_coverage executable not found\n");
        ret = 1;
    }
    
cleanup:
    /* Cleanup temporary files */
    printf("\n--- Cleaning up ---\n");
    if (temp_files) {
        for (int i = 0; i < file_count; i++) {
            if (temp_files[i]) {
                unlink(temp_files[i]);
                free(temp_files[i]);
            }
        }
        free(temp_files);
    }
    
    /* Cleanup generated files */
    unlink("gengtype_coverage");
    unlink("gengtype.o");
    unlink("gengtype.gcno");
    unlink("gengtype.gcda");
    for (int i = 0; i < 10; i++) {
        char fname[32];
        snprintf(fname, sizeof(fname), "output%d.h", i);
        unlink(fname);
    }
    unlink("combined.h");
    unlink("combined.c");
    unlink("debug.h");
    
    printf("\n=== Test %s ===\n", ret == 0 ? "PASSED" : "FAILED");
    return ret;
}
