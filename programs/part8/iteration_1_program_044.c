/* gengtype_coverage_test.c - ISO C99 compliant test generator for gengtype parser */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

/* Test 1: Complex balanced constructs with all delimiter types */
static const char *test1_content =
    "/* Test file for gengtype parser coverage */\n"
    "\n"
    "/* Requirement 1: Balanced construct nesting */\n"
    "%struct ComplexType {\n"
    "    int (*func_ptr)(int, char);  /* Function pointer with parentheses */\n"
    "    int (*nested_func[5])(struct { int x; double y; });\n"
    "    union {\n"
    "        struct {\n"
    "            int matrix[3][4];  /* Nested arrays */\n"
    "            void (*callback)(void);\n"
    "        } s;\n"
    "        int (*arr_of_funcs[10])(double, float);\n"
    "    } u;\n"
    "    struct Node* (*find_node)(struct Tree* (*(*get_tree)(void))());\n"
    "};\n"
    "\n"
    "/* Deeply nested parentheses */\n"
    "typedef int (*(*(*complex_func_ptr)(int[][5]))(void))(char);\n"
    "\n"
    "/* Multiple bracket levels */\n"
    "%struct MultiArray {\n"
    "    unsigned char data[256][128][64];\n"
    "    int (*process[8][4])(float samples[][1024]);\n"
    "};\n"
    "\n"
    "/* Nested braces in unions */\n"
    "%union DeepUnion {\n"
    "    struct {\n"
    "        struct {\n"
    "            struct { int a; char b; } inner;\n"
    "        } middle;\n"
    "    } outer;\n"
    "    int values[5];\n"
    "};\n";

/* Test 2: Unbalanced delimiters for error handling coverage */
static const char *test2_content =
    "/* Requirement 2: Unbalanced edge cases */\n"
    "%struct Unbalanced1 {  /* Missing closing brace */\n"
    "    int x;\n"
    "    char y;\n"
    "    /* The struct never closes */\n"
    "\n"
    "%struct Unbalanced2 {\n"
    "    int arr[10;  /* Missing closing bracket */\n"
    "    void (*func)(int;  /* Missing closing paren */\n"
    "};\n"
    "\n"
    "%union Mismatched {\n"
    "    struct { int a; );  /* Wrong closing delimiter */\n"
    "    float b[5);\n"
    "};\n";

/* Test 3: Comments and macros interleaving with delimiters */
static const char *test3_content =
    "/* Requirement 3: Comments and macros */\n"
    "#define ARRAY_TYPE(T) struct { T data[10]; }\n"
    "#define FUNC_PTR(R, A) R (*)(A)\n"
    "#define NESTED_STRUCT struct { union { int x; }; }\n"
    "\n"
    "/* Parentheses in comments should be ignored */\n"
    "/* int (*ignored_func)(void); */\n"
    "/* [1, 2, 3] */\n"
    "/* { a: 1, b: 2 } */\n"
    "\n"
    "// Line comment with delimiters: ( ) [ ] { }\n"
    "// Should not affect parsing\n"
    "\n"
    "%struct WithComments {\n"
    "    int value;  /* Trailing comment with (parentheses) */\n"
    "    /* Embedded comment with [brackets] */\n"
    "    char name[20];\n"
    "    // Line comment inside struct\n"
    "    void (*handler)(int);\n"
    "};\n"
    "\n"
    "/* Macro expansion with balanced delimiters */\n"
    "ARRAY_TYPE(int) int_array;\n"
    "FUNC_PTR(int, char*) string_processor;\n"
    "NESTED_STRUCT nested;\n";

/* Test 4: GT-specific annotations with embedded delimiters */
static const char *test4_content =
    "/* Requirement 4: GT file specific constructs */\n"
    "%typedef struct {\n"
    "    int (*compare)(const void*, const void*);\n"
    "    void* data[100];\n"
    "} SortedContainer;\n"
    "\n"
    "%union GCTaggedUnion {\n"
    "    struct {\n"
    "        int tag;\n"
    "        union {\n"
    "            int ival;\n"
    "            double fval;\n"
    "            char* sval;\n"
    "        } u;\n"
    "    } tagged;\n"
    "    long long bits;\n"
    "};\n"
    "\n"
    "/* GC roots with complex types */\n"
    "GC roots {\n"
    "    struct GlobalState* state;\n"
    "    void* (*allocators[5])(size_t);\n"
    "    union {\n"
    "        struct Tree* trees[100];\n"
    "        struct Graph* graphs[50];\n"
    "    } storage;\n"
    "}\n"
    "\n"
    "%struct GCManaged {\n"
    "    %union {\n"
    "        struct Node* left;\n"
    "        struct Node* right;\n"
    "    } children;\n"
    "    int depth;\n"
    "};\n";

/* Test 5: Multiple top-level declarations for repeated parsing */
static const char *test5_content =
    "/* Requirement 5: Multiple top-level declarations */\n"
    "enum Color { RED, GREEN, BLUE };\n"
    "\n"
    "typedef enum { MON, TUE, WED, THU, FRI } Weekday;\n"
    "\n"
    "%struct First {\n"
    "    int a;\n"
    "    char b[10];\n"
    "};\n"
    "\n"
    "%union Second {\n"
    "    float x;\n"
    "    double y;\n"
    "    struct { int p; char q; } z;\n"
    "};\n"
    "\n"
    "typedef int (*Comparator)(void*, void*);\n"
    "\n"
    "%struct Third {\n"
    "    Comparator cmp;\n"
    "    void* array[20];\n"
    "    union {\n"
    "        int ival;\n"
    "        float fval;\n"
    "    } value;\n"
    "};\n"
    "\n"
    "GC roots {\n"
    "    struct First* first;\n"
    "    %union Second* second;\n"
    "    %struct Third* third;\n"
    "}\n";

/* Combined test with all requirements */
static const char *combined_test_content =
    "/* Comprehensive test covering all requirements */\n"
    "\n"
    "/* 1. Balanced nesting */\n"
    "%struct MasterType {\n"
    "    /* Function pointer with complex signature */\n"
    "    void (*(*signal_handler[5])(int signum))(void);\n"
    "    \n"
    "    /* Nested arrays */\n"
    "    int multi_dim[3][4][5];\n"
    "    \n"
    "    /* Struct within union within struct */\n"
    "    union {\n"
    "        struct {\n"
    "            struct {\n"
    "                int deep_value;\n"
    "                char (*name_func)(int index[10]);\n"
    "            } inner;\n"
    "        } middle;\n"
    "        float simple;\n"
    "    } choice;\n"
    "};\n"
    "\n"
    "/* 2. Some unbalanced cases for error recovery */\n"
    "%struct BadType1 {\n"
    "    int missing_paren_func(void;  /* Error here */\n"
    "    char arr[5;  /* Another error */\n"
    "};\n"
    "\n"
    "/* 3. In comments and macros */\n"
    "#define WRAP(T) struct Wrapper { T item; }\n"
    "/* Ignored: (test) [test] {test} */\n"
    "// Also ignored: ( ) [ ] { }\n"
    "\n"
    "/* 4. GT annotations */\n"
    "%typedef WRAP(int*) IntPtrWrapper;\n"
    "%union GUnion {\n"
    "    struct { int x; int y; } point;\n"
    "    int coords[2];\n"
    "};\n"
    "\n"
    "/* 5. Multiple declarations */\n"
    "enum Size { SMALL, MEDIUM, LARGE };\n"
    "%struct Another { enum Size size; };\n"
    "typedef %struct MasterType* MasterPtr;\n";

/* Write content to a temporary file */
static int write_temp_file(const char *content, const char *filename) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        fprintf(stderr, "Failed to create %s: %s\n", filename, strerror(errno));
        return -1;
    }
    
    fputs(content, f);
    fclose(f);
    return 0;
}

/* Simple parser simulation to directly test the switch logic */
static void simulate_parser(const char *input) {
    const char *p = input;
    int paren_depth = 0;
    int bracket_depth = 0;
    int brace_depth = 0;
    
    printf("Simulating parser on input...\n");
    
    while (*p) {
        switch (*p) {
            case '(':
                printf("  Found '(' at position %ld\n", (long)(p - input));
                paren_depth++;
                /* Simulate consume_balanced */
                {
                    const char *q = p + 1;
                    int depth = 1;
                    while (*q && depth > 0) {
                        if (*q == '(') depth++;
                        else if (*q == ')') depth--;
                        q++;
                    }
                    if (depth == 0) {
                        printf("    Balanced parentheses consumed, length: %ld\n", 
                               (long)(q - p - 1));
                    }
                }
                break;
                
            case '[':
                printf("  Found '[' at position %ld\n", (long)(p - input));
                bracket_depth++;
                /* Simulate consume_balanced */
                {
                    const char *q = p + 1;
                    int depth = 1;
                    while (*q && depth > 0) {
                        if (*q == '[') depth++;
                        else if (*q == ']') depth--;
                        q++;
                    }
                    if (depth == 0) {
                        printf("    Balanced brackets consumed, length: %ld\n",
                               (long)(q - p - 1));
                    }
                }
                break;
                
            case '{':
                printf("  Found '{' at position %ld\n", (long)(p - input));
                brace_depth++;
                /* Simulate consume_balanced */
                {
                    const char *q = p + 1;
                    int depth = 1;
                    while (*q && depth > 0) {
                        if (*q == '{') depth++;
                        else if (*q == '}') depth--;
                        q++;
                    }
                    if (depth == 0) {
                        printf("    Balanced braces consumed, length: %ld\n",
                               (long)(q - p - 1));
                    }
                }
                break;
                
            case ')':
                paren_depth--;
                break;
            case ']':
                bracket_depth--;
                break;
            case '}':
                brace_depth--;
                break;
                
            default:
                /* advance() simulation */
                break;
        }
        p++;
    }
    
    printf("Final depths: parens=%d, brackets=%d, braces=%d\n",
           paren_depth, bracket_depth, brace_depth);
}

int main(void) {
    const char *temp_files[] = {
        "test1_balanced.gt",
        "test2_unbalanced.gt", 
        "test3_comments.gt",
        "test4_annotations.gt",
        "test5_multiple.gt",
        "combined_test.gt"
    };
    
    const char *contents[] = {
        test1_content,
        test2_content,
        test3_content,
        test4_content,
        test5_content,
        combined_test_content
    };
    
    int num_tests = sizeof(temp_files) / sizeof(temp_files[0]);
    int i;
    
    printf("Generating gengtype test files for parser coverage...\n");
    
    /* Create all test files */
    for (i = 0; i < num_tests; i++) {
        printf("Creating %s...\n", temp_files[i]);
        if (write_temp_file(contents[i], temp_files[i]) < 0) {
            return EXIT_FAILURE;
        }
    }
    
    /* Simulate parser on the combined test */
    printf("\n=== Parser Simulation ===\n");
    simulate_parser(combined_test_content);
    
    /* Instructions for actual gengtype execution */
    printf("\n=== To run actual gengtype parser ===\n");
    printf("1. Build gengtype with coverage instrumentation:\n");
    printf("   g++ -O0 -fprofile-arcs -ftest-coverage -I. -I../../include \\\n");
    printf("       -o gengtype-instr gengtype-parse.cc gengtype.cc gengtype-state.cc\n");
    printf("\n2. Run on generated test files:\n");
    for (i = 0; i < num_tests; i++) {
        printf("   ./gengtype-instr -p %s\n", temp_files[i]);
    }
    printf("\n3. Generate coverage report:\n");
    printf("   gcov gengtype-parse.cc\n");
    
    /* Cleanup */
    printf("\nTest files created. Clean up with:\n");
    printf("  rm -f *.gt *.gcda *.gcno gengtype-instr\n");
    
    return EXIT_SUCCESS;
}
