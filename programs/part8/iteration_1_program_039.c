/* gengtype_coverage_test.c - ISO C99 compliant test generator for gengtype parser */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

/* Simplified parser stub for testing */
typedef struct {
    const char *input;
    size_t pos;
    int line;
    int column;
} parser_state;

static void advance(parser_state *ps) {
    if (ps->input[ps->pos] != '\0') {
        if (ps->input[ps->pos] == '\n') {
            ps->line++;
            ps->column = 1;
        } else {
            ps->column++;
        }
        ps->pos++;
    }
}

static void consume_balanced(parser_state *ps, char open, char close) {
    int depth = 1;
    advance(ps); /* Consume opening delimiter */
    
    while (depth > 0 && ps->input[ps->pos] != '\0') {
        switch (ps->input[ps->pos]) {
            case '(':
                if (open == '(') depth++;
                advance(ps);
                break;
            case ')':
                if (close == ')') depth--;
                advance(ps);
                break;
            case '[':
                if (open == '[') depth++;
                advance(ps);
                break;
            case ']':
                if (close == ']') depth--;
                advance(ps);
                break;
            case '{':
                if (open == '{') depth++;
                advance(ps);
                break;
            case '}':
                if (close == '}') depth--;
                advance(ps);
                break;
            case '\'':
            case '"':
                /* Skip character/string literals */
                advance(ps);
                while (ps->input[ps->pos] != '\0' && 
                       ps->input[ps->pos] != ps->input[ps->pos-1]) {
                    if (ps->input[ps->pos] == '\\') advance(ps);
                    advance(ps);
                }
                if (ps->input[ps->pos] != '\0') advance(ps);
                break;
            default:
                advance(ps);
                break;
        }
    }
}

static void parse_gt_content(const char *content) {
    parser_state ps = {content, 0, 1, 1};
    
    while (ps.input[ps.pos] != '\0') {
        switch (ps.input[ps.pos]) {
            default:
                advance(&ps);
                break;
            case '(':
                consume_balanced(&ps, '(', ')');
                break;
            case '[':
                consume_balanced(&ps, '[', ']');
                break;
            case '{':
                consume_balanced(&ps, '{', '}');
                break;
        }
    }
}

/* Complex .gt test content with all required patterns */
static const char *gt_test_content = 
"/* Test file for gengtype parser coverage */\n"
"/* Balanced construct nesting with all delimiter types */\n"
"\n"
"/* 1. Basic type definitions with parentheses */\n"
"%typedef struct tree_node *tree_ptr;\n"
"%typedef int (*comparator_func)(const void *, const void *);\n"
"%typedef void (*complex_func)(int (*(*)(int))[5]);\n"
"\n"
"/* 2. Struct with nested balanced constructs */\n"
"%struct complex_struct {\n"
"    int matrix[3][4];\n"
"    struct {\n"
"        int (*func_ptr)(int, int);\n"
"        union {\n"
"            char *str;\n"
"            void *ptr;\n"
"        } u;\n"
"    } nested;\n"
"    struct tag {\n"
"        int arr[(sizeof(int) > 2) ? 10 : 5];\n"
"    } tagged;\n"
"};\n"
"\n"
"/* 3. Union with array and function pointers */\n"
"%union data_union {\n"
"    int (*array_of_funcs[5])(double);\n"
"    struct {\n"
"        char *name;\n"
"        int values[]; /* Flexible array member */\n"
"    } flex;\n"
"    void (*signal_handler)(int sig, void (*old)(int));\n"
"};\n"
"\n"
"/* 4. GC roots with complex type expressions */\n"
"GC roots {\n"
"    struct tree_node *global_tree /* Missing semicolon to test recovery */\n"
"    struct {\n"
"        int (*callbacks[10])(void *);\n"
"        union {\n"
"            struct { int x; int y; } point;\n"
"            int coords[2];\n"
"        } u;\n"
"    } graphics_state;\n"
"};\n"
"\n"
"/* 5. Macro definitions with balanced delimiters */\n"
"#define ARRAY_TYPE(T) struct { T data[10]; }\n"
"#define FUNC_PTR(RET, ARGS) RET (*)(ARGS)\n"
"#define NESTED_MACRO(x) struct { int a[(x)]; char b; }\n"
"\n"
"/* 6. Comments containing balanced delimiters */\n"
"/* This comment has (parentheses) [brackets] {braces} inside */\n"
"// Line comment with [unbalanced bracket /* nested */\n"
"\n"
"/* 7. Edge cases - unbalanced constructs for error handling */\n"
"%struct unbalanced_example {\n"
"    int missing_paren = (2 + 3 * (4 - 1); /* Missing closing paren */\n"
"    char bad_array[10 /* Missing closing bracket */\n"
"    struct { int x; ; /* Missing closing brace */\n"
"};\n"
"\n"
"/* 8. Complex nested example hitting all switch cases */\n"
"%typedef struct {\n"
"    void (*(*signal_table[256])(int))(int);\n"
"    union {\n"
"        struct {\n"
"            int (*matrix_ops[3][3])(double[][3]);\n"
"            char *strings[];\n"
"        } ops;\n"
"        int (*simple_func)(void);\n"
"    } u;\n"
"} handler_set;\n"
"\n"
"/* 9. Multiple top-level declarations */\n"
"%enum color { RED = (1 << 0), GREEN = (1 << 1), BLUE = (1 << 2) };\n"
"%struct rgb_triple {\n"
"    enum color colors[3];\n"
"    float alpha;\n"
"};\n"
"\n"
"/* 10. Pointer to array of function pointers */\n"
"%typedef int (*(*complex_array_ptr)[10])(double, char *);\n"
"\n"
"/* End of test file */\n";

/* Additional test with different patterns */
static const char *gt_test_content2 = 
"/* Test with preprocessor conditionals */\n"
"#ifdef __GNUC__\n"
"%struct gcc_specific {\n"
"    __attribute__((aligned(16))) int vector[4];\n"
"};\n"
"#endif\n"
"\n"
"/* Template-like macro usage */\n"
"#define CONTAINER(T, N) struct { T items[N]; size_t count; }\n"
"%typedef CONTAINER(void *, 100) ptr_buffer;\n"
"\n"
"/* Nested in comments and strings */\n"
"char *example = \"String with (parentheses) [and] {braces}\";\n"
"/* Comment with /* nested comment */ still parsing */\n"
"\n"
"/* Very deep nesting */\n"
"%struct deeply_nested {\n"
"    int a[ [ [ [ [ 5 ] ] ] ] ]; /* Multiple brackets */\n"
"    void (*(*(*chain)())[10])();\n"
"};\n";

int main(void) {
    FILE *fp;
    const char *filename = "gengtype_coverage_test.gt";
    
    /* Write test .gt file */
    fp = fopen(filename, "w");
    if (!fp) {
        perror("Failed to create test file");
        return 1;
    }
    
    fprintf(fp, "%s", gt_test_content);
    fclose(fp);
    
    printf("Generated test file: %s\n", filename);
    
    /* Parse the content using our stub parser */
    printf("Parsing test content...\n");
    parse_gt_content(gt_test_content);
    parse_gt_content(gt_test_content2);
    
    /* Create a second test file with different patterns */
    fp = fopen("gengtype_coverage_test2.gt", "w");
    if (fp) {
        fprintf(fp, "%s", gt_test_content2);
        fclose(fp);
        printf("Generated second test file: gengtype_coverage_test2.gt\n");
    }
    
    /* Create a minimal valid .gt file for basic testing */
    const char *minimal_gt = 
    "%typedef int my_int;\n"
    "%struct simple { int x; };\n"
    "GC roots { struct simple *root; };\n";
    
    fp = fopen("minimal_test.gt", "w");
    if (fp) {
        fprintf(fp, "%s", minimal_gt);
        fclose(fp);
    }
    
    printf("\nTest files created. To run actual gengtype:\n");
    printf("  g++ -O2 -g -I. -I../../include -o gengtype gengtype-parse.cc gengtype.cc gengtype-state.cc\n");
    printf("  ./gengtype -p gengtype_coverage_test.gt\n\n");
    
    printf("For coverage analysis:\n");
    printf("  g++ -O0 -fprofile-arcs -ftest-coverage -I. -I../../include \\\n");
    printf("      -o gengtype-instr gengtype-parse.cc gengtype.cc gengtype-state.cc\n");
    printf("  ./gengtype-instr -p gengtype_coverage_test.gt\n");
    printf("  gcov gengtype-parse.cc\n");
    
    return 0;
}
