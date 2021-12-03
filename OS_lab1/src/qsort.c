#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_CHARS 100
#define MAX_LINES 100		/* max #lines to be sorted */



void qsort_ (void *v[], int left, int right, int (*comp) (void *, void *));


int main (int argc, char *argv[])
{

    int nlines;			/*number of input lines read */

    char *word;
    char **word_list;

    scanf("%d\n", &nlines);

    //INIT word AND word_list
    word = (char *) malloc(MAX_CHARS);
    word_list = (char **) malloc(MAX_LINES);

    // READ WORDS AND POINT EACH ELEMENT OF WORD_LIST TO A WORD
    for (int i = 0; i < nlines; i++) {
        //TODO: INSERT YOUR CODE TO FILL word_list WITH THE STD INPUTS
        fgets(word, MAX_CHARS, stdin);
        *(word_list+i) = (char *) malloc(MAX_CHARS);
        strcpy(*(word_list+i), word);
    }


    qsort_((void **) word_list, 0, nlines - 1, (int (*)(void *, void *)) (strcmp));

    // PRINT SORTED ELEMENTS:
    for (int i = 0; i < nlines; i++) {
        //TODO: PRINT THE word_list SORTED
        printf("%s", *(word_list + i));
    }

    free(word);
    free(word_list);
    return 0;
}

void qsort_ (void **v, int left, int right, int (*comp) (void *, void *))
{
    int i, last;
    void swap (void *v[], int, int);

    if (left >= right)
        return;
    swap (v, left, (left + right) / 2);
    last = left;
    for (i = left + 1; i <= right; i++)
        if ((*comp) (v[i], v[left]) < 0)
            swap (v, ++last, i);
    swap (v, left, last);

    qsort_ (v, left, last - 1, comp);
    qsort_ (v, last + 1, right, comp);
}


void swap (void *v[], int i, int j)
{
    void *temp;

    temp = v[i];
    v[i] = v[j];
    v[j] = temp;
}

