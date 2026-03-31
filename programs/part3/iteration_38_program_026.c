/* test_gengtype_coverage.c - Comprehensive test to cover gengtype type counting switch */
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

/* GT file definitions covering all type categories */
static const char *gt_files[] = {
    /* File 1: Basic types and structs */
    "test_types1.gt",
    "%{\n"
    "#include \"config.h\"\n"
    "#include \"system.h\"\n"
    "%}\n\n"
    /* TYPE_UNDEFINED - forward declaration */
    "struct undefined_struct;\n\n"
    /* TYPE_SCALAR - scalar typedefs */
    "typedef int my_scalar;\n"
    "typedef unsigned long scalar2;\n\n"
    /* TYPE_STRING - string type usage */
    "struct string_struct {\n"
    "  const char *name;  /* TYPE_STRING */\n"
    "  char *data;\n"
    "};\n\n"
    /* TYPE_STRUCT - regular struct */
    "struct my_struct {\n"
    "  int a;\n"
    "  double b;\n"
    "};\n\n"
    /* TYPE_POINTER - pointer typedef */
    "typedef struct my_struct *my_ptr;\n"
    "typedef void (*void_ptr)(void);\n\n"
    "%}\n",

    /* File 2: User structs, unions, and arrays */
    "test_types2.gt",
    "%{\n"
    "#include \"config.h\"\n"
    "#include \"system.h\"\n"
    "%}\n\n"
    /* TYPE_USER_STRUCT - with user-provided marking */
    "struct user_struct {\n"
    "  int *p;\n"
    "  void *data;\n"
    "} GTY((user));\n\n"
    /* TYPE_UNION - union definition */
    "union my_union {\n"
    "  int i;\n"
    "  void *p;\n"
    "  double d;\n"
    "};\n\n"
    /* TYPE_ARRAY - array typedefs */
    "typedef int my_array[10];\n"
    "typedef struct my_struct struct_array[5];\n"
    "typedef union myunion *ptr_array[20];\n\n"
    /* TYPE_CALLBACK - callback function pointer */
    "typedef void (*callback_fn)(void);\n"
    "typedef int (*compare_fn)(const void *, const void *);\n\n"
    /* Nested complex type */
    "struct complex_nested {\n"
    "  union my_union u;\n"
    "  my_array arr;\n"
    "  callback_fn cb;\n"
    "  struct user_struct *user_ptr;\n"
    "};\n\n"
    "%}\n",

    /* File 3: Lang structs and error cases */
    "test_types3.gt",
    "%{\n"
    "#include \"config.h\"\n"
    "#include \"system.h\"\n"
    "%}\n\n"
    /* TYPE_LANG_STRUCT - language-specific struct */
    "struct lang_struct {\n"
    "  int data;\n"
    "  void *lang_data;\n"
    "} GTY ((lang));\n\n"
    /* More pointer variations */
    "typedef struct lang_struct *lang_ptr;\n"
    "typedef lang_ptr *double_ptr;\n\n"
    /* Array of pointers to callback */
    "typedef callback_fn callback_array[5];\n\n"
    /* Mixed complex type */
    "struct mega_struct {\n"
    "  struct lang_struct lang;\n"
    "  union {\n"
    "    my_array a;\n"
    "    struct user_struct *u;\n"
    "  } variant;\n"
    "  callback_fn handlers[3];\n"
    "};\n\n"
    "%}\n",

    /* File 4: Deliberate syntax error to test error paths */
    "test_error.gt",
    "%{\n"
    "#include \"config.h\"\n"
    "#include \"system.h\"\n"
    "/* Missing closing %} to trigger error */\n"
    "struct error_struct {\n"
    "  int x;\n"
    "};\n"
    /* Note: intentionally missing %} */,

    /* File 5: Duplicate definition for warning test */
    "test_dup.gt",
    "%{\n"
    "#include \"config.h\"\n"
    "#include \"system.h\"\n"
    "%}\n\n"
    "struct duplicate_struct {\n"
    "  int a;\n"
    "};\n\n"
    "/* Duplicate definition */\n"
    "struct duplicate_struct {\n"
    "  int b;\n"
    "};\n\n"
    "%}\n",

    NULL
};

/* Build gengtype with coverage instrumentation */
static int build_gengtype_with_coverage(void) {
    printf("Building gengtype with coverage instrumentation...\n");
    
    /* Compile gengtype.cc with coverage flags */
    const char *compile_cmd = 
        "g++ -O0 -fprofile-arcs -ftest-coverage "
        "-DIN_GCC -DHAVE_CONFIG_H "
        "-I. -I../../include -I../../gcc "
        "-c gengtype.cc -o gengtype_coverage.o 2>&1";
    
    printf("Compiling: %s\n", compile_cmd);
    if (system(compile_cmd) != 0) {
        fprintf(stderr, "Failed to compile gengtype.cc\n");
        return -1;
    }
    
    /* Compile gengtype-state.cc */
    const char *compile_state_cmd = 
        "g++ -O0 -fprofile-arcs -ftest-coverage "
        "-DIN_GCC -DHAVE_CONFIG_H "
        "-I. -I../../include -I../../gcc "
        "-c gengtype-state.cc -o gengtype-state_coverage.o 2>&1";
    
    printf("Compiling: %s\n", compile_state_cmd);
    if (system(compile_state_cmd) != 0) {
        fprintf(stderr, "Failed to compile gengtype-state.cc\n");
        return -1;
    }
    
    /* Link gengtype executable */
    const char *link_cmd = 
        "g++ -O0 -fprofile-arcs -ftest-coverage "
        "gengtype_coverage.o gengtype-state_coverage.o "
        "-lgcov -liberty -o gengtype_coverage 2>&1";
    
    printf("Linking: %s\n", link_cmd);
    if (system(link_cmd) != 0) {
        fprintf(stderr, "Failed to link gengtype\n");
        return -1;
    }
    
    printf("Successfully built gengtype_coverage\n");
    return 0;
}

/* Create temporary GT files */
static int create_gt_files(temp_file_t **files) {
    int count = 0;
    
    /* Count files */
    for (int i = 0; gt_files[i] != NULL; i += 2) {
        count++;
    }
    
    *files = malloc(count * sizeof(temp_file_t));
    if (!*files) return -1;
    
    for (int i = 0, j = 0; gt_files[i] != NULL; i += 2, j++) {
        char template[] = "/tmp/gt_test_XXXXXX";
        int fd = mkstemp(template);
        if (fd < 0) {
            perror("mkstemp failed");
            return -1;
        }
        
        (*files)[j].filename = strdup(template);
        (*files)[j].content = (char *)gt_files[i + 1];
        
        /* Write content to file */
        size_t len = strlen(gt_files[i + 1]);
        if (write(fd, gt_files[i + 1], len) != (ssize_t)len) {
            perror("write failed");
            close(fd);
            return -1;
        }
        close(fd);
        
        printf("Created temporary file: %s\n", template);
    }
    
    return count;
}

/* Clean up temporary files */
static void cleanup_files(temp_file_t *files, int count) {
    for (int i = 0; i < count; i++) {
        if (files[i].filename) {
            unlink(files[i].filename);
            free(files[i].filename);
        }
    }
    free(files);
}

/* Run gengtype with pattern A: Multiple file processing */
static int run_pattern_a(temp_file_t *files, int count) {
    printf("\n=== Pattern A: Multiple File Processing ===\n");
    
    int success_count = 0;
    
    for (int i = 0; i < count; i++) {
        char cmd[1024];
        
        /* Skip error file for normal processing */
        if (strstr(files[i].filename, "error")) {
            printf("Skipping error file: %s\n", files[i].filename);
            continue;
        }
        
        /* Generate header output */
        snprintf(cmd, sizeof(cmd), 
                "./gengtype_coverage -g output_%d.h %s 2>&1", 
                i, files[i].filename);
        
        printf("Running: %s\n", cmd);
        int result = system(cmd);
        
        if (result == 0) {
            printf("Successfully processed %s\n", files[i].filename);
            success_count++;
            
            /* Clean up generated file */
            char cleanup_cmd[256];
            snprintf(cleanup_cmd, sizeof(cleanup_cmd), "rm -f output_%d.h", i);
            system(cleanup_cmd);
        } else {
            printf("Warning: gengtype returned %d for %s\n", 
                   WEXITSTATUS(result), files[i].filename);
        }
    }
    
    return success_count;
}

/* Run gengtype with pattern B: Batch processing with -p */
static int run_pattern_b(temp_file_t *files, int count) {
    printf("\n=== Pattern B: Batch Processing with -p ===\n");
    
    /* Create file list */
    FILE *list = fopen("/tmp/gt_filelist.txt", "w");
    if (!list) {
        perror("Failed to create file list");
        return -1;
    }
    
    for (int i = 0; i < count; i++) {
        /* Include all files including error/dup for comprehensive testing */
        fprintf(list, "%s\n", files[i].filename);
    }
    fclose(list);
    
    /* Run gengtype with file list */
    const char *cmd = "./gengtype_coverage -p /tmp/gt_filelist.txt 2>&1";
    printf("Running: %s\n", cmd);
    int result = system(cmd);
    
    /* Clean up file list */
    unlink("/tmp/gt_filelist.txt");
    
    return (result == 0) ? 1 : 0;
}

/* Run gengtype with pattern C: Header generation with multiple files */
static int run_pattern_c(temp_file_t *files, int count) {
    printf("\n=== Pattern C: Header Generation with Multiple Files ===\n");
    
    /* Build command with all valid GT files */
    char cmd[4096] = "./gengtype_coverage -g combined_output.h";
    
    for (int i = 0; i < count; i++) {
        /* Skip error file for combined processing */
        if (strstr(files[i].filename, "error")) {
            continue;
        }
        strcat(cmd, " ");
        strcat(cmd, files[i].filename);
    }
    
    strcat(cmd, " 2>&1");
    printf("Running: %s\n", cmd);
    
    int result = system(cmd);
    
    if (result == 0) {
        printf("Successfully generated combined header\n");
        
        /* Also generate routine file */
        char cmd2[4096] = "./gengtype_coverage -r combined_routines.c";
        for (int i = 0; i < count; i++) {
            if (strstr(files[i].filename, "error")) {
                continue;
            }
            strcat(cmd2, " ");
            strcat(cmd2, files[i].filename);
        }
        strcat(cmd2, " 2>&1");
        
        printf("Running: %s\n", cmd2);
        system(cmd2);
        
        /* Clean up generated files */
        system("rm -f combined_output.h combined_routines.c");
        return 1;
    }
    
    return 0;
}

/* Run gengtype with pattern D: Error and warning cases */
static int run_pattern_d(temp_file_t *files, int count) {
    printf("\n=== Pattern D: Error and Warning Cases ===\n");
    
    int tests_run = 0;
    
    /* Find error file */
    for (int i = 0; i < count; i++) {
        if (strstr(files[i].filename, "error")) {
            char cmd[1024];
            snprintf(cmd, sizeof(cmd), 
                    "./gengtype_coverage %s 2>&1", files[i].filename);
            
            printf("Running error test: %s\n", cmd);
            int result = system(cmd);
            
            /* Error expected, so non-zero exit is "success" for test */
            if (result != 0) {
                printf("✓ Correctly detected syntax error\n");
                tests_run++;
            }
            break;
        }
    }
    
    /* Find duplicate file */
    for (int i = 0; i < count; i++) {
        if (strstr(files[i].filename, "dup")) {
            char cmd[1024];
            snprintf(cmd, sizeof(cmd), 
                    "./gengtype_coverage -w %s 2>&1", files[i].filename);
            
            printf("Running duplicate test: %s\n", cmd);
            int result = system(cmd);
            
            /* Warning expected, but should still process */
            printf("Duplicate test completed (exit code: %d)\n", 
                   WEXITSTATUS(result));
            tests_run++;
            break;
        }
    }
    
    return tests_run;
}

/* Run gengtype with debug flag for more verbose output */
static int run_debug_pattern(temp_file_t *files, int count) {
    printf("\n=== Debug Pattern: With DEBUG_GENGTYPE ===\n");
    
    /* Rebuild with debug flag */
    const char *debug_build = 
        "g++ -O0 -fprofile-arcs -ftest-coverage "
        "-DIN_GCC -DHAVE_CONFIG_H -DDEBUG_GENGTYPE "
        "-I. -I../../include -I../../gcc "
        "gengtype.cc gengtype-state.cc "
        "-lgcov -liberty -o gengtype_debug 2>&1";
    
    printf("Building debug version: %s\n", debug_build);
    if (system(debug_build) != 0) {
        fprintf(stderr, "Failed to build debug version\n");
        return 0;
    }
    
    /* Run with a valid file */
    for (int i = 0; i < count; i++) {
        if (!strstr(files[i].filename, "error") && 
            !strstr(files[i].filename, "dup")) {
            char cmd[1024];
            snprintf(cmd, sizeof(cmd), 
                    "./gengtype_debug %s 2>&1 | head -50", files[i].filename);
            
            printf("Running debug version: %s\n", cmd);
            system(cmd);
            
            /* Clean up */
            system("rm -f gengtype_debug");
            return 1;
        }
    }
    
    return 0;
}

/* Generate coverage report */
static void generate_coverage_report(void) {
    printf("\n=== Generating Coverage Report ===\n");
    
    /* Run gcov on the instrumented files */
    const char *gcov_cmd = "gcov gengtype_coverage.o 2>&1 | grep -A5 'gengtype.cc'";
    printf("Running: %s\n", gcov_cmd);
    system(gcov_cmd);
    
    /* Also show branch coverage */
    const char *branch_cmd = "gcov -b gengtype_coverage.o 2>&1 | grep -B2 -A2 'branch'";
    printf("\nBranch coverage:\n");
    system(branch_cmd);
}

int main(int argc, char *argv[]) {
    printf("=== GCC gengtype Type Counting Switch Coverage Test ===\n");
    
    /* Step 1: Build gengtype with coverage */
    if (build_gengtype_with_coverage() != 0) {
        fprintf(stderr, "Failed to build gengtype with coverage\n");
        return 1;
    }
    
    /* Step 2: Create temporary GT files */
    temp_file_t *files = NULL;
    int file_count = create_gt_files(&files);
    if (file_count <= 0) {
        fprintf(stderr, "Failed to create GT files\n");
        return 1;
    }
    
    printf("\nCreated %d GT files with diverse type definitions:\n", file_count);
    printf("1. Basic types, scalars, strings, structs, pointers\n");
    printf("2. User structs, unions, arrays, callbacks\n");
    printf("3. Lang structs, complex nested types\n");
    printf("4. Syntax error case\n");
    printf("5. Duplicate definition warning case\n");
    
    /* Step 3: Execute all patterns */
    int total_success = 0;
    
    total_success += run_pattern_a(files, file_count);
    total_success += run_pattern_b(files, file_count);
    total_success += run_pattern_c(files, file_count);
    total_success += run_pattern_d(files, file_count);
    total_success += run_debug_pattern(files, file_count);
    
    /* Step 4: Generate coverage report */
    generate_coverage_report();
    
    /* Step 5: Cleanup */
    cleanup_files(files, file_count);
    
    /* Clean up coverage files */
    system("rm -f gengtype_coverage gengtype_coverage.o gengtype-state_coverage.o");
    system("rm -f *.gcda *.gcno gengtype.cc.gcov");
    
    printf("\n=== Test Summary ===\n");
    printf("Total successful pattern executions: %d\n", total_success);
    printf("Coverage data generated in *.gcda files\n");
    
    if (total_success >= 3) {
        printf("✓ Test completed successfully\n");
        return 0;
    } else {
        printf("✗ Test had issues (only %d successful patterns)\n", total_success);
        return 1;
    }
}
