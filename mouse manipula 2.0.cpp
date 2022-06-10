#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "GL/glut.h" 

#define MAXVERTEXS 30

#define PHI 3.141572
 
GLenum doubleBuffer;     
 
typedef struct spts 
{ 
    float v[3]; 
} tipoPto; 

//tipoPto ppuntos[MAXVERTEXS];

tipoPto pvertex[MAXVERTEXS];  // vetor de MAXVERTEXS de v[3] como espa�o m�ximo
int gVert=-1;
int windW, windH;     // medidas das dimensoes da janela Canvas: W largura  H: altura
int tipoPoligono;     // poligono prenchido, contorno ou pontos nos v�rtices
int gNumVert = 0;    // nnumero de pontos ou v�rtices do poligono criado
int jaPoligono = 0;   // modo de representacao--> 0: pontos;  1: pol�gono

float gMatriz[3][3];  // matriz de transformacao

int gIndVert = -1;   // indice do v�rtice selecionado
int gTransform = 0;   // transformacao: 0: nula; 1: Translacao; 2:�Rotacao; 3: Escala; 4: Espelha; 5: Cisalha 
float gCen[3];       // centroido do poligono
float gAng =0.0f;    // angulo para rotacao 

void init(void)
{
	int i;

	jaPoligono = 0;
	gNumVert = 0;   // zero pontos
	tipoPoligono = GL_POINTS;

	gTransform = 0; // transforma Nula
	gIndVert = -1;  // indice do v�rtice selecioado nulo

	// inicializa com ZERO o vetor pvertex 
	for(i=0; i<MAXVERTEXS; i++)
	{
		pvertex[i].v[0] = 0.0f;
		pvertex[i].v[1] = 0.0f;
		pvertex[i].v[2] = 1.0f;
	}
	// calcula o angulo b�sico de rotacao 
	gAng = (2.0f * PHI)/180.0f;
}

void matrizIdentidade(void) {
	gMatriz[0][0] = 1.0f;   gMatriz[0][1] = 0.0f;    gMatriz[0][2] = 0.0f;
	gMatriz[1][0] = 0.0f;   gMatriz[1][1] = 1.0f;    gMatriz[1][2] = 0.0f;
	gMatriz[2][0] = 0.0f;   gMatriz[2][1] = 0.0f;    gMatriz[2][2] = 1.0f;
}

static void Reshape(int width, int height)
{
    windW = width/2;
    windH = height/2;

    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    gluOrtho2D(-windW, windW, -windH, windH);

    glMatrixMode(GL_MODELVIEW);
}

static void Key(unsigned char key, int x, int y)
{
    switch (key) 
	{
      case 27:
			exit(0);
    }
}

void coord_line(void)
{
    glLineWidth(1);

	glColor3f(1.0, 0.0, 0.0);

	// vertical line

	glBegin(GL_LINE_STRIP);
		glVertex2f(-windW, 0);
		glVertex2f(windW, 0);
    glEnd();

		glColor3f(0.0, 1.0, 0.0);

	// horizontal line 

    glBegin(GL_LINE_STRIP);
		glVertex2f(0, -windH);
		glVertex2f(0, windH);
    glEnd();
}

void PolygonDraw(void)
{
	int i;

	glColor3f(0.0, 0.0, 0.0); 

	glPolygonMode(GL_FRONT_AND_BACK, tipoPoligono);

	glBegin(tipoPoligono);
	for(i=0; i<gNumVert; i++)
	{
		glVertex2fv(pvertex[i].v);
	}
	glEnd();
}

static void Draw(void)
{
    glClearColor(1.0, 1.0, 1.0, 0.0); 
    glClear(GL_COLOR_BUFFER_BIT);

	coord_line();

	PolygonDraw();

    if (doubleBuffer) 
	{
	   glutSwapBuffers(); 
    } else {
	   glFlush();     
    }
}

void calCentro(float cc[])
{
     int i;
    // computando o centroide
     cc[0] = cc[1] = cc[2] = 0.0f;    
     for (i=0; i<gNumVert; i++) {
		cc[0] += pvertex[i].v[0];
		cc[1] += pvertex[i].v[1];
		cc[2] += pvertex[i].v[2];
	 }
     cc[0] /= gNumVert;     
     cc[1] /= gNumVert;     
     cc[2] /= gNumVert;   
}

//  Vetor = Matriz x Vetor
// ----------------------------
void operaTransforma(float v[]) {
	float temp[3];
	int i, j;
	  // temp = Matriz x vetor
	for(i=0; i<3; i++) {
		temp[i] = 0.0f;
		for(j=0; j<3; j++)
			temp[i] = temp[i] + gMatriz[i][j] * v[j];
	}
	  // copia vetor resultando no vetor original
	for(i=0; i<3; i++)
		v[i] = temp[i];
}


void translate(float dx, float dy)
{
	int i;
    matrizIdentidade();    // gera a matriz de identidade gMatriz
     
     	// preenche matriz translacao:  [ 1  0  dx |  0  1  dy |  0  0  1 ] 
    gMatriz[0][2] = dx;
    gMatriz[1][2] = dy;
    
    	// opera transforma��o de cada vetor v�rtice
	for (i=0; i<gNumVert; i++) 
		operaTransforma (pvertex[i].v);
}

//----- Rotacao de um poligono -- cada v�rtice � um vetor ------------------------------------------------   
//      Calcular vetor de traslacao (vetor do origem ao centro do poligono) --> VC
//      Transladar o pol�gono ao origem do sistema   --->  MatrizTraslacao(VC). Poligono => PoligonoT
//      Definir a matriz de Rotacao --> MatrizRotacao(teta)   
//      Rotacionar o poligono   ---> MatrizRotacao(teta).Poligono ==> PoligonoTR
//      Trasladar para origem original --> MatrizTranslacao(-VC).PoligonoTR ==> PoligonoTRT' = PoligonoR
// ---------------------------------------------------------------------------------------------------------
void rotate(float dx, float dy)
{
     int i; 
     float oo, teta, vc[3]; 
     
     // calculo do angulo 
     // ----------------------------------------------
     // seja vetor do centro para o vertice: vv 
     // dd = (dx, dy) � o vetor deslocalmento do mouse
     // oo = vv x dd  (produto vetorial)
     // se oo positivo ==> rota antihorario;  
     // se oo negativo ==> rota horario
     // ----------------------------------------------
 
     calCentro(vc);                    // calculo vetor vc ao centro geom�tric do pol�gono
     translate(-1*vc[0], -1*vc[1]);    // translada o pol�go ao origem em -vc
     
     // determinando o angulo: 
     
	    // produto vetorial --> (v[0], v[1]) x (dx, dy)  
     oo = pvertex[gIndVert].v[1] * dx - pvertex[gIndVert].v[0] * dy;
     
     	// oo � esacalar positivo (horaria) ou negativo (anti-horaria)

     teta = gAng;                // angulo constante definido no init
     if(oo>0.0f)
        teta = -1.0f * gAng; 		 

		// Define a matriz de Rotacao	 
 	 matrizIdentidade();  
 	 gMatriz[0][0] = cos(teta);    gMatriz[0][1] = -sin(teta);
 	 gMatriz[1][0] = sin(teta);    gMatriz[1][1] = cos(teta);
 	 
    	// opera transforma��o Rotacao de cada vetor v�rtice
	 for (i=0; i<gNumVert; i++) 
		operaTransforma (pvertex[i].v);
 	 
     translate(vc[0], vc[1]);     // o poligo � tranladado para sua posicao original
}



void scale(float dx, float dy)
{
     int i;
     float sx, sy,vc[3];
     
     calCentro(vc);                    
     translate(-1*vc[0], -1*vc[1]);
     
    
     sx = sy = 1.0f;
     if(fabs(pvertex[gIndVert].v[0]) > 0.01f)
        sx = 1.0f + dx / pvertex[gIndVert].v[0];
     if(fabs(pvertex[gIndVert].v[1]) > 0.01f)
        sy = 1.0f + dy / pvertex[gIndVert].v[1];  
		
	matrizIdentidade(); 
	gMatriz[0][0] = sx;
 	gMatriz[1][1] = sy;	  
		 
		 
	for (i=0; i<gNumVert; i++) 
	operaTransforma (pvertex[i].v);	 

     translate(vc[0], vc[1]);                    
}

void mirror(float dx, float dy,int option)
{
     int i; 
     float vc[3]; 
	
	if(option<7)
	{
		calCentro(vc);                    
     	translate(-1*vc[0], -1*vc[1]);    
	}

	 
	 switch (option)
	    {
               case 4: case 7: //X
			  	matrizIdentidade();  
 	 			gMatriz[0][0] = 1;    gMatriz[0][1] = 0;
 	 			gMatriz[1][0] = 0;    gMatriz[1][1] = -1;

	 			for (i=0; i<gNumVert; i++) 
					operaTransforma (pvertex[i].v);  
			   break;     
              
			   case 5: case 8: //Y
			    matrizIdentidade();  
 	 			gMatriz[0][0] = -1;    gMatriz[0][1] = 0;
 	 			gMatriz[1][0] = 0;    gMatriz[1][1] = 1;

	 			for (i=0; i<gNumVert; i++) 
					operaTransforma (pvertex[i].v);
			   break;   
               
			   case 6: case 9: //inverte
			    matrizIdentidade();  
 	 			gMatriz[0][0] = 0;    gMatriz[0][1] = 1;
 	 			gMatriz[1][0] = 1;    gMatriz[1][1] = 0;

	 			for (i=0; i<gNumVert; i++) 
					operaTransforma (pvertex[i].v);
			   break;                  
        } 
		 
	if(option<7)
	{
		translate(vc[0], vc[1]);     
	}	 
   
}



void shear(float dx, float dy)
{
     int i;
     float sx, sy, xy[3],vc[3];
     
     sx = 0.000000000000000001f*dx;
     sy = 0.000000000000000001f*dy;
     
     if(dx>dy)
     {
         if(fabs(pvertex[gIndVert].v[0])>0.1f)
         {
         	sx =  dx / pvertex[gIndVert].v[0];
		 }
            
     }
     else
     {
         if(fabs(pvertex[gIndVert].v[1])>0.1f)
         {
         	sy =  dy / pvertex[gIndVert].v[1]; 
		 }
            
     }
         
  	 calCentro(vc);                    
     translate(-1*vc[0], -1*vc[1]); 
	 
	matrizIdentidade();  
 	gMatriz[1][0] = sx;       
    
    	
	 for (i=0; i<gNumVert; i++)
	 {
	 	xy[0] = pvertex[i].v[0];
		xy[1] = pvertex[i].v[1];
	 	operaTransforma (pvertex[i].v);
	 } 
		
 	 
     translate(vc[0], vc[1]); 
  
}

// -------- verifica se um ponto (x, y) posia se considerar um v�rtice do pol�gono
// ---------------------------------------------------------------------------------
int clipVertex(int x, int y)
{
	int i;
	float d;
	gIndVert=-1;
	    // para cada v�rtice do poligono
	for (i=0; i<gNumVert; i++) {
		   // distancia do ponto (x, y) a cada v�rtice do poligono
		d = sqrt(pow((pvertex[i].v[0]-x), 2.0) + pow((pvertex[i].v[1]-y), 2.0));
		   // se a distancia d � bem proxima ( d < 3 pixel)
		if(d < 5.0){
			gIndVert = i;    // achou o indice do v�rtice
			break;
		}
	}
	return gIndVert;
}

// -------- Evento Arrastando o mouse pressionado o bot�o direito
// -------------- gerando manipulacao direta ---------------------
void motion(int x, int y)
{
	float dx, dy;
	if(gIndVert>-1) {
		x=x-windW; y=windH-y;
		dx = x - pvertex[gIndVert].v[0];
		dy = y - pvertex[gIndVert].v[1];
	    switch (gTransform)
	    {
               case 1:translate(dx, dy); break;     
               case 2:rotate(dx, dy); break;   
               case 3:scale(dx, dy); break; 
               case 4 ... 9:mirror(dx, dy,gTransform); break;  
               case 10:shear(dx, dy); break;     
        }
		Draw();
	}

}

// -------- Eveto CLICK bot�o MOUSE ----------
// -------------------------------------------
void mouse(int button, int state, int x, int y)
{ 
	if(state == GLUT_UP)       //  bot�o SOLTO
	{
        printf("\n jaPoligono %d ", jaPoligono);
		if(button == GLUT_LEFT_BUTTON) 
		{
			if(jaPoligono==0)       // ainda n�o definido o pol�gono
			{
				x = x - windW; 
				y = windH - y;

				glPointSize(3);

				pvertex[gNumVert].v[0] = (float)x;
				pvertex[gNumVert].v[1] = (float)y;
				pvertex[gNumVert].v[2] = 1.0f;
				gNumVert++;
			}
		} 		
	}
	else  // state == GLUT_DOWN  --> Bot�o pressionado
	{
		if(jaPoligono==1) {					// pol�gono j� definido
			if(gTransform) {				// Uma ransformacao selecionada
				if(  button == GLUT_LEFT_BUTTON) {
					x = x - windW; 
					y = windH - y;
					// clipingVertex
					gIndVert = clipVertex(x, y);    // verificar e identificar um v�rtice selecionado
					printf("\n clipVetex: %d ", gIndVert);
				}
			}
		}
	}
	glutPostRedisplay();
}

static void Args(int argc, char **argv)
{
    GLint i;

    doubleBuffer = GL_FALSE;

    for (i = 1; i < argc; i++) 
	{
	   if (strcmp(argv[i], "-sb") == 0)   
	   {
	      doubleBuffer = GL_FALSE;
	   } else if (strcmp(argv[i], "-db") == 0) 
	   {
	      doubleBuffer = GL_TRUE;
	   }
    }
}

// ------------------------------
// Eventos definidos no MENU 
// ------------------------------

// Evento do menu geral
// ---------------------------
void processMenuEvents(int option) 
{
	switch (option) 
	{
		case 0 : 
			init();
			break;
	}
	glutPostRedisplay();
}

// Eventos do menu de geracao de poligono
// --------------------------------------
void processSubMenuEventsPolig(int option) 
{
	if(option == 0)
		tipoPoligono = GL_POINTS;
	else
		if (option == 1)
		{
			tipoPoligono = GL_LINE_LOOP;
			jaPoligono = 1;
        }

	glutPostRedisplay();
}

// Eventos do menu de transformadas
// ----------------------------------
void processSubMenuEventsTransf(int option) 
{
	if(jaPoligono) {
		gTransform = option;
	switch (gTransform)
	    {
            case 1:printf("click e arraste um vertice para mover o poligono");break;     
            case 2:printf("click e arraste um vertice para rodar o poligono");break;   
            case 3:printf("click e arraste um vertice para aumentar e diminuir o poligono");break;
            case 4 ... 9:printf("click e arraste um vertice para espelhar o poligono");break;  
            case 10:printf("click e arraste um vertice para deformar o poligono");break;     
        }
	}

	glutPostRedisplay();
}

// Defini��o do Menu principal
// ---------------------------
void createGLUTMenus() 
{
	int menu,submenu1, submenu2,submenu3,submenu4;

	submenu1 = glutCreateMenu(processSubMenuEventsPolig);
	glutAddMenuEntry("Pontos",0);
	glutAddMenuEntry("Poligono",1);

	submenu2 = glutCreateMenu(processSubMenuEventsTransf);
	glutAddMenuEntry("Transla��o",1);
	glutAddMenuEntry("Rota��o",2);
	glutAddMenuEntry("Escala",3);
	glutAddMenuEntry("Cisalha",10);	

		
	
	submenu3 = glutCreateMenu(processSubMenuEventsTransf);
	glutAddMenuEntry("Inverte X",4);
	glutAddMenuEntry("Inverte Y",5);
	glutAddMenuEntry("Inverte Diagonal",6);
	
	submenu4 = glutCreateMenu(processSubMenuEventsTransf);
	glutAddMenuEntry("Espelho X",7);
	glutAddMenuEntry("Espelho Y",8);
	glutAddMenuEntry("Espelho inverte",9);
		
	menu = glutCreateMenu(processMenuEvents);
	glutAddMenuEntry("Limpar", 0);
	glutAddSubMenu("Tipo Objeto",submenu1);
	glutAddSubMenu("Transforma��o",submenu2);
	glutAddSubMenu("Inverter",submenu3);
	glutAddSubMenu("Espelhar",submenu4);

	glutAttachMenu(GLUT_RIGHT_BUTTON);
}

int main(int argc, char **argv)
{
    GLenum type; 

    glutInit(&argc, argv);
    Args(argc, argv);

    type = GLUT_RGB;
    type |= (doubleBuffer) ? GLUT_DOUBLE : GLUT_SINGLE;

    glutInitDisplayMode(type);
    glutInitWindowSize(600, 500);
    glutCreateWindow("Basic Program Using Glut and Gl");

	init();

    glutReshapeFunc(Reshape);
    glutKeyboardFunc(Key);
    glutDisplayFunc(Draw);

	glutMotionFunc(motion);
	glutMouseFunc(mouse);       // CLICK e RELEASE bot�o de mouse

//	glutIdleFunc(idle);

	createGLUTMenus();  

    glutMainLoop();

	return (0);
}
