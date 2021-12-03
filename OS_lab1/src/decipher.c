// Library imports
#include <stdio.h>
#include <string.h>
#include <math.h>
#include<stdbool.h>

// Variable definitions
#define MAX_CHARS 10000

/*
 * CypherText Decoder
 *   In this problem, you will implement a decoder that produces strings from ciphertexts by:
 *      1. Converting the string into an nxn matrix in column-major order
 *      2. Generating the new string by reading the matrix in row major order
 *   For example, the following matrix would be constructed for the input string "WECGEWHYAAIORTNU":
 *                                         n | 0 1 2 3
 *                                       n    --------
 *                                       0 |  W E A R
 *                                       1 |  E W A T
 *                                       2 |  C H I N
 *                                       3 |  G Y O U
 *
 *   Using row and column indexes, the index of the code can be obtained through the following formula:
 *                                      index = column*n + row
 *
 *                                0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15
 *                                -------------------------------------
 *                                W E C G E W H Y A A I  O  R  T  N  U
 */
int main() {
    // Get number of codes to decipher.
    int t;
    scanf("%d\n", &t);

    // Read code from stdin and print deciphered text t-times.
    char cmd[MAX_CHARS];
    while(t--) {
        // Read from stdin
        fgets(cmd, MAX_CHARS, stdin);           //cmd has current string

        // Calculate useful properties
        int length = (int) strlen(cmd) - 1;     // -1 to avoid counting the \n char from the string.
        double n_double = sqrt(length);         // Calculate row/column length as a double.
        int n_int = sqrt(length);               // Store the integer part of the sqrt.
        bool int_square = n_double == n_int;    // Verify if the sqrt of the length is an integer.

        if (int_square) {
            // The Code can be transformed into an nxn matrix.

            // Row-major traversal of the column-major nxn matrix using the formula.
            for(int row=0; row<n_int; row++) {
                for(int column=0; column<n_int; column++) {
                    printf("%c", *(cmd + column*n_int + row));
                }
            }
            printf("\n");
        }
        else {
            // The Code cannot be transformed into an nxn matrix, and is therefore INVALID.
            printf("INVALID\n");
        }
    }
    return 0;
}
