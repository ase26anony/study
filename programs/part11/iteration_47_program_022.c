x₀ = 0
for (int i = 0; i < n; i++) {
    // Phi node needed here
    x₁ = φ(x₀, x₃)  // Phi node: first iteration uses x₀, subsequent iterations use x₃
    if (x₁ == 0) {
        x₂ = 1;
    } else {
        x₃ = 0;
    }
    // Another phi-like merge needed for the two branches
    x₄ = φ(x₂, x₃)  // This becomes the value for next iteration
}
