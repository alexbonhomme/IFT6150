#include "imagegs.h"
#include <iostream>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include "tools.h"

ImageGS::ImageGS(unsigned width, unsigned height) :
    m_width(width),
    m_height(height)
{

    m_img = new float*[height];
    for (unsigned i = 0; i < height; ++i)
        m_img[i] = new float[width];
}

ImageGS::ImageGS(const ImageGS &in) :
    m_width(in.getWidth()),
    m_height(in.getHeight())
{
    m_img = new float*[m_height];
    for (unsigned i = 0; i < m_height; ++i) {
        m_img[i] = new float[m_width];
        for (unsigned j = 0; j < m_width; ++j)
            m_img[i][j] = in(i, j);
    }
}
ImageGS::ImageGS(const string &filename) :
    m_width(0),
    m_height(0)
{
    string full_filename(filename);
    full_filename.append(".pgm");

    // Ouverture du flux
    FILE* f = fopen(full_filename.c_str(), "r");
    assert(f != NULL);

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

void ImageGS::readPGM(const string &filename) {
    string full_filename(filename);
    full_filename.append(".pgm");

    // Ouverture du flux
    FILE* f = fopen(full_filename.c_str(), "r");
    assert(f != NULL);

    // Lecture du header
    char buf[100];
    fgets(buf, 100, f);
    fgets(buf, 100, f);
    fscanf(f,"%d %d", &m_width, &m_height);
    fscanf(f,"\n");

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

    // Ecriture du header
    fprintf(f,"P5\n#\n%d %d\n255\n", m_width, m_height);

    // Ecriture des données
    for (unsigned i = 0; i < m_height; ++i)
    for (unsigned j = 0; j < m_width; ++j)
        fprintf(f, "%c", (char)m_img[i][j]);

    // Fermeture du flux
    fclose(f);
}

ImageGS* ImageGS::computeVerticalGradient() {
    ImageGS *gradient = new ImageGS(m_width, m_height);

    for (unsigned i = 0; i < m_height; ++i)
    for (unsigned j = 0; j < m_width; ++j) {
        // masque 1 0 -1
        if(j == 0){
            (*gradient)(i, j) = fabs(m_img[i][j] - m_img[i][j+1]);
        } else if(j == m_width-1){
            (*gradient)(i, j) = fabs(m_img[i][j-1] - m_img[i][j]);
        } else {
            (*gradient)(i, j) = fabs((m_img[i][j-1] - m_img[i][j+1])*0.5);
        }
    }

    return gradient;
}

void ImageGS::recal() {
    float max, min;

    // Recherche du minimum
    min = m_img[0][0];
    for (unsigned i = 0; i < m_height; ++i)
    for (unsigned j = 0; j < m_width; ++j)
        if (m_img[i][j] < min)
            min = m_img[i][j];

    for (unsigned i = 0; i < m_height; ++i)
    for (unsigned j = 0; j < m_width; ++j)
        m_img[i][j] -= min;

    // Recherche maximum
    max = m_img[0][0];
    for (unsigned i = 0; i < m_height; ++i)
    for (unsigned j = 0; j < m_width; ++j)
        if (m_img[i][j] > max)
            max = m_img[i][j];

    // Recal
    for (unsigned i = 0; i < m_height; ++i)
    for (unsigned j = 0; j < m_width; ++j)
        m_img[i][j] *= 255. / max;
}

void ImageGS::thresholding(float t) {
    for (unsigned i = 0; i < m_height; ++i)
    for (unsigned j = 0; j < m_width; ++j)
        m_img[i][j] =  m_img[i][j] > t ? 255.f : 0.f;
}
void ImageGS::thresholding() {
    // Détermination du seuil (moyenne)
    double acc = 0.;
    for (unsigned i = 0; i < m_height; ++i)
    for (unsigned j = 0; j < m_width; ++j)
        acc += m_img[i][j];

    float t = acc/(m_height*m_width);
    for (unsigned i = 0; i < m_height; ++i)
    for (unsigned j = 0; j < m_width; ++j)
        m_img[i][j] =  m_img[i][j] > t ? 255.f : 0.f;
}

void ImageGS::erosion(float** mask, int size) {
    float erodedImage[m_height][m_width];

    bool keepPixel;
    for (unsigned i = 0; i < m_height; ++i)
    for (unsigned j = 0; j < m_width; ++j) {
        keepPixel = true;
        for (int m = -(size/2); m < (size/2)+1; ++m)
            for (int n = -(size/2); n < (size/2)+1; ++n) {
            unsigned cur_i = i+m; unsigned cur_j = j+n;
            if(cur_i > 0 && cur_i < m_height && cur_j > 0 && cur_j < m_width) {
                if(m_img[cur_i][cur_j] != mask[m+size/2][n+size/2])
                    keepPixel = false;
            }
        }
/*
        if (i > 0 && m_img[i-1][j] == 0.f) {
            keepPixel = false;
        } else if (j > 0 && m_img[i][j-1] == 0.f) {
            keepPixel = false;
        } else if (i+1 < m_height && m_img[i+1][j] == 0.f) {
            keepPixel = false;
        } else if (j+1 < m_width && m_img[i][j+1] == 0.f) {
            keepPixel = false;
        }
*/
        // Si un des pixel ne correspond pas au masque on ne le garde pas
        erodedImage[i][j] = keepPixel ? 255.f : 0.f;
    }

    for (unsigned i = 0; i < m_height; ++i)
    for (unsigned j = 0; j < m_width; ++j)
        m_img[i][j] =  erodedImage[i][j];
}

void ImageGS::dilatation(float **mask, int size) {
    float dilatedImage[m_height][m_width];

    for (unsigned i = 0; i < m_height; ++i)
    for (unsigned j = 0; j < m_width; ++j)
        if (m_img[i][j] == 255.f){
            for (int m = -(size/2); m < (size/2)+1; ++m)
            for (int n = -(size/2); n < (size/2)+1; ++n) {
                unsigned cur_i = i+m; unsigned cur_j = j+n;
                if(cur_i > 0 && cur_i < m_height && cur_j > 0 && cur_j < m_width) {
                    dilatedImage[cur_i][cur_j] = mask[m+size/2][n+size/2];
                }
            }
/*
            dilatedImage[i][j] = mask[1][1];
            if (i > 0) dilatedImage[i-1][j] = mask[0][1];
            if (j > 0) dilatedImage[i][j-1] = mask[1][0];
            if (i+1 < m_height) dilatedImage[i+1][j] = mask[2][1];
            if (j+1 < m_width) dilatedImage[i][j+1] = mask[1][2];
*/
        }

    for (unsigned i = 0; i < m_height; ++i)
    for (unsigned j = 0; j < m_width; ++j)
        m_img[i][j] =  dilatedImage[i][j];

}

void ImageGS::opening(float **mask, int size) {
    erosion(mask, size);
    dilatation(mask, size);
}

void ImageGS::closing(float **mask, int size) {
    dilatation(mask, size);
    erosion(mask, size);
}
