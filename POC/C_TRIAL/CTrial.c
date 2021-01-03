#include <stdio.h>
#include <stdlib.h>

#include <time.h>

struct person {
    int age;
    int height;
    struct person* next;
};

int main() {

    struct person* p = calloc(10, sizeof(struct person));

    free(p);

    int i;
    for (i=0; i<10; i++) {
        p[i].age = i;
        p[i].height = i + 1;
    }

    printf("7: %d %d\n", p[7].age, p[7].height);

    if (p[0].next == NULL) {
        printf("Default null\n");
    }

    int **ptr;
    int x;

    x = 5;

    ptr = malloc(sizeof(int *) * 10);
    ptr[0] = &x;
    /* etc */

    printf("%d\n", *ptr[0]);

    free(ptr);
    // return 0;

    // time_t t = time(NULL);
    // printf("%ld %ld\n", t, t + 100);
    // double d = difftime(t, t + 100);
    // printf("%lf\n", d);

    // printf("%d\n", 20 >> 2);

    // return 0;
}

