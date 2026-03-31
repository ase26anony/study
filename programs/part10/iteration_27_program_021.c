parallel for j = 0 to 3:
    m = mul[j]
    // Each thread handles a chunk of input
    // Perform tree reduction to get total transformation
    // Let chunk_size = 5000 / P, where P = num_threads_per_j
    // Each core computes (m^chunk_size, weighted_sum) for its chunk
    // Then combine chunks in log(P) steps
    // Finally apply to init[j] to get result[j]
