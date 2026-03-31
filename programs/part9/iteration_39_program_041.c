int i₀ = 0;
int i₁ = φ(i₀, i₂);  // Loop Phi node

while (i₁ < n) {
    if (i₁ == 0) {    // Only true on first iteration
        // body
    }
    i₂ = i₁ + 1;
    i₁ = φ(i₀, i₂);  // Next iteration's Phi (implicit)
}
