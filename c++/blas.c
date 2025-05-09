#include <cblas.h>
#include <stdio.h>

// https://zhuanlan.zhihu.com/p/104287878

int main()
{
    double A[6] = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0};
    double B[6] = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0};
    double C[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
    double D[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
    double E[16] = { 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0, 16.0};
    double F[4] = {0, 0, 0, 0};

    /*按行主序展开*/
    //1、都无转置
    // a: [        b: [         [
    //     1,2    *   1,2,3  =     9, 12, 15
    //     3,4        4,5,6        19, 26, 45
    //     5,6      ]              29, 40, 51
    // ]                        ]
    cblas_dgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, 3, 3, 2, 1, A, 2, B, 3, 1, C, 3);
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
            printf("%lf ", C[i * 3 + j]);
        printf("\n");
    }
    printf("\n");

    /*按列主序展开*/
    //1、都无转置
    // a: [        b: [         [
    //     1,4    *   1,3,5  =     9, 19, 29
    //     2,5        2,4,6        12, 26, 40
    //     3,6      ]              15, 45, 51
    // ]                        ]
    cblas_dgemm(CblasColMajor, CblasNoTrans, CblasNoTrans, 3, 3, 2, 1, A, 3, B, 2, 0, C, 3);
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
            printf("%lf ", C[i * 3 + j]);
        printf("\n");
    }
    printf("\n");

    //2\矩阵B转置
    // a: [        b: [    T    [
    //     1,2    *   1,2    =     5, 11, 17
    //     3,4        3,4          11, 25, 39
    //     5,6        5,6          17, 39, 61
    // ]            ]            ]
    cblas_dgemm(CblasRowMajor, CblasNoTrans, CblasTrans, 3, 3, 2, 1, A, 2, B, 2, 1, D, 3);
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
            printf("%lf ", D[i * 3 + j]);
        printf("\n");
    }
    printf("\n");
    /**
       按列主序展开
       矩阵AB都转置
       E = [1,  2,  3,  4, 
            5,  6,  7,  8, 
            9,  10, 11, 12, 
            13, 14, 15, 16,]

        A: [     T  B: [     T      [
            6,10    *   1,3,5    =     67, 103
            7,11        2,4,6          88, 136
            8,12       ]             ]
        ]                           
    */
    cblas_dgemm(CblasColMajor, CblasTrans, CblasTrans, 2, 2, 3, 1, E+5, 4, B, 2, 1, F, 2);
    for (int i = 0; i < 2; i++)
    {
        for (int j = 0; j < 2; j++)
            printf("%lf ", F[i * 2 + j]);
        printf("\n");
    }
    printf("\n");

    return 0;
}