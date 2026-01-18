#include <stdio.h>
#include <math.h>

int main(){

    double square_root, average = 0;

    int arr[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

    int n = sizeof(arr)/ sizeof(arr[0]);

    for(int i = 0; i < n; i++){
        
        printf("%d ", arr[i]);

    }

    printf("\n\nCalculating the square root of each number\n");


    for(int i = 0; i < n; i++){
        
        square_root = (double)sqrt(arr[i]);

        printf("The square root of %d is %0.2lf\n", arr[i], square_root);

        average = (square_root + average)/10;

    }

    printf("\nThe average of their square roots is %0.2lf\n", average);

    return 0;
}