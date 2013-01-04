#ifndef IMAGEGS_H
#define IMAGEGS_H

#include "image.h"
#include <string>
using namespace std;

class ImageGS : public Image
{
public:
    /**
     * @brief ImageGS
     * @param width
     * @param height
     */
    ImageGS(unsigned width, unsigned height);

    /**
     * @brief ImageGS
     * @param in
     */
    ImageGS(const ImageGS &in);

    /**
     * @brief ImageGS
     * @param filename
     */
    ImageGS(const string &filename);

    ~ImageGS();


    /**
     * @brief operator ()
     * @param i
     * @param j
     * @return
     */
    inline float& operator() (unsigned i, unsigned j) { return m_img[i][j]; }

    /**
     * @brief operator ()
     * @param i
     * @param j
     * @return
     */
    inline float operator() (unsigned i, unsigned j) const { return m_img[i][j]; }

    /**
     * @brief readPGM
     * @param filename
     */
    void readPGM(const string &filename);

    /**
     * @brief writePGM
     * @param filename
     */
    void writePGM(const string &filename);

    /**
     * Calcule le gradient horizontallement
     * @return Le gradient de l'image selon x
     */
    ImageGS* computeHorizontalGradient();

    /**
     * Calcule le gradient verticallement
     * @return Le gradient de l'image selon y
     */
    ImageGS* computeVerticalGradient();

    /**
     * Calcule la projection horizontal de l'image
     * @brief computeHorizontalProjection
     * @return Un vecteur de taille m_width
     */
    float* computeHorizontalProjection();

    /**
     * Calcule la projection verticale de l'image
     * @brief computeVerticalProjection
     * @return Un vecteur de taille m_height
     */
    float* computeVerticalProjection();

    /**
     * @brief computeHistogram
     * @return
     */
    float *computeHistogram();

    /**
     * Recal les valeur de la matrice image entre 0 et 255
     * @brief recal
     */
    void recal();

    /**
     * Seuillage de l'image (binarisation)
     * @brief thresholding
     * @param t
     */
    void thresholding(float t);

    /**
     * @brief thresholdingHist
     * @param p
     */
    void thresholdingHist(float p);

    /**
     * Binarisation selon la technique de Otsu
     * @brief thresholding
     */
    void thresholdingOstu();


    void thresholdingSmart(float p);

    // Opérateurs morphologiques
    void erosion(float **mask, int width, int height);
    void dilatation(float **mask, int width, int height);
    void opening(float **mask, int width, int height);
    void closing(float **mask, int width, int height);

private:
    float** m_img;
};

#endif // IMAGEGS_H
