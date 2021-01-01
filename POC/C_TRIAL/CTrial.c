#include <stdio.h>


#include <time.h>



int main() {

    time_t t = time(NULL);
    printf("%ld %ld\n", t, t + 100);
    double d = difftime(t, t + 100);
    printf("%lf\n", d);

    printf("%d\n", 20 >> 2);

    return 0;
}

