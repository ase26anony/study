/* driver.c - Test program to exercise gengtype type counting logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

/* Temporary file management */
#define MAX_TEMP_FILES 10
#define MAX_PATH_LEN 256

typedef struct {
    char filename[MAX_PATH_LEN];
    int fd;
} temp_file_t;

/* gengtype input files with diverse type definitions */
const char *gt_files[] = {
    /* File 1: Basic types and structs */
    "test1.gt",
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
    "\n"
    /* TYPE_ARRAY - array typedef */
    "typedef int my_array[10];\n"
    "typedef struct my_struct struct_array[5];\n"
    "%}\n",
    
    /* File 2: User structs, unions, and callbacks */
    "test2.gt",
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
    /* TYPE_CALLBACK - callback function pointer */
    "typedef void (*callback_fn)(void);\n"
    "typedef int (*compare_fn)(const void *, const void *);\n"
    "\n"
    /* TYPE_LANG_STRUCT - language-specific struct */
    "struct lang_struct {\n"
    "  int data;\n"
    "  void *extra;\n"
    "} GTY ((lang));\n"
    "\n"
    /* Complex nested type combining multiple categories */
    "struct complex_nested {\n"
    "  union my_union *uptr;      /* pointer to union */\n"
    "  callback_fn handler;       /* callback */\n"
    "  struct user_struct *users; /* pointer to user struct */\n"
    "  int matrix[3][4];          /* 2D array */\n"
    "};\n"
    "%}\n",
    
    /* File 3: More complex types and edge cases */
    "test3.gt",
    "%{\n"
    "#include \"config.h\"\n"
    "#include \"system.h\"\n"
    "%}\n"
    "\n"
    /* Multiple scalar types */
    "typedef char byte;\n"
    "typedef short int16;\n"
    "typedef long long int64;\n"
    "\n"
    /* String-heavy struct */
    "struct string_container {\n"
    "  const char *title;\n"
    "  char *buffer;\n"
    "  const char *const *string_array; /* pointer to pointer to const char */\n"
    "};\n"
    "\n"
    /* Array of pointers to callbacks */
    "typedef void (*action_fn)(int);\n"
    "action_fn action_table[10];\n"
    "\n"
    /* Union containing arrays */
    "union array_union {\n"
    "  int ints[20];\n"
    "  char chars[80];\n"
    "  void *ptrs[5];\n"
    "};\n"
    "\n"
    /* Struct with all type kinds */
    "struct kitchen_sink {\n"
    "  int scalar_field;                 /* scalar */\n"
    "  const char *string_field;         /* string */\n"
    "  struct my_struct *struct_ptr;     /* pointer to struct */\n"
    "  union my_union union_field;       /* union */\n"
    "  int array_field[15];              /* array */\n"
    "  callback_fn callback_field;       /* callback */\n"
    "  struct user_struct user_field;    /* user struct */\n"
    "};\n"
    "%}\n",
    
    /* File 4: File with syntax error (to test error paths) */
    "test4.gt",
    "%{\n"
    "#include \"config.h\"\n"
    "#include \"system.h\"\n"
    "\n"
    "struct bad_struct {\n"
    "  int missing_semicolon\n"
    "  /* Missing %} to cause parse error */\n",
    
    /* File 5: Duplicate definitions (to test warning paths) */
    "test5.gt",
    "%{\n"
    "#include \"config.h\"\n"
    "#include \"system.h\"\n"
    "%}\n"
    "\n"
    "struct duplicate_struct {\n"
    "  int x;\n"
    "};\n"
    "\n"
    "/* Duplicate definition */\n"
    "struct duplicate_struct {\n"
    "  int y;\n"
    "};\n"
    "%}\n",
    
    NULL
};

/* Compilation command for gengtype with coverage */
const char *gengtype_compile_cmd =
    "g++ -O0 -fprofile-arcs -ftest-coverage "
    "-DIN_GCC -DHAVE_CONFIG_H "
    "-I. -I../../include -I../../gcc "
    "-c gengtype.cc -o gengtype.o && "
    "g++ -O0 -fprofile-arcs -ftest-coverage "
    "-DIN_GCC -DHAVE_CONFIG_H "
    "-I. -I../../include -I../../gcc "
    "-c gengtype-state.cc -o gengtype-state.o && "
    "g++ -O0 -fprofile-arcs -ftest-coverage "
    "gengtype.o gengtype-state.o "
    "-lgcov -liberty -o gengtype-coverage";

/* Create temporary files with gengtype content */
int create_temp_files(temp_file_t *files, int max_files) {
    int count = 0;
    
    for (int i = 0; gt_files[i] != NULL && count < max_files; i += 2) {
        char template[] = "/tmp/gt_test_XXXXXX";
        int fd = mkstemp(template);
        if (fd < 0) {
            perror("mkstemp failed");
            continue;
        }
        
        strcpy(files[count].filename, template);
        files[count].fd = fd;
        
        /* Write the content */
        const char *content = gt_files[i + 1];
        ssize_t written = write(fd, content, strlen(content));
        if (written != (ssize_t)strlen(content)) {
            perror("write failed");
            close(fd);
            unlink(template);
            continue;
        }
        
        close(fd);
        printf("Created temporary file: %s\n", template);
        count++;
    }
    
    return count;
}

/* Clean up temporary files */
void cleanup_temp_files(temp_file_t *files, int count) {
    for (int i = 0; i < count; i++) {
        unlink(files[i].filename);
    }
}

/* Run gengtype with different patterns to trigger type counting */
int run_gengtype_patterns(temp_file_t *files, int file_count) {
    int success = 0;
    char cmd[1024];
    
    /* Pattern A: Process each file individually */
    printf("\n=== Pattern A: Processing files individually ===\n");
    for (int i = 0; i < file_count; i++) {
        if (strstr(files[i].filename, "test4.gt") || 
            strstr(files[i].filename, "test5.gt")) {
            /* Skip error/warning files for individual processing */
            continue;
        }
        
        snprintf(cmd, sizeof(cmd), 
                 "./gengtype-coverage -g output_%d.h %s", 
                 i, files[i].filename);
        
        printf("Running: %s\n", cmd);
        int ret = system(cmd);
        if (ret != 0) {
            printf("Command failed with exit code %d\n", WEXITSTATUS(ret));
        } else {
            success++;
            /* Clean up generated output */
            char output_file[256];
            snprintf(output_file, sizeof(output_file), "output_%d.h", i);
            unlink(output_file);
        }
    }
    
    /* Pattern B: Batch processing with -p flag */
    printf("\n=== Pattern B: Batch processing with -p ===\n");
    FILE *filelist = fopen("/tmp/gt_filelist.txt", "w");
    if (filelist) {
        for (int i = 0; i < file_count; i++) {
            fprintf(filelist, "%s\n", files[i].filename);
        }
        fclose(filelist);
        
        system("./gengtype-coverage -p /tmp/gt_filelist.txt");
        unlink("/tmp/gt_filelist.txt");
        success++;
    }
    
    /* Pattern C: Multiple files with header generation */
    printf("\n=== Pattern C: Multiple files with header generation ===\n");
    snprintf(cmd, sizeof(cmd), "./gengtype-coverage -g combined.h ");
    for (int i = 0; i < file_count && i < 3; i++) {
        strcat(cmd, files[i].filename);
        strcat(cmd, " ");
    }
    
    printf("Running: %s\n", cmd);
    int ret = system(cmd);
    if (ret == 0) {
        success++;
        unlink("combined.h");
    }
    
    /* Pattern D: Generate routine file */
    printf("\n=== Pattern D: Generating routine file ===\n");
    snprintf(cmd, sizeof(cmd), 
             "./gengtype-coverage -r gtype-desc.c %s %s",
             files[0].filename, files[1].filename);
    
    printf("Running: %s\n", cmd);
    ret = system(cmd);
    if (ret == 0) {
        success++;
        unlink("gtype-desc.c");
    }
    
    return success;
}

/* Check coverage by examining gcov output */
void check_coverage() {
    printf("\n=== Checking Coverage ===\n");
    
    /* Run gcov on gengtype */
    system("gcov gengtype.cc");
    
    /* Look for the specific switch statement coverage */
    FILE *gcov_file = fopen("gengtype.cc.gcov", "r");
    if (gcov_file) {
        char line[512];
        int in_target_block = 0;
        int line_num = 0;
        
        while (fgets(line, sizeof(line), gcov_file)) {
            line_num++;
            
            /* Look for lines around the target switch (lines 182-213) */
            if (line_num >= 180 && line_num <= 215) {
                printf("Line %3d: %s", line_num, line);
                
                /* Check if this line was executed */
                if (line_num >= 182 && line_num <= 213) {
                    char *count_str = strtok(line, ":");
                    if (count_str && strcmp(count_str, "        -") != 0 && 
                        strcmp(count_str, "        0") != 0) {
                        printf("  [EXECUTED]\n");
                    }
                }
            }
        }
        fclose(gcov_file);
    }
    
    /* Also check the gengtype.gcda file exists */
    if (access("gengtype.gcda", F_OK) == 0) {
        printf("\nCoverage data file generated: gengtype.gcda\n");
    }
}

/* Main test driver */
int main(int argc, char *argv[]) {
    printf("=== GCC gengtype Type Counting Coverage Test ===\n");
    
    /* Step 1: Compile gengtype with coverage instrumentation */
    printf("\nStep 1: Compiling gengtype with coverage flags...\n");
    int compile_result = system(gengtype_compile_cmd);
    if (compile_result != 0) {
        fprintf(stderr, "Failed to compile gengtype with coverage\n");
        return 1;
    }
    printf("gengtype compiled successfully with coverage instrumentation\n");
    
    /* Step 2: Create temporary .gt files */
    printf("\nStep 2: Creating temporary .gt files...\n");
    temp_file_t temp_files[MAX_TEMP_FILES];
    int file_count = create_temp_files(temp_files, MAX_TEMP_FILES);
    
    if (file_count == 0) {
        fprintf(stderr, "Failed to create temporary files\n");
        return 1;
    }
    printf("Created %d temporary .gt files\n", file_count);
    
    /* Step 3: Run gengtype with various patterns */
    printf("\nStep 3: Running gengtype with different input patterns...\n");
    int patterns_success = run_gengtype_patterns(temp_files, file_count);
    printf("\nSuccessfully executed %d patterns\n", patterns_success);
    
    /* Step 4: Check coverage results */
    check_coverage();
    
    /* Step 5: Cleanup */
    printf("\nStep 5: Cleaning up temporary files...\n");
    cleanup_temp_files(temp_files, file_count);
    
    /* Clean up gengtype executable */
    unlink("gengtype-coverage");
    unlink("gengtype.o");
    unlink("gengtype-state.o");
    
    printf("\n=== Test Complete ===\n");
    printf("The switch statement for type counting should now have coverage\n");
    printf("for all enum gty_token cases:\n");
    printf("  TYPE_UNDEFINED, TYPE_SCALAR, TYPE_STRING,\n");
    printf("  TYPE_STRUCT, TYPE_USER_STRUCT, TYPE_UNION,\n");
    printf("  TYPE_POINTER, TYPE_ARRAY, TYPE_CALLBACK,\n");
    printf("  TYPE_LANG_STRUCT, TYPE_NONE\n");
    
    return 0;
}
