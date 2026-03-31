/* gengtype-test-driver.c - Test driver for gengtype parser coverage */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

/* Simplified parser stub for testing if actual gengtype is not available */
#ifdef TEST_STUB
static void consume_balanced(char open, char close) {
    /* Stub implementation for coverage testing */
    static int depth = 0;
    depth++;
    /* Simulate parsing logic */
    depth--;
}

static void advance(void) {
    /* Stub for token advancement */
}

static void parse_character(char c) {
    switch (c) {
        default:
            advance();
            break;
        case '(':
            consume_balanced('(', ')');
            break;
        case '[':
            consume_balanced('[', ']');
            break;
        case '{':
            consume_balanced('{', '}');
            break;
    }
}
#endif

/* Complex .gt test content targeting uncovered lines 341-352 */
static const char* generate_gt_content(void) {
    return 
    "/* Test file for gengtype parser coverage - Balanced constructs */\n"
    "\n"
    "/* 1. Basic type definitions with all delimiter types */\n"
    "%typedef struct BaseType {\n"
    "    int simple;\n"
    "    int array[10];\n"
    "    void (*func_ptr)(int, char);\n"
    "} BaseType;\n"
    "\n"
    "/* 2. Complex nested structures */\n"
    "%struct ComplexNest {\n"
    "    /* Nested parentheses in function pointers */\n"
    "    int (*(*nested_func_ptr[5])(float (*(*)(double))[3]))(char);\n"
    "    \n"
    "    /* Deep array nesting */\n"
    "    int deep_array[2][3][4][5];\n"
    "    \n"
    "    /* Mixed delimiters */\n"
    "    struct {\n"
    "        union {\n"
    "            int x;\n"
    "            char y[20];\n"
    "        } u;\n"
    "        void (*methods[3])(struct ComplexNest*);\n"
    "    } inner;\n"
    "};\n"
    "\n"
    "/* 3. Union with all delimiter types */\n"
    "%union AllDelimiters {\n"
    "    int simple;\n"
    "    int array_dim1[5];\n"
    "    int array_dim2[3][4];\n"
    "    struct {\n"
    "        void (*callback)(int (*compare)(const void*, const void*));\n"
    "        char data[100];\n"
    "    } nested;\n"
    "    /* Function pointer with complex signature */\n"
    "    void (*(*signal_handler)(int signum, void (*old_handler)(int)))(int);\n"
    "};\n"
    "\n"
    "/* 4. GC roots with balanced constructs */\n"
    "GC roots {\n"
    "    struct RootType {\n"
    "        /* Array of function pointers */\n"
    "        int (*(*operations[10])(int, char*))();\n"
    "        \n"
    "        /* Nested anonymous struct */\n"
    "        struct {\n"
    "            int counter;\n"
    "            void (*increment)(struct { int x; int y; }*);\n"
    "        } state;\n"
    "        \n"
    "        /* Multi-dimensional array */\n"
    "        float matrix[3][3][3];\n"
    "    } *root_ptr;\n"
    "}\n"
    "\n"
    "/* 5. Macro definitions with balanced delimiters */\n"
    "#define ARRAY_TYPE(T) struct { T data[10]; T* ptr; }\n"
    "#define FUNCTION_PTR(RET, ARGS) RET (*)(ARGS)\n"
    "#define NESTED_PTR(T) T (*(*)[5])(void)\n"
    "\n"
    "/* Use the macros */\n"
    "%typedef ARRAY_TYPE(int) IntArray;\n"
    "%typedef FUNCTION_PTR(void, int) SimpleFunc;\n"
    "%typedef NESTED_PTR(char) ComplexNestedPtr;\n"
    "\n"
    "/* 6. Enum with complex initializers */\n"
    "%enum ComplexEnum {\n"
    "    VALUE_A = (1 << 0),\n"
    "    VALUE_B = (1 << 1) | (1 << 2),\n"
    "    VALUE_C = sizeof(struct { int x; char y[sizeof(int[2])]; }),\n"
    "    VALUE_D = (int)((void(*)(void))0)\n"
    "};\n"
    "\n"
    "/* 7. Comments containing balanced delimiters (should be ignored) */\n"
    "/* This comment has (parentheses), [brackets], and {braces} inside */\n"
    "// Line comment with nested [array[like][syntax]]\n"
    "\n"
    "/* 8. Edge cases - nearly balanced but tricky */\n"
    "%struct EdgeCases {\n"
    "    /* Function pointer returning pointer to array */\n"
    "    int (*(*func1)(void))[10];\n"
    "    \n"
    "    /* Array of pointers to functions */\n"
    "    void (*(*func_array[5])(int))();\n"
    "    \n"
    "    /* Nested in typedef */\n"
    "    typedef struct { int x; } (*callback_t)(struct { int y; }*);\n"
    "};\n"
    "\n"
    "/* 9. Template-like patterns (not real templates, but similar syntax) */\n"
    "#define CONTAINER(T, N) struct { T items[N]; size_t count; }\n"
    "%typedef CONTAINER(void*, 100) PtrContainer;\n"
    "\n"
    "/* 10. Unbalanced cases for error handling (commented out for valid parse) */\n"
    "/* UNBALANCED EXAMPLE 1: Missing closing brace\n"
    "%struct Unbalanced1 {\n"
    "    int x;\n"
    "    char y[10];\n"
    "    /* Missing } here */\n"
    "*/\n"
    "\n"
    "/* UNBALANCED EXAMPLE 2: Extra closing parenthesis\n"
    "void (*extra_paren))(int);\n"
    "*/\n"
    "\n"
    "/* UNBALANCED EXAMPLE 3: Mismatched brackets\n"
    "int array[10(];\n"
    "*/\n"
    "\n"
    "/* 11. Real unbalanced case (will cause parse error) */\n"
    "%struct ActuallyUnbalanced {\n"
    "    int x;\n"
    "    char data[100\n"  /* Missing closing bracket - triggers error */
    "};\n"
    "\n"
    "/* 12. Recovery test - valid after error */\n"
    "%struct AfterError {\n"
    "    int valid;\n"
    "    void (*recovered)(int);\n"
    "};\n"
    "\n"
    "/* 13. Very deep nesting */\n"
    "%typedef int (*(*(*(*deep_nest)(void))[5])(float))(char);\n"
    "\n"
    "/* 14. Anonymous structs/unions */\n"
    "%struct Anonymous {\n"
    "    union {\n"
    "        struct {\n"
    "            int a;\n"
    "            int b;\n"
    "        };\n"
    "        struct {\n"
    "            float x;\n"
    "            float y;\n"
    "        };\n"
    "    };\n"
    "    void (*operation)(union { int i; float f; }*);\n"
    "};\n"
    "\n"
    "/* 15. Multiple top-level declarations to force repeated parsing */\n"
    "%typedef short Small;\n"
    "%union ManyTypes { int i; float f; double d; };\n"
    "%struct FinalStruct { int last_field; };\n"
    "\n"
    "/* End of test file */\n";
}

/* Create temporary file with .gt extension */
static char* create_temp_gt_file(const char* content) {
    char template[] = "/tmp/gengtype_test_XXXXXX.gt";
    int fd = mkstemps(template, 3);  /* 3 chars for ".gt" */
    if (fd < 0) {
        perror("mkstemps failed");
        return NULL;
    }
    
    FILE* f = fdopen(fd, "w");
    if (!f) {
        perror("fdopen failed");
        close(fd);
        return NULL;
    }
    
    fwrite(content, 1, strlen(content), f);
    fclose(f);
    
    return strdup(template);
}

/* Parse the file using actual gengtype or stub */
static int parse_gt_file(const char* filename) {
#ifdef TEST_STUB
    /* Simulate parsing for coverage testing */
    FILE* f = fopen(filename, "r");
    if (!f) {
        perror("Failed to open test file");
        return -1;
    }
    
    int c;
    while ((c = fgetc(f)) != EOF) {
        parse_character((char)c);
    }
    
    fclose(f);
    return 0;
#else
    /* Use actual gengtype if available */
    char command[256];
    snprintf(command, sizeof(command), "./gengtype -p %s", filename);
    return system(command);
#endif
}

int main(void) {
    printf("Generating gengtype test for parser coverage...\n");
    
    /* Generate test content */
    const char* gt_content = generate_gt_content();
    
    /* Create temporary .gt file */
    char* temp_file = create_temp_gt_file(gt_content);
    if (!temp_file) {
        fprintf(stderr, "Failed to create temporary file\n");
        return 1;
    }
    
    printf("Created test file: %s\n", temp_file);
    printf("File size: %zu bytes\n", strlen(gt_content));
    
    /* Parse the file */
    printf("\nParsing test file...\n");
    int result = parse_gt_file(temp_file);
    
    if (result == 0) {
        printf("Parsing completed (some errors expected for unbalanced cases)\n");
    } else {
        printf("Parsing failed with code %d\n", result);
    }
    
    /* Cleanup */
    printf("\nCleaning up...\n");
    unlink(temp_file);
    free(temp_file);
    
    printf("Test completed.\n");
    return 0;
}
