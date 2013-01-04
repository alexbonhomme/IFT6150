#ifndef TOOLS_H
#define TOOLS_H

#include "imagergb.h"
#include "imagegs.h"
#include <string>
#include <vector>
using namespace std;

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

/**
 * @brief guassianFilter
 * @param vect
 * @param w
 * @param sigma
 */
void guassianFilter(float* vect, int height, int w, float sigma);

/**
 * @brief writeVect
 * @param vect
 * @param height
 * @param filename
 */
void writeVect(float* vect, unsigned height, string filename);

/**
 * @brief foundConnectedComponents
 * @param img
 */
std::vector<int *> *foundConnectedComponents(const ImageGS& img, ImageRGB* out);

#endif // TOOLS_H
