#ifndef IMAGERGB_H
#define IMAGERGB_H

#include "image.h"
#include "imagegs.h"
#include <string>

using namespace std;
class ImageGS;

class ImageRGB : public Image
{
public:
    /**
     * @brief The RGBColor struct
     */
    struct RGBColor{
        float r;
        float g;
        float b;

        /**
         * @brief operator []
         * @param i
         * @return
         */
        inline float& operator[] (unsigned i) {
            if(i == 0)
                return r;
            else if(i == 1)
                return g;
            else
                return b;
        }

        /**
         * @brief operator []
         * @param i
         * @return
         */
        inline float  operator[] (unsigned i) const {
            if(i == 0)
                return r;
            else if(i == 1)
                return g;
            else
                return b;
        }
    };

    /**
     * @brief ImageRGB
     * @param width
     * @param height
     */
    ImageRGB(unsigned width, unsigned height);

    /**
     * @brief ImageRGB
     * @param in
     */
    ImageRGB(const ImageRGB &in);

    /**
     * @brief ImageRGB
     * @param in
     */
    ImageRGB(const ImageGS &in);

    /**
     * @brief ImageRGB
     * @param filename
     */
    ImageRGB(const string &filename);

    /**
     * Destructor
     */
    ~ImageRGB();

    /**
     * @brief operator ()
     * @param i
     * @param j
     * @return
     */
    inline RGBColor& operator() (unsigned i, unsigned j) { return m_img[i][j]; }

    /**
     * @brief operator ()
     * @param i
     * @param j
     * @return
     */
    inline RGBColor operator() (unsigned i, unsigned j) const { return m_img[i][j]; }

    /**
     * @brief readPPM
     * @param filename
     */
    void readPPM(const string &filename);

    /**
     * @brief writePPM
     * @param filename
     */
    void writePPM(const string &filename);

    /**
     * Dessine un rectangle rouge avec une diagonale allant du point (x1, y1) au point (x2, y2)
     * @brief drawRect
     * @param x1
     * @param y1
     * @param x2
     * @param y2
     */
    void drawRect(unsigned x1, unsigned y1, unsigned x2, unsigned y2);

private:
    RGBColor** m_img;
};

#endif // IMAGERGB_H
