#include <stdio.h>
#include <assert.h>
#include <iostream>

#include "imagegs.h"
#include "imagergb.h"
#include "tools.h"

int main(int argc, char *argv[])
{

    ImageGS car("991211-001");
    //ImageGS car("991213-002");
    ImageGS *grad = car.computeHorizontalGradient();
    grad->recal();
    grad->writePGM("vert_grad");

    // Projection horizontal
    float* vect;
    vect = grad->computeHorizontalProjection();
    writeVect(vect, grad->getHeight(), "data_brut.dat");

    // Filtre gaussien du vecteur de projection
    guassianFilter(vect, grad->getHeight(), 6, 0.05f);
    writeVect(vect, grad->getHeight(), "data_filter.dat");

    /* Seuillage 'vertical' on ne garde que les ligne qui on une projection supérieur a T */
    // averaging
    float avg = 0.;
    for (unsigned i = 0; i < grad->getHeight(); ++i)
        avg += vect[i];
    avg /= grad->getHeight();

    // T
    float T = 1.2f*avg;

    // seuillage vertical
    for (unsigned i = 0; i < grad->getHeight(); ++i) {
        // si la projection de cette ligne est sous le seuil on la supprime
        if(vect[i] < T) {
            for (unsigned j = 0; j < grad->getWidth(); ++j)
                (*grad)(i, j) = 0.f;
        }
    }
    //grad->recal();
    grad->writePGM("vertical_filter");

    // binarisation par seuillage global
    grad->thresholdingSmart(0.9);
    grad->writePGM("bin_filter");

    // Opération morphologique pour éliminer le bruit, etc..
    int width, height;
    float** mask;

#if 0
    width=7; height=7;
    mask = new float*[height];
    for (int i = 0; i < height; ++i) {
        mask[i] = new float[width];
        for (int j = 0; j < width; ++j)
            mask[i][j] = 255.f;
    }
    grad->closing(mask, width, height);
    grad->writePGM("close_grad");
#endif

#if 1
    width=1; height=9;
    mask = new float*[height];
    for (int i = 0; i < height; ++i) {
        mask[i] = new float[width];
        for (int j = 0; j < width; ++j)
            mask[i][j] = 255.f;
    }

    grad->opening(mask, width, height);
    grad->writePGM("open_grad");
#endif

#if 1
    width=7; height=7;
    mask = new float*[height];
    for (int i = 0; i < height; ++i) {
        mask[i] = new float[width];
        for (int j = 0; j < width; ++j)
            mask[i][j] = 255.f;
    }
    grad->closing(mask, width, height);
    grad->writePGM("close_grad");
#endif

    // On vérifit la 8 connexité



    delete grad; // free

    ImageGS dilate(erode);
    dilate.dilatation(mask, width, height);

    // On ne veux garder que les contours, donc on va soustraire les deux images
    for (unsigned i = 0; i < dilate.getHeight(); ++i)
    for (unsigned j = 0; j < dilate.getWidth(); ++j) {
        dilate(i, j) -= erode(i, j);
    }
    dilate.writePGM("rect");

    // On applique les rectangle sur l'image de base pour visualiser
    // les zone detectées comme potentiellement des plaques d'immatriculation
    ImageRGB car_detect(car);
    for (unsigned i = 0; i < dilate.getHeight(); ++i)
    for (unsigned j = 0; j < dilate.getWidth(); ++j) {
        if(dilate(i, j) == 255.f) {
            car_detect(i, j)[0] = 255.f;
            car_detect(i, j)[1] = 0.f;
            car_detect(i, j)[2] = 0.f;
        }
    }
    car_detect.writePPM("plate_detect");



    delete[] mask; // free

    std::cout << "Happy Ending !\n";

    return 0;
}
