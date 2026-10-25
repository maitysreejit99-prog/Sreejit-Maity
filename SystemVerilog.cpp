#include <stdio.h>

typedef struct {
    unsigned char result;
    unsigned char carry_flag;
    unsigned char zero_flag;
} ALU_Output;

ALU_Output run_alu_cycle(unsigned char a, unsigned char b, unsigned char op) {
    ALU_Output out;
    unsigned short temp = 0;

    a &= 0x0F; // Keep 4 bits
    b &= 0x0F;

    if (op == 0)      temp = a + b;
    else if (op == 1) temp = (a - b) & 0x1F;
    else if (op == 2) temp = (a & b);
    else if (op == 3) temp = (a ^ b);

    out.result = temp & 0x0F;
    out.carry_flag = (temp >> 4) & 1;
    out.zero_flag = (out.result == 0);

    return out;
}

int main() {
    printf("--- Running 4-bit ALU Testbench in C ---\n");
    
    unsigned char a = 9, b = 7;
    // Operation 0: Addition (9 + 7 = 16 -> Overflow/Carry out)
    ALU_Output r1 = run_alu_cycle(a, b, 0);
    printf("ADD (9+7): Result = %d, Carry = %d, Zero = %d\n", 
           r1.result, r1.carry_flag, r1.zero_flag);

    // Operation 3: XOR (9 ^ 9 = 0)
    ALU_Output r2 = run_alu_cycle(a, a, 3);
    printf("XOR (9^9): Result = %d, Carry = %d, Zero = %d\n", 
           r2.result, r2.carry_flag, r2.zero_flag);

    return 0;
}
