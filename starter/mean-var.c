#include <stdio.h>

int main(void)
{
    int n = 1000;

    double sum, sum_squares = 0;

    for (int i=1; i <= 1000; i++)
    {
        sum += i;
        sum_squares += i*i;
    }

    double mean, var;

    mean = sum/n;
    var = (sum_squares-(sum*sum/n))/(n-1);

    printf("%f \n", mean);
    printf("%f \n", var);
}
