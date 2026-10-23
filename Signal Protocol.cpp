#include <stdio.h>
#include <string.h>

// Simple toy hash function to act as KDF for demonstration
unsigned int toy_hash(unsigned int input) {
    return (input * 2654435761u) ^ 0x5bf03635;
}

int main() {
    printf("--- Quantum-Safe Ratchet Demo ---\n");

    // Step 1: Hybrid Shared Secret
    unsigned int classical_shared = 0x1234; 
    unsigned int pqc_shared = 0xABCD;       
    
    // Mix both secrets together
    unsigned int root_key = toy_hash(classical_shared ^ pqc_shared);
    printf("Root Key initialized: 0x%X\n\n", root_key);

    // Step 2: Message Ratchet loop
    unsigned int chain_key = root_key;
    char *messages[3] = {"Hello", "How are you?", "Quantum secure text"};

    for (int i = 0; i < 3; i++) {
        // Derive message key and advance the chain
        unsigned int message_key = toy_hash(chain_key + 1);
        chain_key = toy_hash(chain_key);

        printf("Message %d: \"%s\"\n", i + 1, messages[i]);
        printf("  -> Message Key : 0x%X\n", message_key);
        printf("  -> Next Chain Key: 0x%X\n", chain_key);
    }

    return 0;
}
