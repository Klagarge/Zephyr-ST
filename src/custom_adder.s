    .syntax unified
    .thumb
    .text
    .global custom_adder

/*
 * custom_adder function
 * Parameters:
 *   r0 - first operand (int a)
 *   r1 - second operand (int b) 
 * Returns:
 *   r0 - sum of a and b
 */
custom_adder:
    add r0, r0, r1      // Add r1 to r0, store result in r0
    bx lr               // Return to caller

    .size custom_adder, .-custom_adder
    .end
