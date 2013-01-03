#include "tools.h"
#include <malloc.h>
#include <math.h>

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
