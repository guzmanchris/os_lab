/*
 * Tribonacci
 *
 * In mathematics, the Fibonacci numbers are the numbers in the following integer sequence, called the Fibonacci
 * sequence, and characterized by the fact that every number after the first two is the sum of the two preceding ones:
 *                         0, 1, 1, 2, 3, 5, 8, 13, 21, 34, 55, 89, 144, 233, 377...
 *
 * By definition, the first two numbers in the Fibonacci sequence are either 1 and 1, or 0 and 1, depending on the
 * chosen starting point of the sequence, and each subsequent number is the sum of the previous two.
 *
 * This series can be broken down as the following series:
 *         Fib0 = 0
 *         Fib1 = 1
 *         Fibn = Fibn-1 + Fibn-2 , n > 1
 *
 * Tribonacci sequence:
 * The tribonacci sequence is a generalization of the Fibonacci sequence where each term is the sum of the three
 * preceding terms. This series can be broken down as the following series:
 *         Trib0= 0
 *         Trib1= 1
 *         Trib2= 1
 *         Tribn= Tribn-1 + Tribn-2 + Tribn-3, n>2
 *
 * The sequence begins
 * You will create an array smaller than n using malloc/calloc to store the values of the series on the heap. You will
 * use realloc to resize the array as needed. You will recompute the Tribonacci series with every iteration (even if you
 * already have the values).
 *
 * Input Format
 * Input will contain one integer, n, as the amount of series to compute and print.
 *
 * Output Format
 * You will print n tribonacci series up to i where {i|0<=i<=n} in new lines separeted by a single space. Each new line
 * will print the tribonacci series for i+1.
 *
 * Sample Input 0
 * 6
 *
 * Sample Output 0
 * 0
 * 0 1
 * 0 1 1
 * 0 1 1 2
 * 0 1 1 2 4
 * 0 1 1 2 4 7
 */
#include <stdio.h>
#include <stdlib.h>

// Calculates the n-th element of the tribonacci sequence. The start index being '1' instead of '0'
int tribonacci_calculation(int n);

//n=amount of numbers in the series to compute, seq=array to store series
void tribonacci(int n, int* seq){
    switch (n) {
        //Store base cases in seq or calculate the n-th element in sequence and store.
        case 1:
            *seq = 0;
            break;
        case 2:
            *seq = 0;
            *(seq+1) = 1;
            break;
        case 3:
            *seq = 0;
            *(seq+1) = 1;
            *(seq+2) = 1;
            break;
        default:
            // Calculates the entire tribonacci series, and stores in i=n-1; seq[i]
            *(seq + (n-1)) = tribonacci_calculation(n);
            break;
    }
}

int tribonacci_calculation(int n){
    switch (n) {
        case 1:
            return 0;
        case 2:
        case 3:
            return 1;
        default:
            return tribonacci_calculation(n-1) + tribonacci_calculation(n-2) + tribonacci_calculation(n-3);
    }
}

int main(){

    int n;
    //n, amount of series to compute
    scanf("%d",&n);

    //initialize array to 1, using malloc/calloc
    int *seq = (int *) malloc(1);

    int i;
    for(i = 1; i <= n; i++){

        //recompute the whole series
        tribonacci(i, seq);

        //print array
        int j;
        for(j = 0; j < i; j++){
            printf("%d ", *(seq+j));
        }

        //resize array, with realloc
        int newSize=i+1;
        seq = realloc(seq, newSize);

        printf("\n");
    }
    //free array
    free(seq);
    return 0;
}
