#include <stdio.h>

int main(int argc, char *argv[])
{
    printf("Du an Midterm - Implement ls(1)\n");

    if (argc == 1) {
        printf("Chua truyen doi so. Thu muc mac dinh se la: .\n");
    } else {
        printf("Cac doi so da nhan:\n");

        for (int i = 1; i < argc; i++) {
            printf("  argv[%d] = %s\n", i, argv[i]);
        }
    }

    return 0;
}