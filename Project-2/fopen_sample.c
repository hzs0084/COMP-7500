/* 
 * Reference: https://stackoverflow.com/questions/7582619/why-does-fopen-fail-when-given-value-from-main-argv
 * Compile: gcc fopen_sample.c -o fopen_sample
 * Usage: ./fopen_sample <file_name>
 */
#include <stdio.h>
#include <stdlib.h>

int main( int argc, char *argv[] )
{
    FILE *fd;
    char *file_name;

    file_name = argv[1];

    fd=fopen(file_name,"r");

    if( fd==NULL )
    {
        printf("Can't open file. \n");
    }
    else
    {
        printf("File loaded: '%s'", file_name);
    }

    return 0;
}
