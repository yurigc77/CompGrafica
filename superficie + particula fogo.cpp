#include <GL/gl.h>
#include "GL/glut.h"


#define PASSO_TEMP 0.1  // passo de tempo para integrar ODE por Euler 


#define MAXPARTICULAS  10000
#define randomico() ((float) rand()/ RAND_MAX) //Numero aleatorio entre 0 e 1

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include <GL/glut.h>

#define Linha -1
#define Solido -2 
#define Pontos -3

#define X 0
#define Y 1
#define Z 2
#define W 3

#define Escalar 4 // Redimensionar

#define RotarX 5
#define RotarY 6
#define RotarZ 7
#define RotarXYZ 71

#define TransladaX 8
#define TransladaY 9
#define TransladaZ 10
#define Translada 101

#define PtsControle 19
#define RandomPtsControle 191
#define fogo 192
#define BEZIER      20
#define BSPLINE     21
#define CATMULLROM  22

#define sair 0

#define local_scale 0.22   // Valor de escala para apresentar no CANVAS

bool ligaParticulas=false;

float ppx[121],ppy[121],ppz[121];

int windW, windH; 
int gIndVert = -1; 
bool preenchido=0; //flag para saber se a superficie está preenchida

typedef float f4d[4];   // O tipo de dado f4d é um vetor de 4 elementos

typedef struct st_matriz
{
    int n, m;
    f4d **ponto;
} matriz;

typedef struct{
        GLfloat pos[3];
        GLfloat vel[3];
        GLfloat ace[3];
        GLfloat massa;
        GLfloat cor[3];
        GLfloat tempoVida;
        GLfloat transparencia;
}part2;
 
int comando; 

int tipoView = GL_LINE_STRIP;



float VARIA = 0.1f;         // variacao do parametros Ex. t = j * VARIA para j = 0, ....< n


f4d matTransf[4];   // matriz transposta 4 x 4

f4d MatBase[4];   // matriz de base 4x4

f4d pView = {10.0, 10.0, -20.0, 0.0};

bool R=1,G=1,B=1;
int FR=0,FG=1,FB=2;

float pfx,pfy,pfz;


part2 Particulas[MAXPARTICULAS];










f4d vcolor[4] = {{R, G, B, 0.0},//RGB
				 {0.0, 1.0, 0.0, 0.0},
				 {0.0, 0.0, 1.0, 0.0},
				 {1.0, 1.0, 0.0, 0.0}};
				 
void matrizCor(bool R,bool G,bool B)
{
	vcolor[0][0]=R;
	vcolor[0][1]=G;
	vcolor[0][2]=B;
}



matriz *pControle = NULL;  // matriz de pontos de controle  LIDOS

matriz *pcPatch = NULL;    // matriz de pontos para um patch 

matriz *ptsPatch = NULL;   // matriiz de PONTOS na superficie PATCH calclulados

void DisenaSuperficie(void);

void MontaMatrizBase(int tipoSup)
{
	if(tipoSup==BEZIER)
	{
		MatBase[0][0] = -1.0f; MatBase[0][1] = 3.0f;  MatBase[0][2] = -3.0f; MatBase[0][3] = 1.0f;
		MatBase[1][0] =  3.0f; MatBase[1][1] = -6.0f; MatBase[1][2] =  3.0f; MatBase[1][3] = 0.0f;
		MatBase[2][0] = -3.0f; MatBase[2][1] = 3.0f;  MatBase[2][2] =  0.0f; MatBase[2][3] = 0.0f;
		MatBase[3][0] =  1.0f; MatBase[3][1] = 0.0f;  MatBase[3][2] =  0.0f; MatBase[3][3] = 0.0f;
	}

	if(tipoSup==BSPLINE)
	{
		MatBase[0][0] = -1.0f/6.0;  MatBase[0][1] = 3.0f/6.0;  MatBase[0][2] = -3.0f/6.0;  MatBase[0][3] = 1.0f/6.0;
		MatBase[1][0] =  3.0f/6.0; MatBase[1][1] = -6.0f/6.0;   MatBase[1][2] =  3.0f/6.0;   MatBase[1][3] = 0.0f;
		MatBase[2][0] = -3.0f/6.0;  MatBase[2][1] = 0.0f/6.0;  MatBase[2][2] =  3.0f/6.0;  MatBase[2][3] = 0.0f;
		MatBase[3][0] =  1.0f/6.0; MatBase[3][1] = 4.0f/6.0;  MatBase[3][2] =  1.0f/6.0; MatBase[3][3] = 0.0;
	}

	if(tipoSup==CATMULLROM)
	{
		MatBase[0][0] = -1.0f/2.0; MatBase[0][1] = 3.0f/2.0;  MatBase[0][2] = -3.0f/2.0; MatBase[0][3] = 1.0f/2.0;
		MatBase[1][0] =  2.0f/2.0; MatBase[1][1] = -5.0f/2.0; MatBase[1][2] =  4.0f/2.0; MatBase[1][3] = -1.0f/2.0;
		MatBase[2][0] = -1.0f/2.0; MatBase[2][1] = 0.0f/2.0;  MatBase[2][2] =  1.0f/2.0; MatBase[2][3] = 0.0f/2.0;
		MatBase[3][0] =  0.0f/2.0; MatBase[3][1] = 2.0f/2.0;  MatBase[3][2] =  0.0f/2.0; MatBase[3][3] = 0.0f/2.0;
	}
}

matriz* liberaMatriz(matriz* sup)
{
	int i;

	if(sup)
	{
	    for(i=0; i< sup->n; i++)
	        free(sup->ponto[i]);

	    free(sup->ponto);
		free(sup);
	}
	return NULL;
}


matriz* AlocaMatriz(int n, int m)
{
	matriz *matTemp;
	int j;

    if((matTemp = (matriz*) malloc(sizeof(matriz)))==NULL)
    {
		printf("\n Error en alocacion de memoria para uma matriz");
		return 0;
    }

    matTemp->n = n;
    matTemp->m = m;
    matTemp->ponto = (f4d**) calloc(n, sizeof(f4d*));

    for(j=0; j<matTemp->n; j++)
         matTemp->ponto[j] = (f4d*) calloc(m, sizeof(f4d));

    return matTemp;
}


void MatrizIdentidade()
{
	int a,b;
    for(a=0; a<4; a++)
    {   for(b=0; b<4; b++)
		{   if(a==b)
		       matTransf[a][b] = 1;
		    else
		       matTransf[a][b] = 0;
		}
    }
}

void MultMatriz()
{ 
	int j, k;
    f4d aux;
    for(j=0; j< pControle->n; j++)
	{
		for(k = 0; k< pControle->m; k++)
		{
         aux[X] = pControle->ponto[j][k][X]; 
		 aux[Y] = pControle->ponto[j][k][Y]; 
		 aux[Z] = pControle->ponto[j][k][Z];
		 aux[W] = 1.0; //pControle->ponto[j][k][W];

		 //  Pj = MatTransf4x4 . Pj  <--- transformada homogenea 

         pControle->ponto[j][k][X] = matTransf[X][X] * aux[X] + 
							         matTransf[Y][X] * aux[Y] + 
							         matTransf[Z][X] * aux[Z] +
									 matTransf[W][X] * aux[W];

		 pControle->ponto[j][k][Y] = matTransf[X][Y] * aux[X] + 
							         matTransf[Y][Y] * aux[Y] + 
							         matTransf[Z][Y] * aux[Z] +
									 matTransf[W][Y] * aux[W];

		 pControle->ponto[j][k][Z] = matTransf[X][Z] * aux[X] + 
							         matTransf[Y][Z] * aux[Y] + 
							         matTransf[Z][Z] * aux[Z] +
									 matTransf[W][Z] * aux[W];

		 pControle->ponto[j][k][W] = matTransf[X][W] * aux[X] + 
							         matTransf[Y][W] * aux[Y] + 
							         matTransf[Z][W] * aux[Z] +
									 matTransf[W][W] * aux[W];
		}
	}
}

void prod_VetParam_MatBase(float t, float *tt, float *vr)
{
	int i, j;

	// Vr = [t^3  t^2  t  1] [MatBase]4x4  <-- base avaliado em t.
	// ---------------------------------------------------------
	tt[0] = pow(t,3);
	tt[1] = pow(t,2);
	tt[2] = t;
	tt[3] = 1.0;

	for(i=0; i<4; i++)
	{
		vr[i] = 0.0f;
		for(j=0; j<4; j++)
			vr[i] += MatBase[j][i] * tt[j];
	}
}

void prod_VetMatriz(float *v, f4d **pc, f4d *vr)
{
	int i, j;

	// Vr = V . P   <----- combinacao linear
	// --------------------------------------
	for(i=0; i<4; i++)
	{
		vr[i][0] = vr[i][1] = vr[i][2] = 0.0;
		for(j=0; j<4; j++)
		{
			vr[i][0] += v[j] * pc[j][i][0];
			vr[i][1] += v[j] * pc[j][i][1];
			vr[i][2] += v[j] * pc[j][i][2];
		}	
	}
}

void ptsSuperficie(matriz *pcPatch)
{
    int i, j, h, n, m,k=0;
	float t,s;
	float tmp[4], vsm[4], vtm[4];
	f4d va[4];

	if(!pcPatch) return;
	
	// calcular o tamanho (n,m) da matriz de pontos de PATCH
	// em função do VARIA

    n = 0;

    for(s = 0; s<=1.01; s+=VARIA) n += 1;

    m = n;
    
    // define o espaço de um patch

    if (ptsPatch) ptsPatch = liberaMatriz(ptsPatch);

    ptsPatch=AlocaMatriz(n,m);

	// Gera uma matriz de ptsPatch de n x m pontos de cada PATCH
	// para  ( 0 <= s <= 1 , 0 <= s <= 1 )
	
	s=0.0f;
	for(i = 0; i < ptsPatch->n; i++)
    {
        t = 0.0f;
        for(j = 0; j < ptsPatch->m; j++)
        {
				// calcula cada ponto: p(s, t) = S G P G^t T

			prod_VetParam_MatBase(s, tmp, vsm);    // vsm = S G
			prod_VetParam_MatBase(t, tmp, vtm);    // vtm = G^t T

			prod_VetMatriz(vsm, pcPatch->ponto, va);    // va = S G P = vsm P

			ptsPatch->ponto[i][j][0] = 0.0f;
			ptsPatch->ponto[i][j][1] = 0.0f;
			ptsPatch->ponto[i][j][2] = 0.0f;

			for(h=0; h<4; h++)						// p = S G P G^t T = va vtm
			{
				ptsPatch->ponto[i][j][0] += va[h][0] * vtm[h];
				ptsPatch->ponto[i][j][1] += va[h][1] * vtm[h];
				ptsPatch->ponto[i][j][2] += va[h][2] * vtm[h];
			}
	        
		
			ppx[k+j]= ptsPatch->ponto[i][j][0];
			ppy[k+j]= ptsPatch->ponto[i][j][1];
			ppz[k+j]= ptsPatch->ponto[i][j][2];	
	
			t+=VARIA;
        }

        k=k+11;
        s+=VARIA;
    }
    
}

void MostrarUmPatch(int cc)
{
	int i, j;
	float t,v,s;
    f4d a,b,n,l;

    if(!ptsPatch)  return;

    switch(tipoView)
    {
        case GL_POINTS:
          glColor3f(0.0f, 0.0f, 0.7f);
          glPointSize(1.0);
          for(i = 0; i < ptsPatch->n; i++)
          {
              glBegin(tipoView);
              for(j = 0; j < ptsPatch->m; j++)
                 glVertex3fv(ptsPatch->ponto[i][j]);
              glEnd();
          }
          break;

        case GL_LINE_STRIP:
          glColor3f(0.0f, 0.0f, 0.7f);
          for(i = 0; i < ptsPatch->n; i++)
          {
              glBegin(tipoView);
              for(j = 0; j < ptsPatch->m; j++)
                 glVertex3fv(ptsPatch->ponto[i][j]);
              glEnd();
          }

          for(j = 0; j < ptsPatch->n; j++)
          {
              glBegin(tipoView);
              for(i = 0; i < ptsPatch->m; i++)
                 glVertex3fv(ptsPatch->ponto[i][j]);
              glEnd();
          }
          break;

        case GL_QUADS:///////////////////////////////////////////triangulo na diagola secundaria
 	      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
          for(i = 0; i < ptsPatch->n-1; i++)
          {
              for(j = 0; j < ptsPatch->m-1; j++)
              {
				a[X] = ptsPatch->ponto[i+1][j][X] - ptsPatch->ponto[i][j][X];
				a[Y] = ptsPatch->ponto[i+1][j][Y] - ptsPatch->ponto[i][j][Y];
				a[Z] = ptsPatch->ponto[i+1][j][Z] - ptsPatch->ponto[i][j][Z];

				b[X] = ptsPatch->ponto[i][j+1][X] - ptsPatch->ponto[i][j][X];
				b[Y] = ptsPatch->ponto[i][j+1][Y] - ptsPatch->ponto[i][j][Y];
				b[Z] = ptsPatch->ponto[i][j+1][Z] - ptsPatch->ponto[i][j][Z];

				n[X] = a[Y]*b[Z] - a[Z]*b[Y];
				n[Y] = a[Z]*b[X] - a[X]*b[Z];
				n[Z] = a[X]*b[Y] - a[Y]*b[X];

				s = sqrt(n[X]*n[X]+n[Y]*n[Y]+n[Z]*n[Z]);

				n[X] /=s; n[Y] /=s; n[Z] /=s;

				l[X] = pView[X] - ptsPatch->ponto[i][j][X];
				l[Y] = pView[Y] - ptsPatch->ponto[i][j][Y];
				l[Z] = pView[Z] - ptsPatch->ponto[i][j][Z];

				s = n[X]*l[X]+n[Y]*l[Y]+n[Z]*l[Z];

				v = sqrt(l[X]*l[X]+l[Y]*l[Y]+l[Z]*l[Z]);
				t = s / v;
				
				if(t<0.0f)
					t *= -1.00f;

				glBegin(GL_POLYGON);
					glColor3f(t*vcolor[cc][X],t*vcolor[cc][Y],t*vcolor[cc][Z]);
					glNormal3fv(n);
                    glVertex3fv(ptsPatch->ponto[i][j]);
                    glVertex3fv(ptsPatch->ponto[i][j+1]);
                    glVertex3fv(ptsPatch->ponto[i+1][j]);
				glEnd();
			
				// criando 2° triangulo


				a[X] = ptsPatch->ponto[i][j+1][X] - ptsPatch->ponto[i+1][j+1][X];
				a[Y] = ptsPatch->ponto[i][j+1][Y] - ptsPatch->ponto[i+1][j+1][Y];
				a[Z] = ptsPatch->ponto[i][j+1][Z] - ptsPatch->ponto[i+1][j+1][Z];

				b[X] = ptsPatch->ponto[i+1][j][X] - ptsPatch->ponto[i+1][j+1][X];
				b[Y] = ptsPatch->ponto[i+1][j][Y] - ptsPatch->ponto[i+1][j+1][Y];
				b[Z] = ptsPatch->ponto[i+1][j][Z] - ptsPatch->ponto[i+1][j+1][Z];

				n[X] = a[Y]*b[Z] - a[Z]*b[Y];
				n[Y] = a[Z]*b[X] - a[X]*b[Z];
				n[Z] = a[X]*b[Y] - a[Y]*b[X];

				s = sqrt(n[X]*n[X]+n[Y]*n[Y]+n[Z]*n[Z]);

				n[X] /=s; n[Y] /=s; n[Z] /=s;

				l[X] = pView[X] - ptsPatch->ponto[i+1][j+1][X];
				l[Y] = pView[Y] - ptsPatch->ponto[i+1][j+1][Y];
				l[Z] = pView[Z] - ptsPatch->ponto[i+1][j+1][Z];

				s = n[X]*l[X]+n[Y]*l[Y]+n[Z]*l[Z];

				v = sqrt(l[X]*l[X]+l[Y]*l[Y]+l[Z]*l[Z]);
				t = s / v;
				
				if(t<0.0f)
					t *= -1.00f;

				glBegin(GL_POLYGON);
					glColor3f(t*vcolor[cc][X],t*vcolor[cc][Y],t*vcolor[cc][Z]);
					glNormal3fv(n);
                    glVertex3fv(ptsPatch->ponto[i][j+1]);
                    glVertex3fv(ptsPatch->ponto[i+1][j+1]);
                    glVertex3fv(ptsPatch->ponto[i+1][j]);
				glEnd();
							
				

              }
          }
          break;
    }

}

void MostrarPtosPoligControle(matriz *sup)
{
	int i, j;

    glColor3f(0.0f, 0.8f, 0.0f);
	glPolygonMode(GL_FRONT_AND_BACK, tipoView);
	glPointSize(7.0);
	for(i=0; i<sup->n; i++)
	{
		glBegin(GL_POINTS);
		for(j=0; j<sup->m; j++)
			glVertex3fv(sup->ponto[i][j]);
		glEnd();

		glBegin(GL_LINE_STRIP);
		for(j=0; j<sup->m; j++)
			glVertex3fv(sup->ponto[i][j]);
		glEnd();
	}

	for(i=0; i<sup->m; i++)
	{
		glBegin(GL_LINE_STRIP);
		for(j=0; j<sup->n; j++)
			glVertex3fv(sup->ponto[j][i]);
		glEnd();
	}
}

void copiarPtosControlePatch(int i0, int j0, matriz *pcPat)
{
	int i, j, jj, ii;

	// copiar n x m pontos desde (i0, j0) apartir da matriz pControle
	for(i=0; i<pcPat->n; i++)
	{
		ii = i0 + i;
		for(j=0; j<pcPat->m; j++)
		{
		    jj = j0 + j;
	        pcPat->ponto[i][j][0] = pControle->ponto[ii][jj][0];
			pcPat->ponto[i][j][1] = pControle->ponto[ii][jj][1];
	        pcPat->ponto[i][j][2] = pControle->ponto[ii][jj][2];
	        pcPat->ponto[i][j][3] = pControle->ponto[ii][jj][3];
		}
	}
}

void DisenaSuperficie(void)
{
	int i, j, nn, mm, idCor;

	nn = pControle->n - 3;   // numero de descolamentos (patchs)

	for (i=0; i<nn; i++)
	{
		mm = pControle->m - 3;
		for(j=0; j<mm; j++)
		{
		    copiarPtosControlePatch(i, j, pcPatch);  // copiar ptos de controle em matriz 4 x 4
		    ptsSuperficie(pcPatch);        // calculos pontos do PATCH com os ptos de Contrle em pcPatch 
			idCor = (i+j)%4;               // indice de paleta de COR 
		    MostrarUmPatch(idCor);
		}
	}
}







void reshape(int w, int h)
{
	windW=w/2; windH=h/2;
	 
   glViewport(0, 0, (GLsizei) w, (GLsizei) h);
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   if (w <= h)
      glOrtho(-10.0, 10.0, -10.0*(GLfloat)h/(GLfloat)w,
              10.0*(GLfloat)h/(GLfloat)w, -10.0, 30.0);
   else
      glOrtho(-10.0*(GLfloat)w/(GLfloat)h,
              10.0*(GLfloat)w/(GLfloat)h, -10.0, 10.0, -10.0, 30.0);
   glMatrixMode(GL_MODELVIEW);
}

void keyboard(int key, int x, int y)
{
    MatrizIdentidade();  // identidade em matTransf : matriz de transforma
    switch(comando)
    {   case Escalar:
            if (key == GLUT_KEY_LEFT || key == GLUT_KEY_DOWN) {
                matTransf[0][0] = 0.95;
	            matTransf[1][1] = 0.95;
                matTransf[2][2] = 0.95;
            } else
                if ( key == GLUT_KEY_RIGHT || key == GLUT_KEY_UP) {
	                matTransf[0][0] = 1.05;
	                matTransf[1][1] = 1.05;
                    matTransf[2][2] = 1.05;                		
				}
            break;

        case RotarXYZ:
        	if(glutGetModifiers() == GLUT_ACTIVE_SHIFT)
        	{
        		if(key == GLUT_KEY_RIGHT)
            	{  
                    matTransf[0][0] = cos(-0.01);
                    matTransf[0][1] = sin(-0.01);
                    matTransf[1][0] = -sin(-0.01);
                    matTransf[1][1] = cos(-0.01);
				} else
					if (key == GLUT_KEY_LEFT)
					{
                    	matTransf[0][0] = cos(0.01);
                    	matTransf[0][1] = sin(0.01);
                    	matTransf[1][0] = -sin(0.01);
                    	matTransf[1][1] = cos(0.01);
					}
			}
			else
			{
				if(key == GLUT_KEY_DOWN )
            	{  
                    matTransf[1][1] = cos(0.01);
                    matTransf[1][2] = sin(0.01);
                    matTransf[2][1] = -sin(0.01);
                    matTransf[2][2] = cos(0.01);
				} 
				if (key == GLUT_KEY_UP )
				{
                    matTransf[1][1] = cos(-0.01);
                    matTransf[1][2] = sin(-0.01);
                    matTransf[2][1] = -sin(-0.01);
                    matTransf[2][2] = cos(-0.01);
				}
				if(key == GLUT_KEY_LEFT )
            	{  
                    matTransf[0][0] = cos(-0.01);
                    matTransf[0][2] = -sin(-0.01);
                    matTransf[2][0] = sin(-0.01);
                    matTransf[2][2] = cos(-0.01);
				} 
				if (key == GLUT_KEY_RIGHT )
				{
                    matTransf[0][0] = cos(0.01);
                    matTransf[0][2] = -sin(0.01);
                    matTransf[2][0] = sin(0.01);
                    matTransf[2][2] = cos(0.01);
				}	
			}
			break;    

	case Translada:
			if(key == GLUT_KEY_LEFT )
                    matTransf[3][0] = -0.10;
			else if (key == GLUT_KEY_RIGHT )
                    matTransf[3][0] = 0.10;
			else if(key == GLUT_KEY_DOWN )
                    matTransf[3][1] = -0.10;
			else if (key == GLUT_KEY_UP )
                    matTransf[3][1] = 0.10;
    		break;
   }
    MultMatriz();
    glutPostRedisplay();
}

int CarregaPontos( char *arch)
{
  FILE *fobj;
  char token[40];
  float px, py, pz;
  int i, j, n, m;

  printf(" \n ler  %s  \n", arch);

  if((fobj=fopen(arch,"rt"))==NULL)
  {
     printf("Error en la apertura del archivo %s \n", arch);
     return 0;
  }

  fgets(token, 40, fobj);
  fscanf(fobj, "%s %d %d", token, &n, &m);

  if (pControle) pControle = liberaMatriz(pControle);

  pControle =AlocaMatriz(n,m);
  
  fscanf(fobj, "%s", token);  // leitura da linha 0

  for(j=0; j<pControle->n; j++)
  {
    for(i=0; i<pControle->m; i++)
	 {
	     fscanf(fobj, "%s %f %f %f", token, &px, &py, &pz);

         pControle->ponto[j][i][0] = px * local_scale;
         pControle->ponto[j][i][1] = py * local_scale;
         pControle->ponto[j][i][2] = pz * local_scale;
         pControle->ponto[j][i][3] = 1.0f;
	 }
	fscanf(fobj, "%s", token);  // leitura da linha j+1;
  }

  // espaco de matriz para um patch
  if(pcPatch) pcPatch = liberaMatriz(pcPatch);
  pcPatch = AlocaMatriz(4, 4);

}

int GeraPontos( char *arch)
{
  //int random_number = rand() % 50 + 0; 
  FILE *fobj;
  char token[40];
  float px, py, pz;
  int i, j, n, m;

  printf(" \n ler  %s  \n", arch);

  if((fobj=fopen(arch,"rt"))==NULL)
  {
     printf("Error en la apertura del archivo %s \n", arch);
     return 0;
  }

  fgets(token, 40, fobj);
  fscanf(fobj, "%s %d %d", token, &n, &m);

  if (pControle) pControle = liberaMatriz(pControle);

  pControle =AlocaMatriz(n,m);
  
  fscanf(fobj, "%s", token);  // leitura da linha 0

  for(j=0; j<pControle->n; j++)
  {
    for(i=0; i<pControle->m; i++)
	 {
	     fscanf(fobj, "%s %f %f %f", token, &px, &py, &pz);

         pControle->ponto[j][i][0] = (px+rand() % 30 +0 - (rand() % 30 +0) ) * local_scale;
         pControle->ponto[j][i][1] = (py+rand() % 30 +0- (rand() % 30 +0) ) * local_scale;
         pControle->ponto[j][i][2] = (pz+rand() % 30 +0- (rand() % 30 +0) ) * local_scale;
         pControle->ponto[j][i][3] = 1.0f;
	 }
	fscanf(fobj, "%s", token);  // leitura da linha j+1;
  }

  // espaco de matriz para um patch
  if(pcPatch) pcPatch = liberaMatriz(pcPatch);
  pcPatch = AlocaMatriz(4, 4);

}

void processMenuEvents(int option)
{
    MatrizIdentidade();
    if(option==Solido)
		preenchido=1;
	else preenchido=0;

    
switch(option)
{
	case fogo:
		if(ligaParticulas)
		{
			ligaParticulas=false;			
		}
		else
		{
			ligaParticulas=true;
		}

		break;
	case RandomPtsControle:
		GeraPontos("ptosControleSuperficie4x4.txt");
		break;
	
	case PtsControle:
		CarregaPontos("ptosControleSuperficie4x4.txt");
		break;
	
	case Pontos:
		tipoView = GL_POINTS;
		break;
	
	case Linha:
		tipoView = GL_LINE_STRIP;
		break;
		
	case Solido:
		tipoView = GL_QUADS;
		break;
		
	case sair:
		exit(0);
		break;
		
	case BEZIER: case BSPLINE: case CATMULLROM:
		MontaMatrizBase(option);
		break;
	
	default:
		comando = option;
		switch(comando)
		{
			case Escalar:
				printf("Use as setas para cima ou direita para aumentar, e usa as setas para baixo ou para a esquerda para diminuir.\n");
				break;
			case RotarXYZ:
				printf("Para rotacionar no eixo(X,Y) use as setas para rotacionar para o lado desejado e para rotacionar no eixo Z use SHIFT+Seta para esquerda ou direita.\n");
				break;
			case Translada:
				printf("Use as setas para mover para o lado desejado.\n");
		}
		break;		
}
glutPostRedisplay();
}

void processColorMenuEvents(int option)//mudar a cor ---------------------
{
	printf("Depois de selecionar uma cor ative o modo preencher para vela.\n");

	
	switch(option)
	{
		case 1:R=1;G=0;B=0;
				if(preenchido==1)
					processMenuEvents(-2);
				break;
		case 2:R=0;G=1;B=0;
				if(preenchido==1)
					processMenuEvents(-2);				
				break;
		case 3:R=0;G=0;B=1;
				if(preenchido==1)
					processMenuEvents(-2);
				break;
		case 4:R=1;G=1;B=0;
				if(preenchido==1)
					processMenuEvents(-2);
				break;
		case 5:R=0;G=1;B=1;
				if(preenchido==1)
					processMenuEvents(-2);
				break;
		case 6:R=1;G=0;B=1;
				if(preenchido==1)
					processMenuEvents(-2);
				break;
		case 7:R=1;G=1;B=1;
				if(preenchido==1)
					processMenuEvents(-2);
				break; 
		case 8:printf("digite uma cor:");
			   scanf("%d%d%d",&R,&G,&B);
			break;
	}
	matrizCor(R,G,B);
	
}

void FogoColorMenuEvents(int option)//mudar a cor ---------------------
{
	
	switch(option)
	{
		case 0:FR=0;FG=1;FB=2;
			break;
		case 1:FR=0;FG=1;FB=1;
			break;
		case 2:FR=1;FG=0;FB=1;
			break;
		case 3:FR=1;FG=1;FB=0;
			break;
		case 4:FR=0;FG=0;FB=1;
			break;
		case 5:FR=1;FG=0;FB=0;
			break;
		case 6:FR=0;FG=1;FB=0;
			break;
		case 7:FR=0;FG=0;FB=0;
			break;
		case 8:printf("digite uma cor:");
			   scanf("%d%d%d",&FR,&FG,&FB);
			break;
	}
	
}

void createGLUTMenus()
{
	int menu, submenu, SUBmenuTransladar, SUBmenuGirar,SUBmenuSuperficie,SUBmenuPintar,SUBmenuCores,SUBmenuFogoCores;

    SUBmenuSuperficie = glutCreateMenu(processMenuEvents);
    glutAddMenuEntry("Bezier", BEZIER);
	glutAddMenuEntry("B-Spline",BSPLINE);
	glutAddMenuEntry("Catmull-Rom",CATMULLROM);

	SUBmenuPintar = glutCreateMenu(processMenuEvents);
	glutAddMenuEntry("Pontos",Pontos);
	glutAddMenuEntry("Malha",Linha);
	glutAddMenuEntry("Preenchido",Solido);
	
	SUBmenuCores = glutCreateMenu(processColorMenuEvents);//menu de core -----------------------
	glutAddMenuEntry("Vermelho",1);
	glutAddMenuEntry("Verde",2);
	glutAddMenuEntry("Azul",3);
	glutAddMenuEntry("Amarelo",4);
	glutAddMenuEntry("Ciano",5);
	glutAddMenuEntry("Magenta",6);
	glutAddMenuEntry("Branco",7);
	glutAddMenuEntry("Personalizado",8);
	
	SUBmenuFogoCores = glutCreateMenu(FogoColorMenuEvents);//menu de core -----------------------
	glutAddMenuEntry("Padrão",0);
	glutAddMenuEntry("Vermelho",1);
	glutAddMenuEntry("Verde",2);
	glutAddMenuEntry("Azul",3);
	glutAddMenuEntry("Amarelo",4);
	glutAddMenuEntry("Ciano",5);
	glutAddMenuEntry("Magenta",6);
	glutAddMenuEntry("Branco",7);
	glutAddMenuEntry("Personalizado",8);

	menu = glutCreateMenu(processMenuEvents);
	glutAddMenuEntry("Control Point ...", PtsControle);
	glutAddMenuEntry("Random Control Point ...", RandomPtsControle);
	glutAddMenuEntry("Ligar/Desligar Fogo", fogo);
	glutAddSubMenu("Surfaces", SUBmenuSuperficie);
	glutAddSubMenu("Objet View", SUBmenuPintar);
	glutAddSubMenu("Cores", SUBmenuCores);
	glutAddSubMenu("Cores do Fogo", SUBmenuFogoCores);
	glutAddMenuEntry("Scale", Escalar);
	glutAddMenuEntry("Rotate", RotarXYZ);
	glutAddMenuEntry("Translate",Translada);
	glutAddMenuEntry("Quit",sair);
	glutAttachMenu(GLUT_RIGHT_BUTTON);
}


//PARTÍCULAS===============================================================================================================================




void conceberParticulas (int i){    
	int random_number = rand() % 121 + 0; 
     //GLfloat r, alpha;   //Raio, Angulo
     GLfloat alpha, beta;                              
     GLfloat raio = 0.1 * randomico() + 0.06;
     alpha = 2 * M_PI * randomico();
     beta = M_PI * randomico();
      

      

      	     //Coordenadas do ponto de origem da particulas    
     Particulas[i].pos[0] = ppx[random_number];//pfx;//rand() % 8 - 4;   // posicao em x
     Particulas[i].pos[1] = ppy[random_number];//pfy;//5.0;               // posicao em y altura
     Particulas[i].pos[2] = ppz[random_number];//pfz;//rand() % 2 - 2;    // posicao em z
     
	  

     Particulas[i].vel[0]=  raio * cos(alpha) * sin(beta);  // velocidade em x
     Particulas[i].vel[1]=  raio * cos(beta);               // velocidade em y
     Particulas[i].vel[2]=  raio * sin(alpha) * sin(beta);  // velocidade em z   
     
     Particulas[i].ace[0] = 0.0;    // acelera em x
     Particulas[i].ace[1] = 0.025;  // acelera em y 
     Particulas[i].ace[2] = 0.0;    // acelera em z
     
     Particulas[i].massa = 0.01*randomico();  // massa da particula
     
     Particulas[i].cor[0] = randomico();        // R
     Particulas[i].cor[1] = 0.1*randomico();    // G 
     Particulas[i].cor[2] = 0.01*randomico();;  // B  
     
     Particulas[i].tempoVida = 0.8 + 0.98 * randomico();  // define o tempo de vida da particula
     Particulas[i].transparencia = 1.0;                  // grau de transaperencia da particula
}

void extinguirParticulas (int i){
     if(Particulas[i].tempoVida < 0.001){     
        conceberParticulas(i);                            
     } 
}

void iniciaParticulas (void){
     int i=0;

	   for(i; i< MAXPARTICULAS; i++)
		{
			if(ligaParticulas)
			{
				//conceberParticulas(i);
			}
        	
     	}

}
/*
void DesligaParticulas (void){
     int i;

     for(i = 0; i< MAXPARTICULAS; i++){
        conceberParticulas(i);
     }
}*/
/*
void inicializa(void){
     iniciaParticulas(); 
}*/

//         DESENHO
// -------------------------
void desenhaParticula2(void){
     int i;
    
     glDisable(GL_DEPTH_TEST);            //"Rastro"
     glEnable(GL_BLEND);                  //Habilita a transparencia
     glBlendFunc(GL_SRC_ALPHA, GL_ONE);   //Ativa a Transparência
     glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);    //Perspectiva    
     
    for(i = 0; i<MAXPARTICULAS; i++){
        glColor4f(Particulas[i].cor[FR], Particulas[i].cor[FG], Particulas[i].cor[FB], Particulas[i].transparencia);             
        glPointSize(20.0);
        glPushMatrix();
        glBegin(GL_POINTS);
            glVertex3f(Particulas[i].pos[0], Particulas[i].pos[1], Particulas[i].pos[2]);
        glEnd();
        glPopMatrix();
             
            // calculando EDO com Euler
        Particulas[i].pos[0] +=  PASSO_TEMP * Particulas[i].vel[0];
        Particulas[i].pos[1] +=  PASSO_TEMP * Particulas[i].vel[1];
        Particulas[i].pos[2] +=  PASSO_TEMP * Particulas[i].vel[2];
             
        Particulas[i].vel[0] += PASSO_TEMP * Particulas[i].ace[0];
        Particulas[i].vel[1] += PASSO_TEMP * Particulas[i].ace[1];
        Particulas[i].vel[2] += PASSO_TEMP * Particulas[i].ace[2];
             
        Particulas[i].tempoVida -= 0.01;
        Particulas[i].transparencia -= 0.01;
        extinguirParticulas(i);
    }
    glDisable(GL_BLEND);
}

//    DESENHA TUDO  



void idleF (void){
     glutPostRedisplay();
}



static void init(void)
{
   glClearColor(0.0, 0.0, 0.0, 1.0);
   glEnable(GL_DEPTH_TEST);
   glEnable(GL_MAP2_VERTEX_3);
   glEnable(GL_AUTO_NORMAL);
   glMapGrid2f(20, 0.0, 1.0, 20, 0.0, 1.0);
   //iniciaParticulas(); 
}
void display(void)
{
	 glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	 if(ligaParticulas)
	 {
	 	desenhaParticula2();
	 }
         

   glPushMatrix();

   if(pControle)
   {
	   MostrarPtosPoligControle(pControle);  // mostrando pontos de controle
       DisenaSuperficie();                   // disenhando un objeto
   }
   glutSwapBuffers();
}








int main(int argc, char** argv)
{
   glutInit(&argc, argv);

   glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
   glutInitWindowSize(700, 700);
   glutCreateWindow("Superficies ");

   init();

   glutReshapeFunc(reshape);
   glutSpecialFunc(keyboard);
   glutDisplayFunc(display);
   createGLUTMenus();
   
    glutIdleFunc(idleF);   // fun?ão de animacao


   glutMainLoop();
   return 0;
}
