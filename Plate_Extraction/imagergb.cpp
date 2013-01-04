#include "imagergb.h"
#include <stdio.h>
#include <assert.h>

ImageRGB::ImageRGB(unsigned width, unsigned height) :
    Image(width, height)
{
    m_img = new RGBColor*[m_height];
    for (unsigned i = 0; i < m_height; ++i)
        m_img[i] = new RGBColor[m_width];
}

ImageRGB::ImageRGB(const ImageRGB &in) :
    Image(in.getWidth(), in.getHeight())
{
    m_img = new RGBColor*[m_height];
    for (unsigned i = 0; i < m_height; ++i) {
        m_img[i] = new RGBColor[m_width];
        for (unsigned j = 0; j < m_width; ++j)
            m_img[i][j] = in(i, j);
    }
}

ImageRGB::ImageRGB(const string &filename) :
    Image(0, 0)
{
    string full_filename(filename);
    full_filename.append(".ppm");

    // Ouverture du flux
    FILE* f = fopen(full_filename.c_str(), "r");
    assert(f != NULL);

    // Lecture du header
    char buf[100];
    fgets(buf, 100, f);
    fgets(buf, 100, f);
    fscanf(f,"%d %d\n", &m_width, &m_height);
    fgets(buf, 100, f);

    // Lecture des données
    unsigned char tmp;
    m_img = new RGBColor*[m_height];
    for (unsigned i = 0; i < m_height; ++i) {
        m_img[i] = new RGBColor[m_width];
        for (unsigned j = 0; j < m_width; ++j) {
            fread(&tmp, 1, 1, f);
            m_img[i][j].r = tmp;
            fread(&tmp, 1, 1, f);
            m_img[i][j].g = tmp;
            fread(&tmp, 1, 1, f);
            m_img[i][j].b = tmp;
        }
    }

    // Fermeture du flux
    fclose(f);
}

ImageRGB::ImageRGB(const ImageGS &in) :
    Image(in.getWidth(), in.getHeight())
{
    m_img = new RGBColor*[m_height];
    for (unsigned i = 0; i < m_height; ++i) {
        m_img[i] = new RGBColor[m_width];
        for (unsigned j = 0; j < m_width; ++j)
            m_img[i][j].r = m_img[i][j].g = m_img[i][j].b = in(i, j);
    }
}

ImageRGB::~ImageRGB(){
    delete[] m_img;
}

void ImageRGB::readPPM(const string &filename) {
    string full_filename(filename);
    full_filename.append(".ppm");

    // Ouverture du flux
    FILE* f = fopen(full_filename.c_str(), "r");
    assert(f != NULL);

    // Lecture du header
    char buf[100];
    fgets(buf, 100, f);
    fgets(buf, 100, f);
    fscanf(f,"%d %d\n", &m_width, &m_height);
    fgets(buf, 100, f);

    // Lecture des données
    unsigned char tmp;
    for (unsigned i = 0; i < m_height; ++i)
    for (unsigned j = 0; j < m_width; ++j) {
        fread(&tmp, 1, 1, f);
        m_img[i][j].r = tmp;
        fread(&tmp, 1, 1, f);
        m_img[i][j].g = tmp;
        fread(&tmp, 1, 1, f);
        m_img[i][j].b = tmp;
    }

    // Fermeture du flux
    fclose(f);
}

void ImageRGB::writePPM(const string &filename) {
    string full_filename(filename);
    full_filename.append(".ppm");

    // Ouverture du flux
    FILE* f = fopen(full_filename.c_str(), "w");
    assert(f != NULL);

    // Ecriture du header
    fprintf(f,"P6\n#\n%d %d\n255\n", m_width, m_height);

    // Ecriture des données
    for (unsigned i = 0; i < m_height; ++i)
    for (unsigned j = 0; j < m_width; ++j)
        fprintf(f, "%c%c%c", (char)m_img[i][j].r, (char)m_img[i][j].g, (char)m_img[i][j].b);

    // Fermeture du flux
    fclose(f);
}

void ImageRGB::drawRect(unsigned x1, unsigned y1, unsigned x2, unsigned y2) {
    for (unsigned i = 0; i < m_height; ++i)
    for (unsigned j = 0; j < m_width; ++j) {
        if(((i == y1 || i == y2) && j >= x1 && j <= x2) ||
           ((j == x1 || j == x2) && i >= y1 && i <= y2) ) {
            m_img[i][j].r = 255.f;
            m_img[i][j].g = 0.f;
            m_img[i][j].b = 0.f;
        }
    }
}
