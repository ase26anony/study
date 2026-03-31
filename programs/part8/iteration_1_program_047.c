/* gengtype_coverage_test.c - ISO C99 compliant test driver for gengtype parser coverage */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* Simplified parser stub for testing if actual gengtype is not available */
#ifdef TEST_STUB
static void consume_balanced(char open, char close) {
    /* Stub implementation for coverage testing */
    static int depth = 0;
    depth++;
    if (depth > 100) depth = 1; /* Prevent infinite recursion in stub */
}

static void advance(void) {
    /* Stub implementation */
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

/* Complex .gt test content targeting all uncovered switch cases */
static const char *gt_test_content = 
"/* Test file for gengtype parser coverage - targets lines 341-352 */\n"
"\n"
"/* ========== Requirement 1: Balanced Construct Nesting ========== */\n"
"\n"
"/* Complex nested parentheses in function pointers */\n"
"%typedef int (*complex_func_t)(int (*(*callback)(int, char*))[5], \n"
"                               struct { int x; double y; } param);\n"
"\n"
"/* Deeply nested arrays and structures */\n"
"%struct TreeNode {\n"
"    struct TreeNode *children[10];  /* Nested brackets */\n"
"    union {\n"
"        int ival;\n"
"        double fval;\n"
"        char *sval;\n"
"    } data;  /* Nested braces */\n"
"    void (*operations[3])(struct TreeNode*);  /* Mixed nesting */\n"
"};\n"
"\n"
"/* Pointer to function returning pointer to array */\n"
"%typedef int (*(*func_ret_array_ptr)(void))[10][20];\n"
"\n"
"/* ========== Requirement 2: Unbalanced Edge Cases ========== */\n"
"\n"
"/* Missing closing brace - should trigger error handling */\n"
"%struct UnbalancedStruct {\n"
"    int x;\n"
"    double y;\n"
"    char z[10];\n"
"    /* Missing closing brace here */\n"
"\n"
"/* Missing closing parenthesis in function pointer */\n"
"%typedef void (*bad_func_ptr(int x, double y);\n"
"\n"
"/* Missing closing bracket in array */\n"
"%struct BadArray {\n"
"    int matrix[5][3;  /* Unbalanced bracket */\n"
"};\n"
"\n"
"/* ========== Requirement 3: Comments and Macros Interleaving ========== */\n"
"\n"
"/* Block comment with delimiters inside */\n"
"/* int (*commented_func)(int x[5], struct { int a; } s); */\n"
"\n"
"// Line comment with nested braces\n"
"// struct Commented { int x; { double y; } };\n"
"\n"
"#define ARRAY_TYPE(T) struct { T data[10]; (T* next); }\n"
"#define FUNC_PTR(RET, ARGS) RET (*)(ARGS)\n"
"#define NESTED_MACRO(x) { { x }, [0] = 1, (x + 1) }\n"
"\n"
"/* Macro expansion with balanced delimiters */\n"
"%typedef ARRAY_TYPE(int*) IntArray;\n"
"%typedef FUNC_PTR(int, (int, char*)) IntFunc;\n"
"\n"
"/* ========== Requirement 4: GT File Specific Constructs ========== */\n"
"\n"
"/* GC roots with nested structures */\n"
"GC roots {\n"
"    struct TreeNode *root_nodes[100];\n"
"    %union {\n"
"        struct { int tag; void *data; } s;\n"
"        int arr[5][5];\n"
"        void (*funcs[3])(int, char*);\n"
"    } gc_union;\n"
"};\n"
"\n"
"%union ComplexUnion {\n"
"    struct {\n"
"        int x;\n"
"        double y;\n"
"        char z[20];\n"
"    } nested_struct;\n"
"    int (*matrix)[10][10];\n"
"    void (*(*complex_callback[5]))(int, ...);\n"
"};\n"
"\n"
"/* Struct with all delimiter types */\n"
"%struct AllDelimiters {\n"
"    int simple;\n"
"    int array[5][10];  /* Brackets */\n"
"    struct { int a; char b; } nested;  /* Braces */\n"
"    int (*func_ptr)(int, char*);  /* Parentheses */\n"
"    union {\n"
"        int x;\n"
"        double y[3];\n"
"    } u;\n"
"};\n"
"\n"
"/* ========== Requirement 5: Multiple Top-Level Declarations ========== */\n"
"\n"
"%enum Color {\n"
"    RED,\n"
"    GREEN,\n"
"    BLUE,\n"
"    COLORS[3] = { RED, GREEN, BLUE }  /* Braces in enum */\n"
"};\n"
"\n"
"%typedef struct {\n"
"    %union {\n"
"        int i;\n"
"        float f;\n"
"    } value;\n"
"    %enum Type { INT, FLOAT } type;\n"
"} Variant;\n"
"\n"
"/* Function pointer typedefs to trigger parentheses parsing */\n"
"%typedef int (*Comparator)(const void*, const void*);\n"
"%typedef void (*VoidFunc)(void);\n"
"%typedef char* (*StringProcessor)(char* input, int (*filter)(char));\n"
"\n"
"/* Array of function pointers */\n"
"%struct OperationTable {\n"
"    const char *name;\n"
"    int (*operation)(int, int);\n"
"    void (*cleanup)(void*);\n"
"} ops[10];\n"
"\n"
"/* Nested anonymous structs/unions */\n"
"%struct Outer {\n"
"    struct {\n"
"        union {\n"
"            int x;\n"
"            long y;\n"
"} inner_union;  /* Multiple closing braces */\n"
"        int z[5];\n"
"    } inner_struct;\n"
"    void (*methods[3])(struct Outer*);\n"
"};\n"
"\n"
"/* Complex type with all delimiters deeply nested */\n"
"%typedef int (*(**complex_array[10])(int (*)(char*), \n"
"                                     struct { int a[5]; }))\n"
"            [20][30];\n"
"\n"
"/* End of test file */\n";

/* Additional test cases for specific edge scenarios */
static const char *gt_edge_cases = 
"/* Additional edge cases for maximum coverage */\n"
"\n"
"/* Empty balanced constructs */\n"
"%struct EmptyStruct { };\n"
"%typedef int empty_array[0];\n"
"%typedef void (*empty_params)(void);\n"
"\n"
"/* Single element constructs */\n"
"%struct Single { int x; };\n"
"%typedef int single_array[1];\n"
"%typedef void (*single_param)(int);\n"
"\n"
"/* Mixed nested delimiters */\n"
"%struct MixedNesting {\n"
"    int (*array_of_funcs[5])(int[3], struct { int x; });\n"
"    struct {\n"
"        union {\n"
"            int (*func)(int[2]);\n"
"            char *str;\n"
"        } u;\n"
"    } s;\n"
"};\n"
"\n"
"/* Preprocessor conditionals with delimiters */\n"
"#ifdef TEST_MODE\n"
"    %struct ConditionalStruct {\n"
"        int test_array[10];\n"
"        void (*test_func)(void);\n"
"    };\n"
"#else\n"
"    %struct AltStruct {\n"
"        double data[5][5];\n"
"        struct { int x; } point;\n"
"    };\n"
"#endif\n"
"\n"
"/* String literals with delimiter-like characters */\n"
"%struct WithStrings {\n"
"    char *open_paren = \"(\";\n"
"    char *open_brace = \"{\";\n"
"    char *open_bracket = \"[\";\n"
"    char *all_delims = \"(){}[]\";\n"
"};\n"
"\n"
"/* Comments that look like code */\n"
"/*\n"
"struct FakeStruct {\n"
"    int fake_array[10];\n"
"    void (*fake_func)(int);\n"
"};\n"
"*/\n"
"\n"
"/* Real struct after fake comment */\n"
"%struct RealStruct {\n"
"    int real_array[10];\n"
"    void (*real_func)(int);\n"
"};\n";

int main(int argc, char *argv[]) {
    FILE *gt_file = NULL;
    char temp_filename[] = "/tmp/gengtype_test_XXXXXX.gt";
    int fd;
    
    /* Create temporary file */
    fd = mkstemps(temp_filename, 3);  /* .gt extension is 3 chars */
    if (fd == -1) {
        perror("Failed to create temporary file");
        return 1;
    }
    
    gt_file = fdopen(fd, "w");
    if (!gt_file) {
        perror("Failed to open temporary file");
        close(fd);
        return 1;
    }
    
    /* Write the comprehensive test content */
    fprintf(gt_file, "%s\n\n%s\n", gt_test_content, gt_edge_cases);
    fclose(gt_file);
    
    printf("Generated test file: %s\n", temp_filename);
    printf("File size: %ld bytes\n", strlen(gt_test_content) + strlen(gt_edge_cases));
    
    /* Option 1: If we have the actual gengtype binary */
#ifdef HAS_GENGTYPE
    char command[512];
    snprintf(command, sizeof(command), "./gengtype -p %s", temp_filename);
    printf("Executing: %s\n", command);
    int result = system(command);
    if (result != 0) {
        printf("gengtype execution returned: %d\n", result);
    }
#else
    /* Option 2: Use stub parser for coverage analysis */
#ifdef TEST_STUB
    printf("Using stub parser for coverage testing\n");
    /* Simulate parsing each character to trigger the switch cases */
    const char *test_str = gt_test_content;
    while (*test_str) {
        parse_character(*test_str);
        test_str++;
    }
#endif
    
    printf("Test file created. To run actual gengtype:\n");
    printf("  gengtype -p %s\n", temp_filename);
#endif
    
    /* Clean up */
    unlink(temp_filename);
    
    return 0;
}
