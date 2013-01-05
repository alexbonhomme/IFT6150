#ifndef TOOLS_H
#define TOOLS_H

#include "imagergb.h"
#include "imagegs.h"
#include <string>
#include <vector>
using namespace std;

/**
 * @brief guassianFilter
 * @param vect
 * @param w
 * @param sigma
 */
void gaussianFilter(float* vect, int height, int w, float sigma);

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
std::vector<int *> *foundConnectedComponents(const ImageGS& img, ImageRGB* out, float ratio_min, float ratio_max, unsigned width_min, unsigned width_max, unsigned height_min, unsigned height_max);

/**
 * @brief Conv
 * @param Img1
 * @param Img2
 * @param ImgConv
 * @param lgth
 * @param wdth
 */
void Conv(float** Img1, float** Img2, float** ImgConv, int lgth, int wdth);

/**
 * @brief gaussianMask
 * @param mat
 * @param lgth
 * @param wdth
 * @param sigma
 */
void gaussianMask(float** mat, int lgth, int wdth, float sigma);

#endif // TOOLS_H
