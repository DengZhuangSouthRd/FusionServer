#include <stdlib.h>
#include "PCA.h"

/*
用豪斯荷尔德（Householder）变换将n阶实对称矩阵约化为对称三对角阵
徐士良. 常用算法程序集（C语言描述），第3版. 清华大学出版社. 2004

float a[] --- 维数为nxn。存放n阶实对称矩阵A
int n      --- 实对称矩阵A的阶数
float q[] --- 维数nxn。返回Householder变换的乘积矩阵Q。当与
Tri_Symmetry_Diagonal_Eigenvector()函数联用，若将
Q矩阵作为该函数的一个参数时，可计算实对称阵A的特征
值与相应的特征向量
float b[] --- 维数为n。返回对称三对角阵中的主对角线元素
float c[] --- 长度n。前n－1个元素返回对称三对角阵中的次对角线元素。
*/
void PCA::Householder_Tri_Symetry_Diagonal(float a[], int n, float q[],
                                           float b[], float c[])
{
    int i, j, k, u;
    float h, f, g, h2;

    for (i = 0; i <= n - 1; i++)
        for (j = 0; j <= n - 1; j++)
        {
            u = i * n + j;
            q[u] = a[u];
        }
    for (i = n - 1; i >= 1; i--)
    {
        h = 0.0;
        if (i > 1)
            for (k = 0; k <= i - 1; k++)
            {
                u = i * n + k;
                h = h + q[u] * q[u];
            }
        if (h + 1.0 == 1.0)
        {
            c[i] = 0.0;
            if (i == 1) c[i] = q[i*n + i - 1];
            b[i] = 0.0;
        }
        else
        {
            c[i] = sqrt(h);
            u = i * n + i - 1;
            if (q[u] > 0.0) c[i] = -c[i];
            h = h - q[u] * c[i];
            q[u] = q[u] - c[i];
            f = 0.0;
            for (j = 0; j <= i - 1; j++)
            {
                q[j*n + i] = q[i*n + j] / h;
                g = 0.0;
                for (k = 0; k <= j; k++)
                    g = g + q[j*n + k] * q[i*n + k];
                if (j + 1 <= i - 1)
                    for (k = j + 1; k <= i - 1; k++)
                        g = g + q[k*n + j] * q[i*n + k];
                c[j] = g / h;
                f = f + g * q[j*n + i];
            }
            h2 = f / (h + h);
            for (j = 0; j <= i - 1; j++)
            {
                f = q[i*n + j];
                g = c[j] - h2 * f;
                c[j] = g;
                for (k = 0; k <= j; k++)
                {
                    u = j * n + k;
                    q[u] = q[u] - f * c[k] - g * q[i*n + k];
                }
            }
            b[i] = h;
        }
    }
    for (i = 0; i <= n - 2; i++) c[i] = c[i + 1];
    c[n - 1] = 0.0;
    b[0] = 0.0;
    for (i = 0; i <= n - 1; i++)
    {
        if ((b[i] != 0.0) && (i - 1 >= 0))
            for (j = 0; j <= i - 1; j++)
            {
                g = 0.0;
                for (k = 0; k <= i - 1; k++)
                    g = g + q[i*n + k] * q[k*n + j];
                for (k = 0; k <= i - 1; k++)
                {
                    u = k * n + j;
                    q[u] = q[u] - g * q[k*n + i];
                }
            }
        u = i * n + i;
        b[i] = q[u]; q[u] = 1.0;
        if (i - 1 >= 0)
            for (j = 0; j <= i - 1; j++)
            {
                q[i*n + j] = 0.0; q[j*n + i] = 0.0;
            }
    }

    return;
}

/*
计算实对称三对角阵的全部特征值与对应特征向量
徐士良. 常用算法程序集（C语言描述），第3版. 清华大学出版社. 2004

int n    --- 实对称三对角阵的阶数
float b --- 长度为n，存放n阶对称三对角阵的主对角线上的各元素；
返回时存放全部特征值
float c --- 长度为n，前n－1个元素存放n阶对称三对角阵的次对角
线上的元素
float q --- 维数为nxn，若存放单位矩阵，则返回n阶实对称三对角
阵T的特征向量组；若存放Householder_Tri_Symetry_Diagonal()
函数所返回的一般实对称矩阵A的豪斯荷尔得变换的乘
积矩阵Q，则返回实对称矩阵A的特征向量组。其中，q中
的的j列为与数组b中第j个特征值对应的特征向量
float eps --- 本函数在迭代过程中的控制精度要求
int l    --- 为求得一个特征值所允许的最大迭代次数
返回值：
若返回标记小于0，则说明迭代了l次还未求得一个特征值，并有fail
信息输出；若返回标记大于0，则说明程序工作正常，全部特征值由一
维数组b给出，特征向量组由二维数组q给出
*/
int PCA::Tri_Symmetry_Diagonal_Eigenvector(int n, float b[], float c[],
                                           float q[], float eps, int l)
{
    int i, j, k, m, it, u, v;
    float d, f, h, g, p, r, e, s;

    c[n - 1] = 0.0; d = 0.0; f = 0.0;
    for (j = 0; j <= n - 1; j++) {
        it = 0;
        h = eps * (fabs(b[j]) + fabs(c[j]));
        if (h > d) d = h;
        m = j;
        while ((m <= n - 1) && (fabs(c[m]) > d)) m = m + 1;
        if (m != j) {
            do {
                if (it == l) {
                    return(-1);
                }
                it = it + 1;
                g = b[j];
                p = (b[j + 1] - g) / (2.0 * c[j]);
                r = sqrt(p * p + 1.0);
                if (p >= 0.0)
                    b[j] = c[j] / (p + r);
                else
                    b[j] = c[j] / (p - r);
                h = g - b[j];
                for (i = j + 1; i <= n - 1; i++)
                    b[i] = b[i] - h;
                f = f + h; p = b[m]; e = 1.0; s = 0.0;
                for (i = m - 1; i >= j; i--) {
                    g = e * c[i]; h = e * p;
                    if (fabs(p) >= fabs(c[i])) {
                        e = c[i] / p; r = sqrt(e * e + 1.0);
                        c[i + 1] = s * p * r; s = e / r; e = 1.0 / r;
                    } else {
                        e = p / c[i]; r = sqrt(e * e + 1.0);
                        c[i + 1] = s * c[i] * r;
                        s = 1.0 / r; e = e / r;
                    }
                    p = e * b[i] - s * g;
                    b[i + 1] = h + s * (e * g + s * b[i]);
                    for (k = 0; k <= n - 1; k++) {
                        u = k * n + i + 1; v = u - 1;
                        h = q[u]; q[u] = s * q[v] + e * h;
                        q[v] = e * q[v] - s * h;
                    }
                }
                c[j] = s * p; b[j] = e * p;
            } while (fabs(c[j]) > d);
        }
        b[j] = b[j] + f;
    }
    for (i = 0; i <= n - 1; i++) {
        k = i; p = b[i];
        if (i + 1 <= n - 1) {
            j = i + 1;
            while ((j <= n - 1) && (b[j] <= p)) {
                k = j; p = b[j]; j = j + 1;
            }
        }
        if (k != i) {
            b[k] = b[i]; b[i] = p;
            for (j = 0; j <= n - 1; j++) {
                u = j * n + i; v = j * n + k;
                p = q[u]; q[u] = q[v]; q[v] = p;
            }
        }
    }

    return 1;
}


/*
计算实对称阵的全部特征值与对应特征向量
int n              --- 实对称阵的阶数
float * CovMatrix --- 维数为nxn，存放n阶对称阵的各元素；
float * Eigen     --- 长度为n，为n个特征值
float * EigenVector --- 维数为nxn，返回n阶实对称阵的特征向量组其中，
EigenVector中的j列为与数组Eigen中第j个特征值
对应的特征向量
返回值：
若返回标记小于0，则说明迭代了Iteration次还未求得一个特征值；
若返回标记大于0，则说明程序工作正常，全部特征值由一
维数组Eigen给出，特征向量组由二维数组EigenVector给出
*/
int PCA::SymmetricRealMatrix_Eigen(float CovMatrix[], int n,
                                   float Eigen[], float EigenVector[]) {
    int k;
    float * subDiagonal;

    subDiagonal = (float *)malloc(sizeof(float) * n);
    if (subDiagonal==NULL) {
        cerr<<"Memory Error.\n";
        cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
        return -1;
    }
    Householder_Tri_Symetry_Diagonal(CovMatrix, n, EigenVector, Eigen, subDiagonal);
    k = Tri_Symmetry_Diagonal_Eigenvector(n, Eigen, subDiagonal, EigenVector, EPS, Iteration);
    free(subDiagonal);

    return(k);
}

/*
PCA: Perform PCA using covariance.
data     --- MxN matrix of input data ( M dimensions, N trials )
行数为原始数据维数；每列数据为一个样本
signals  --- MxN matrix of projected data
PC       --- each column is a PC
V        --- Mx1 matrix of variances
row = M dimensions, col = N trials
*/
int PCA::PCAProject(float * data, int row, int col, float * mean, float * signals, float * PC, float * V)
{
    int x, y, k;

    float * data1, *tPC, *tV;
    float * covariance, temp;
    int rvalue, *no, tp;

    if (row <= 1 || col <= 1) return(-1);
    /* subtract off the mean for each dimension */
    data1 = (float *)malloc(sizeof(float) * row * col);
    if (data1==NULL) {
        cerr<<"Memory Error.\n";
        cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
        return -1;
    }
    for (y = 0; y < row; y++) {
        mean[y] = 0;
        for (x = 0; x < col; x++)
            mean[y] += data[y*col + x];
    }
    for (y = 0; y < row; y++) mean[y] = mean[y] / col;
    for (y = 0; y < row; y++) /* subtract mean */
        for (x = 0; x < col; x++)
        {
            data1[y*col + x] = data[y*col + x] - mean[y];
        }

    /* calculate the covariance matrix */
    covariance = (float *)malloc(sizeof(float) * row * row);
    if (covariance==NULL)
    {
        cerr<<"Memory Error.\n";
        cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
        free(data1);
        return  -1;
    }
    for (y = 0; y < row; y++)
        for (x = 0; x < row; x++)
        {
            temp = 0;
            for (k = 0; k < col; k++)
                temp += data1[y*col + k] * data1[x*col + k];
            temp = temp / (col - 1);
            covariance[x*row + y] = temp;
        }

    /* find the eigenvectors and eigenvalues */
    rvalue = SymmetricRealMatrix_Eigen(covariance, row, V, PC);
    if (rvalue<0) {
        cerr<<"PCA：Find the eigenvectors and eigenvalues error.\n";
        cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
        free(covariance);
        free(data1);
        return rvalue;
    }
    free(covariance);covariance=NULL;
    /* sort the variances in decreasing order */
    no = (int *)malloc(sizeof(int) * row);
    if (no==NULL) {
        cerr<<"Memory Error.\n";
        cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
        free(data1);
        return -1;
    }
    for (x = 0; x < row; x++)
        no[x] = x;
    for (x = 0; x < row - 1; x++) {
        temp = V[x];
        for (k = x; k < row; k++)
            if (temp < V[k]) {
                tp = no[k];
                no[k] = no[x];
                no[x] = tp;
                temp = V[k];
            }
    }
    /* exchange eigen value and vector in decreasing order */
    tV = (float *)malloc(sizeof(float) * row);
    tPC = (float *)malloc(sizeof(float) * row * row);
    if (tV==NULL||tPC==NULL) {
        cerr<<"Memory Error.\n";
        cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
        free(no);
        free(data1);
        if(tV != NULL) free(tV);
        if(tPC != NULL) free(tPC);
        return -1;
    }

    for (x = 0; x < row; x++)
        tV[x] = V[x];
    for (x = 0; x < row; x++)
        for (y = 0; y < row; y++)
            tPC[x*row + y] = PC[x*row + y];
    for (x = 0; x < row; x++)
    {
        if (no[x] != x)
        {
            for (y = 0; y < row; y++)
                PC[y*row + x] = tPC[y*row + no[x]];
            V[x] = tV[no[x]];
        }
    }
    free(no);no=NULL;
    free(tV);tV=NULL;
    free(tPC);tPC=NULL;

    /* project the original data: signals = PC' * data; */
    for (y = 0; y < row; y++)
        for (x = 0; x < col; x++)
        {
            signals[y*col + x] = 0.0;
            for (k = 0; k < row; k++)
                signals[y*col + x] += PC[k*row + y] * data1[k*col + x];
        }

    free(data1);data1=NULL;

    return rvalue;
}

int PCA::PCAReproject(float * signal, int row, int col, float * mean, float * PC, float * newdata){

    int i;
    int x, y;
    /* back project the original data: newdata = PC * signal; */
    for (y = 0; y < row; y++)
        for (x = 0; x < col; x++)
        {
            newdata[y*col + x] = 0;
            for (i = 0; i < row; i++)
                newdata[y*col + x] += PC[y*row + i] * signal[i*col + x];

            newdata[y*col + x] = newdata[y*col + x] + mean[y];

        }
    return 1;
}

int PCA::PCAProject(float * data, int row, int col, float * mean, float * PC, float * V) {
    int x, y, k;

    float *tPC, *tV;
    float * covariance, temp;
    int rvalue, *no, tp;

    if (row <= 1 || col <= 1)
        return(-1);
    /* subtract off the mean for each dimension */

    for (y = 0; y < row; y++) /* calculate mean */
    {
        mean[y] = 0;
        for (x = 0; x < col; x++)
            mean[y] += data[y*col + x];
    }
    for (y = 0; y < row; y++) mean[y] = mean[y] / col;
    for (y = 0; y < row; y++) /* subtract mean */
        for (x = 0; x < col; x++)
        {
            data[y*col + x] = data[y*col + x] - mean[y];
        }

    /* calculate the covariance matrix */
    covariance = (float *)malloc(sizeof(float) * row * row);
    if (covariance==NULL) {
        cerr<<"Memory Error.\n";
        cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
        return -1;
    }
    for (y = 0; y < row; y++){
        for (x = 0; x < row; x++)
        {
            temp = 0;
            for (k = 0; k < col; k++)
                temp += data[y*col + k] * data[x*col + k];
            temp = temp / (col - 1);
            covariance[x*row + y] = temp;
        }
    }
    /* find the eigenvectors and eigenvalues */
    rvalue = SymmetricRealMatrix_Eigen(covariance, row, V, PC);
    free(covariance);covariance=NULL;
    if (rvalue<0) {
        cerr<<"PCA：Find the eigenvectors and eigenvalues error.\n";
        cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
        return -1;
    }

    /* sort the variances in decreasing order */
    no = (int *)malloc(sizeof(int) * row);
    if (no==NULL) {
        cerr<<"Memory Error.\n";
        cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
        return -1;
    }
    for (x = 0; x < row; x++) no[x] = x;
    for (x = 0; x < row - 1; x++)
    {
        temp = V[x];
        for (k = x; k < row; k++)
            if (temp < V[k])
            {
                tp = no[k];
                no[k] = no[x];
                no[x] = tp;
                temp = V[k];
            }
    }
    /* exchange eigen value and vector in decreasing order */
    tV = (float *)malloc(sizeof(float) * row);
    tPC = (float *)malloc(sizeof(float) * row * row);
    if (tV==NULL||tPC==NULL) {
        cerr<<"Memory Error.\n";
        cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
        free(no);
        if(tV != NULL) free(tV);
        if(tPC != NULL) free(tPC);
        return -1;
    }

    for (x = 0; x < row; x++) tV[x] = V[x];
    for (x = 0; x < row; x++)
        for (y = 0; y < row; y++)
            tPC[x*row + y] = PC[x*row + y];
    for (x = 0; x < row; x++)
    {
        if (no[x] != x)
        {
            for (y = 0; y < row; y++)
                PC[y*row + x] = tPC[y*row + no[x]];
            V[x] = tV[no[x]];
        }
    }
    free(no);no=NULL;
    free(tV);tV=NULL;
    free(tPC);tPC=NULL;

    float* tmp = (float*)malloc(sizeof(float) * row);
    if (NULL == tmp) {
        cerr<<"Memory Error.\n";
        cerr<<"file: "<<__FILE__<<"line: "<<__LINE__<<"time: "<<__DATE__<<" "<<__TIME__<<endl;
        return -1;
    }
    for (x = 0; x < col; x++){
        for (y = 0; y < row; y++)
        {
            tmp[y] = 0.0;
            for (k = 0; k < row; k++)
                tmp[y] += PC[k*row + y] * data[k*col + x];
        }
        for (y = 0; y < row; y++)
        {
            data[y*col + x] = tmp[y];
        }
    }

    free(tmp);
    return rvalue;
}

int PCA::PCAReproject(float * signal, int row, int col, float * mean, float * PC){

    int i;
    int x, y;
    /* back project the original data: newdata = PC * signal; */
    float* tmp = (float*)malloc(sizeof(float)*row);
    if (NULL == tmp) {
        cerr<<"Memory Error.\n";
        cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
        return -1;
    }
    for (x = 0; x < col; x++){
        for (y = 0; y < row; y++)
        {
            tmp[y] = 0;
            for (i = 0; i < row; i++)
                tmp[y] += PC[y*row + i] * signal[i*col + x];

            tmp[y] = tmp[y] + mean[y];

        }
        for (y = 0; y < row; y++)
        {
            signal[y*col + x] = tmp[y];
            //处理小于0的点,截断
            if (signal[y*col + x] < 0)
            {
                signal[y*col + x] = 0;
            }
        }

    }
    free(tmp);
    return 1;
}

