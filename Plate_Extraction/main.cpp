#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <iostream>
#include <vector>
#include <ctime>
#include "imagegs.h"
#include "imagergb.h"
#include "tools.h"

//#define GAUSS_PRE_FILTER

#define PROJ_HORIZONTAL 1.2f
#define PROJ_VERTICAL 1.f

#define RATIO_MIN 0.1f
#define RATIO_MAX 0.7f
#define WIDTH_MIN 35
#define WIDTH_MAX 200
#define HEIGHT_MIN 7
#define HEIGHT_MAX 60

#define T_R1_PLATE 0.32f
#define T_R2_PLATE 0.2f
#define T_1_PLATE 30.f
#define T_2_PLATE 52.f

#define MARGIN_H 3
#define MARGIN_W 1
#define T_BIN 0.9f

int main(int argc, char *argv[]) {
    // Début compteur
    clock_t d_start, d_stop;
    d_start = clock();

    ImageGS *car = NULL;
    if(argc > 2) {
        string arg(argv[1]);

        if(arg.compare("-pgm") == 0)
            car = new ImageGS(argv[2]);
        else if(arg.compare("-ppm") == 0)
            car = new ImageGS(ImageRGB(argv[2]));
        else
            std::cout << "Usage: " << argv[0] << " [-ppm|-pgm] FILE\n";
    } else {
        car = new ImageGS("../data/991211-001");
        //car = new ImageGS("../data/991211-002");
        //car = new ImageGS("../data/991211-003");
        //car = new ImageGS(ImageRGB("../data/0003"));
        //car = new ImageGS("../data/99122-2");
        //car = new ImageGS("../data/991213-002");
        //car = new ImageGS("../data/991213-010");
        //car = new ImageGS("../data/991213-006");
        //car = new ImageGS("../data/991213-005");
    }

    // mesure de la variance (indicatif)
#ifdef _DEBUG_
    std::cout << "Variance de l'image : " << car->computeVariance() << std::endl;
#endif

#ifdef GAUSS_PRE_FILTER
    // filtrage gaussien
    ImageGS in(*car);
    in.gaussianFilter(0.5f);
    in.writePGM("0_gauss_filter");
#else
    ImageGS in = *car;
#endif

    /* Calcule du gradient en x */
    ImageGS *grad = in.computeHorizontalGradient();
    grad->recal();
#ifdef _DEBUG_
    grad->writePGM("0_grad");
#endif

    /* Projection horizontal */
    float* vect;
    vect = grad->computeHorizontalProjection();
#ifdef _DEBUG_
    writeVect(vect, grad->getHeight(), "data_h_brut.dat");
#endif

    // Filtre gaussien sur le vecteur de projection
    gaussianFilter(vect, grad->getHeight(), 6, 0.05f);
#ifdef _DEBUG_
    writeVect(vect, grad->getHeight(), "data_h_filter.dat");
#endif

    /* Seuillage 'vertical' on ne garde que les ligne qui on une projection supérieur a un seuil */
    // averaging
    float avg = 0.f;
    for (unsigned i = 0; i < grad->getHeight(); ++i)
        avg += vect[i];
    avg /= grad->getHeight();

    // seuillage vertical
    for (unsigned i = 0; i < grad->getHeight(); ++i) {
        // si la projection de cette ligne est sous le seuil on la supprime
        if( vect[i] < PROJ_HORIZONTAL*avg ) {
            for (unsigned j = 0; j < grad->getWidth(); ++j) {
                (*grad)(i, j) = 0.f;
            }
        }
    }
    delete[] vect; // free
#ifdef _DEBUG_
    grad->writePGM("1_vertical_filter");
#endif

    // Projection vertical
    vect = grad->computeVerticalProjection();
#ifdef _DEBUG_
    writeVect(vect, grad->getHeight(), "data_v_brut.dat");
#endif

    // Filtrage gaussien
    gaussianFilter(vect, grad->getWidth(), 6, 0.05f);
#ifdef _DEBUG_
    writeVect(vect, grad->getWidth(), "data_v_filter.dat");
#endif

    /* Seuillage 'vertical' on ne garde que les ligne qui on une projection supérieur a T */
    // averaging
    avg = 0.f;
    for (unsigned i = 0; i < grad->getWidth(); ++i)
        avg += vect[i];
    avg /= grad->getWidth();

    // seuillage horizontal
    for (unsigned j = 0; j < grad->getWidth(); ++j) {
        // si la projection de cette ligne est sous le seuil on la supprime
        if( vect[j] < PROJ_VERTICAL*avg ) {
            for (unsigned i = 0; i < grad->getHeight(); ++i) {
                (*grad)(i, j) = 0.f;
            }
        }
    }
    delete[] vect; // free
#ifdef _DEBUG_
    grad->writePGM("1_horizontal_filter");
#endif

    /* binarisation par seuillage global */
    grad->thresholdingSmart(T_BIN);
#ifdef _DEBUG_
    grad->writePGM("2_bin_filter");
#endif

    /* Opération morphologique pour éliminer le bruit, etc..*/
    int width, height;
    float** mask;

#if 0
    width=2; height=2;
    mask = new float*[height];
    for (int i = 0; i < height; ++i) {
        mask[i] = new float[width];
        for (int j = 0; j < width; ++j)
            mask[i][j] = 255.f;
    }
    grad->erosion(mask, width, height);
    grad->writePGM("3_2_close_grad");
#endif

#if 1
    width=20; height=3;
    mask = new float*[height];
    for (int i = 0; i < height; ++i) {
        mask[i] = new float[width];
        for (int j = 0; j < width; ++j)
            mask[i][j] = 255.f;
    }
    grad->closing(mask, width, height);
#ifdef _DEBUG_
    grad->writePGM("3_2_close_grad");
#endif
#endif

#if 1
    width=1; height=5;
    mask = new float*[height];
    for (int i = 0; i < height; ++i) {
        mask[i] = new float[width];
        for (int j = 0; j < width; ++j)
            mask[i][j] = 255.f;
    }
    grad->opening(mask, width, height);
#ifdef _DEBUG_
    grad->writePGM("4_open_grad");
#endif
#endif

    // On applique les rectangle sur l'image de base pour visualiser
    // les zone detectées comme potentiellement des plaques d'immatriculation
    ImageRGB *car_detect = new ImageRGB(*car);
    std::vector<int*> *listPlate = foundConnectedComponents(*grad, car_detect,
                                                            RATIO_MIN, RATIO_MAX,
                                                            WIDTH_MIN, WIDTH_MAX,
                                                        HEIGHT_MIN, HEIGHT_MAX);
#ifdef _DEBUG_
    car_detect->writePPM("5_plate_detect");
#endif
    delete grad; // free

    // On construit la liste des (potentiels) plaque extraites de l'image
    std::cout << "\n>> Analyse des zone retenues\n";
    std::vector<ImageGS*> *listImgPlate = new std::vector<ImageGS*>();
    char filename[100]; // Chaine de caractère pour la sauvergarde des images
    for (unsigned n = 0; n < listPlate->size(); ++n) {
        //  on fixe si possible une marge de quelques pixel (pour eviter le crop)
        int x1 = max((*listPlate)[n][0]-MARGIN_W, 0),
            y1 = max((*listPlate)[n][1]-MARGIN_H, 0),
            x2 = min((*listPlate)[n][2]+MARGIN_W, (int)car->getWidth()),
            y2 = min((*listPlate)[n][3]+MARGIN_H, (int)car->getHeight());
        delete[] (*listPlate)[n]; // free

        unsigned w = x2-x1, h = y2-y1;

        // On crée la miniature
        ImageGS *img = new ImageGS(w, h);
        for (unsigned i = 0; i < h; ++i)
        for (unsigned j = 0; j < w; ++j)
            (*img)(i, j) = (*car)(i+y1, j+x1);

        // On effectué un derniere verification basé sur les edge de la plaque
        ImageGS *plate = new ImageGS(*img);
        plate->inverse();
#ifdef _DEBUG_
        std::cout << "Variacance: " << plate->computeVariance() << std::endl;
#endif

        // Seuillage automatique
        plate->thresholdingOstu();

#if 0
        width=1; height=3;
        mask = new float*[height];
        for (int i = 0; i < height; ++i) {
            mask[i] = new float[width];
            for (int j = 0; j < width; ++j)
                mask[i][j] = 255.f;
        }
        plate->opening(mask, width, height);
#endif

        /* Erostion / Disatation extraire les contours */
        ImageGS erode(*plate);
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
        width=1; height=3;
        mask = new float*[height];
        for (int i = 0; i < height; ++i) {
            mask[i] = new float[width];
            for (int j = 0; j < width; ++j)
                mask[i][j] = 255.f;
        }
        plate->dilatation(mask, width, height);
#endif

        // on soustrait l'érodé au dilaté pour garder que les contours
        (*plate) -= erode;

#ifdef _DEBUG_
        // Save
        sprintf(filename, "t_plate_%d", n);
        plate->writePGM(filename);
#endif

        float var=0, var_cpt=0;
        for (unsigned j = 1; j < w; ++j) {
            // on compte le nombre de variation
            var_cpt = 0;
            if((*plate)(h/3, j) != (*plate)(h/3, j-1))
                ++var_cpt;
            if((*plate)(h/2, j) != (*plate)(h/2, j-1))
                ++var_cpt;
            if((*plate)(h-h/3, j) != (*plate)(h-h/3, j-1))
                ++var_cpt;

            // On pondère le résultat suivant le nombre variations simultanées
            if(var_cpt == 3)
                var += 2.f;
            else if(var_cpt == 2)
                var += 1.f;
            else if(var_cpt == 1)
                var += .5f;
        }

        float varRatio = var/(float)(plate->getWidth()+plate->getHeight());
#ifdef _DEBUG_
        std::cout << "Variations (seuil 1: "<< T_1_PLATE << " - seuil 2: " << T_2_PLATE <<
                     ") - Variations: " << var <<
                     " Ratio de variation: " << varRatio <<
                     "\n" << std::endl;
#endif

        /*
         * On Compare d'abord les seuils classiques (variation minimal et maximal)
         * Puis on regarde si le ratio 'var/taille de la zone' est respecté.
         *
         * Si le premier teste n'est pas valide on regarde seulement si le ratio
         * est suppérieur a un autre seuil. C'est une "sorte" de seuillage par hystéresis.
         */
        if(var > T_1_PLATE && var < T_2_PLATE && varRatio > T_R2_PLATE)
            listImgPlate->push_back(img);
        else if(varRatio > T_R1_PLATE)
            listImgPlate->push_back(img);

        delete plate; // free
    }
    delete listPlate;

    // Sauvegarde des plaques reconnues
    for (unsigned n = 0; n < listImgPlate->size(); ++n) {
        sprintf(filename, "plate_%d", n);
        (*listImgPlate)[n]->writePGM(filename);
        sprintf(filename, "display plate_%d.pgm&", n);
        system(filename);
        delete (*listImgPlate)[n]; // free
    }

    std::cout << "\n>> Programme terminé : " << listImgPlate->size() << " plaques ont/a été reconnue(s).\n";

    // free
    if(car != NULL) {
        delete listImgPlate;
        delete car;
        delete car_detect;
        delete[] mask;
    }

    // Fin compteur
    d_stop = clock();
    double duration = d_stop-d_start;
    std::cout << "Temps d'éxecution total : " << duration / (double)CLOCKS_PER_SEC << "s\n";

    return 0;
}
