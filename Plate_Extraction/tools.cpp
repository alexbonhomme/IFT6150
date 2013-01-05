#include "tools.h"
#include <malloc.h>
#include <math.h>
#include <algorithm>
#include <assert.h>
#include <iostream>
#include <vector>

// Fonctions du TP5
#include "demo5.h"

#define _USE_MATH_DEFINES
#define GAUSS_FILTER_CENTERED_1D(x, mu, var) (exp(-(pow(x-mu, 2.f))/(2*var))/(2.f*M_PI*var))
#define GAUSS_FILTER_2D(x,y, var) (exp(-((x*x)+(y*y))/(2.f*var))/(2.f*M_PI*var))

using namespace std;

#define H(j, sig) (exp(-(j*sig*sig)/2.f))

void gaussianFilter(float* vect, int height, int w, float sigma) {
    float k = 0.f;

    // calcule du coef k
    for (int i = 1; i <= w; ++i) {
        k += H(i, sigma) + 1.f;
    }
    k *= 2.f;

    // Filtrage
    float* newVect = new float[height];
    for (int i = 0; i < height; ++i) {
        float acc = 0.f;
        for (int j = 1; j <= w; ++j) {
            acc += vect[max(i-j, 0)]*H(j, sigma)
                 + vect[min(i+j, height-1)]*H(j, sigma);
        }
        newVect[i] = (1.f/k)*(vect[i] + acc);
    }

    // Second filtrage par une gaussienne simple centre au deux tier du vecteur
#if 1
    int mu = height - height/3;
    float var = height/3.f;
    for (int i = 0; i < height; ++i) {
        std::cout << GAUSS_FILTER_CENTERED_1D(i, mu, height);
        newVect[i] *= (1.f/(var*sqrt(2.f*M_PI)))*exp(-pow(i - mu, 2.f)/(2.f*var*var));
    }
#endif

    // copy
    for (int i = 0; i < height; ++i)
        vect[i] = newVect[i];

    delete[] newVect;
}

void writeVect(float* vect, unsigned height, string filename) {
    // Output to a text file
    FILE* f = fopen(filename.c_str(), "w");
    assert(f != NULL);

    // Ecriture des données
    std::cout << ">> Ecriture des données dans " << filename << std::endl;
    for (unsigned i = 0; i < height; ++i)
        fprintf(f, "%d %f\n", i, vect[i]);

    // Fermeture du flux
    fclose(f);
}

std::vector<int*>* foundConnectedComponents(const ImageGS& img, ImageRGB* out,
                                            float ratio_min, float ratio_max,
                                            unsigned width_min, unsigned width_max,
                                            unsigned height_min, unsigned height_max) {
    int x1, y1, x2, y2;
    std::vector<int*> *connectList = new std::vector<int*>();

    for (int i = 1; i < (int)img.getHeight()-1; ++i)
    for (int j = 1; j < (int)img.getWidth()-1; ++j) {
        // Si on trouve un pixel blanc
        if(img(i, j) == 255.f) {

            // check d'abord si le pixel n'est pas deja dans un rectangle
            bool knowPt = false;
            for (unsigned n = 0; n < connectList->size(); ++n) {
                x1 = (*connectList)[n][0]; y1 = (*connectList)[n][1];
                x2 = (*connectList)[n][2]; y2 = (*connectList)[n][3];

                if(i >= y1 && i <= y2 && j >= x1 && j <= x2) {
                    knowPt = true;
                    break;
                }
            }

            // Le point est inconnu on peut travailler avec
            if( !knowPt ) {

                // on verifit ca 8 connexité
                bool connex = true;
                for (int i_p = i-1; i_p <= i+1; ++i_p)
                for (int j_p = j-1; j_p <= j+1; ++j_p) {
                    if(img(i_p, j_p) != 255.f)
                        connex = false;
                }

                // Le pixel est 8 connexe
                if(connex) {
                    // On construit un rectangle emglobant
                    x1 = j-1; y1 = i-1;
                    x2 = j+1; y2 = i+1;

                    // On agrandit le rectangle et on check les nouveaux points
                    int prev_x2, prev_y2;
                    do {
                        bool end_x, end_y, found;
                        prev_x2 = x2; prev_y2 = y2;

                        do { // recherche en x
                            end_x = true;

                            ++x2;
                            if(x2 < (int)img.getWidth()) {
                                end_x = false;

                                found = false;
                                for (int i_p = y1; i_p <= min(y2, (int)(img.getHeight()-1)); ++i_p) {
                                    if(img(i_p, x2) == 255.f) { // on a trouvé un nouveau point
                                       found = true;
                                       break;
                                    }
                                }

                                // Si l'on n'a pas trouvé de nouveau point on remet x2 a sa valeur
                                if(!found) {
                                    --x2;
                                    end_x = true; // on arrete d'agrandir en x
                                }
                            } else {
                                --x2;
                            }
                        } while ( !end_x );

                        do { // recherche en y
                            end_y = true;

                            ++y2;
                            if(y2 < (int)img.getHeight()) {
                                end_y = false;

                                found = false;
                                for (int j_p = x1; j_p <= min(x2, (int)(img.getWidth()-1)); ++j_p) {
                                    if(img(y2, j_p) == 255.f) { // on a trouvé un nouveau point
                                       found = true;
                                       break;
                                    }

                                }

                                // Si l'on n'a pas trouvé de nouveau point on remet y2 a sa valeur
                                if(!found) {
                                    --y2;
                                    end_y = true; // on arrete d'agrandir en y
                                }
                            } else {
                                --y2;
                            }
                        } while ( !end_y );

                    } while ( prev_x2 != x2 || prev_y2 != y2 ); // On continu tant qu'il que l'on rouve de nouveaux points


                    // On vérifit qu'il n'est pas superposé a un autre
                    // (au quel cas on prendra le plus grand des deux)
                    bool addRect = true;
                    for (unsigned n = 0; n < connectList->size(); ++n) {
                        int old_x1 = (*connectList)[n][0], old_y1 = (*connectList)[n][1];
                        int old_x2 = (*connectList)[n][2], old_y2 = (*connectList)[n][3];

                        // On check juste le point en bas a droite car
                        // il est impossible qu'ils aient le meme point de départ
                        // et logiquement l'algo converge au meme poit d'arrivé
                        if(x2 == old_x2 && y2 == old_y2) {
                            addRect = false;

                            // On calculer la taille pour ne garder que le plus grand
                            if( (x2 - x1)+(y2 - y1) > (old_x2 - old_x1)+(old_y2 - old_y1) ) {
                                // nouveau est plus grand que l'ancien
                                // on modifit la list
                                (*connectList)[n][0] = x1;
                                (*connectList)[n][1] = y1;
                            }

                            break;
                        }
                    }

                    if(addRect) {
                        // ajout des coordonnées du rectangle a la list
                        int *coord = new int[4];
                        coord[0] = x1; coord[1] = y1;
                        coord[2] = x2; coord[3] = y2;
                        connectList->push_back(coord);
                    }
                }
            } // if( !knowPt )
        }
    }

    // On inscrit les rectangle sur l'image de sortie
    std::vector<int*> *outList = new std::vector<int*>();
    for (unsigned n = 0; n < connectList->size(); ++n) {
        x1 = (*connectList)[n][0]; y1 = (*connectList)[n][1];
        x2 = (*connectList)[n][2]; y2 = (*connectList)[n][3];

        // Ce filtrage est empirique mais permet d'éliminer facilement la majorité des faux positifs
        unsigned width = x2 - x1;
        unsigned height = y2 - y1;
        float ratio = height / (float)width;

        // On vérifit d'abord le ration (la plaque est un rectangle)
        if( ratio > ratio_min && ratio < ratio_max ) {
            printf("DEBUG :: n=%d :: Width: %d  Height: %d - Ratio: %f\n", n, width, height, ratio);

            // On vérifit que le rectangle n'est pas trop petit ou trop grand
            if( width > width_min && width < width_max && height > height_min && height < height_max) {
                out->drawRect(x1, y1, x2, y2);
                printf("DEBUG :: n=%d :: x1:%d y1:%d x2:%d y2:%d\n", n, x1, y1, x2, y2);
                outList->push_back((*connectList)[n]);
            }
        }
    }
    delete connectList;

    return outList;
}

/*
 * Fonctions du TP2
 */
void copy(float** src, float** dst, int lgth, int wdth){
    //dirty
    int i,j;
    for(i=0;i<lgth;i++)
    for(j=0;j<wdth;j++){
        dst[i][j] = src[i][j];
    }
}

void ReplaceCenter(float** mat, float** out, int lgth, int wdth){
    int halfLenght = lgth/2, halfWidth = wdth/2;
    int i,j;

    for (i = 0; i < halfLenght; i++)
    for (j = 0; j < halfWidth; j++){
        out[i][j] = mat[i+halfLenght][j+halfWidth];
        out[i+halfLenght][j+halfWidth] = mat[i][j];
    }
    for (i = halfLenght; i < lgth; i++)
    for (j = 0; j < halfWidth; j++){
        out[i][j] = mat[i-halfLenght][j+halfWidth];
        out[i-halfLenght][j+halfWidth] = mat[i][j];
    }
}

void Conv(float** Img1, float** Img2, float** ImgConv, int lgth, int wdth){
    float** MatriceImgR1=fmatrix_allocate_2d(lgth,wdth);
    float** MatriceImgI1=fmatrix_allocate_2d(lgth,wdth);
    float** MatriceImgR2=fmatrix_allocate_2d(lgth,wdth);
    float** MatriceImgI2=fmatrix_allocate_2d(lgth,wdth);
    float** MatriceImgConvR=fmatrix_allocate_2d(lgth,wdth);
    float** MatriceImgConvI=fmatrix_allocate_2d(lgth,wdth);

    int i,j;

    copy(Img1, MatriceImgR1, lgth, wdth);
    ReplaceCenter(Img2, MatriceImgR2, lgth, wdth);

    for(i=0;i<lgth;i++)
    for(j=0;j<wdth;j++){
        MatriceImgI1[i][j]=0.0;
        MatriceImgI2[i][j]=0.0;
    }

    // On fait la FFT sur chaque image
    FFTDD(MatriceImgR1,MatriceImgI1,lgth,wdth);
    FFTDD(MatriceImgR2,MatriceImgI2,lgth,wdth);

    // On convolu (donc on multiplit)
    MultMatrix(MatriceImgConvR, MatriceImgConvI,
                         MatriceImgR1, MatriceImgI1,
                         MatriceImgR2, MatriceImgI2,
                         lgth, wdth);

    // FFT inverse
    IFFTDD(MatriceImgConvR,MatriceImgConvI,lgth,wdth);

    // On calcul le module
    Mod(ImgConv,MatriceImgConvR,MatriceImgConvI,lgth,wdth);

    //free time
    free_fmatrix_2d(MatriceImgI1);
    free_fmatrix_2d(MatriceImgR1);
    free_fmatrix_2d(MatriceImgI2);
    free_fmatrix_2d(MatriceImgR2);
    free_fmatrix_2d(MatriceImgConvR);
    free_fmatrix_2d(MatriceImgConvI);
}

void gaussianMask(float** mat, int lgth, int wdth, float sigma){
    // Creation du masque
    int halfLenght = lgth*0.5, halfWidth = wdth*0.5, halfSize = (6*sigma + 1)*0.5;
    int i,j;
    float coef=0.0;

    for(i=0;i<lgth;i++)
    for(j=0;j<wdth;j++){
            if(abs(halfLenght-i) < halfSize && abs(halfWidth-j) < halfSize) {
                mat[i][j] = GAUSS_FILTER_2D(i-halfLenght, j-halfWidth, sigma);
                coef += mat[i][j]; // calcul du coef
            }
            else
                mat[i][j] = 0.0;
        }

    coef = 1.0/coef;
    for(i=0;i<lgth;i++)
    for(j=0;j<wdth;j++){
        if(mat[i][j] > 0.0)
            mat[i][j] = coef*mat[i][j];
    }
}
