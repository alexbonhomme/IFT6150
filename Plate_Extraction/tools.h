#ifndef TOOLS_H
#define TOOLS_H

/*------------------------------------------------*/
/* DEFINITIONS -----------------------------------*/
/*------------------------------------------------*/
#define SWAP(a,b) tempr=(a);(a)=(b);(b)=tempr
#define SQUARE(X) ((X)*(X))
#define MAX(i,j)  ((i)>(j)?(i):(j))
#define MIN(i,j)  ((i)>(j)?(i):(j))

#define NBCHAR 200

#define FFT   1
#define IFFT -1
#define FFT2D 2

#define GREY_LEVEL 255
#define PI 3.141592654

#define WHITE 255
#define BLACK 0

/**
 * Calcule la FFT
 * @brief FFTDD
 * @param mtxR
 * @param mtxI
 * @param lgth
 * @param wdth
 */
void FFTDD(float** mtxR,float** mtxI,int lgth, int wdth);

/**
 * Calcule la FFT inverse
 * @brief IFFTDD
 * @param mtxR
 * @param mtxI
 * @param lgth
 * @param wdth
 */
void IFFTDD(float** mtxR,float**  mtxI,int lgth,int wdth);

#endif // TOOLS_H
