#include <stdio.h>
#include <stdlib.h>
#include <assert.h>


int main(int argc, char* argv[])
{
    if (argc > 2)
    {
        FILE* file = fopen(argv[2], "w");

        fprintf(file, "%u %u\n", 20, atoi(argv[1]));

        for (size_t i = 0; i < atoi(argv[1]); i++)
        {
            fprintf(file, "%u ", rand() % 100);
        }

        fclose(file);
        return 0;
    }
    printf("Not enough parametrs\n");
}
