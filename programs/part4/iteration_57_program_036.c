// Simple case:
arr[1:10]

// Complex array expression needs parentheses:
(a + b)[1:10]      // Without parentheses: a + b[1:10] would be ambiguous
                   // With parentheses: (a + b)[1:10] is clear

// Function call as array:
func()[1:10]       // No parentheses needed - function call has high precedence
