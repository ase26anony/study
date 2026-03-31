int main(int argc, char *argv[]) {
    int opt;
    
    while ((opt = getopt(argc, argv, "hvlprs")) != -1) {
        switch (opt) {
            // ... existing cases ...
            default:
                fprintf(stderr, "Usage: %s [-hvlprs]\n", argv[0]);
                return EXIT_FAILURE;
        }
    }
    
    // Rest of program logic
}
