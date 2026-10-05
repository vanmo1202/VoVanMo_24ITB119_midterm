#include "listing.h"
#include "options.h"

#include <locale.h>

int main(int argc, char *argv[])
{
    setlocale(LC_ALL, "");
    LsOptions options;
    int first_operand;

    if (parse_options(argc, argv, &options, &first_operand) != 0) {
        return 1;
    }

    return list_operands(argc, argv, first_operand, &options);
}
