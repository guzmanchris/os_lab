#include <stdio.h>
#include <stdlib.h>

void update(int *a,int *b) {
    int tmp_a = *a;
    *a = *a + *b;
    if(*a%2 == 0) {
        *b = abs(tmp_a - *b);
    }
    else {
        *b = tmp_a * *b;
    }
}

int main() {
    int a, b;
    int *pa = &a, *pb = &b;

    scanf("%d %d", &a, &b);
    update(pa, pb);
    printf("%d\n%d", a, b);

    return 0;
}
