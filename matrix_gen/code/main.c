#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include <math.h>

typedef enum mat_type
{
    MAT_TYPE_UNK = 0,
    MAT_TYPE_ROT,
} mat_type_t;

void usage_show()
{
    printf("Usage: tco_matrix_gen.bin <[-t type | --type type] args...> \n"
           "Possible types are:\n"
           "- rotation: To generate a rotation matrix. It takes one 'angle' argument in degrees and generates a 2x2 rotation matrix.\n");
}

/* Print out the points in a C designated initializer format. */
void print_mat_designated_initializer(long double *const mat_data, uint16_t const length)
{
    printf("{");
    for (uint16_t val_idx = 0; val_idx < length; val_idx++)
    {
        printf("%.12Lff", mat_data[val_idx]);
        if (val_idx + 1 < length)
        {
            printf(", ");
        }
    }
    printf("}\n");
}

double deg_to_rad(long double const deg)
{
    return deg * (M_PI / 180.0L);
}

void mat_gen_rot(long double const angle)
{
    long double const angle_rad = deg_to_rad(angle);
    long double mat_data[4] = {cosl(angle_rad), -sinl(angle_rad), sinl(angle_rad), cosl(angle_rad)};
    print_mat_designated_initializer((long double *)&mat_data, 4);
}

int main(int argc, char const *argv[])
{
    mat_type_t mat_type_gen = MAT_TYPE_UNK;
    if (argc >= 3 && (!(strcmp(argv[1], "-t") == 0) != !(strcmp(argv[1], "--type") == 0)))
    {
        if (strcmp(argv[2], "rotation") == 0 && argc == 4)
        {
            mat_type_gen = MAT_TYPE_ROT;
            mat_gen_rot(strtold(argv[3], NULL));
        }
    }
    else
    {
        usage_show();
        return EXIT_FAILURE;
    }

    return 0;
}
