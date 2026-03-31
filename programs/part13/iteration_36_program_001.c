Input: "func(a, b, [1, 2, {x: 3}])"
- When '(' is encountered: consume_balanced('(', ')') processes everything until matching ')'
- Inside that, when '[' is encountered: consume_balanced('[', ']') processes the array
- Inside the array, when '{' is encountered: consume_balanced('{', '}') processes the object
