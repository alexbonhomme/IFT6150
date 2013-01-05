#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <iostream>
#include <vector>
#include <ctime>

#include "imagegs.h"
#include "imagergb.h"
#include "tools.h"

#define RATIO_MIN 0.2f
#define RATIO_MAX 0.7f
#define WIDTH_MIN 30
#define WIDTH_MAX 130
#define HEIGHT_MIN 7
#define HEIGHT_MAX 60
#define T_1_PLATE 16
#define T_2_PLATE 10
#define MARGIN 2
#define T_BIN 0.9

int main(int argc, char *argv[]) {
    // Début compteur
    clock_t d_start, d_stop;
    d_start = clock();

    ImageGS *car;

    if(argc > 1) {
        car = new ImageGS(argv[1]);
    } else {

#if 0
    //car = new ImageGS("../data/991211-001");
    car = new ImageGS("../data/991211-002");
    //car = new ImageGS("../data/991211-003");
#else
    car = new ImageGS(ImageRGB("../data/0003"));
    //car = new ImageGS("../data/99122-2"); // plaque trop longue
    //car = new ImageGS("../data/991213-002");
    //car = new ImageGS("../data/991213-010");
    //car = new ImageGS("../data/991213-006");
    //car = new ImageGS("../data/991213-005");
#endif
    }
    // filtrage gaussien
    ImageGS gauss(*car);
    gauss.gaussianFilter(1.f);
    //car->recal();
    gauss.writePGM("0_gauss_filter");

    // Calcule du gradient
    ImageGS *grad = gauss.computeHorizontalGradient();
    grad->recal();
    grad->writePGM("0_vert_grad");

    // Projection horizontal
    float* vect;
    vect = grad->computeHorizontalProjection();
    writeVect(vect, grad->getHeight(), "data_brut.dat");

    // Filtre gaussien sur le vecteur de projection
    gaussianFilter(vect, grad->getHeight(), 6, 0.05f);
    writeVect(vect, grad->getHeight(), "data_filter.dat");

    /* Seuillage 'vertical' on ne garde que les ligne qui on une projection supérieur a T */
    // averaging
    float avg = 0.f;
    for (unsigned i = 0; i < grad->getHeight(); ++i)
        avg += vect[i];
    avg /= grad->getHeight();

    // T est empirique
    float T = 1.2f*avg;

    // seuillage vertical
    for (unsigned i = 0; i < grad->getHeight(); ++i) {
        // si la projection de cette ligne est sous le seuil on la supprime
        if(vect[i] < T) {
            for (unsigned j = 0; j < grad->getWidth(); ++j)
                (*grad)(i, j) = 0.f;
        }
    }
    delete[] vect; // free

    //grad->recal();
    grad->writePGM("1_vertical_filter");

    // binarisation par seuillage global
    grad->thresholdingSmart(T_BIN);
    grad->writePGM("2_bin_filter");

    // Opération morphologique pour éliminer le bruit, etc..
    int width, height;
    float** mask;

#if 1
    width=30; height=1;
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

    // On applique les rectangle sur l'image de base pour visualiser
    // les zone detectées comme potentiellement des plaques d'immatriculation
    ImageRGB *car_detect = new ImageRGB(*car);
    std::vector<int*> *listPlate = foundConnectedComponents(*grad, car_detect,
                                                            RATIO_MIN, RATIO_MAX,
                                                            WIDTH_MIN, WIDTH_MAX,
                                                            HEIGHT_MIN, HEIGHT_MAX);
    car_detect->writePPM("5_plate_detect");

    // On construit la liste des (potentiels) plaque extraites de l'image
    std::vector<ImageGS*> *listImgPlate = new std::vector<ImageGS*>();
    for (unsigned n = 0; n < listPlate->size(); ++n) {
        //  on fixe si possible une marge de quelques pixel (pour eviter le crop)
        int x1 = max((*listPlate)[n][0]-MARGIN, 0),
            y1 = max((*listPlate)[n][1]-MARGIN, 0),
            x2 = min((*listPlate)[n][2]+MARGIN, (int)car->getWidth()),
            y2 = min((*listPlate)[n][3]+MARGIN, (int)car->getHeight());
        delete[] (*listPlate)[n]; // free

        unsigned w = x2-x1, h = y2-y1;

        // On crée la miniature
        ImageGS *img = new ImageGS(w, h);
        for (unsigned i = 0; i < h; ++i)
        for (unsigned j = 0; j < w; ++j)
            (*img)(i, j) = (*car)(i+y1, j+x1);

        // On effectué un derniere verification basé sur les edge de la plaque
        ImageGS *plate_grad = new ImageGS(*img);
        //plate_grad->thresholdingHist(0.8);
        plate_grad->inverse();
        plate_grad->thresholdingOstu();

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

        /* Erostion / Disatation extraire les contours */
        ImageGS erode(*plate_grad);
#if 1
        width=2; height=2;
        mask = new float*[height];
        for (int i = 0; i < height; ++i) {
            mask[i] = new float[width];
            for (int j = 0; j < width; ++j)
                mask[i][j] = 255.f;
        }

        erode.erosion(mask, width, height);
#endif

#if 1
        width=2; height=2;
        mask = new float*[height];
        for (int i = 0; i < height; ++i) {
            mask[i] = new float[width];
            for (int j = 0; j < width; ++j)
                mask[i][j] = 255.f;
        }

        plate_grad->dilatation(mask, width, height);
#endif

        // on soustrait l'érodé au dilaté pour garder que les contours
        (*plate_grad) -= erode;

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
        delete plate_grad; // free

        std::cout << "Variations (seuil 1: "<< T_1_PLATE << " - seuil 2: " << T_2_PLATE <<
                     ") - H/3: " << var1 <<
                     " H/2: " << var2 <<
                     " H-H/3: " << var3 << "\n" << std::endl;

        float cpt=0.f;
        if( var1 > T_1_PLATE ) {
            cpt += 1.f;
        } else if( var1 > T_2_PLATE ) {
            cpt += 0.5f;
        }

        if( var2 > T_1_PLATE ) {
            cpt += 1.f;
        }

        if( var2 > T_1_PLATE ) {
            cpt += 1.f;
        } else if( var2 > T_2_PLATE ) {
            cpt += 0.5f;
        }

        if(cpt >= 2.f)
            // ajout a la list
            listImgPlate->push_back(img);
    }
    delete listPlate;

    // Sauvegarde des plaques reconnues
    char filename[100];
    for (unsigned n = 0; n < listImgPlate->size(); ++n) {
        sprintf(filename, "plate_%d", n);
        (*listImgPlate)[n]->writePGM(filename);
        sprintf(filename, "display plate_%d.pgm&", n);
        system(filename);
        delete (*listImgPlate)[n]; // free
    }

    std::cout << "\n>> Programme terminé : " << listImgPlate->size() << " plaques ont/a été reconnue(s).\n";

    // free
    delete listImgPlate;
    delete grad;
    delete car;
    delete car_detect;
    delete[] mask;

    // Fin compteur
    d_stop = clock();
    double duration = d_stop-d_start;
    std::cout << "Temps d'éxecution total : " << duration / (double)CLOCKS_PER_SEC << "s\n";

    std::cout << "\nHappy Ending !\n";

    return 0;
}
