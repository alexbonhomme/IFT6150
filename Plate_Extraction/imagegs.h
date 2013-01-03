#ifndef IMAGEGS_H
#define IMAGEGS_H

#include <string>
using namespace std;

class ImageGS
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
    inline float  operator() (unsigned i, unsigned j) const { return m_img[i][j]; }

    /**
     * @brief getWidth
     * @return La largeur de l'image
     */
    inline unsigned getWidth() const { return m_width; }
    /**
     * @brief getHeight
     * @return La hauteur de l'image
     */
    inline unsigned getHeight() const { return m_height; }

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
     * Calcule le gradient vertical (du moins pour obtenir les edges verticaux)
     * @return Le gradient de l'image selon x
     */
    ImageGS* computeVerticalGradient();

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
    void thresholding();

    // Opérateurs morphologiques
    void erosion(float **mask, int size);
    void dilatation(float **mask, int size);
    void opening(float **mask, int size);
    void closing(float **mask, int size);

private:
    float** m_img;
    unsigned m_width, m_height;
};

#endif // IMAGEGS_H
