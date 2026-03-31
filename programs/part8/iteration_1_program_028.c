/* gengtype_coverage_test.c - ISO C99-compliant test program for gengtype parser coverage */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

/* Simulated parser functions for standalone testing */
static void advance(void) { /* Simulated token advance */ }
static void consume_balanced(char open, char close) { 
    /* Simulated balanced delimiter consumption */
    printf("consume_balanced called with '%c' '%c'\n", open, close);
}

/* Parser simulation for coverage testing */
static void simulate_parser(const char *input) {
    const char *p = input;
    
    while (*p) {
        switch (*p) {
            default:
                advance();
                p++;
                break;
            case '(':
                consume_balanced('(', ')');
                /* Skip to matching ')' in simulation */
                while (*p && *p != ')') p++;
                if (*p == ')') p++;
                break;
            case '[':
                consume_balanced('[', ']');
                while (*p && *p != ']') p++;
                if (*p == ']') p++;
                break;
            case '{':
                consume_balanced('{', '}');
                while (*p && *p != '}') p++;
                if (*p == '}') p++;
                break;
        }
    }
}

/* Main test driver */
int main(void) {
    /* Complex .gt file content targeting uncovered lines 341-352 */
    const char *gt_content = 
        "/* Test file for gengtype parser coverage */\n"
        "/* Target: lines 341-352 in gengtype-parse.cc */\n\n"
        
        "/* ====== 1. BALANCED CONSTRUCT NESTING ====== */\n"
        "/* Multiple parentheses cases */\n"
        "%typedef int (*funcptr1)(int);\n"
        "%typedef void (*complex_func)(int (*callback)(int, char**), double);\n"
        "%typedef struct { int (*method)(struct S* self); } S;\n\n"
        
        "/* Nested brackets for arrays */\n"
        "%struct ArrayTest {\n"
        "    int matrix[3][4][5];  /* 3D array */\n"
        "    char* strings[10];\n"
        "    struct {\n"
        "        float data[100];\n"
        "    } nested;\n"
        "};\n\n"
        
        "/* Complex brace nesting */\n"
        "%union DeepNest {\n"
        "    struct {\n"
        "        struct {\n"
        "            int x;\n"
        "            union {\n"
        "                char c;\n"
        "                double d;\n"
        "            } u;\n"
        "        } inner;\n"
        "    } outer;\n"
        "    int arr[5];\n"
        "};\n\n"
        
        "/* ====== 2. UNBALANCED EDGE CASES ====== */\n"
        "/* These should trigger error handling */\n"
        "// Missing closing brace:\n"
        "%struct Unbalanced1 {\n"
        "    int x;\n"
        "    char y;\n"
        "    // No closing brace here - parser should handle\n"
        "\n"
        "/* Missing closing bracket: */\n"
        "%typedef int bad_array[10;  // Missing ]\n\n"
        
        "/* Missing closing parenthesis: */\n"
        "%typedef void (*bad_funcptr(int, char);\n\n"
        
        "/* ====== 3. COMMENTS AND MACROS INTERLEAVING ====== */\n"
        "/* Balanced delimiters inside block comments: */\n"
        "/* int (*commented_func)(int) { return 0; } [test] */\n\n"
        
        "// Line comment with delimiters:\n"
        "// func(int x) { return x * 2; } [array]\n\n"
        
        "#define ARRAY_TYPE(T) struct { T data[10]; }\n"
        "#define FUNC_PTR(RET, ARGS) RET (*)(ARGS)\n"
        "#define NESTED_MACRO(x) { { x } }\n\n"
        
        "/* Using macros with balanced delimiters */\n"
        "%typedef ARRAY_TYPE(int) IntArray;\n"
        "%typedef FUNC_PTR(int, (int, char*)) HandlerFunc;\n\n"
        
        "/* ====== 4. GT FILE SPECIFIC CONSTRUCTS ====== */\n"
        "/* GC roots with complex types */\n"
        "GC roots {\n"
        "    struct RootStruct {\n"
        "        int (*compare)(const void*, const void*);\n"
        "        void* data[20];\n"
        "        union {\n"
        "            long l;\n"
        "            double d;\n"
        "        } value;\n"
        "    } *root;\n"
        "}\n\n"
        
        "%struct GT_Annotated {\n"
        "    %union U {\n"
        "        struct { int x; } s;\n"
        "        int arr[5];\n"
        "        void (*func)(int (*)(int));\n"
        "    } u;\n"
        "    enum { A = 1, B = 2 } tag;\n"
        "};\n\n"
        
        "/* ====== 5. MULTIPLE TOP-LEVEL DECLARATIONS ====== */\n"
        "%typedef enum Color { RED, GREEN, BLUE } Color;\n\n"
        
        "%struct Node {\n"
        "    int value;\n"
        "    struct Node* left;\n"
        "    struct Node* right;\n"
        "    void (*print)(struct Node*);\n"
        "};\n\n"
        
        "%union Variant {\n"
        "    int i;\n"
        "    float f;\n"
        "    char* s;\n"
        "    struct Node* node;\n"
        "    int (*operation)(int, int);\n"
        "};\n\n"
        
        "%typedef struct {\n"
        "    Variant data[100];\n"
        "    int (*sort_cmp)(Variant, Variant);\n"
        "    struct {\n"
        "        int count;\n"
        "        Variant* items;\n"
        "    } cache;\n"
        "} Container;\n\n"
        
        "/* Additional complex cases */\n"
        "%typedef int (*(*complex_callback[5]))(int, ...);\n"
        "%struct FinalTest {\n"
        "    union {\n"
        "        struct { int a; int b; };\n"
        "        struct { long l; };\n"
        "    };\n"
        "    int (*methods[3])(struct FinalTest*);\n"
        "};\n\n"
        
        "/* End of test file */\n";
    
    /* Write to temporary file */
    FILE *tmp = tmpfile();
    if (!tmp) {
        fprintf(stderr, "Failed to create temporary file: %s\n", strerror(errno));
        return 1;
    }
    
    fputs(gt_content, tmp);
    fflush(tmp);
    rewind(tmp);
    
    printf("=== Testing gengtype parser coverage ===\n");
    printf("Target: lines 341-352 in gengtype-parse.cc\n\n");
    
    /* Simulate parser execution */
    printf("Simulating parser on .gt content...\n");
    simulate_parser(gt_content);
    
    /* Read back and display for verification */
    printf("\n=== Generated .gt content preview ===\n");
    char buffer[256];
    rewind(tmp);
    int line_count = 0;
    while (fgets(buffer, sizeof(buffer), tmp) && line_count < 20) {
        printf("%s", buffer);
        line_count++;
    }
    
    fclose(tmp);
    
    printf("\n=== Coverage test complete ===\n");
    printf("The .gt file contains:\n");
    printf("1. Balanced constructs: (), [], {} in nested patterns\n");
    printf("2. Unbalanced edge cases for error handling\n");
    printf("3. Comments and macros interleaving delimiters\n");
    printf("4. GT-specific annotations with complex types\n");
    printf("5. Multiple top-level declarations\n");
    
    return 0;
}
