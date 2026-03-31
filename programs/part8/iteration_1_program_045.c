/* gengtype-coverage-test.c - ISO C99-compliant test driver for gengtype parser coverage */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

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

/* Complex .gt test content with balanced and unbalanced constructs */
static const char *gt_test_content = 
"/* Test file for gengtype parser coverage */\n"
"/* Line 1: Basic type definitions with balanced parentheses */\n"
"%typedef int (*comparator_t)(const void *, const void *);\n"
"%typedef void (*complex_func_t)(int (*)(double), char **);\n"
"\n"
"/* Line 2: Struct with nested balanced constructs */\n"
"%struct TreeNode {\n"
"    struct TreeNode *left;\n"
"    struct TreeNode *right;\n"
"    int data;\n"
"    void (*print)(struct TreeNode *);\n"
"};\n"
"\n"
"/* Line 3: Union with array and function pointer */\n"
"%union DataUnion {\n"
"    int int_array[10];\n"
"    struct {\n"
"        char *name;\n"
"        double values[5][5];\n"
"    } nested;\n"
"    void (*operations[3])(int, float);\n"
"};\n"
"\n"
"/* Line 4: Complex macro with balanced delimiters */\n"
"#define ARRAY_WRAPPER(T, N) struct { T data[N]; size_t count; }\n"
"#define FUNCTION_PTR(RET, ARGS) RET (*)(ARGS)\n"
"\n"
"/* Line 5: GC roots with balanced constructs */\n"
"GC roots {\n"
"    struct TreeNode *tree_root;\n"
"    %union DataUnion *data_pool[100];\n"
"    int (*sort_funcs[])(int *, size_t);\n"
"};\n"
"\n"
"/* Line 6: Enum with complex initializers */\n"
"%enum ErrorCodes {\n"
"    ERR_NONE = 0,\n"
"    ERR_PARSE = (1 << 0),\n"
"    ERR_MEM = (1 << 1),\n"
"    ERR_MAX = (1 << 31)\n"
"};\n"
"\n"
"/* Line 7: Deeply nested balanced constructs */\n"
"%struct DeepNest {\n"
"    int (*(*complex[2])(char *))[10];\n"
"    struct {\n"
"        union {\n"
"            int x;\n"
"            struct { char a; char b; };\n"
"        };\n"
"        float matrix[3][3];\n"
"    } inner;\n"
"};\n"
"\n"
"/* Line 8: Comments containing balanced delimiters (should be ignored) */\n"
"/* This comment has (parentheses), [brackets], and {braces} inside */\n"
"// Line comment with [ignored] delimiters\n"
"\n"
"/* Line 9: Pointer to array of function pointers */\n"
"%typedef int (*(*callback_array_t)[5])(void);\n"
"\n"
"/* Line 10: Template-like macro expansion */\n"
"#define CONTAINER_OF(ptr, type, member) ((type *)((char *)(ptr) - offsetof(type, member)))\n"
"\n"
"/* Line 11: Unbalanced constructs for error handling coverage */\n"
"/* UNCOMMENT TO TEST ERROR CASES:\n"
"%struct Unbalanced1 {\n"
"    int x;\n"
"    /* Missing closing brace here */\n"
"\n"
"%typedef int (*unbalanced_func(int, float);  /* Missing closing parenthesis */\n"
"\n"
"int array_missing_bracket[10;  /* Missing closing bracket */\n"
"*/\n"
"\n"
"/* Line 12: Valid complex type after unbalanced section */\n"
"%struct RecoveryTest {\n"
"    int valid_field;\n"
"    void (*valid_funcptr)(int (*)(double));\n"
"};\n"
"\n"
"/* Line 13: Multiple top-level declarations to force repeated parsing */\n"
"%typedef unsigned int uint32_t;\n"
"%typedef uint32_t mask_t;\n"
"\n"
"%struct List {\n"
"    struct List *next;\n"
"    void *data;\n"
"};\n"
"\n"
"GC roots {\n"
"    struct List *global_list;\n"
"    void *(*allocators[4])(size_t);\n"
"};\n"
"\n"
"/* Line 14: Mixed delimiters in complex expression */\n"
"%typedef int matrix_t[3][3];\n"
"%typedef int (*transform_fn)(matrix_t, int (*)(int));\n"
"\n"
"/* Line 15: Final test with all delimiter types */\n"
"%struct AllDelimiters {\n"
"    int simple;\n"
"    int array[10];\n"
"    struct { int a; int b; } nested;\n"
"    void (*func)(int, char *);\n"
"    union {\n"
"        long l;\n"
"        double d;\n"
"    } value;\n"
"};\n";

/* Additional test with explicit unbalanced cases */
static const char *gt_unbalanced_test = 
"/* Test with unbalanced delimiters */\n"
"%struct MissingBrace {\n"
"    int x;\n"
"    char y;\n"
"    /* No closing brace - parser should handle this */\n"
"\n"
"%typedef int (*missing_paren(int, float);\n"
"\n"
"int bad_array[10;\n"
"\n"
"/* Recovery type */\n"
"%struct AfterError { int ok; };\n";

/* Write test content to file */
static int write_test_file(const char *filename, const char *content) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        fprintf(stderr, "Error creating test file %s: %s\n", 
                filename, strerror(errno));
        return -1;
    }
    
    fputs(content, f);
    fclose(f);
    return 0;
}

/* Parse file using stub or actual gengtype */
static int parse_test_file(const char *filename) {
#ifdef TEST_STUB
    /* Use stub parser for direct coverage testing */
    FILE *f = fopen(filename, "r");
    if (!f) {
        fprintf(stderr, "Error opening %s: %s\n", filename, strerror(errno));
        return -1;
    }
    
    int c;
    while ((c = fgetc(f)) != EOF) {
        parse_character((char)c);
    }
    
    fclose(f);
    return 0;
#else
    /* In real testing, this would invoke the actual gengtype binary */
    char command[256];
    snprintf(command, sizeof(command), "./gengtype -p %s", filename);
    return system(command);
#endif
}

int main(void) {
    const char *test_files[] = {
        "test_balanced.gt",
        "test_unbalanced.gt"
    };
    
    const char *test_contents[] = {
        gt_test_content,
        gt_unbalanced_test
    };
    
    int num_tests = sizeof(test_files) / sizeof(test_files[0]);
    int ret = 0;
    
    printf("Generating gengtype test files for parser coverage...\n");
    
    /* Create and parse each test file */
    for (int i = 0; i < num_tests; i++) {
        printf("Creating %s...\n", test_files[i]);
        
        if (write_test_file(test_files[i], test_contents[i]) != 0) {
            ret = 1;
            continue;
        }
        
        printf("Parsing %s...\n", test_files[i]);
        if (parse_test_file(test_files[i]) != 0) {
            fprintf(stderr, "Warning: Parse test %d may have failed\n", i);
            /* Continue with other tests even if one fails */
        }
        
        /* Clean up */
        remove(test_files[i]);
    }
    
    /* Additional test: Direct string parsing for coverage */
    printf("\nDirect string parsing for switch case coverage...\n");
#ifdef TEST_STUB
    const char *direct_test = "( [ { } ] ) [ ( ) ] { [ ] }";
    for (const char *p = direct_test; *p; p++) {
        parse_character(*p);
    }
#endif
    
    printf("\nCoverage test completed. Check gcov results for:\n");
    printf("  - consume_balanced('(', ')')\n");
    printf("  - consume_balanced('[', ']')\n");
    printf("  - consume_balanced('{', '}')\n");
    printf("  - default: advance() case\n");
    
    return ret;
}
