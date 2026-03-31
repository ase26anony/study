/* gengtype_coverage_test.c - ISO C99 compliant test generator for gengtype parser */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* Parser stub for testing - simulates the actual gengtype parser logic */
typedef enum {
    TOK_EOF,
    TOK_CHAR,
    TOK_ERROR
} token_type;

static const char *input_ptr;
static int line_num = 1;

static void advance(void) {
    if (*input_ptr) input_ptr++;
}

static void consume_balanced(char open, char close) {
    int depth = 1;
    advance(); /* Skip opening delimiter */
    
    while (*input_ptr && depth > 0) {
        switch (*input_ptr) {
            case '(':
                if (open == '(') depth++;
                break;
            case ')':
                if (close == ')') depth--;
                break;
            case '[':
                if (open == '[') depth++;
                break;
            case ']':
                if (close == ']') depth--;
                break;
            case '{':
                if (open == '{') depth++;
                break;
            case '}':
                if (close == '}') depth--;
                break;
            case '\'':
            case '"':
                /* Skip character/string literals */
                advance();
                while (*input_ptr && *input_ptr != (*(input_ptr-1))) {
                    if (*input_ptr == '\\') advance();
                    advance();
                }
                if (*input_ptr) advance();
                continue;
            case '/':
                if (*(input_ptr + 1) == '*') {
                    /* Skip block comment */
                    advance(); advance();
                    while (*input_ptr && !(*input_ptr == '*' && *(input_ptr + 1) == '/')) {
                        if (*input_ptr == '\n') line_num++;
                        advance();
                    }
                    if (*input_ptr) advance();
                    if (*input_ptr) advance();
                } else if (*(input_ptr + 1) == '/') {
                    /* Skip line comment */
                    while (*input_ptr && *input_ptr != '\n') advance();
                }
                break;
        }
        if (*input_ptr == '\n') line_num++;
        advance();
    }
}

static token_type get_next_token(void) {
    while (*input_ptr) {
        switch (*input_ptr) {
            case ' ': case '\t': case '\r': case '\n':
                if (*input_ptr == '\n') line_num++;
                advance();
                continue;
            case '(':
                consume_balanced('(', ')');
                continue;
            case '[':
                consume_balanced('[', ']');
                continue;
            case '{':
                consume_balanced('{', '}');
                continue;
            default:
                advance();
                return TOK_CHAR;
        }
    }
    return TOK_EOF;
}

/* Test the parser with the provided input */
static void test_parser(const char *input) {
    input_ptr = input;
    line_num = 1;
    
    printf("Testing parser with %zu bytes of input...\n", strlen(input));
    
    while (get_next_token() != TOK_EOF) {
        /* Token processing happens in get_next_token */
    }
    
    printf("Parser completed. Processed %d lines.\n", line_num);
}

/* Generate a complex .gt test file with balanced constructs */
static char* generate_test_gt_content(void) {
    /* This builds a complex .gt file with all required patterns */
    static char buffer[16384];
    char *ptr = buffer;
    
    ptr += sprintf(ptr, "/* Test .gt file for gengtype parser coverage */\n");
    ptr += sprintf(ptr, "/* Target: lines 341-352 in gengtype-parse.cc */\n\n");
    
    /* 1. Balanced Construct Nesting with all delimiter pairs */
    ptr += sprintf(ptr, "/* Complex nested structures */\n");
    ptr += sprintf(ptr, "%%struct TreeNode {\n");
    ptr += sprintf(ptr, "  struct TreeNode *left;\n");
    ptr += sprintf(ptr, "  struct TreeNode *right;\n");
    ptr += sprintf(ptr, "  int data[10];\n");
    ptr += sprintf(ptr, "  void (*compare)(struct TreeNode *other);\n");
    ptr += sprintf(ptr, "};\n\n");
    
    /* Pointer-to-function syntax with parentheses */
    ptr += sprintf(ptr, "/* Function pointer types */\n");
    ptr += sprintf(ptr, "%%typedef int (*Comparator)(const void *, const void *);\n");
    ptr += sprintf(ptr, "%%typedef void (*ComplexFunc)(int (*)(double), struct { int x; });\n\n");
    
    /* Deeply nested constructs */
    ptr += sprintf(ptr, "/* Deep nesting */\n");
    ptr += sprintf(ptr, "%%struct DeepNest {\n");
    ptr += sprintf(ptr, "  int matrix[3][3][3];  /* Triple array */\n");
    ptr += sprintf(ptr, "  union {\n");
    ptr += sprintf(ptr, "    struct { int a; } s;\n");
    ptr += sprintf(ptr, "    int arr[5];\n");
    ptr += sprintf(ptr, "  } u;\n");
    ptr += sprintf(ptr, "  void (*funcs[5])(int, char);\n");
    ptr += sprintf(ptr, "};\n\n");
    
    /* 2. Unbalanced Edge Cases (for error handling) */
    ptr += sprintf(ptr, "/* Unbalanced constructs - should trigger error handling */\n");
    ptr += sprintf(ptr, "%%struct Unbalanced1 {\n");
    ptr += sprintf(ptr, "  int x;\n");
    ptr += sprintf(ptr, "  /* Missing closing brace here - parser should recover */\n\n");
    
    ptr += sprintf(ptr, "%%struct Unbalanced2 {\n");
    ptr += sprintf(ptr, "  int arr[10;  /* Missing closing bracket */\n");
    ptr += sprintf(ptr, "};\n\n");
    
    ptr += sprintf(ptr, "%%struct Unbalanced3 {\n");
    ptr += sprintf(ptr, "  void (*func( int x );  /* Mismatched parentheses */\n");
    ptr += sprintf(ptr, "};\n\n");
    
    /* 3. Comments and Macros Interleaving */
    ptr += sprintf(ptr, "/* Comments with balanced delimiters inside */\n");
    ptr += sprintf(ptr, "/* This comment has (parentheses) [brackets] {braces} */\n");
    ptr += sprintf(ptr, "// Line comment with [brackets] and {braces}\n\n");
    
    ptr += sprintf(ptr, "/* Preprocessor-like macros */\n");
    ptr += sprintf(ptr, "#define ARRAY_TYPE(T) struct { T data[10]; }\n");
    ptr += sprintf(ptr, "#define FUNC_PTR(RET) RET (*)(int, char)\n");
    ptr += sprintf(ptr, "#define NESTED_MACRO(x) { { x }, [0] = (x) }\n\n");
    
    ptr += sprintf(ptr, "/* Using the macros */\n");
    ptr += sprintf(ptr, "%%typedef ARRAY_TYPE(int) IntArray;\n");
    ptr += sprintf(ptr, "%%typedef FUNC_PTR(void) VoidFuncPtr;\n\n");
    
    /* 4. GT File Specific Constructs with annotations */
    ptr += sprintf(ptr, "/* GC roots with complex types */\n");
    ptr += sprintf(ptr, "GC roots {\n");
    ptr += sprintf(ptr, "  struct TreeNode *tree_root;\n");
    ptr += sprintf(ptr, "  struct {\n");
    ptr += sprintf(ptr, "    int count;\n");
    ptr += sprintf(ptr, "    void *data[20];\n");
    ptr += sprintf(ptr, "  } storage;\n");
    ptr += sprintf(ptr, "}\n\n");
    
    ptr += sprintf(ptr, "/* Union with nested structures */\n");
    ptr += sprintf(ptr, "%%union ComplexUnion {\n");
    ptr += sprintf(ptr, "  struct {\n");
    ptr += sprintf(ptr, "    int x;\n");
    ptr += sprintf(ptr, "    double y;\n");
    ptr += sprintf(ptr, "  } point;\n");
    ptr += sprintf(ptr, "  int matrix[2][2];\n");
    ptr += sprintf(ptr, "  void (*callback)(int result);\n");
    ptr += sprintf(ptr, "};\n\n");
    
    /* 5. Multiple Top-Level Declarations */
    ptr += sprintf(ptr, "/* Enum with complex initializers */\n");
    ptr += sprintf(ptr, "%%enum Flags {\n");
    ptr += sprintf(ptr, "  FLAG_A = (1 << 0),\n");
    ptr += sprintf(ptr, "  FLAG_B = (1 << 1),\n");
    ptr += sprintf(ptr, "  FLAG_C = (1 << 2) | (1 << 3)\n");
    ptr += sprintf(ptr, "};\n\n");
    
    ptr += sprintf(ptr, "/* Typedef with function pointer array */\n");
    ptr += sprintf(ptr, "%%typedef struct Handler {\n");
    ptr += sprintf(ptr, "  const char *name;\n");
    ptr += sprintf(ptr, "  int (*operations[5])(void *ctx);\n");
    ptr += sprintf(ptr, "  union {\n");
    ptr += sprintf(ptr, "    int i;\n");
    ptr += sprintf(ptr, "    double d;\n");
    ptr += sprintf(ptr, "  } value;\n");
    ptr += sprintf(ptr, "} Handler;\n\n");
    
    ptr += sprintf(ptr, "/* Another struct with all delimiter types */\n");
    ptr += sprintf(ptr, "%%struct AllDelimiters {\n");
    ptr += sprintf(ptr, "  int a;                     /* Simple member */\n");
    ptr += sprintf(ptr, "  int b[10];                 /* Array brackets */\n");
    ptr += sprintf(ptr, "  struct { int x; } c;       /* Nested braces */\n");
    ptr += sprintf(ptr, "  void (*d)(int);            /* Function ptr parens */\n");
    ptr += sprintf(ptr, "  int (*e[3])(char *str);    /* Array of func ptrs */\n");
    ptr += sprintf(ptr, "  union {\n");
    ptr += sprintf(ptr, "    int f;\n");
    ptr += sprintf(ptr, "    struct { int g; } h;\n");
    ptr += sprintf(ptr, "  } i;\n");
    ptr += sprintf(ptr, "};\n\n");
    
    ptr += sprintf(ptr, "/* End of test file */\n");
    
    return buffer;
}

/* Generate a minimal .gt file for quick testing */
static char* generate_minimal_gt_content(void) {
    static char buffer[2048];
    char *ptr = buffer;
    
    ptr += sprintf(ptr, "/* Minimal test for consume_balanced calls */\n");
    ptr += sprintf(ptr, "%%struct Test {\n");
    ptr += sprintf(ptr, "  int a;          /* Member */\n");
    ptr += sprintf(ptr, "  int b[5];       /* Array - triggers '[' case */\n");
    ptr += sprintf(ptr, "  struct {        /* Nested struct - triggers '{' case */\n");
    ptr += sprintf(ptr, "    int x;\n");
    ptr += sprintf(ptr, "  } c;\n");
    ptr += sprintf(ptr, "  void (*d)(int); /* Function ptr - triggers '(' case */\n");
    ptr += sprintf(ptr, "};\n");
    
    return buffer;
}

int main(void) {
    char *test_content;
    char tmp_filename[] = "/tmp/gengtype_test_XXXXXX.gt";
    int fd;
    FILE *fp;
    
    printf("=== GCC gengtype Parser Coverage Test ===\n\n");
    
    /* Create temporary file */
    fd = mkstemps(tmp_filename, 3);
    if (fd == -1) {
        perror("Failed to create temporary file");
        return 1;
    }
    close(fd);
    
    /* Write test content to file */
    fp = fopen(tmp_filename, "w");
    if (!fp) {
        perror("Failed to open temporary file");
        return 1;
    }
    
    test_content = generate_test_gt_content();
    fprintf(fp, "%s", test_content);
    fclose(fp);
    
    printf("Generated test file: %s\n", tmp_filename);
    printf("File size: %zu bytes\n\n", strlen(test_content));
    
    /* Test with the parser stub */
    printf("--- Testing with parser stub ---\n");
    test_parser(test_content);
    
    /* Also test minimal case */
    printf("\n--- Testing minimal case ---\n");
    test_parser(generate_minimal_gt_content());
    
    /* Print instructions for actual gengtype testing */
    printf("\n=== To test with actual gengtype ===\n");
    printf("1. Build gengtype with coverage instrumentation:\n");
    printf("   g++ -O0 -fprofile-arcs -ftest-coverage -I. -I../../include \\\n");
    printf("       -o gengtype-instr gengtype-parse.cc gengtype.cc gengtype-state.cc\n\n");
    printf("2. Run gengtype on the test file:\n");
    printf("   ./gengtype-instr -p %s\n\n", tmp_filename);
    printf("3. Generate coverage report:\n");
    printf("   gcov gengtype-parse.cc\n");
    
    /* Clean up */
    unlink(tmp_filename);
    
    return 0;
}
