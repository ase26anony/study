/* driver.c - Test driver for gengtype coverage of type counting switch */
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

/* Generated .gt file contents */
static const char *gt_files[] = {
    /* File 1: Basic type definitions */
    "file1.gt",
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
    "/* TYPE_STRING */\n"
    "struct string_struct {\n"
    "  const char *name;  /* string type */\n"
    "  char *data;\n"
    "};\n"
    "\n"
    "/* TYPE_STRUCT */\n"
    "struct my_struct {\n"
    "  int a;\n"
    "  float b;\n"
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
    "typedef int (*callback_with_arg)(int, char*);\n",

    /* File 2: Advanced and user-defined types */
    "file2.gt",
    "%{\n"
    "/* Test file 2: Advanced types */\n"
    "#include <stdlib.h>\n"
    "%}\n"
    "\n"
    "/* TYPE_USER_STRUCT with user-provided marking */\n"
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
    "/* TYPE_LANG_STRUCT */\n"
    "struct lang_struct {\n"
    "  int data;\n"
    "  void *lang_data;\n"
    "} GTY((lang));\n"
    "\n"
    "/* Complex nested types */\n"
    "struct complex_nested {\n"
    "  union my_union u;          /* TYPE_UNION */\n"
    "  struct my_struct s;        /* TYPE_STRUCT */\n"
    "  int *ptr_array[5];         /* TYPE_ARRAY of TYPE_POINTER */\n"
    "  callback_fn cb;            /* TYPE_CALLBACK */\n"
    "  const char *description;   /* TYPE_STRING */\n"
    "};\n"
    "\n"
    "/* Another undefined type */\n"
    "struct another_undefined;\n"
    "\n"
    "/* Array of pointers to callback */\n"
    "typedef callback_fn callback_array[3];",

    /* File 3: Mixed types with potential errors */
    "file3.gt",
    "%{\n"
    "/* Test file 3: Mixed types for comprehensive coverage */\n"
    "%}\n"
    "\n"
    "/* More scalar types */\n"
    "typedef char byte;\n"
    "typedef short int16;\n"
    "\n"
    "/* String in union */\n"
    "union string_union {\n"
    "  const char *str;\n"
    "  int value;\n"
    "};\n"
    "\n"
    "/* Pointer to array */\n"
    "typedef int (*array_ptr)[10];\n"
    "\n"
    "/* Struct containing all type categories */\n"
    "struct all_types_container {\n"
    "  my_scalar scalar_field;        /* TYPE_SCALAR */\n"
    "  const char *string_field;      /* TYPE_STRING */\n"
    "  struct my_struct struct_field; /* TYPE_STRUCT */\n"
    "  union my_union union_field;    /* TYPE_UNION */\n"
    "  struct_ptr pointer_field;      /* TYPE_POINTER */\n"
    "  my_array array_field;          /* TYPE_ARRAY */\n"
    "  callback_fn callback_field;    /* TYPE_CALLBACK */\n"
    "};\n"
    "\n"
    "/* Multiple language structs */\n"
    "struct lang_struct2 {\n"
    "  long id;\n"
    "  void *ptr;\n"
    "} GTY((lang));",

    /* File 4: File with syntax error (to test error paths) */
    "file4_error.gt",
    "%{\n"
    "/* File with deliberate syntax error - missing closing %}\n"
    "/* This should trigger error handling paths */\n"
    "\n"
    "struct error_struct {\n"
    "  int x;\n"
    "};\n"
    "\n"
    "/* Missing the closing %} */\n",

    /* File 5: Duplicate definitions (to test warning paths) */
    "file5_dup.gt",
    "%{\n"
    "/* File with duplicate definitions */\n"
    "%}\n"
    "\n"
    "struct duplicate_struct {\n"
    "  int a;\n"
    "};\n"
    "\n"
    "/* Duplicate definition */\n"
    "struct duplicate_struct {\n"
    "  int b;\n"
    "};\n"
    "\n"
    "typedef int my_scalar;\n"
    "typedef int my_scalar;  /* Duplicate typedef */"
};

/* Compile gengtype with coverage instrumentation */
static int compile_gengtype(void) {
    pid_t pid;
    int status;
    
    printf("Compiling gengtype with coverage instrumentation...\n");
    
    pid = fork();
    if (pid == 0) {
        /* Child process: compile gengtype */
        execlp("g++", "g++", 
               "-O0",                    /* No optimization */
               "-fprofile-arcs",         /* Coverage instrumentation */
               "-ftest-coverage",        /* Test coverage */
               "-DIN_GCC",               /* Required define */
               "-DHAVE_CONFIG_H",        /* Config header */
               "-I.",                    /* Current directory */
               "-I../../include",        /* GCC includes */
               "-I../../gcc",            /* More GCC includes */
               "-c", "gengtype.cc",      /* Source file */
               "-o", "gengtype_coverage.o", /* Output object */
               NULL);
        perror("execlp failed");
        exit(1);
    } else if (pid > 0) {
        waitpid(pid, &status, 0);
        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            printf("gengtype compilation successful\n");
            return 1;
        }
    }
    
    printf("Warning: Could not compile gengtype with g++\n");
    printf("Trying alternative: using pre-existing gengtype if available\n");
    return 0;
}

/* Create temporary .gt files */
static int create_temp_files(temp_file_t **files, int *count) {
    int num_files = sizeof(gt_files) / (2 * sizeof(char*));
    *files = malloc(num_files * sizeof(temp_file_t));
    if (!*files) return 0;
    
    *count = 0;
    
    for (int i = 0; i < num_files; i++) {
        const char *filename = gt_files[i * 2];
        const char *content = gt_files[i * 2 + 1];
        
        /* Create temporary file */
        char template[] = "/tmp/gt_test_XXXXXX";
        int fd = mkstemp(template);
        if (fd < 0) {
            perror("mkstemp failed");
            continue;
        }
        
        /* Write content */
        size_t len = strlen(content);
        if (write(fd, content, len) != (ssize_t)len) {
            close(fd);
            unlink(template);
            continue;
        }
        close(fd);
        
        /* Store file info */
        (*files)[*count].filename = strdup(template);
        (*files)[*count].content = NULL; /* Not needed after creation */
        (*count)++;
        
        /* Rename to original filename in temp dir for clarity */
        char newname[256];
        snprintf(newname, sizeof(newname), "/tmp/%s", filename);
        rename(template, newname);
        free((*files)[*count-1].filename);
        (*files)[*count-1].filename = strdup(newname);
        
        printf("Created test file: %s\n", newname);
    }
    
    return *count > 0;
}

/* Run gengtype on a single file */
static int run_gengtype_single(const char *filename, const char *output_base) {
    pid_t pid;
    int status;
    char output_header[256];
    char output_routine[256];
    
    snprintf(output_header, sizeof(output_header), "%s.h", output_base);
    snprintf(output_routine, sizeof(output_routine), "%s.c", output_base);
    
    printf("Running gengtype on %s...\n", filename);
    
    pid = fork();
    if (pid == 0) {
        /* Child: execute gengtype */
        execl("./gengtype_coverage", "./gengtype_coverage",
              "-g", output_header,    /* Generate header */
              "-r", output_routine,   /* Generate routine */
              filename,               /* Input file */
              NULL);
        perror("execl failed");
        exit(1);
    } else if (pid > 0) {
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)) {
            printf("gengtype exited with status %d\n", WEXITSTATUS(status));
            return WEXITSTATUS(status);
        }
    }
    
    return -1;
}

/* Run gengtype with file list (-p option) */
static int run_gengtype_batch(temp_file_t *files, int count) {
    pid_t pid;
    int status;
    FILE *listfile;
    char listfilename[] = "/tmp/gt_filelist.txt";
    
    /* Create file list */
    listfile = fopen(listfilename, "w");
    if (!listfile) {
        perror("Failed to create file list");
        return 0;
    }
    
    for (int i = 0; i < count; i++) {
        /* Skip error files for batch processing */
        if (strstr(files[i].filename, "error") || 
            strstr(files[i].filename, "dup")) {
            continue;
        }
        fprintf(listfile, "%s\n", files[i].filename);
    }
    fclose(listfile);
    
    printf("Running gengtype in batch mode with file list...\n");
    
    pid = fork();
    if (pid == 0) {
        /* Child: execute gengtype with -p option */
        execl("./gengtype_coverage", "./gengtype_coverage",
              "-p", listfilename,    /* Process file list */
              NULL);
        perror("execl failed");
        exit(1);
    } else if (pid > 0) {
        waitpid(pid, &status, 0);
        unlink(listfilename);
        if (WIFEXITED(status)) {
            return WEXITSTATUS(status) == 0;
        }
    }
    
    unlink(listfilename);
    return 0;
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

/* Main test driver */
int main(int argc, char *argv[]) {
    temp_file_t *files = NULL;
    int file_count = 0;
    int success = 0;
    
    printf("=== gengtype Type Counting Switch Coverage Test ===\n\n");
    
    /* Step 1: Create test .gt files */
    printf("1. Creating test .gt files...\n");
    if (!create_temp_files(&files, &file_count)) {
        fprintf(stderr, "Failed to create test files\n");
        return 1;
    }
    printf("Created %d test files\n\n", file_count);
    
    /* Step 2: Try to compile gengtype with coverage */
    #if COMPILE_GENGTYPE
    printf("2. Compiling gengtype...\n");
    if (!compile_gengtype()) {
        /* Try to use existing gengtype */
        struct stat st;
        if (stat("gengtype", &st) == 0) {
            printf("Using existing gengtype binary\n");
            system("cp gengtype gengtype_coverage");
        } else {
            fprintf(stderr, "No gengtype available\n");
            cleanup_files(files, file_count);
            return 1;
        }
    } else {
        /* Link the object file */
        pid_t pid = fork();
        if (pid == 0) {
            execlp("g++", "g++",
                   "-O0",
                   "-fprofile-arcs",
                   "-ftest-coverage",
                   "gengtype_coverage.o",
                   "-o", "gengtype_coverage",
                   "-liberty",          /* Link with libiberty */
                   "-lgcov",           /* Coverage library */
                   NULL);
            exit(1);
        } else {
            int status;
            waitpid(pid, &status, 0);
        }
    }
    printf("\n");
    #endif
    
    /* Step 3: Run gengtype on individual files */
    printf("3. Processing files individually...\n");
    for (int i = 0; i < file_count; i++) {
        char output_base[256];
        const char *fname = files[i].filename;
        const char *basename = strrchr(fname, '/');
        basename = basename ? basename + 1 : fname;
        
        snprintf(output_base, sizeof(output_base), "/tmp/output_%s", basename);
        
        int result = run_gengtype_single(fname, output_base);
        
        /* Clean up output files */
        char cmd[512];
        snprintf(cmd, sizeof(cmd), "rm -f /tmp/output_%s.*", basename);
        system(cmd);
        
        /* Note: Error files should fail, that's expected */
        if (strstr(fname, "error") || strstr(fname, "dup")) {
            if (result != 0) {
                printf("  Expected failure for %s: exit code %d\n", basename, result);
                success++;
            }
        } else if (result == 0) {
            printf("  Successfully processed %s\n", basename);
            success++;
        }
    }
    printf("\n");
    
    /* Step 4: Run gengtype in batch mode */
    printf("4. Processing files in batch mode...\n");
    if (run_gengtype_batch(files, file_count)) {
        printf("  Batch processing successful\n");
        success++;
    } else {
        printf("  Batch processing completed (may have warnings)\n");
    }
    printf("\n");
    
    /* Step 5: Additional test with combined output generation */
    printf("5. Testing with combined output generation...\n");
    pid_t pid = fork();
    if (pid == 0) {
        /* Combine first 3 valid files */
        execl("./gengtype_coverage", "./gengtype_coverage",
              "-g", "/tmp/combined.h",
              "-r", "/tmp/combined.c",
              files[0].filename,
              files[1].filename,
              files[2].filename,
              NULL);
        exit(1);
    } else {
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            printf("  Combined output generation successful\n");
            success++;
            system("rm -f /tmp/combined.h /tmp/combined.c");
        }
    }
    printf("\n");
    
    /* Step 6: Cleanup and report */
    printf("6. Cleaning up...\n");
    cleanup_files(files, file_count);
    
    /* Remove coverage binary */
    system("rm -f gengtype_coverage gengtype_coverage.o");
    
    printf("\n=== Test Complete ===\n");
    printf("Successfully executed %d test operations\n", success);
    
    /* Generate coverage report if gcov is available */
    printf("\nGenerating coverage report...\n");
    system("gcov gengtype.cc 2>/dev/null | grep -A 20 'Lines executed:'");
    
    return 0;
}
