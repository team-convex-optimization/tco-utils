#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "tco_linalg.h"

void usage_show()
{
    printf("Usage: tco_circle_vector_gen.bin <-r radius>\n");
}

/* Reverse a list. */
void arr_reverse(uint8_t *const arr, uint8_t const el_size, uint16_t const length)
{
    for (uint16_t arr_idx = 0; arr_idx < (length / 2); arr_idx++)
    {
        uint8_t tmp[el_size];
        uint16_t const swap_offset1 = arr_idx * el_size;
        uint16_t const swap_offset2 = (length - arr_idx - 1) * el_size;
        memcpy(&tmp, &arr[swap_offset1], el_size);
        memcpy(&arr[swap_offset1], &arr[swap_offset2], el_size);
        memcpy(&arr[swap_offset2], &tmp, el_size);
    }
}

/* Print out the points in a C designated initializer format. */
void print_vec_designated_initializer(vec2_t *const arr, uint16_t const length)
{
    printf("{");
    for (uint16_t vec_idx = 0; vec_idx < length; vec_idx++)
    {
        printf("\n{%hi,%hi},", arr[vec_idx].x, arr[vec_idx].y);
    }
    printf("\n}\n");
}

int main(int argc, char const *argv[])
{
    if (argc != 3 || strcmp(argv[1], "-r") != 0)
    {
        usage_show();
        return EXIT_FAILURE;
    }
    uint16_t r = strtoul(argv[2], 0L, 10);

    vec2_t quadrant4_up[2 * r];
    vec2_t quadrant4_down[2 * r];
    uint16_t quadrant4_up_size = 0;
    uint16_t quadrant4_down_size = 0;
    memset(quadrant4_up, 0, 2 * r * sizeof(vec2_t));
    memset(quadrant4_down, 0, 2 * r * sizeof(vec2_t));

    vec2_t up = {0, -r};
    vec2_t left = {-r, 0};
    vec2_t right = {r, 0};
    vec2_t down = {0, r};

    int16_t x = r;
    int16_t y = 0;
    int16_t p = 1 - r;

    while (x > y)
    {
        y += 1;

        if (p <= 0)
        {
            p = p + (2 * y) + 1;
        }
        else
        {
            x -= 1;
            p = p + (2 * y) - (2 * x) + 1;
        }

        if (x < y)
        {
            break;
        }

        memcpy(&quadrant4_up[quadrant4_up_size++], &(vec2_t){x, y}, sizeof(vec2_t));
        if (x != y)
        {
            memcpy(&quadrant4_down[quadrant4_down_size++], &(vec2_t){y, x}, sizeof(vec2_t));
        }
    }

    uint16_t const quadrant_size = quadrant4_up_size + quadrant4_down_size; /* Excluding points directly below/above or left/right to center. */
    vec2_t all[(4 * quadrant_size) + 4];                                    /* "all" holds all points on the circle. */
    memset(all, 0, sizeof(all));

    /* Offsets to quadrants and special elements inide of "all" array. */
    uint16_t const up_offset = 0;
    uint16_t const quadrant1_offset = up_offset + 1;
    uint16_t const left_offset = quadrant1_offset + quadrant_size;
    uint16_t const quadrant3_offset = left_offset + 1;
    uint16_t const down_offset = quadrant3_offset + quadrant_size;
    uint16_t const quadrant4_offset = down_offset + 1;
    uint16_t const right_offset = quadrant4_offset + quadrant_size;
    uint16_t const quadrant2_offset = right_offset + 1;

    /* Q4 */
    arr_reverse((uint8_t *)quadrant4_down, sizeof(vec2_t), quadrant4_down_size);
    memcpy(&all[quadrant4_offset], quadrant4_up, quadrant4_up_size * sizeof(vec2_t));
    memcpy(&all[quadrant4_offset + quadrant4_up_size], quadrant4_down, quadrant4_down_size * sizeof(vec2_t));
    arr_reverse((uint8_t *)&all[quadrant4_offset], sizeof(vec2_t), quadrant_size);

    /* Q1 */
    memcpy(&all[quadrant1_offset], &all[quadrant4_offset], quadrant_size * sizeof(vec2_t));
    for (uint16_t q1_idx = 0; q1_idx < quadrant_size; q1_idx++)
    {
        all[quadrant1_offset + q1_idx].x *= -1;
        all[quadrant1_offset + q1_idx].y *= -1;
    }

    /* Q2 */
    memcpy(&all[quadrant2_offset], &all[quadrant4_offset], quadrant_size * sizeof(vec2_t));
    for (uint16_t q2_idx = 0; q2_idx < quadrant_size; q2_idx++)
    {
        all[quadrant2_offset + q2_idx].y *= -1;
    }
    arr_reverse((uint8_t *)&all[quadrant2_offset], sizeof(vec2_t), quadrant_size);

    /* Q3 */
    memcpy(&all[quadrant3_offset], &all[quadrant4_offset], quadrant_size * sizeof(vec2_t));
    for (uint16_t q3_idx = 0; q3_idx < quadrant_size; q3_idx++)
    {
        all[quadrant3_offset + q3_idx].x *= -1;
    }
    arr_reverse((uint8_t *)&all[quadrant3_offset], sizeof(vec2_t), quadrant_size);

    /* Special points */
    memcpy(&all[up_offset], &up, sizeof(vec2_t));
    memcpy(&all[left_offset], &left, sizeof(vec2_t));
    memcpy(&all[down_offset], &down, sizeof(vec2_t));
    memcpy(&all[right_offset], &right, sizeof(vec2_t));

    print_vec_designated_initializer(all, sizeof(all) / sizeof(vec2_t));
    return 0;
}
