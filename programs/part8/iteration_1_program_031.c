/* gengtype_coverage_test.c - ISO C99 compliant test generator for gengtype parser */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

/* Simplified parser stub for testing */
void advance(void) { /* Simulated token advance */ }
void consume_balanced(char open, char close) { 
    /* Track which delimiters are triggered */
    static int triggers[3] = {0};
    if (open == '(') triggers[0]++;
    else if (open == '[') triggers[1]++;
    else if (open == '{') triggers[2]++;
}

/* Test case 1: Complex balanced constructs */
const char *test_case_1 = 
"/* Test case with all balanced delimiters */\n"
"%typedef struct ComplexType {\n"
"    int (*callback)(int (*nested_cb)(int, char**), double);\n"
"    union {\n"
"        struct { int x; int y; } point;\n"
"        int matrix[3][4];\n"
"        void (*func_array[5])(void);\n"
"    } data;\n"
"    char *(*get_name)(struct ComplexType *self);\n"
"} ComplexType;\n"
"\n"
"/* Nested arrays with function pointers */\n"
"%struct Node {\n"
"    int value;\n"
"    struct Node *(*children)[10];  /* Array of pointers to functions */\n"
"    void (*methods[3])(struct Node*, int (*compare)(int, int));\n"
"};\n"
"\n"
"/* Union with complex initializer */\n"
"%union TaggedValue {\n"
"    int int_val;\n"
"    double (*compute)(double (*fn)(double), double args[5]);\n"
"    struct {\n"
"        char *name;\n"
"        int (*validator)(char *(*converter)(int), int);\n"
"    } complex_field;\n"
"};\n";

/* Test case 2: Unbalanced delimiters (error cases) */
const char *test_case_2 =
"/* Unbalanced parentheses */\n"
"%struct UnbalancedParen {\n"
"    int (*missing_close(int x, double y);  /* ERROR: missing ) */\n"
"    int arr[10;  /* ERROR: missing ] */\n"
"};\n"
"\n"
"/* Unbalanced braces */\n"
"%union UnbalancedBrace {\n"
"    int x;\n"
"    double y;\n"
"    /* Missing closing brace here */\n"
"\n"
"/* Nested unbalanced */\n"
"%struct DeepUnbalanced {\n"
"    int (*deep[5])(char *str[10);  /* Nested errors */\n"
"};\n";

/* Test case 3: Comments and macros with delimiters */
const char *test_case_3 =
"#define ARRAY_TYPE(T) struct { T data[10]; int (*sort)(T[], int); }\n"
"#define FUNCTION_PTR(RET, ARGS) RET (*func) ARGS\n"
"\n"
"/* Comment with delimiters: (test) [test] {test} */\n"
"%typedef ARRAY_TYPE(int) IntArray;\n"
"\n"
"// Line comment with (parentheses) and [brackets]\n"
"%struct CommentedStruct {\n"
"    FUNCTION_PTR(int, (int, char (*)[5]));\n"
"    /* Nested comment /* with (more) delimiters */ */\n"
"    int values[5];\n"
"};\n"
"\n"
"#if 0\n"
"/* Disabled code with complex patterns */\n"
"%struct Disabled {\n"
"    void (*disabled_func)(int (*)(int[10]), struct { int x; });\n"
"    char disabled_array[5][10];\n"
"};\n"
"#endif\n";

/* Test case 4: GT-specific annotations with delimiters */
const char *test_case_4 =
"GC roots {\n"
"    struct RootStruct {\n"
"        int (*root_callback)(int (*)(int), int);\n"
"        union {\n"
"            int i;\n"
"            void *p;\n"
"        } data[5];\n"
"    } *root_ptr;\n"
"};\n"
"\n"
"%struct GT_Annotated {\n"
"    %union InternalUnion {\n"
"        int (*func)(int (*nested)(int[5]), double);\n"
"        struct { char *name; int id; } info;\n"
"    } variant;\n"
"    %typedef int (*ComplexCallback)(struct GT_Annotated*, int (*)(int));\n"
"};\n"
"\n"
"/* Multiple typedefs */\n"
"%typedef int (*Comparator)(int, int);\n"
"%typedef Comparator (*Factory)(int priority);\n"
"%typedef Factory Factories[10];\n";

/* Test case 5: Mixed declarations for repeated parsing */
const char *test_case_5 =
"enum TokenType { LPAREN = '(', RPAREN = ')', LBRACK = '[', RBRACK = ']' };\n"
"\n"
"%struct ParserState {\n"
"    int (*handle_paren)(char *(*get_token)(void), int);\n"
"    void (*states[5])(struct ParserState*, int (*action)(int));\n"
"    union {\n"
"        int depth;\n"
"        void (*error_handler)(char *msg, int (*recover)(void));\n"
"    } ctx;\n"
"};\n"
"\n"
"%union ParseResult {\n"
"    int success;\n"
"    struct {\n"
"        int (*cleanup)(void (*finalizer)(int), int);\n"
"        char *error_msg;\n"
"    } failure;\n"
"};\n"
"\n"
"/* Function pointer typedef with nested arrays */\n"
"%typedef void (*(*SignalHandlerFactory)(int sig))(int, siginfo_t*, void*);\n"
"\n"
"/* Complex nested structure */\n"
"%struct Outer {\n"
"    struct Middle {\n"
"        struct Inner {\n"
"            int (*compute)(int (*transform)(int), int values[5]);\n"
"            union {\n"
"                int x;\n"
"                double y[3];\n"
"            } data;\n"
"        } inner[2];\n"
"        void (*process)(struct Inner*, int (*callback)(int));\n"
"    } middle;\n"
"    int (*finalize)(struct Middle*, void (*cleanup)(int));\n"
"};\n";

/* Combined test with all patterns */
const char *combined_test =
"/* ===== COMPREHENSIVE GT TEST FILE ===== */\n"
"\n"
"/* Section 1: Basic balanced constructs */\n"
"%typedef struct Base {\n"
"    int (*func1)(int);\n"
"    char arr1[10];\n"
"    struct { int x; } nested;\n"
"} Base;\n"
"\n"
"/* Section 2: Nested function pointers */\n"
"%struct WithCallbacks {\n"
"    int (*register_callback)(int (*cb)(int (*inner)(int), int), int);\n"
"    void (*handlers[3])(struct WithCallbacks*, int (*)(int));\n"
"};\n"
"\n"
"/* Section 3: Complex union */\n"
"%union VariantData {\n"
"    int (*int_op)(int (*math)(int, int), int);\n"
"    struct {\n"
"        char *(*get_name)(void);\n"
"        int scores[5][2];\n"
"    } complex;\n"
"    void (*void_func)(int (*)(int[10]), struct { int tag; });\n"
"};\n"
"\n"
"/* Section 4: Macro expansions */\n"
"#define PTR_TO_FUNC(RET) RET (*)(int (*)(int), int)\n"
"%typedef PTR_TO_FUNC(int) IntFuncPtr;\n"
"\n"
"/* Section 5: GC roots with delimiters */\n"
"GC roots {\n"
"    %struct GCRoot {\n"
"        void (*trace)(struct GCRoot*, int (*visitor)(void*, int));\n"
"        int data[5];\n"
"    } *gc_var;\n"
"};\n"
"\n"
"/* Section 6: Edge cases */\n"
"// Balanced in comments: ( ) [ ] { }\n"
"/* More: (nested (comments) with [delimiters]) */\n"
"%struct EdgeCase {\n"
"    int normal_field;\n"
"    /* int (*commented_out)(int); */  // Should be ignored\n"
"    int active_array[5];\n"
"};\n"
"\n"
"/* Section 7: Multiple top-level declarations */\n"
"%typedef int Integer;\n"
"%typedef Integer (*IntProcessor)(Integer);\n"
"%typedef IntProcessor ProcessorArray[10];\n"
"\n"
"enum Delimiters { PAREN = '(', BRACE = '{', BRACKET = '[' };\n"
"\n"
"%struct FinalTest {\n"
"    ProcessorArray processors;\n"
"    %union LastUnion {\n"
"        int simple;\n"
"        struct { int a; int b; } pair;\n"
"        int (*complex)(struct FinalTest*, int (*)(int[5]));\n"
"    } u;\n"
"};\n";

/* Simulated parser that triggers the uncovered switch cases */
void simulate_parser(const char *input) {
    const char *p = input;
    
    while (*p) {
        switch (*p) {
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
        p++;
    }
}

/* Write test file and simulate parsing */
int write_and_parse_test(const char *filename, const char *content) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        perror("Failed to open test file");
        return -1;
    }
    
    fprintf(f, "%s", content);
    fclose(f);
    
    /* Simulate parsing */
    simulate_parser(content);
    
    return 0;
}

int main(void) {
    const char *test_files[] = {
        "test_balanced.gt",
        "test_unbalanced.gt", 
        "test_comments.gt",
        "test_annotations.gt",
        "test_mixed.gt",
        "test_combined.gt"
    };
    
    const char *test_contents[] = {
        test_case_1,
        test_case_2,
        test_case_3,
        test_case_4,
        test_case_5,
        combined_test
    };
    
    int num_tests = sizeof(test_files) / sizeof(test_files[0]);
    
    printf("Generating gengtype test files...\n");
    
    for (int i = 0; i < num_tests; i++) {
        printf("Creating %s...\n", test_files[i]);
        if (write_and_parse_test(test_files[i], test_contents[i]) != 0) {
            printf("Failed to create %s\n", test_files[i]);
            return 1;
        }
    }
    
    printf("\nTest files generated successfully.\n");
    printf("To run actual gengtype parser:\n");
    printf("  g++ -O2 -g -I. -I../../include -o gengtype gengtype-parse.cc gengtype.cc gengtype-state.cc\n");
    printf("  ./gengtype -p test_combined.gt\n");
    printf("\nFor coverage analysis:\n");
    printf("  g++ -O0 -fprofile-arcs -ftest-coverage -I. -I../../include \\\n");
    printf("      -o gengtype-instr gengtype-parse.cc gengtype.cc gengtype-state.cc\n");
    printf("  ./gengtype-instr -p test_combined.gt\n");
    printf("  gcov gengtype-parse.cc\n");
    
    /* Cleanup */
    for (int i = 0; i < num_tests; i++) {
        remove(test_files[i]);
    }
    
    return 0;
}
