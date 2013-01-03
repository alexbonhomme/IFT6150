#include <QCoreApplication>
#include "imagegs.h"
#include <stdio.h>

int main(int argc, char *argv[])
{

    ImageGS lena("991211-001");


    ImageGS *grad = lena.computeVerticalGradient();
    grad->recal();
    grad->writePGM("vert_grad");

    grad->thresholding(90.f); //TODO trouver une solution plus "automatique"
    grad->writePGM("bin_grad");

    int size = 9;
    float** mask = new float*[size];
    for (int i = 0; i < size; ++i) {
        mask[i] = new float[size];
        for (int j = 0; j < size; ++j)
            mask[i][j] = 255.f;
    }
    grad->closing(mask, size);
    //grad->dilatation(mask, size);
    grad->writePGM("close_grad");
/*
    for (int i = 0; i < size; ++i)
        for (int j = 0; j < size; ++j)
            mask[i][j] = i > size/4 && i < 3*(size/4) ? 255.f : 0.f;
//*/
    grad->opening(mask, size);
    grad->writePGM("open_grad");

    return 0;
}
