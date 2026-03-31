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

/* Create a temporary file with given content */
char *create_temp_file(const char *content, const char *suffix) {
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
        perror("fdopen");
        close(fd);
        return NULL;
    }
    
    fwrite(content, 1, strlen(content), f);
    fclose(f);
    
    return strdup(template);
}

/* Clean up temporary files */
void cleanup_temp_files(char **files, int count) {
    for (int i = 0; i < count; i++) {
        if (files[i]) {
            unlink(files[i]);
            free(files[i]);
        }
    }
}

/* GT file 1: Basic types and structs */
const char *gt_file1_content = 
"%{\n"
"/* Test file 1: Basic types */\n"
"#include \"config.h\"\n"
"%}\n"
"\n"
"/* TYPE_UNDEFINED: Forward declaration */\n"
"struct undefined_struct;\n"
"\n"
"/* TYPE_SCALAR: Scalar typedef */\n"
"typedef int my_scalar;\n"
"typedef unsigned long my_ulong;\n"
"\n"
"/* TYPE_STRING: String type */\n"
"struct string_struct {\n"
"  const char *name;  /* TYPE_STRING */\n"
"  char *buffer;\n"
"};\n"
"\n"
"/* TYPE_STRUCT: Regular struct */\n"
"struct my_struct {\n"
"  int a;\n"
"  float b;\n"
"  struct my_struct *next;\n"
"};\n"
"\n"
"/* TYPE_POINTER: Pointer typedef */\n"
"typedef struct my_struct *my_ptr;\n"
"typedef my_scalar *scalar_ptr;\n"
"%}\n";

/* GT file 2: Advanced types and unions */
const char *gt_file2_content =
"%{\n"
"/* Test file 2: Advanced types */\n"
"#include \"config.h\"\n"
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
"typedef union myunion *ptr_array[20];\n"
"\n"
"/* TYPE_CALLBACK: Callback function pointer */\n"
"typedef void (*callback_fn)(void);\n"
"typedef int (*compare_fn)(const void *, const void *);\n"
"\n"
"/* Complex nested type */\n"
"struct complex_nested {\n"
"  union my_union data;\n"
"  my_array buffer;\n"
"  callback_fn handler;\n"
"  struct complex_nested **children;  /* Pointer to pointer */\n"
"};\n"
"%}\n";

/* GT file 3: Language structs and edge cases */
const char *gt_file3_content =
"%{\n"
"/* Test file 3: Language structs and edge cases */\n"
"#include \"config.h\"\n"
"%}\n"
"\n"
"/* TYPE_LANG_STRUCT: Language-specific struct */\n"
"struct lang_struct {\n"
"  int data;\n"
"  void *extra;\n"
"} GTY ((lang));\n"
"\n"
"/* Another language struct with nested types */\n"
"struct lang_tree_node {\n"
"  int type;\n"
"  struct lang_tree_node *left;\n"
"  struct lang_tree_node *right;\n"
"  union {\n"
"    int ival;\n"
"    double dval;\n"
"    const char *sval;\n"
"  } u;\n"
"} GTY ((lang));\n"
"\n"
"/* More pointer variations */\n"
"typedef void (*void_func)(void);\n"
"typedef void_func func_ptr;\n"
"\n"
"/* Array of pointers to callbacks */\n"
"typedef callback_fn callback_table[8];\n"
"\n"
"/* Struct containing all types */\n"
"struct kitchen_sink {\n"
"  my_scalar scalar;           /* TYPE_SCALAR */\n"
"  const char *str;            /* TYPE_STRING */\n"
"  struct my_struct s;         /* TYPE_STRUCT */\n"
"  union my_union u;           /* TYPE_UNION */\n"
"  my_array arr;               /* TYPE_ARRAY */\n"
"  callback_fn cb;             /* TYPE_CALLBACK */\n"
"  struct lang_struct *lang;   /* TYPE_LANG_STRUCT via pointer */\n"
"};\n"
"%}\n";

/* GT file 4: With syntax error (for error path testing) */
const char *gt_file4_content =
"%{\n"
"/* Test file 4: With deliberate syntax error */\n"
"#include \"config.h\"\n"
"/* Missing closing %} to trigger error */\n"
"\n"
"struct error_struct {\n"
"  int x;\n"
"};\n"
"/* No closing %} */\n";

/* GT file 5: Duplicate definitions (for warning testing) */
const char *gt_file5_content =
"%{\n"
"/* Test file 5: Duplicate definitions */\n"
"#include \"config.h\"\n"
"%}\n"
"\n"
"/* Duplicate struct definition */\n"
"struct duplicate_struct {\n"
"  int a;\n"
"};\n"
"\n"
"struct duplicate_struct {\n"
"  int b;  /* This should trigger a warning */\n"
"};\n"
"%}\n";

/* Build and run gengtype with coverage */
int build_and_run_gengtype(char **input_files, int file_count, 
                          const char *output_header, const char *output_routine) {
    int status;
    pid_t pid;
    
    /* Build gengtype command */
    char cmd[4096];
    
    /* Pattern C: Generate header and routine files */
    if (output_header && output_routine) {
        snprintf(cmd, sizeof(cmd), "./gengtype -g %s -r %s", 
                output_header, output_routine);
        for (int i = 0; i < file_count; i++) {
            strcat(cmd, " ");
            strcat(cmd, input_files[i]);
        }
    } 
    /* Pattern B: Process file list */
    else {
        /* Create file list */
        char *list_file = create_temp_file("", ".list");
        FILE *f = fopen(list_file, "w");
        if (!f) {
            perror("fopen list file");
            free(list_file);
            return -1;
        }
        for (int i = 0; i < file_count; i++) {
            fprintf(f, "%s\n", input_files[i]);
        }
        fclose(f);
        
        snprintf(cmd, sizeof(cmd), "./gengtype -p %s", list_file);
        
        /* Clean up list file after execution */
        pid = fork();
        if (pid == 0) {
            /* Child process */
            system(cmd);
            unlink(list_file);
            free(list_file);
            exit(0);
        } else if (pid > 0) {
            waitpid(pid, &status, 0);
            return WEXITSTATUS(status);
        }
        return -1;
    }
    
    printf("Executing: %s\n", cmd);
    
    /* Execute command */
    status = system(cmd);
    if (status == -1) {
        perror("system");
        return -1;
    }
    
    return WEXITSTATUS(status);
}

/* Main test driver */
int main(int argc, char *argv[]) {
    int ret = 0;
    char *temp_files[10];
    int file_count = 0;
    
    printf("=== Gengtype Coverage Test Driver ===\n");
    
    /* Create all test GT files */
    temp_files[file_count++] = create_temp_file(gt_file1_content, ".gt");
    temp_files[file_count++] = create_temp_file(gt_file2_content, ".gt");
    temp_files[file_count++] = create_temp_file(gt_file3_content, ".gt");
    temp_files[file_count++] = create_temp_file(gt_file4_content, ".gt");
    temp_files[file_count++] = create_temp_file(gt_file5_content, ".gt");
    
    /* Verify all files were created */
    for (int i = 0; i < file_count; i++) {
        if (!temp_files[i]) {
            fprintf(stderr, "Failed to create temp file %d\n", i);
            cleanup_temp_files(temp_files, file_count);
            return 1;
        }
        printf("Created: %s\n", temp_files[i]);
    }
    
    /* Pattern A: Process each file individually */
    printf("\n--- Pattern A: Individual file processing ---\n");
    for (int i = 0; i < 3; i++) {  /* First 3 valid files */
        char cmd[512];
        snprintf(cmd, sizeof(cmd), "./gengtype -g /tmp/output%d.h -r /tmp/routines%d.c %s",
                i, i, temp_files[i]);
        printf("Running: %s\n", cmd);
        int status = system(cmd);
        if (status != 0) {
            printf("Warning: gengtype exited with status %d for file %d\n", 
                   WEXITSTATUS(status), i);
        }
    }
    
    /* Pattern C: Process all valid files together */
    printf("\n--- Pattern C: Batch processing with header generation ---\n");
    char *header_file = "/tmp/gtype_test.h";
    char *routine_file = "/tmp/gtype_test.c";
    
    int result = build_and_run_gengtype(temp_files, 3, header_file, routine_file);
    if (result == 0) {
        printf("Successfully generated header and routine files\n");
        
        /* Verify files were created */
        FILE *f = fopen(header_file, "r");
        if (f) {
            printf("Header file created successfully\n");
            fclose(f);
        } else {
            printf("Warning: Header file not created\n");
        }
        
        f = fopen(routine_file, "r");
        if (f) {
            printf("Routine file created successfully\n");
            fclose(f);
        } else {
            printf("Warning: Routine file not created\n");
        }
    } else {
        printf("gengtype exited with status %d\n", result);
    }
    
    /* Pattern D: Test error cases */
    printf("\n--- Pattern D: Error case testing ---\n");
    for (int i = 3; i < file_count; i++) {
        char cmd[512];
        snprintf(cmd, sizeof(cmd), "./gengtype %s 2>&1", temp_files[i]);
        printf("Testing error file %d: %s\n", i-2, temp_files[i]);
        
        FILE *fp = popen(cmd, "r");
        if (fp) {
            char buffer[256];
            while (fgets(buffer, sizeof(buffer), fp)) {
                printf("  Output: %s", buffer);
            }
            pclose(fp);
        }
    }
    
    /* Clean up */
    cleanup_temp_files(temp_files, file_count);
    
    /* Remove output files */
    unlink(header_file);
    unlink(routine_file);
    for (int i = 0; i < 3; i++) {
        char buf[256];
        snprintf(buf, sizeof(buf), "/tmp/output%d.h", i);
        unlink(buf);
        snprintf(buf, sizeof(buf), "/tmp/routines%d.c", i);
        unlink(buf);
    }
    
    printf("\n=== Test completed ===\n");
    
    return ret;
}
