#include <stdio.h>


struct MyStruct {

    union {
        struct {
            int a;
            short b;
        };
        struct {
            long c;
            char d;
        };
    };

};

int main() {

    struct MyStruct s;
    printf("size [%d]\n", sizeof(s));
    s.b = 10;
    s.a = 20;
    s.c = 123;
    s.d = 'A';
    printf("a: %d\n", s.a);
    printf("b: %d\n", s.b);
    // a.a = 10;
    // a.b = 20;
    // printf("a.a [%s]\n", a.a);
    // printf("a.b [%s]\n", a.b);

    return 0;
}

