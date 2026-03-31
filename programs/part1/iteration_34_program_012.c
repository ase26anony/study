int opt;
while ((opt = getopt(argc, argv, "hvlprs")) != -1) {
    switch (opt) {
        // ... cases shown above ...
        default:
            fprintf(stderr, "unknown flag `%c'\n", opt);
            exit(EXIT_FAILURE);
    }
}
