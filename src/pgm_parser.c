// PGM Parser

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TESTS_DIR "tests/"

typedef struct {
    char header[3];
    int height;
    int width;
    int max_pixel;
    int *pixels;
} ParsedPgmFile;

int parse_pgm_file(const char *filename, ParsedPgmFile *parsed_output);
int read_pgm_file(FILE *pgm_file_ptr, int *out_val);
void free_parsed_file(ParsedPgmFile *parsed_output);

int main() {
    ParsedPgmFile parsed_pgm_p2 = {0};

    if (parse_pgm_file(TESTS_DIR "sample_p2.pgm", &parsed_pgm_p2) != 0) {
        return 1;
    }

    printf("HEADER: %s\n", parsed_pgm_p2.header);
    printf("WIDTH: %d\n", parsed_pgm_p2.width);
    printf("HEIGHT: %d\n", parsed_pgm_p2.height);
    printf("MAX PIXEL VALUE: %d\n", parsed_pgm_p2.max_pixel);

    int idx = 0;
    for (int y = 0; y < parsed_pgm_p2.height; y++) {
        for (int x = 0; x < parsed_pgm_p2.width; x++) {
            printf("%d ", parsed_pgm_p2.pixels[idx]);
            idx++;
        }
        printf("\n");
    }

    free_parsed_file(&parsed_pgm_p2);

    return 0;
}

/*
 * What should it do?
 * 1. Take a file - a pgm file
 * 2. Read the first string which would be 'P2' or 'P5'. Proceed accordingly.
 * 3. Keep reading...
 *      - If got '#', ignore everything until hit new line ('\n')
 *      - Put the 1st integer inside a width variable
 *      - Put the next integer inside a height variable
 *      - Put the next integer inside a maximum_pixel_value variable
 * 4. After step 3, all the other integers, put them inside an array of size WIDTH * HEIGHT.
 * That's it!
 *
 */

int parse_pgm_file(const char *filename, ParsedPgmFile *parsed_output) {
    FILE *pgm_file_ptr = fopen(filename, "r");
    if (pgm_file_ptr == NULL) {
        perror("Error opening file");
        return -1;
    }

    // Header - P2 OR P5 only
    if (fscanf(pgm_file_ptr, "%2s", parsed_output->header) != 1 || (strcmp(parsed_output->header, "P2") != 0 && strcmp(parsed_output->header, "P5") != 0)) {
        fprintf(stderr, "Error: Invalid or missing header.\n");
        fclose(pgm_file_ptr);
        return -1;
    }

    // Read width, height and max_pixel
    if (read_pgm_file(pgm_file_ptr, &parsed_output->width) != 0 || read_pgm_file(pgm_file_ptr, &parsed_output->height) != 0 || read_pgm_file(pgm_file_ptr, &parsed_output->max_pixel) != 0) {
        fprintf(stderr, "Error: Failed to read width/height/max_pixel.\n");
        fclose(pgm_file_ptr);
        return -1;
    }

    if (parsed_output->width <= 0 || parsed_output->height <= 0 || parsed_output->max_pixel <= 0 || parsed_output->max_pixel > 65535) {
        fprintf(stderr, "Error: Width/height/max_pixel must be positive and max_pixel must be less than 65536.\n");
        fclose(pgm_file_ptr);
        return -1;
    }

    size_t no_of_pixels = (size_t)parsed_output->width * (size_t)parsed_output->height;
    parsed_output->pixels = malloc(no_of_pixels * sizeof(int));
    if (parsed_output->pixels == NULL) {
        perror("Memory allocation failed");
        fclose(pgm_file_ptr);
        return -1;
    }

    for (size_t i = 0; i < no_of_pixels; i++) {
        if (read_pgm_file(pgm_file_ptr, &parsed_output->pixels[i]) != 0) {
            fprintf(stderr, "Error: File ended early or had bad data at item %zu.\n", i);
            free(parsed_output->pixels);
            parsed_output->pixels = NULL;
            fclose(pgm_file_ptr);
            return -1;
        }
    }

    fclose(pgm_file_ptr);
    return 0;
}

int read_pgm_file(FILE *pgm_file_ptr, int *out_val) {
    char token[64];

    while (fscanf(pgm_file_ptr, "%63s", token) == 1) {
        if (token[0] == '#') {
            fscanf(pgm_file_ptr, "%*[^\n]");
            continue;
        }

        // Convert token to integer
        char *endptr;
        long val = strtol(token, &endptr, 10);
        if (*endptr != '\0') {
            fprintf(stderr, "Error: Expected integer or #, found '%s'\n", token);
            return -1;
        }

        *out_val = (int)val;
        return 0;
    }

    return -1; // Reached EOF before finding an integer
}

void free_parsed_file(ParsedPgmFile *parsed_output) {
    free(parsed_output->pixels);
    parsed_output->pixels = NULL;
}
