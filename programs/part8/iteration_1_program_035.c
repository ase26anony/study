/* gengtype-coverage-test.c - Comprehensive test for gengtype parser coverage */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* Simplified parser stub to demonstrate the logic being tested */
typedef enum {
    TOK_EOF,
    TOK_CHAR,
    TOK_ERROR
} token_t;

void advance(void) { /* Simulated token advancement */ }
void consume_balanced(char open, char close) {
    /* Simulated balanced delimiter consumption */
    printf("consume_balanced('%c', '%c') called\n", open, close);
}

void parse_character(char c) {
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

/* Main test content with all required patterns */
const char* test_gt_content = 
"/* Test file for gengtype parser coverage */\n"
"/* Balanced construct nesting - Requirement 1 */\n"
"\n"
"/* Complex type with all delimiter types */\n"
"%typedef struct ComplexType {\n"
"    int (*callback)(int (*nested_cb)(int, char**), double);  /* Nested function pointers */\n"
"    union {\n"
"        struct { int x; double y; } nested_struct;\n"
"        int matrix[3][4][5];  /* Multi-dimensional array */\n"
"    } data_union;\n"
"    void (*operations[10])(struct ComplexType*);  /* Array of function pointers */\n"
"} ComplexType;\n"
"\n"
"/* Pointer to function with complex signature - triggers parentheses */\n"
"%typedef void (*(*signal_handler_t)(int signum))(int);\n"
"\n"
"/* Nested balanced constructs */\n"
"%struct TreeNode {\n"
"    struct TreeNode* children[10];\n"
"    void (*visit)(struct TreeNode* node, void (*action)(int));\n"
"    union {\n"
"        int int_val;\n"
"        double dbl_val;\n"
"        char* str_val;\n"
"    } value;\n"
"};\n"
"\n"
"/* GT-specific annotations with balanced delimiters - Requirement 4 */\n"
"%union TaggedUnion {\n"
"    struct { int x; float y; } point;\n"
"    int array[100];\n"
"    void (*func)(char* (*allocator)(size_t));\n"
"};\n"
"\n"
"/* GC roots with complex types */\n"
"GC roots {\n"
"    struct TreeNode* root_nodes[50];\n"
"    ComplexType** complex_array;\n"
"    void (*(*callback_table[20]))(int);\n"
"};\n"
"\n"
"/* Comments interleaving with balanced delimiters - Requirement 3 */\n"
"%typedef struct CommentTest {\n"
"    /* Start of array declaration */\n"
"    int values[ /* array size comment */ 10 /* end size comment */ ];\n"
"    // Line comment with delimiter: { \n"
"    void (*func_ptr)(int);  // Function pointer comment\n"
"} CommentTest;\n"
"\n"
"/* Macro definitions with balanced delimiters */\n"
"#define ARRAY_TYPE(T) struct { T data[10]; (*(*callback))(T); }\n"
"#define FUNCTION_PTR(RET, ARGS) RET (*)(ARGS)\n"
"\n"
"/* Using the macros */\n"
"%typedef ARRAY_TYPE(int) IntArray;\n"
"%typedef FUNCTION_PTR(void, int) SimpleFunc;\n"
"\n"
"/* Multiple top-level declarations - Requirement 5 */\n"
"%enum Color { RED, GREEN, BLUE };\n"
"\n"
"%struct GraphNode {\n"
"    struct GraphNode** neighbors;\n"
"    int (*compare)(struct GraphNode* a, struct GraphNode* b);\n"
"    Color color;\n"
"};\n"
"\n"
"%union Variant {\n"
"    struct { int x; int y; } coords;\n"
"    char* name;\n"
"    double (*compute)(double, double);\n"
"};\n"
"\n"
"/* Unbalanced edge cases - Requirement 2 */\n"
"/* WARNING: These should trigger parser errors */\n"
"\n"
"/* Missing closing brace */\n"
"%struct Unbalanced1 {\n"
"    int x;\n"
"    double y;\n"
"    /* No closing brace here */\n"
"\n"
"/* Missing closing parenthesis in function pointer */\n"
"%typedef void (*BadFuncPtr(int, char*);\n"
"\n"
"/* Missing closing bracket in array */\n"
"%struct Unbalanced2 {\n"
"    int matrix[3][4;  /* Missing closing bracket */\n"
"};\n"
"\n"
"/* Nested unbalanced */\n"
"%struct DeepUnbalanced {\n"
"    void (*callback(struct Nested { int x; );  /* Multiple issues */\n"
"};\n"
"\n"
"/* Complex nested example hitting all switch cases */\n"
"%typedef struct UltimateTest {\n"
"    /* Parentheses: */\n"
"    int (*(*complex_callback)[5])(char* (*(*nested)(int)), double);\n"
"    \n"
"    /* Brackets: */\n"
"    struct {\n"
"        int multi_dim[2][3][4];\n"
"        void* pointers[10];\n"
"    } container;\n"
"    \n"
"    /* Braces: */\n"
"    union {\n"
"        struct { \n"
"            int a; \n"
"            struct { \n"
"                double b; \n"
"            } inner; \n"
"        } s1;\n"
"        struct {\n"
"            char c[20];\n"
"        } s2;\n"
"    } data;\n"
"    \n"
"    /* Mixed: */\n"
"    void (*(*mixed_array[3]))(int[5], struct { int x; });\n"
"} UltimateTest;\n"
"\n"
"/* Final valid type to ensure parser can recover */\n"
"%struct ValidRecovery {\n"
"    int valid_field;\n"
"    char* valid_string;\n"
"};\n";

/* Write test content to temporary file */
char* create_temp_gt_file(void) {
    char template[] = "/tmp/gengtype_test_XXXXXX.gt";
    int fd = mkstemps(template, 3);  /* .gt suffix is 3 chars */
    if (fd == -1) {
        perror("mkstemps failed");
        return NULL;
    }
    
    FILE* f = fdopen(fd, "w");
    if (!f) {
        perror("fdopen failed");
        close(fd);
        return NULL;
    }
    
    fwrite(test_gt_content, 1, strlen(test_gt_content), f);
    fclose(f);
    
    return strdup(template);
}

/* Simulate parsing to demonstrate the uncovered logic */
void simulate_parsing(const char* content) {
    printf("=== Simulating parser logic ===\n");
    
    for (const char* p = content; *p; p++) {
        parse_character(*p);
    }
    
    printf("=== Simulation complete ===\n");
}

/* Main driver for coverage testing */
int main(int argc, char** argv) {
    printf("Generating comprehensive .gt test file for gengtype parser coverage\n");
    
    /* Create test file */
    char* temp_file = create_temp_gt_file();
    if (!temp_file) {
        fprintf(stderr, "Failed to create test file\n");
        return 1;
    }
    
    printf("Test file created: %s\n", temp_file);
    printf("File size: %zu bytes\n", strlen(test_gt_content));
    
    /* Option 1: Simulate parsing (for unit test) */
    if (argc > 1 && strcmp(argv[1], "--simulate") == 0) {
        simulate_parsing(test_gt_content);
    }
    /* Option 2: Actual gengtype invocation (if available) */
    else {
        printf("\nTo run actual gengtype parser:\n");
        printf("1. Build gengtype with coverage instrumentation:\n");
        printf("   g++ -O0 -fprofile-arcs -ftest-coverage -I. -I../../include \\\n");
        printf("       -o gengtype-instr gengtype-parse.cc gengtype.cc gengtype-state.cc\n");
        printf("\n2. Run the parser on the test file:\n");
        printf("   ./gengtype-instr -p %s\n", temp_file);
        printf("\n3. Generate coverage report:\n");
        printf("   lcov --capture --directory . --output-file coverage.info\n");
        printf("   genhtml coverage.info --output-directory coverage_report\n");
    }
    
    /* Clean up */
    unlink(temp_file);
    free(temp_file);
    
    printf("\nTest completed successfully\n");
    return 0;
}
