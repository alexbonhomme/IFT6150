#include "tools.h"
#include <malloc.h>
#include <math.h>
#include <algorithm>
#include <assert.h>
#include <iostream>
#include <vector>

using namespace std;

/*------------------------------------------------*/
/* DEFINITIONS -----------------------------------*/
/*------------------------------------------------*/
#define SWAP(a,b) tempr=(a);(a)=(b);(b)=tempr
#define SQUARE(X) ((X)*(X))
#define MAX(i,j)  ((i)>(j)?(i):(j))
#define MIN(i,j)  ((i)>(j)?(i):(j))

#define NBCHAR 200

#define FFT   1
#define IFFT -1
#define FFT2D 2

#define GREY_LEVEL 255
#define PI 3.141592654

#define WHITE 255
#define BLACK 0
/*------------------------------------------------*/

#define H(j, sig) (exp(-(j*pow(sig, 2.f))/2.f))

void guassianFilter(float* vect, int height, int w, float sigma) {
    float k = 0.f;

    // calcule du coef k
    for (int i = 1; i <= w; ++i) {
        k += H(i, sigma) + 1.f;
    }
    k *= 2.f;

    // Filtrage
    float* newVect = new float[height];
    for (int i = 0; i < height; ++i) {
        float acc = 0.f;
        for (int j = 1; j <= w; ++j) {
            acc += vect[max(i-j, 0)]*H(j, sigma)
                 + vect[min(i+j, height-1)]*H(j, sigma);
        }
        newVect[i] = (1.f/k)*(vect[i] + acc);
    }

    // copy
    for (int i = 0; i < height; ++i)
        vect[i] = newVect[i];

    delete[] newVect;
}

void writeVect(float* vect, unsigned height, string filename) {
    // Output to a text file
    FILE* f = fopen(filename.c_str(), "w");
    assert(f != NULL);

    // Ecriture des données
    std::cout << ">> Ecriture des données dans " << filename << std::endl;
    for (unsigned i = 0; i < height; ++i)
        fprintf(f, "%d %f\n", i, vect[i]);

    // Fermeture du flux
    fclose(f);
}

std::vector<int*>* foundConnectedComponents(const ImageGS& img, ImageRGB* out) {
    int x1, y1, x2, y2;
    std::vector<int*> *connectList = new std::vector<int*>();

    for (int i = 1; i < img.getHeight()-1; ++i)
    for (int j = 1; j < img.getWidth()-1; ++j) {
        // Si on trouve un pixel blanc
        if(img(i, j) == 255.f) {

            // check d'abord si le pixel n'est pas deja dans un rectangle
            bool knowPt = false;
            for (unsigned n = 0; n < connectList->size(); ++n) {
                x1 = (*connectList)[n][0]; y1 = (*connectList)[n][1];
                x2 = (*connectList)[n][2]; y2 = (*connectList)[n][3];

                if(i >= y1 && i <= y2 && j >= x1 && j <= x2) {
                    knowPt = true;
                    break;
                }
            }

            // Le point est inconnu on peut travailler avec
            if( !knowPt ) {

                // on verifit ca 8 connexité
                bool connex = true;
                for (int i_p = i-1; i_p <= i+1; ++i_p)
                for (int j_p = j-1; j_p <= j+1; ++j_p) {
                    if(img(i_p, j_p) != 255.f)
                        connex = false;
                }

                // Le pixel est 8 connexe
                if(connex) {
                    // On construit un rectangle emglobant
                    x1 = j-1; y1 = i-1;
                    x2 = j+1; y2 = i+1;

                    // On agrandit le rectangle et on check les nouveaux points
                    int prev_x2, prev_y2;
                    do {
                        bool end_x, end_y, found;
                        prev_x2 = x2; prev_y2 = y2;

                        do { // recherche en x
                            end_x = true;

                            ++x2;
                            if(x2 < (int)img.getWidth()) {
                                end_x = false;

                                found = false;
                                for (int i_p = y1; i_p <= min(y2, (int)(img.getHeight()-1)); ++i_p) {
                                    if(img(i_p, x2) == 255.f) { // on a trouvé un nouveau point
                                       found = true;
                                       break;
                                    }
                                }

                                // Si l'on n'a pas trouvé de nouveau point on remet x2 a sa valeur
                                if(!found) {
                                    --x2;
                                    end_x = true; // on arrete d'agrandir en x
                                }
                            } else {
                                --x2;
                            }
                        } while ( !end_x );

                        do { // recherche en y
                            end_y = true;

                            ++y2;
                            if(y2 < (int)img.getHeight()) {
                                end_y = false;

                                found = false;
                                for (int j_p = x1; j_p <= min(x2, (int)(img.getWidth()-1)); ++j_p) {
                                    if(img(y2, j_p) == 255.f) { // on a trouvé un nouveau point
                                       found = true;
                                       break;
                                    }

                                }

                                // Si l'on n'a pas trouvé de nouveau point on remet y2 a sa valeur
                                if(!found) {
                                    --y2;
                                    end_y = true; // on arrete d'agrandir en y
                                }
                            } else {
                                --y2;
                            }
                        } while ( !end_y );

                    } while ( prev_x2 != x2 || prev_y2 != y2 ); // On continu tant qu'il que l'on rouve de nouveaux points


                    // On vérifit qu'il n'est pas superposé a un autre
                    // (au quel cas on prendra le plus grand des deux)
                    bool addRect = true;
                    for (unsigned n = 0; n < connectList->size(); ++n) {
                        int old_x1 = (*connectList)[n][0], old_y1 = (*connectList)[n][1];
                        int old_x2 = (*connectList)[n][2], old_y2 = (*connectList)[n][3];

                        // On check juste le point en bas a droite car
                        // il est impossible qu'ils aient le meme point de départ
                        // et logiquement l'algo converge au meme poit d'arrivé
                        if(x2 == old_x2 && y2 == old_y2) {
                            addRect = false;

                            // On calculer la taille pour ne garder que le plus grand
                            if( (x2 - x1)+(y2 - y1) > (old_x2 - old_x1)+(old_y2 - old_y1) ) {
                                // nouveau est plus grand que l'ancien
                                // on modifit la list
                                (*connectList)[n][0] = x1;
                                (*connectList)[n][1] = y1;
                            }

                            break;
                        }
                    }

                    if(addRect) {
                        // ajout des coordonnées du rectangle a la list
                        int *coord = new int[4];
                        coord[0] = x1; coord[1] = y1;
                        coord[2] = x2; coord[3] = y2;
                        connectList->push_back(coord);
                    }
                }
            } // if( !knowPt )
        }
    }

    // On inscrit les rectangle sur l'image de sortie
    std::vector<int*> *outList = new std::vector<int*>();
    for (unsigned n = 0; n < connectList->size(); ++n) {
        x1 = (*connectList)[n][0]; y1 = (*connectList)[n][1];
        x2 = (*connectList)[n][2]; y2 = (*connectList)[n][3];

        // Ce filtrage est empirique mais permet d'éliminer facilement la majorité des faux positifs
        unsigned width = x2 - x1;
        unsigned height = y2 - y1;
        float ratio = height / (float)width;

        // On vérifit d'abord le ration (la plaque est un rectangle)
        if( ratio > 0.2 && ratio < 0.8 ) {

            printf("%d %d\n", width, height);
            // On vérifit que le rectangle n'est pas trop petit ou trop grand
            if( width > 30 && width < 200 && height > 7 && height < 60) {
                out->drawRect(x1, y1, x2, y2);
                printf("%d %d %d %d\n", x1, y1, x2, y2);
                outList->push_back((*connectList)[n]);
            }
        }
        //delete[] (*connectList)[n];
    }
    delete connectList;

    return outList;
}

/*
 * Fonctions importées du TP5 (FonctionDemo5.c)
 */

/*--------------*/
/* FOURIER -----*/
/*--------------*/
/*------------------------------------------------*/
/*  FOURN ----------------------------------------*/
/*------------------------------------------------*/
void fourn(float data[], unsigned long nn[], int ndim, int isign)
{
    int idim;
    unsigned long i1,i2,i3,i2rev,i3rev,ip1,ip2,ip3,ifp1,ifp2;
    unsigned long ibit,k1,k2,n,nprev,nrem,ntot;
    float tempi,tempr;
    double theta,wi,wpi,wpr,wr,wtemp;

    for (ntot=1,idim=1;idim<=ndim;idim++)
        ntot *= nn[idim];
    nprev=1;
    for (idim=ndim;idim>=1;idim--) {
        n=nn[idim];
        nrem=ntot/(n*nprev);
        ip1=nprev << 1;
        ip2=ip1*n;
        ip3=ip2*nrem;
        i2rev=1;
        for (i2=1;i2<=ip2;i2+=ip1) {
            if (i2 < i2rev) {
                for (i1=i2;i1<=i2+ip1-2;i1+=2) {
                    for (i3=i1;i3<=ip3;i3+=ip2) {
                        i3rev=i2rev+i3-i2;
                        SWAP(data[i3],data[i3rev]);
                        SWAP(data[i3+1],data[i3rev+1]);
                    }
                }
            }
            ibit=ip2 >> 1;
            while (ibit >= ip1 && i2rev > ibit) {
                i2rev -= ibit;
                ibit >>= 1;
            }
            i2rev += ibit;
        }
        ifp1=ip1;
        while (ifp1 < ip2) {
            ifp2=ifp1 << 1;
            theta=isign*6.28318530717959/(ifp2/ip1);
            wtemp=sin(0.5*theta);
            wpr = -2.0*wtemp*wtemp;
            wpi=sin(theta);
            wr=1.0;
            wi=0.0;
            for (i3=1;i3<=ifp1;i3+=ip1) {
                for (i1=i3;i1<=i3+ip1-2;i1+=2) {
                    for (i2=i1;i2<=ip3;i2+=ifp2) {
                        k1=i2;
                        k2=k1+ifp1;
                        tempr=(float)wr*data[k2]-(float)wi*data[k2+1];
                        tempi=(float)wr*data[k2+1]+(float)wi*data[k2];
                        data[k2]=data[k1]-tempr;
                        data[k2+1]=data[k1+1]-tempi;
                        data[k1] += tempr;
                        data[k1+1] += tempi;
                    }
                }
                wr=(wtemp=wr)*wpr-wi*wpi+wr;
                wi=wi*wpr+wtemp*wpi+wi;
            }
            ifp1=ifp2;
        }
        nprev *= n;
    }
}

/*----------------------------------------------------------*/
/* FFTDD                                                    */
/*----------------------------------------------------------*/
void FFTDD(float** mtxR,float** mtxI,int lgth, int wdth)
{
 int i,j;
 int posx,posy;

 float* data;
 float* ImgFreqR;
 float* ImgFreqI;
 unsigned long* nn;

 /*allocation memoire*/
 data=(float*)malloc(sizeof(float)*(2*wdth*lgth)+1);
 ImgFreqR=(float*)malloc(sizeof(float)*(wdth*lgth));
 ImgFreqI=(float*)malloc(sizeof(float)*(wdth*lgth));
 nn=(unsigned long*)malloc(sizeof(unsigned long)*(FFT2D+1));

 /*Remplissage de nn*/
 nn[1]=lgth; nn[2]=wdth;

 /*Remplissage de data*/
 for(i=0;i<lgth;i++) for(j=0;j<wdth;j++)
   { data[2*(i*lgth+j)+1]=mtxR[i][j];
     data[2*(i*lgth+j)+2]=mtxI[i][j]; }

 /*FFTDD*/
 fourn(data,nn,FFT2D,FFT);

 /*Remplissage*/
 for(i=0;i<(wdth*lgth);i++)
  { ImgFreqR[i]=data[(2*i)+1];
    ImgFreqI[i]=data[(2*i)+2];  }

 /*Conversion en Matrice*/
 for(i=0;i<(wdth*lgth);i++)
  { posy=(int)(i/wdth);
    posx=(int)(i%wdth);

    mtxR[posy][posx]=ImgFreqR[i];///(wdth*lgth);
    mtxI[posy][posx]=ImgFreqI[i];}///(wdth*lgth); }

 /*Liberation memoire*/
 free(data);
 free(ImgFreqR);
 free(ImgFreqI);
 free(nn);
}


/*----------------------------------------------------------*/
/* IFFTDD                                                   */
/*----------------------------------------------------------*/
void IFFTDD(float** mtxR,float**  mtxI,int lgth,int wdth)
{
 int i,j;
 int posx,posy;

 float* data;
 float* ImgFreqR;
 float* ImgFreqI;
 unsigned long* nn;

 /*allocation memoire*/
 data=(float*)malloc(sizeof(float)*(2*wdth*lgth)+1);
 ImgFreqR=(float*)malloc(sizeof(float)*(wdth*lgth));
 ImgFreqI=(float*)malloc(sizeof(float)*(wdth*lgth));
 nn=(unsigned long*)malloc(sizeof(unsigned long)*(FFT2D+1));

 /*Recadrege*/

 /*Remplissage de nn*/
 nn[1]=lgth; nn[2]=wdth;

 /*Remplissage de data*/
 for(i=0;i<lgth;i++) for(j=0;j<wdth;j++)
   { data[2*(i*lgth+j)+1]=mtxR[i][j];
     data[2*(i*lgth+j)+2]=mtxI[i][j]; }

 /*FFTDD*/
 fourn(data,nn,FFT2D,IFFT);

 /*Remplissage*/
 for(i=0;i<(wdth*lgth);i++)
  { ImgFreqR[i]=data[(2*i)+1];
    ImgFreqI[i]=data[(2*i)+2]; }

 /*Conversion en Matrice*/
 for(i=0;i<(wdth*lgth);i++)
  { posy=(int)(i/wdth);
    posx=(int)(i%wdth);

   mtxR[posy][posx]=ImgFreqR[i]/(wdth*lgth);
   mtxI[posy][posx]=ImgFreqI[i]/(wdth*lgth); }

 /*Liberation memoire*/
 free(data);
 free(ImgFreqR);
 free(ImgFreqI);
 free(nn);
}
