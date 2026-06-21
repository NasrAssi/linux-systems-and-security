#include <stdio.h>
#include <stdlib.h>

/*
 * count_digits: counts how many characters in the input string are numeric digits ('0'..'9').
 * Returns the count as an integer.
 */
int count_digits(const char *s) {
    int count = 0;
    while (*s) {
        if (*s >= '0' && *s <= '9')
            count++;
        s++;
    }
    return count;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <string>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *input = argv[1];
    int result = count_digits(input);

    /*
     * Print just the number of digits, followed by a newline.
     */
    printf("%d\n", result);
    return EXIT_SUCCESS;
}