#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    /* CORRECCION 1: i < 5 en vez de i <= 5 */
    int *p = malloc(5 * sizeof(int));
    for (int i = 0; i < 5; i++)
        p[i] = i;

    /* CORRECCION 2: liberar q */
    char *q = malloc(100);
    strcpy(q, "hola mundo");
    printf("%s\n", q);
    free(q);

    /* CORRECCION 3: usar p[0] ANTES de liberarlo */
    printf("p[0] = %d\n", p[0]);
    free(p);

    return 0;
}