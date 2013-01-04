#include <stdio.h>
#include <assert.h>
#include <iostream>
#include <vector>

#include "imagegs.h"
#include "imagergb.h"
#include "tools.h"

int main(int argc, char *argv[])
{

#if 1
    ImageGS car("../data/991211-001");
    //ImageGS car("../data/991211-002");
    //ImageGS car("../data/991211-003");
#else
    //ImageGS car("../data/991213-002");
    //ImageGS car("../data/991213-010");
    //ImageGS car("../data/991213-006");
    ImageGS car("../data/991213-005");
#endif
    ImageGS *grad = car.computeHorizontalGradient();
    grad->recal();
    grad->writePGM("0_vert_grad");

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
    grad->writePGM("1_vertical_filter");

    // binarisation par seuillage global
    grad->thresholdingSmart(0.9);
    grad->writePGM("2_bin_filter");

    // Opération morphologique pour éliminer le bruit, etc..
    int width, height;
    float** mask;

#if 0
    width=1; height=3;
    mask = new float*[height];
    for (int i = 0; i < height; ++i) {
        mask[i] = new float[width];
        for (int j = 0; j < width; ++j)
            mask[i][j] = 255.f;
    }
    grad->opening(mask, width, height);
    grad->writePGM("3_1_1_erode_grad");
#endif

#if 0
    width=3; height=1;
    mask = new float*[height];
    for (int i = 0; i < height; ++i) {
        mask[i] = new float[width];
        for (int j = 0; j < width; ++j)
            mask[i][j] = 255.f;
    }
    grad->opening(mask, width, height);
    grad->writePGM("3_1_2_erode_grad");
#endif

#if 1
    width=12; height=1;
    mask = new float*[height];
    for (int i = 0; i < height; ++i) {
        mask[i] = new float[width];
        for (int j = 0; j < width; ++j)
            mask[i][j] = 255.f;
    }
    grad->closing(mask, width, height);
    grad->writePGM("3_2_close_grad");
#endif

#if 1
    width=1; height=9;
    mask = new float*[height];
    for (int i = 0; i < height; ++i) {
        mask[i] = new float[width];
        for (int j = 0; j < width; ++j)
            mask[i][j] = 255.f;
    }
    grad->closing(mask, width, height);
    grad->writePGM("3_3_close_grad");
#endif

#if 1
    width=6; height=1;
    mask = new float*[height];
    for (int i = 0; i < height; ++i) {
        mask[i] = new float[width];
        for (int j = 0; j < width; ++j)
            mask[i][j] = 255.f;
    }

    grad->opening(mask, width, height);
    grad->writePGM("4_open_grad");
#endif

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


    // On applique les rectangle sur l'image de base pour visualiser
    // les zone detectées comme potentiellement des plaques d'immatriculation
    ImageRGB *car_detect = new ImageRGB(car);
    std::vector<int*> *listPlate = foundConnectedComponents(*grad, car_detect);
    car_detect->writePPM("5_plate_detect");

    // On construit la liste des (potentiels) plaque extraites de l'image
    std::vector<ImageGS*> *listImgPlate = new std::vector<ImageGS*>();
    for (unsigned n = 0; n < listPlate->size(); ++n) {
        int x1 = (*listPlate)[n][0], y1 = (*listPlate)[n][1],
            x2 = (*listPlate)[n][2], y2 = (*listPlate)[n][3];
        unsigned w = x2-x1, h = y2-y1;

        // On crée la miniature
        ImageGS *img = new ImageGS(w, h);
        for (unsigned i = 0; i < h; ++i)
        for (unsigned j = 0; j < w; ++j) {
            (*img)(i, j) = car(i+y1, j+x1);
        }

        // On effectué un derniere verification basé sur les edge de la plaque
        ImageGS *plate_grad = new ImageGS(*img);
        //plate_grad->thresholdingHist(0.8);
        plate_grad->inverse();
        plate_grad->thresholdingOstu();

        //plate_grad = plate_grad->computeHorizontalGradient();
        //plate_grad->recal();
#if 1
    width=1; height=3;
    mask = new float*[height];
    for (int i = 0; i < height; ++i) {
        mask[i] = new float[width];
        for (int j = 0; j < width; ++j)
            mask[i][j] = 255.f;
    }

    plate_grad->opening(mask, width, height);
#endif

    char filename[100];
    sprintf(filename, "t_plate_%d", n);
    plate_grad->writePGM(filename);

        //plate_grad->writePGM("grad");
        int var1=0, var2=0, var3=0;
        for (unsigned j = 1; j < w; ++j) {
            // on compte le nombre de variation
            if((*plate_grad)(h/3, j) != (*plate_grad)(h/3, j-1))
                ++var1;
            if((*plate_grad)(h/2, j) != (*plate_grad)(h/2, j-1))
                ++var2;
            if((*plate_grad)(h-h/3, j) != (*plate_grad)(h-h/3, j-1))
                ++var3;
        }

        int T = 11; // seuil de variation (empirique)
        std::cout << "Variations (seuil "<< T << ") - H/3: " << var1 <<
                     " H/2: " << var2 <<
                     " H-H/3: " << var3 << "\n" << std::endl;

        if( var1 > T && var2 > T && var3 > T)
            // ajout a la list
            listImgPlate->push_back(img);
    }

    // Sauvegarde des plaques reconnues
    char filename[100];
    for (unsigned n = 0; n < listImgPlate->size(); ++n) {
        sprintf(filename, "plate_%d", n);
        (*listImgPlate)[n]->writePGM(filename);
    }

    std::cout << "\n>> Programme terminé : " << listImgPlate->size() << " plaques ont/a été reconnue(s).\n";

    delete grad;
    delete car_detect;
    delete[] mask; // free

    std::cout << "\nHappy Ending !\n";

    return 0;
}
