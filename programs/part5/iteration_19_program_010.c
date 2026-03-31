void consume_balanced(char open, char close) {
    int depth = 1;
    while (depth > 0 && !eof()) {
        char c = next_char();
        if (c == open) depth++;
        else if (c == close) depth--;
        // Handle escaped characters, strings, comments
    }
}
