#include <stdio.h>
#include <stdlib.h>
const double EPS=0.01;
typedef struct
{
    unsigned char R,G,B;

}pixel;
typedef struct
{
    unsigned int DIMENSIUNE_IMAGINE_PIXELI, LATIME, LUNGIME;

}DETALII_IMAGINE;
typedef struct
{
    int x,y,sters,cifra;
    double val;
}corelatii;
typedef struct
{
    int x,y;
}punct;
void citire_imagine (char *NUME, unsigned char **RETINE_HEADER,DETALII_IMAGINE *y, pixel **RETINE_CONTINUT_IMAGINE)
{
    int i,j;
    FILE *fisier=NULL;
    fisier=fopen(NUME,"rb");
    if (fisier==NULL)
    {
        printf("FISIERUL CU NUMELE INTRODUS NU EXISTA\n");
        exit(-1);
    }
    (*RETINE_HEADER)=(unsigned char *)malloc(54*sizeof(unsigned char));
    if ((*RETINE_HEADER)==NULL)
    {
        printf("NU S-A PUTUT ALOCA MEMORIE PENTRU HEADER\n");
        exit(-1);
    }

    fread(*RETINE_HEADER,sizeof(unsigned char),54,fisier);

    fseek(fisier,2,SEEK_SET);
    fread((unsigned int *)&((*y).DIMENSIUNE_IMAGINE_PIXELI),sizeof(unsigned int),1,fisier);
    fseek(fisier,18,SEEK_SET);
    fread((unsigned int *)&((*y).LATIME),sizeof(unsigned int),1,fisier);
    fread((unsigned int *)&((*y).LUNGIME),sizeof(unsigned int),1,fisier);

    int padding;
    if((*y).LATIME % 4 != 0)
        padding = 4 - (3 * (*y).LATIME) % 4;
    else
        padding = 0;

    (*RETINE_CONTINUT_IMAGINE)=(pixel *)malloc((*y).LATIME*(*y).LUNGIME*sizeof(pixel));
    if ((*RETINE_CONTINUT_IMAGINE)==NULL)
    {
        printf("NU S-A PUTUT ALOCA MEMORIE CONTINUTUL IMAGINII\n");
        exit(-1);
    }
    fseek(fisier,54,SEEK_SET);
    int size=(*y).LATIME*(*y).LUNGIME;
    for (i=1;i<=(*y).LUNGIME;i++)
    {
        for (j=0;j<(*y).LATIME;j++)
        {
            fread(&((*RETINE_CONTINUT_IMAGINE)[size-(i*(*y).LATIME)+j].B),sizeof(unsigned char),1,fisier);
            fread(&((*RETINE_CONTINUT_IMAGINE)[size-(i*(*y).LATIME)+j].G),sizeof(unsigned char),1,fisier);
            fread(&((*RETINE_CONTINUT_IMAGINE)[size-(i*(*y).LATIME)+j].R),sizeof(unsigned char),1,fisier);
        }
        fseek(fisier,padding,SEEK_CUR);
    }
    fclose(fisier);
}
void scriere_imagine(char *NUME, unsigned char *RETINE_HEADER, DETALII_IMAGINE y, pixel *RETINE_CONTINUT_IMAGINE)
{
    int i;
    FILE *imagine=NULL;
    imagine=fopen(NUME,"wb");
    if (imagine==NULL)
    {
        printf("FISIERUL NU S-A PUTUT CREEA\n");
        exit(-1);
    }

    fwrite(RETINE_HEADER,sizeof(unsigned char),54,imagine);

    int w=1, padding;
    if(y.LATIME % 4 != 0)
        padding = 4 - (3 * y.LATIME) % 4;
    else
        padding = 0;

    for (i=(y.LUNGIME-1)*y.LATIME;i>=0;i++)
    {
        if (i%y.LATIME==0 && w!=1)
        {
            int j;
            unsigned char t=0;
            i-=y.LATIME*2;
            for (j=0;j<padding;j++)
                fwrite(&t,sizeof(unsigned char),1,imagine);
        }
        w=0;
        if (i<0) break;
        fwrite(&RETINE_CONTINUT_IMAGINE[i].B,sizeof(unsigned char),1,imagine);
        fwrite(&RETINE_CONTINUT_IMAGINE[i].G,sizeof(unsigned char),1,imagine);
        fwrite(&RETINE_CONTINUT_IMAGINE[i].R,sizeof(unsigned char),1,imagine);
    }
    fclose(imagine);
}
void xorshift32(char *NUME, unsigned int **VECTOR, DETALII_IMAGINE y)
{
    FILE *f=NULL;
    f=fopen(NUME,"rt");
    if (f==NULL)
    {
        printf("NU S-A GASIT FISIERUL CARE CONTINE CHEIA SECRETA\n");
        exit(-1);
    }
    (*VECTOR)=(unsigned int *)malloc((2*y.LATIME*y.LUNGIME)*sizeof(unsigned int));
    if ((*VECTOR)==NULL)
    {
        printf("NU S-A PUTUT ALOCA MEMORIE PENTRU PERMUTAREA ALEATOARE\n");
        exit(-1);
    }
    fscanf(f,"%ud",&((*VECTOR)[0]));
    (*VECTOR)[0]=123456789;
    int i;
    for (i=1;i<2*y.LATIME*y.LUNGIME;i++)
    {
        unsigned int x=(*VECTOR)[0];
        x ^= x << 13;
        x ^= x >> 17;
        x ^= x << 5;
        (*VECTOR)[0]=x;
        (*VECTOR)[i]=x;
    }
    fclose(f);
}
void permutare(unsigned int *VECTOR, unsigned int **PERMUTARE, DETALII_IMAGINE y)
{
    (*PERMUTARE)=(unsigned int *)malloc(y.LATIME*y.LUNGIME*sizeof(unsigned int));
    if ((*PERMUTARE)==NULL)
    {
        printf("NU S-A PUTUT ALOCA MEMORIE PENTRU PERMUTARE\n");
        exit(-1);
    }
    int i;
    for (i=0;i<y.LATIME*y.LUNGIME;i++)
        (*PERMUTARE)[i]=i;
    for (i=y.LATIME*y.LUNGIME-1;i>0;i--)
    {
        int random=VECTOR[y.LATIME*y.LUNGIME-i]%(i+1), aux;
        aux=(*PERMUTARE)[i];
        (*PERMUTARE)[i]=(*PERMUTARE)[random];
        (*PERMUTARE)[random]=aux;
    }
}
void permuta_pixeli(pixel *imagine_normala, pixel **imagine_modificata, unsigned int *PERMUTARE,DETALII_IMAGINE y)
{
    (*imagine_modificata)=(pixel*)malloc((y.LATIME*y.LUNGIME)*sizeof(pixel));
    if ((*imagine_modificata)==NULL)
    {
        printf("NU S-A PUTUT ALOCA MEMORIE\n");
        exit(-1);
    }
    int i;
    for (i=0;i<y.LATIME*y.LUNGIME;i++)
        (*imagine_modificata)[PERMUTARE[i]]=imagine_normala[i];
}
void cripteaza_imaginea(char *NUME_IMAGINE, char *NUME_IMAGINE_CRIPTATA, char *NUME_CHEIA_SECRETA)
{
    unsigned int *R, *P, SV;
    unsigned char *HEADER;
    DETALII_IMAGINE y;
    pixel *IMAGINE, *IMAGINE_MODIFICATA, *IMAGINE_CRIPTATA;

    citire_imagine(NUME_IMAGINE, &HEADER, &y, &IMAGINE);
    xorshift32(NUME_CHEIA_SECRETA, &R, y);
    permutare(R, &P, y);
    permuta_pixeli(IMAGINE, &IMAGINE_MODIFICATA, P, y);


    int i, size=y.LATIME*y.LUNGIME;
    IMAGINE_CRIPTATA=(pixel *)malloc(size*sizeof(pixel));
    if (IMAGINE_CRIPTATA==NULL)
    {
        printf("NU S-A PUTUT ALOCA MEMORIE PENTRU CONTINUTUL IMAGINII CRIPTATE\n");
        exit(-1);
    }

    FILE *f=NULL;
    f=fopen(NUME_CHEIA_SECRETA,"rt");
    if (f==NULL)
    {
        printf("FISIERUL NU SE POATE DESCHIDE\n");
        exit(-1);
    }

    fscanf(f,"%d",&SV);
    fscanf(f,"%d",&SV);
    fclose(f);

    char *c=(char *)&SV, *d=(char *)&R[size];
    IMAGINE_CRIPTATA[0].B=(*c)^IMAGINE_MODIFICATA[0].B;
    c++;
    IMAGINE_CRIPTATA[0].G=(*c)^IMAGINE_MODIFICATA[0].G;
    c++;
    IMAGINE_CRIPTATA[0].R=(*c)^IMAGINE_MODIFICATA[0].R;


    IMAGINE_CRIPTATA[0].B=(IMAGINE_CRIPTATA[0].B)^(*d);
    d++;
    IMAGINE_CRIPTATA[0].G=(IMAGINE_CRIPTATA[0].G)^(*d);
    d++;
    IMAGINE_CRIPTATA[0].R=(IMAGINE_CRIPTATA[0].R)^(*d);
    for (i=1;i<size;i++)
    {
        IMAGINE_CRIPTATA[i].R=(IMAGINE_CRIPTATA[i-1].R)^(IMAGINE_MODIFICATA[i].R);
        IMAGINE_CRIPTATA[i].G=(IMAGINE_CRIPTATA[i-1].G)^(IMAGINE_MODIFICATA[i].G);
        IMAGINE_CRIPTATA[i].B=(IMAGINE_CRIPTATA[i-1].B)^(IMAGINE_MODIFICATA[i].B);
        c=(char *)&R[size+i];
        IMAGINE_CRIPTATA[i].B=(IMAGINE_CRIPTATA[i].B)^(*c);
        c++;
        IMAGINE_CRIPTATA[i].G=(IMAGINE_CRIPTATA[i].G)^(*c);
        c++;
        IMAGINE_CRIPTATA[i].R=(IMAGINE_CRIPTATA[i].R)^(*c);
    }
    scriere_imagine(NUME_IMAGINE_CRIPTATA, HEADER, y, IMAGINE_CRIPTATA);
    free(IMAGINE), free(IMAGINE_CRIPTATA), free(IMAGINE_MODIFICATA), free(HEADER), free(R), free(P);
}
void permutarea_inversa(unsigned int **PERMUTARE_INVERSA, unsigned int *PERMUTARE, DETALII_IMAGINE y)
{
    int i, size=y.LUNGIME*y.LATIME;
    (*PERMUTARE_INVERSA)=(unsigned int *)malloc(size*sizeof(unsigned int));
    if ((*PERMUTARE_INVERSA)==NULL)
    {
        printf("NU S-A PUTUT ALOCA MEMORIE PENTRU VECTORUL PERMUTARII INVERSE\n");
        exit(-1);
    }
    for (i=0;i<size;i++)
        (*PERMUTARE_INVERSA)[PERMUTARE[i]]=i;
}
void permuta_imaginea_decriptata(pixel **imagine_decriptata, pixel *imagine_decriptata_partial, unsigned int *permutarea_inversa, DETALII_IMAGINE y)
{
    int i, size=y.LATIME*y.LUNGIME;
    (*imagine_decriptata)=(pixel *)malloc(size*sizeof(pixel));
    if ((*imagine_decriptata)==NULL)
    {
        printf("NU S-A PUTUT ALOCA MEMORIE");
        exit(-1);
    }
    for (i=0;i<size;i++)
        (*imagine_decriptata)[permutarea_inversa[i]]=imagine_decriptata_partial[i];
}
void decripteaza_imaginea(char *NUME_IMAGINE_DECRIPTATA, char *NUME_IMAGINE_CRIPTATA, char *NUME_CHEIA_SECRETA)
{

    unsigned int *R, *P, *P_INVERSA, SV;
    unsigned char *HEADER;
    DETALII_IMAGINE y;
    pixel *IMAGINE_CRIPTATA, *IMAGINE_DECRIPTATA_PARTIAL, *IMAGINE_DECRIPTATA;

    citire_imagine(NUME_IMAGINE_CRIPTATA, &HEADER, &y, &IMAGINE_CRIPTATA);
    xorshift32(NUME_CHEIA_SECRETA, &R, y);
    permutare(R, &P, y);
    permutarea_inversa(&P_INVERSA, P, y);

    int i, size=y.LATIME*y.LUNGIME;
    IMAGINE_DECRIPTATA_PARTIAL=(pixel *)malloc(size*sizeof(pixel));
    if (IMAGINE_DECRIPTATA_PARTIAL==NULL)
    {
        printf("NU S-A PUTUT ALOCA MEMORIE");
        exit(-1);
    }
    FILE *f=NULL;
    f=fopen(NUME_CHEIA_SECRETA,"rt");
    fscanf(f,"%d",&SV);
    fscanf(f,"%d",&SV);
    fclose(f);
    char *c=(char *)&SV, *d=(char *)&R[size];
    IMAGINE_DECRIPTATA_PARTIAL[0].B=(*c)^IMAGINE_CRIPTATA[0].B;
    c++;
    IMAGINE_DECRIPTATA_PARTIAL[0].G=(*c)^IMAGINE_CRIPTATA[0].G;
    c++;
    IMAGINE_DECRIPTATA_PARTIAL[0].R=(*c)^IMAGINE_CRIPTATA[0].R;



    IMAGINE_DECRIPTATA_PARTIAL[0].B=(IMAGINE_DECRIPTATA_PARTIAL[0].B)^(*d);
    d++;
    IMAGINE_DECRIPTATA_PARTIAL[0].G=(IMAGINE_DECRIPTATA_PARTIAL[0].G)^(*d);
    d++;
    IMAGINE_DECRIPTATA_PARTIAL[0].R=(IMAGINE_DECRIPTATA_PARTIAL[0].R)^(*d);
    for (i=1;i<size;i++)
    {
        IMAGINE_DECRIPTATA_PARTIAL[i].R=(IMAGINE_CRIPTATA[i-1].R)^(IMAGINE_CRIPTATA[i].R);
        IMAGINE_DECRIPTATA_PARTIAL[i].G=(IMAGINE_CRIPTATA[i-1].G)^(IMAGINE_CRIPTATA[i].G);
        IMAGINE_DECRIPTATA_PARTIAL[i].B=(IMAGINE_CRIPTATA[i-1].B)^(IMAGINE_CRIPTATA[i].B);
        c=(char *)&R[size+i];
        IMAGINE_DECRIPTATA_PARTIAL[i].B=(IMAGINE_DECRIPTATA_PARTIAL[i].B)^(*c);
        c++;
        IMAGINE_DECRIPTATA_PARTIAL[i].G=(IMAGINE_DECRIPTATA_PARTIAL[i].G)^(*c);
        c++;
        IMAGINE_DECRIPTATA_PARTIAL[i].R=(IMAGINE_DECRIPTATA_PARTIAL[i].R)^(*c);
    }
    permuta_imaginea_decriptata(&IMAGINE_DECRIPTATA, IMAGINE_DECRIPTATA_PARTIAL, P_INVERSA, y);
    scriere_imagine(NUME_IMAGINE_DECRIPTATA, HEADER, y, IMAGINE_DECRIPTATA);

    free(R), free(P), free(P_INVERSA), free(HEADER), free(IMAGINE_CRIPTATA), free(IMAGINE_DECRIPTATA), free(IMAGINE_DECRIPTATA_PARTIAL);
}
void chi_patrat(char *NUME_IMAGINE)
{
    pixel *IMAGINE;
    DETALII_IMAGINE y;
    unsigned char *HEADER;

    citire_imagine(NUME_IMAGINE, &HEADER, &y, &IMAGINE);
    int i, size=y.LATIME*y.LUNGIME;
    unsigned int *frR, *frB, *frG;
    frR=(unsigned int *)calloc(256,sizeof(unsigned int));
    frB=(unsigned int *)calloc(256,sizeof(unsigned int));
    frG=(unsigned int *)calloc(256,sizeof(unsigned int));
    for (i=0;i<size;i++)
    {
        frR[(unsigned int)IMAGINE[i].R]++;
        frG[(unsigned int)IMAGINE[i].G]++;
        frB[(unsigned int)IMAGINE[i].B]++;
    }
    double VR, VG, VB;
    const double f_barat=(double)size/256;
    VR=VG=VB=0;
    for (i=0;i<256;i++)
    {
        VR=VR+(frR[i]-f_barat)*(frR[i]-f_barat)/f_barat;
        VG=VG+(frG[i]-f_barat)*(frG[i]-f_barat)/f_barat;
        VB=VB+(frB[i]-f_barat)*(frB[i]-f_barat)/f_barat;
    }
    printf("R: %0.2lf\nG: %0.2lf\nB: %0.2lf",VR,VG,VB);
}
void citire_matrice(char *NUME, pixel ***RETINE_CONTINUT_SABLON, unsigned char **RETINE_HEADER,DETALII_IMAGINE *y)
{
    int i, j;
    FILE *sablon=NULL;
    sablon=fopen(NUME,"rb");
    if (sablon==NULL)
    {
        printf("FISIERUL NU S-A PUTUT DESCHIDE\n");
        exit(-1);
    }
    (*RETINE_HEADER)=(unsigned char *)malloc(54*sizeof(unsigned char));
    if ((*RETINE_HEADER)==NULL)
    {
        printf("NU S-A PUTUT ALOCA MEMORIE\n");
        exit(-1);
    }
    fread(*RETINE_HEADER,sizeof(unsigned char),54,sablon);

    fseek(sablon,2,SEEK_SET);
    fread((unsigned int *)&((*y).DIMENSIUNE_IMAGINE_PIXELI),sizeof(unsigned int),1,sablon);
    fseek(sablon,18,SEEK_SET);
    fread((unsigned int *)&((*y).LATIME),sizeof(unsigned int),1,sablon);
    fread((unsigned int *)&((*y).LUNGIME),sizeof(unsigned int),1,sablon);

    (*RETINE_CONTINUT_SABLON)=(pixel **)malloc((*y).LUNGIME*sizeof(pixel *));

    if ((*RETINE_CONTINUT_SABLON)==NULL)
    {
        printf("NU S-A PUTUT ALOCA MEMOREI\n");
        exit(-1);
    }
    fseek(sablon,54,SEEK_SET);
    int padding;
    if((*y).LATIME % 4 != 0)
        padding = 4 - (3 * (*y).LATIME) % 4;
    else
        padding = 0;
    for (i=0;i<(*y).LUNGIME;i++)
        (*RETINE_CONTINUT_SABLON)[i]=(pixel *)malloc((*y).LATIME*sizeof(pixel));


    for (i=(*y).LUNGIME-1;i>=0;i--)
    {
        for (j=0;j<(*y).LATIME;j++)
        {
            fread(&((*RETINE_CONTINUT_SABLON)[i][j].B),sizeof(unsigned char),1,sablon);
            fread(&((*RETINE_CONTINUT_SABLON)[i][j].G),sizeof(unsigned char),1,sablon);
            fread(&((*RETINE_CONTINUT_SABLON)[i][j].R),sizeof(unsigned char),1,sablon);
        }
        fseek(sablon,padding,SEEK_CUR);
    }
    fclose(sablon);
}
void afisare_matrice(char *NUME, pixel **RETINE_CONTINUT_SABLON, unsigned char *RETINE_HEADER, DETALII_IMAGINE y)
{
    int i,j;
    FILE *sablon_nou=NULL;
    sablon_nou=fopen(NUME,"wb");
    if (sablon_nou==NULL)
    {
        printf("NU S-A PUTUT UTILIZA FISIERUL\n");
        exit(-1);
    }

    fwrite(RETINE_HEADER,sizeof(unsigned char),54,sablon_nou);
    int padding;
    if(y.LATIME % 4 != 0)
        padding = 4 - (3 * y.LATIME) % 4;
    else
        padding = 0;
    unsigned char zero=0;
    for (i=y.LUNGIME-1;i>=0;i--)
    {
        for (j=0;j<y.LATIME;j++)
        {
            fwrite(&RETINE_CONTINUT_SABLON[i][j].B,sizeof(unsigned char),1,sablon_nou);
            fwrite(&RETINE_CONTINUT_SABLON[i][j].G,sizeof(unsigned char),1,sablon_nou);
            fwrite(&RETINE_CONTINUT_SABLON[i][j].R,sizeof(unsigned char),1,sablon_nou);
        }
        fwrite(&zero,sizeof(unsigned char),padding,sablon_nou);
    }
    fclose(sablon_nou);
}
void grayscale_image(char* nume_fisier_sursa,pixel ***IMAGINE, unsigned char **HEADER_IMAGINE, DETALII_IMAGINE *img)
{
    unsigned int latime_img, inaltime_img;
    unsigned char aux;

   	citire_matrice(nume_fisier_sursa, IMAGINE, HEADER_IMAGINE, img);
   	latime_img=(*img).LATIME;
   	inaltime_img=(*img).LUNGIME;

	int i,j;
	for(i = 0; i < inaltime_img; i++)
	{
		for(j = 0; j < latime_img; j++)
		{
			//fac conversia in pixel gri
			aux = 0.299*(*IMAGINE)[i][j].R + 0.587*(*IMAGINE)[i][j].G + 0.114*(*IMAGINE)[i][j].B;
			(*IMAGINE)[i][j].R = (*IMAGINE)[i][j].G = (*IMAGINE)[i][j].B = aux;
		}
	}
}
double corelatie(int x, int y, pixel **IMAGINE, pixel **SABLON, DETALII_IMAGINE di, DETALII_IMAGINE ds)
{
    int i,j, n=ds.LUNGIME*ds.LATIME;
    double f_barat=0, s_barat=0, sigma_s=0, sigma_f=0, s=0;
    for (i=0;i<ds.LUNGIME;i++)
        for (j=0;j<ds.LATIME;j++)
        {
            unsigned char pixel_imagine=(unsigned char)IMAGINE[x+i][y+j].R;
            unsigned char pixel_sablon=(unsigned char)SABLON[i][j].R;
            f_barat=f_barat+pixel_imagine;
            s_barat=s_barat+pixel_sablon;
        }
    f_barat=(double)f_barat/n;
    s_barat=(double)s_barat/n;
     for (i=0;i<ds.LUNGIME;i++)
        for (j=0;j<ds.LATIME;j++)
        {
            unsigned char pixel_imagine=(unsigned char)IMAGINE[x+i][y+j].R;
            unsigned char pixel_sablon=(unsigned char)SABLON[i][j].R;
            sigma_s=sigma_s+(pixel_imagine-s_barat)*(pixel_imagine-s_barat);
            sigma_f=sigma_f+(pixel_sablon-f_barat)*(pixel_sablon-f_barat);
        }
    sigma_s=(double)sigma_s/(n-1);
    sigma_f=(double)sigma_f/(n-1);
    sigma_s=sqrt(sigma_s);
    sigma_f=sqrt(sigma_f);
    for (i=0;i<ds.LUNGIME;i++)
        for (j=0;j<ds.LATIME;j++)
        {
            unsigned char pixel_imagine=(unsigned char)IMAGINE[x+i][y+j].R;
            unsigned char pixel_sablon=(unsigned char)SABLON[i][j].R;
            double aux=(double)(pixel_imagine-f_barat)*(pixel_sablon-s_barat);
            aux=(double)aux/(sigma_s*sigma_f);
            s=s+aux;
        }
    return (double)s/n;
}
void deseneaza_sablon(int x, int y, pixel ***IMAGINE, DETALII_IMAGINE d, pixel C)
{
    int i,j;
    for (j=0;j<d.LATIME;j++)
    {
        (*IMAGINE)[x][y+j].R=C.R;
        (*IMAGINE)[x][y+j].G=C.G;
        (*IMAGINE)[x][y+j].B=C.B;
        (*IMAGINE)[x+d.LUNGIME-1][y+j].R=C.R;
        (*IMAGINE)[x+d.LUNGIME-1][y+j].G=C.G;
        (*IMAGINE)[x+d.LUNGIME-1][y+j].B=C.B;
    }
    for (i=0;i<d.LUNGIME;i++)
    {
        (*IMAGINE)[x+i][y].R=C.R;
        (*IMAGINE)[x+i][y].G=C.G;
        (*IMAGINE)[x+i][y].B=C.B;
        (*IMAGINE)[x+i][y+d.LATIME-1].R=C.R;
        (*IMAGINE)[x+i][y+d.LATIME-1].G=C.G;
        (*IMAGINE)[x+i][y+d.LATIME-1].B=C.B;
    }
}
void template_matching(char *NUME_SABLON, float PS ,pixel ***IMAGINE, pixel ***SABLON,  unsigned char **HEADER_SABLON ,
                       DETALII_IMAGINE DIM_IMG, DETALII_IMAGINE *DIM_S, int *k, corelatii **d,int indice)
{
    int p, q;
    citire_matrice(NUME_SABLON, SABLON, HEADER_SABLON, DIM_S);
    for (p=0;p<=DIM_IMG.LUNGIME-(*DIM_S).LUNGIME;p++)
        for (q=0;q<=DIM_IMG.LATIME-(*DIM_S).LATIME;q++)
        {
            double cor=corelatie(p, q, *IMAGINE, *SABLON, DIM_IMG, *DIM_S);
            if ((cor+EPS>=PS &&cor<=PS) || (cor-EPS<=PS && cor>=PS) ||cor>=PS)
            {
                if ((*k)==0)
                    (*d)=(corelatii *)malloc(sizeof(corelatii));
                else
                    (*d)=(corelatii *)realloc((*d),((*k)+1)*sizeof(corelatii));
                (*d)[(*k)].val=cor;
                (*d)[(*k)].x=p;
                (*d)[(*k)].y=q;
                (*d)[(*k)].sters=0;
                (*d)[(*k)].cifra=indice;
                (*k)++;
            }
        }
    free(*HEADER_SABLON);
    for (p=0;p<(*DIM_S).LUNGIME;p++)
        free((*SABLON)[p]);
    free(*SABLON);
}
int cmp(const void *x, const void *y)
{
    corelatii xa,ya;
    xa=*(corelatii *)x;
    ya=*(corelatii *)y;
    if (xa.val<ya.val)  return 1;
    if (xa.val>ya.val)  return -1;
    return 0;
}
int max(int a, int b)
{
    if (a>b)    return a;
    return b;
}
int min(int a, int b)
{
    if (a<b)    return a;
    return b;
}
int verifica_intersectia(punct l1, punct r1, punct l2, punct r2)
{
    int dx=min(r1.x,r2.x)-max(l1.x,l2.x);
    int dy=min(r1.y,r2.y)-max(l1.y,l2.y);
    if (dx>0 && dy>0)
    {
        int aria1, aria2, f_dreptunghi, s_dreptunghi;
        aria1=dx*dy;
        f_dreptunghi=(r1.x-l1.x)*(r1.y-l1.y);
        s_dreptunghi=(r2.x-l2.x)*(r2.y-l2.y);
        aria2=f_dreptunghi+s_dreptunghi-aria1;
        double suprapunere;
        if (aria2!=0)
            suprapunere=(double)aria1/aria2;
        if (suprapunere>0.2)    return 1;
    }
    return 0;
}
void elim_suprapuneri(corelatii **d, int k, DETALII_IMAGINE s)
{
    int i,j;
    punct l1,r1,l2,r2;
    for (i=0;i<k-1;i++)
        if((*d)[i].sters==0)
        {
            l1.x=(*d)[i].x;
            l1.y=(*d)[i].y;
            r1.x=(*d)[i].x+s.LUNGIME-1;
            r1.y=(*d)[i].y+s.LATIME-1;
            for (j=i+1;j<k;j++)
                if ((*d)[j].sters==0)
                {
                    l2.x=(*d)[j].x;
                    l2.y=(*d)[j].y;
                    r2.x=(*d)[j].x+s.LUNGIME-1;
                    r2.y=(*d)[j].y+s.LATIME-1;
                    (*d)[j].sters=verifica_intersectia(l1,r1,l2,r2);
                }
        }
}
int main()
{
    FILE *date=NULL;
    date=fopen("date.txt","rt");
    if (date==NULL)
    {
        printf("NU S-A PUTUT CITI FISIERUL DE DATE");
        exit(-1);
    }
//PRIMA PARTE
    char calea_imaginii_in[30], calea_imaginii_out[30], nume_cheia_secreta[30], nume_imagine_criptata[30];
    int i;

    fscanf(date,"%30s",calea_imaginii_in);
    fscanf(date,"%30s",nume_imagine_criptata);
    fscanf(date,"%30s",calea_imaginii_out);
    fscanf(date,"%30s",nume_cheia_secreta);

    cripteaza_imaginea(calea_imaginii_in, nume_imagine_criptata, nume_cheia_secreta);
    decripteaza_imaginea(calea_imaginii_out, nume_imagine_criptata, nume_cheia_secreta);
    printf("Chi-squared pentru peppers.bmp:\n");
    chi_patrat(calea_imaginii_in);
    printf("\nChi-squared pentru imaginea criptata(peppers_criptata.bmp):\n");
    chi_patrat(nume_imagine_criptata);

//A 2 PARTE

    int nr_detectii=0;
    unsigned char *HEADER_SABLON, *HEADER_IMAGINE, *HEADER_IMAGINE_MODIFICATA;
    corelatii *DETECTII;
    pixel **SABLON, **IMAGINE, **IMAGINE_MODIFICATA;
    pixel CULORI[10]={{255,0,0} , {255,255,0} , {0,255,0} , {0,255,255}, {255,0,255} , {0,0,255}, {192,192,192}, {255,140,0}, {128,0,128}, {128,0,0}};

    DETALII_IMAGINE DIM_SABLON, DIM_IMG, DIM_IMG_MODIF;
	char NUMELE_IMAGINII_IN[30], NUMELE_IMAGINII_OUT[30];
    char NUMELE_SABLONULUI[10][12];
    for (i=0;i<=9;i++)
       fscanf(date,"%30s",NUMELE_SABLONULUI[i]);
    fscanf(date,"%30s",NUMELE_IMAGINII_IN);
    fscanf(date,"%30s",NUMELE_IMAGINII_OUT);

    grayscale_image(NUMELE_IMAGINII_IN, &IMAGINE, &HEADER_IMAGINE, &DIM_IMG);
    citire_matrice(NUMELE_IMAGINII_IN, &IMAGINE_MODIFICATA, &HEADER_IMAGINE_MODIFICATA, &DIM_IMG_MODIF);
    for (i=0;i<10;i++)
    {
        printf("\nSe aplica template_matching pentru %s",NUMELE_SABLONULUI[i]);
        template_matching(NUMELE_SABLONULUI[i], 0.50, &IMAGINE, &SABLON, &HEADER_SABLON, DIM_IMG, &DIM_SABLON, &nr_detectii, &DETECTII, i);
    }
    qsort(DETECTII, nr_detectii, sizeof(corelatii),cmp);
    elim_suprapuneri(&DETECTII, nr_detectii, DIM_SABLON);
    for (i=0;i<nr_detectii;i++)
        if(DETECTII[i].sters==0)
            deseneaza_sablon(DETECTII[i].x, DETECTII[i].y, &IMAGINE_MODIFICATA,
                              DIM_SABLON, CULORI[DETECTII[i].cifra]);
    afisare_matrice(NUMELE_IMAGINII_OUT,
                    IMAGINE_MODIFICATA, HEADER_IMAGINE_MODIFICATA, DIM_IMG_MODIF);

    free(HEADER_IMAGINE), free(HEADER_IMAGINE_MODIFICATA), free(DETECTII);
    for (i=0;i<DIM_IMG.LUNGIME;i++)
        free(IMAGINE[i]);
    for (i=0;i<DIM_IMG_MODIF.LUNGIME;i++)
        free(IMAGINE_MODIFICATA[i]);
    fclose(date);
    return 0;
}
