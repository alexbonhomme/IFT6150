#ifndef IMAGERGB_H
#define IMAGERGB_H

#include "image.h"
#include "imagegs.h"
#include <string>
using namespace std;

class ImageRGB : public Image
{
public:
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
     * @param filename
     */
    ImageRGB(const string &filename);

    /**
     * @brief ImageRGB
     * @param in
     */
    ImageRGB(const ImageGS &in);

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

    void drawRect(unsigned x1, unsigned y1, unsigned x2, unsigned y2);

private:
    RGBColor** m_img;
};

#endif // IMAGERGB_H
