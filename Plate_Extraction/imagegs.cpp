#include "imagegs.h"
#include <iostream>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include "tools.h"

#define _USE_MATH_DEFINES
#define GAUSS_FILTER_2D(x,y, var) (exp(-((x*x)+(y*y))/(2*var))/(2.0*M_PI*var))

#define GRAY_SCALE 256

ImageGS::ImageGS(unsigned width, unsigned height) :
    Image(width, height)
{
    m_img = new float*[m_height];
    for (unsigned i = 0; i < m_height; ++i)
        m_img[i] = new float[m_width];
}

ImageGS::ImageGS(const ImageGS &in) :
    Image(in.getWidth(), in.getHeight())
{
    m_img = new float*[m_height];
    for (unsigned i = 0; i < m_height; ++i) {
        m_img[i] = new float[m_width];
        for (unsigned j = 0; j < m_width; ++j)
            m_img[i][j] = in(i, j);
    }
}

ImageGS::ImageGS(const ImageRGB &in) :
    Image(in.getWidth(), in.getHeight())
{
    m_img = new float*[m_height];
    for (unsigned i = 0; i < m_height; ++i) {
        m_img[i] = new float[m_width];
        for (unsigned j = 0; j < m_width; ++j)
            m_img[i][j] = 0.114*in(i, j)[0] + 0.587*in(i, j)[1] + 0.299*in(i, j)[2];
    }

}

ImageGS::ImageGS(const string &filename) :
    Image(0, 0)
{
    string full_filename(filename);
    full_filename.append(".pgm");

    // Ouverture du flux
    FILE* f = fopen(full_filename.c_str(), "r");
    assert(f != NULL);

    std::cout << "Lecture du fichier PGM : " << full_filename << std::endl;

    // Lecture du header
    char buf[100];
    fgets(buf, 100, f);
    fgets(buf, 100, f);
    fscanf(f,"%d %d\n", &m_width, &m_height);
    fgets(buf, 100, f);

    // Initialisation de la matrice / Lecture des données
    unsigned char tmp;
    m_img = new float*[m_height];
    for (unsigned i = 0; i < m_height; ++i) {
        m_img[i] = new float[m_width];
        for (unsigned j = 0; j < m_width; ++j) {
            fread(&tmp, 1, 1, f);
            m_img[i][j] = tmp;
        }
    }

    // Fermeture du flux
    fclose(f);
}

ImageGS::~ImageGS() {
    delete[] m_img;
}

void ImageGS::readPGM(const string &filename) {
    string full_filename(filename);
    full_filename.append(".pgm");

    // Ouverture du flux
    FILE* f = fopen(full_filename.c_str(), "r");
    assert(f != NULL);

    std::cout << "Lecture du fichier PGM : " << full_filename << std::endl;

    // Lecture du header
    char buf[100];
    fgets(buf, 100, f);
    fgets(buf, 100, f);
    fscanf(f,"%d %d", &m_width, &m_height);
    fgets(buf, 100, f);

    // Lecture des données
    unsigned char tmp;
    for (unsigned i = 0; i < m_height; ++i)
    for (unsigned j = 0; j < m_width; ++j) {
        fread(&tmp, 1, 1, f);
        m_img[i][j] = tmp;
    }

    // Fermeture du flux
    fclose(f);
}

void ImageGS::writePGM(const string &filename) {
    string full_filename(filename);
    full_filename.append(".pgm");

    // Ouverture du flux
    FILE* f = fopen(full_filename.c_str(), "w");
    assert(f != NULL);

    std::cout << "Sauvergarde du fichier PGM : " << full_filename << std::endl;

    // Ecriture du header
    fprintf(f,"P5\n#\n%d %d\n255\n", m_width, m_height);

    // Ecriture des données
    for (unsigned i = 0; i < m_height; ++i)
    for (unsigned j = 0; j < m_width; ++j)
        fprintf(f, "%c", (char)m_img[i][j]);

    // Fermeture du flux
    fclose(f);
}

ImageGS* ImageGS::computeHorizontalGradient() {
    ImageGS *gradient = new ImageGS(m_width, m_height);

    for (unsigned i = 0; i < m_height; ++i)
    for (unsigned j = 0; j < m_width; ++j) {
        // masque 1 0 -1
        if(j == 0){
            (*gradient)(i, j) = fabs(m_img[i][j+1] - m_img[i][j]);
        } else if(j == m_width-1){
            (*gradient)(i, j) = fabs(m_img[i][j] - m_img[i][j-1]);
        } else {
            (*gradient)(i, j) = fabs((m_img[i][j+1] - m_img[i][j-1])*0.5);
        }
    }

    return gradient;
}

ImageGS* ImageGS::computeVerticalGradient() {
    ImageGS *gradient = new ImageGS(m_width, m_height);

    for (unsigned i = 0; i < m_height; ++i)
    for (unsigned j = 0; j < m_width; ++j) {
        // masque 1
        //        0
        //       -1
        if(i == 0){
            (*gradient)(i, j) = fabs(m_img[i+1][j] - m_img[i][j]);
        } else if(i == m_height-1){
            (*gradient)(i, j) = fabs(m_img[i][j] - m_img[i-1][j]);
        } else {
            (*gradient)(i, j) = fabs((m_img[i+1][j] - m_img[i-1][j])*0.5);
        }
    }

    return gradient;
}

float *ImageGS::computeHorizontalProjection() {
    float* h_projection = new float[m_height];
    for (unsigned i = 0; i < m_height; ++i) {
        h_projection[i] = 0.f;
        for (unsigned j = 0; j < m_width; ++j)
            h_projection[i] += m_img[i][j];
    }

    return h_projection;
}

float *ImageGS::computeVerticalProjection() {
    float* v_projection = new float[m_width];
    for (unsigned j = 0; j < m_width; ++j) {
        v_projection[j] = 0.f;
        for (unsigned i = 0; i < m_height; ++i)
            v_projection[j] += m_img[i][j];
    }

    return v_projection;
}

float *ImageGS::computeHistogram() {
    float* hist = new float[GRAY_SCALE];
    for (int i = 0; i < GRAY_SCALE; ++i)
        hist[i] = 0.f;

    for (unsigned i = 0; i < m_height; ++i)
    for (unsigned j = 0; j < m_width; ++j)
        hist[(unsigned)m_img[i][j]]++;

    return hist;
}

//TODO resultats erronés
float ImageGS::computeVariance() {
    // On calcule les pi
    float* p = computeHistogram();
    float nPixels = m_height*m_width;
    for (int i = 0; i < GRAY_SCALE; ++i)
        p[i] /= nPixels;

    // Calcule de la moyenne des pixels de l'image
    float avg = 0.f;
    for (unsigned i = 0; i < m_height; ++i)
    for (unsigned j = 0; j < m_width; ++j)
        avg += m_img[i][j];
    avg /= nPixels;


    // On estime la variance
    float var = 0.f;
    for (unsigned i = 0; i < m_height; ++i)
    for (unsigned j = 0; j < m_width; ++j) {
        var += p[(unsigned)m_img[i][j]]*pow(m_img [i][j], 2.f);
    }

    // on soustrait le barycentre
    return var - avg;
}

void ImageGS::recal() {
    // Recherche du minimum
    float min = m_img[0][0];
    for (unsigned i = 0; i < m_height; ++i)
    for (unsigned j = 0; j < m_width; ++j)
        if (m_img[i][j] < min)
            min = m_img[i][j];

    for (unsigned i = 0; i < m_height; ++i)
    for (unsigned j = 0; j < m_width; ++j)
        m_img[i][j] -= min;

    // Recherche maximum
    float max = m_img[0][0];
    for (unsigned i = 0; i < m_height; ++i)
    for (unsigned j = 0; j < m_width; ++j)
        if (m_img[i][j] > max)
            max = m_img[i][j];

    // Recal
    for (unsigned i = 0; i < m_height; ++i)
    for (unsigned j = 0; j < m_width; ++j)
        m_img[i][j] *= 255.f / max;
}

void ImageGS::inverse() {
    for (unsigned i = 0; i < m_height; ++i)
    for (unsigned j = 0; j < m_width; ++j)
        m_img[i][j] = (GRAY_SCALE-1) - m_img[i][j];
}

/*
 * Implémentation par boucle car les image ne respecte pas
 * toutes les prérequis pour fourrier
 */
void ImageGS::gaussianFilter(float sigma) {
    // Creation du masque
    int halfSize = (6*sigma + 1)*0.5;
    float coef = 0.f;

    float mask[halfSize*2][halfSize*2];
    for(int i = -halfSize; i < halfSize; ++i)
    for(int j = -halfSize; j < halfSize; ++j) {
        mask[i+halfSize][j+halfSize] = GAUSS_FILTER_2D(i, j, sigma);
        coef += mask[i+halfSize][j+halfSize]; // calcul du coef

    }

    coef = 1.0/coef;
    for(int i = -halfSize; i < halfSize; ++i)
    for(int j = -halfSize; j < halfSize; ++j) {
        mask[i][j] *= coef;
    }

    // Filtrage
    float filtered[m_height][m_width];
    for (unsigned i = halfSize; i < m_height-halfSize; ++i)
    for (unsigned j = halfSize; j < m_width-halfSize; ++j) {
        filtered[i][j] = 0.f;
        for(int i_m = -halfSize; i_m < halfSize; ++i_m)
        for(int j_m = -halfSize; j_m < halfSize; ++j_m) {
            filtered[i][j] += m_img[i+i_m][j+j_m] * mask[i_m+halfSize][j_m+halfSize];
        }
    }

    // Recopie
    for (unsigned i = halfSize; i < m_height-halfSize; ++i)
    for (unsigned j = halfSize; j < m_width-halfSize; ++j)
        m_img[i][j] = filtered[i][j];
}

void ImageGS::thresholding(float t) {
    for (unsigned i = 0; i < m_height; ++i)
    for (unsigned j = 0; j < m_width; ++j)
        m_img[i][j] =  m_img[i][j] > t ? 255.f : 0.f;
}

void ImageGS::thresholdingHist(float p) {
    float* hist = computeHistogram();

    // Calcul de l'histogramme cumulé
    float histAcc[GRAY_SCALE];
    histAcc[0] = hist[0];
    for (int i = 1; i < GRAY_SCALE; ++i) {
        histAcc[i] = histAcc[i-1] + hist[i];
    }
    delete[] hist;

    // Détermination du seuil à p pourcent de l'histogramme commulé
    float T;
    int i = 0;
    //printf("%f %f", histAcc[0], p*histAcc[GRAY_SCALE-1]);
    while ( histAcc[i] < p*histAcc[GRAY_SCALE-1]) {
        T = i;
        ++i;
    }

    // Seuillage
    std::cout << "Seuillage par Histogramme (seuil calculé: " << T << ")\n";
    thresholding(T);
}

void ImageGS::thresholdingSmart(float p) {
    float* hist = computeHistogram();

    // Calcul de l'histogramme cumulé
    // sans la valeur 0
    float histAcc[GRAY_SCALE-1];
    histAcc[0] = hist[1];
    for (int i = 1; i < GRAY_SCALE-1; ++i) {
        histAcc[i] = histAcc[i-1] + hist[i+1];
    }
    delete[] hist;

    // Détermination du seuil à p pourcent de l'histogramme commulé
    float T;
    int i = 0;
    while ( histAcc[i] < p*histAcc[GRAY_SCALE-2])
        T = ++i;

    // Seuillage
    std::cout << "Seuillage par Histogramme (seuil calculé: " << T << ")\n";
    thresholding(T);
}

void ImageGS::thresholdingOstu() {
    // Calcul de l'histogramme
    float* hist = computeHistogram();

    // normalisation
    for (int i = 0; i < GRAY_SCALE; ++i)
        hist[i] /= (float)(m_height*m_width);

    // Calcule de la variance inter-classe
    float s[GRAY_SCALE] = {0.f};
    float mu_t = 0.f;
    for (int j = 0; j < GRAY_SCALE; ++j) { // Calcul de mu(256)
        mu_t += (j+1)*hist[j];
    }

    for (int i = 1; i < GRAY_SCALE-1; ++i) { // trick, sa ne sers a rien de calculer 0 et 255
        float mu_k = 0.f, w_k = 0.f;
        for (int j = 0; j < i; ++j) {
            mu_k += (j+1)*hist[j];  // mu(k)
            w_k += hist[j];         // w(k)
        }
        // calcule de la variance en k
        s[i] = w_k * (1.f - w_k)* pow(mu_t*w_k - mu_k, 2.f);
    }
    delete[] hist;

    // Maximisation de la variance pour trouver T (le seuil)
    float T, max = 0.f;
    for (int i = 1; i < GRAY_SCALE-1; ++i) {
        if( s[i] > max ) {
            max = s[i];
            T = (float)i;
        }
    }

    // Seuillage
    std::cout << "Seuillage par Otsu (seuil calculé: " << T << ")\n";
    thresholding(T);
}

void ImageGS::erosion(float** mask, int width, int height) {
    float erodedImage[m_height][m_width];

    bool keepPixel;
    for (unsigned i = 0; i < m_height; ++i)
    for (unsigned j = 0; j < m_width; ++j) {
        keepPixel = true;

        for (int m = -(height/2); m < (height/2)+(height%2); ++m)
        for (int n = -(width/2); n < (width/2)+(width%2); ++n) {
            int cur_i = i+m; int cur_j = j+n;
            if(cur_i >= 0 && cur_i < (int)m_height && cur_j >= 0 && cur_j < (int)m_width) {
                if(m_img[cur_i][cur_j] != mask[m+height/2][n+width/2])
                    keepPixel = false;
            }

        }

        // Si un des pixel ne correspond pas au masque on ne le garde pas
        erodedImage[i][j] = keepPixel ? 255.f : 0.f;
    }

    for (unsigned i = 0; i < m_height; ++i)
    for (unsigned j = 0; j < m_width; ++j)
        m_img[i][j] =  erodedImage[i][j];
}

void ImageGS::dilatation(float **mask, int width, int height) {
    float dilatedImage[m_height][m_width];

    for (unsigned i = 0; i < m_height; ++i)
    for (unsigned j = 0; j < m_width; ++j)
        if (m_img[i][j] == 255.f){
            for (int m = -(height/2); m < (height/2)+height%2; ++m)
            for (int n = -(width/2); n < (width/2)+width%2; ++n) {
                int cur_i = i+m; int cur_j = j+n;
                if(cur_i >= 0 && cur_i < (int)m_height && cur_j >= 0 && cur_j < (int)m_width) {
                    dilatedImage[cur_i][cur_j] = mask[m+height/2][n+width/2];
                }
            }
        }

    for (unsigned i = 0; i < m_height; ++i)
    for (unsigned j = 0; j < m_width; ++j)
        m_img[i][j] =  dilatedImage[i][j];

}

void ImageGS::opening(float **mask, int width, int height) {
    erosion(mask, width, height);
    dilatation(mask, width, height);
}

void ImageGS::closing(float **mask, int width, int height) {
    dilatation(mask, width, height);
    erosion(mask, width, height);
}
