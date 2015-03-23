#include <stdlib.h>
#include <Windows.h>
#include <iostream>
#include <tchar.h>
#include <vector>
#include <locale.h>
#include <math.h>
#include "glut.h"
#include "GLAUX.H"
#pragma comment(lib, "glut32.lib")
#pragma comment(lib, "GLAUX.LIB") 

int size = 25;
int R = 5*size;
int log_oper = 0;
int background = 0;
#define Translate_circle 1
#define Rotate_circle 2
float red, green, blue;
int flag_translate = 0;
int flag_rotate = 0;
int first = 1;
using namespace std;
struct color
{
	int R, G, B;
};

struct Point
{
	int x, y;
	color Color;
};
struct circle
{
	Point P;//Параметры текущей фигуры
	int R=5;
	int translate_x = 0;
	int translate_y = 0;
	int rotate_angle = 0;
	int is_texture = 0;
};
GLint Widht = 720, Height = 720;//Window sets
Point TPoint;//Temp TPoint
circle TCircle;//Temp circle
vector<circle> Circle;//circle stack
GLuint TexId;
AUX_RGBImageRec* pImage; 
char* strFile = "1.bmp";
int flag_texture = 0;
void TexInit(){
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	//--Создание идентификатора для текстуры- —--------------------------//
	glGenTextures(1, &TexId);
	//--Загрузка изображения в память-------------------------------------//
	pImage = auxDIBImageLoad(strFile);

	int BmpWidth = pImage->sizeX;
	int BmpHeight = pImage->sizeY;
	void* BmpBits = pImage->data;
	//--Начало описания свойств текстуры----------------------------------//
	glBindTexture(GL_TEXTURE_2D, TexId);
	//--Создание уровней детализации и инициализация текстуры —----------//
	gluBuild2DMipmaps(GL_TEXTURE_2D, 3, BmpWidth, BmpHeight, GL_RGB, GL_UNSIGNED_BYTE, BmpBits);
	//--Задание параметров текстуры---------------------------------------//
	//--Повтор изображения по параметрическим осям s и t--GL_REPEAT----------------//
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	//--Не использовать интерполяцию при выборе точки на текстуреGL_LINEAR---------//
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//--Совмещать текстуру и материал объекта----------GL_MODULATE-------------------//
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	//glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
}

void drawpixel(int x, int y,color Color)
{	
	glColor3f(Color.R, Color.G, Color.B);
	if (flag_texture)
		glEnable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);
	glTexCoord2f(0, 0);	glVertex2f(x, y);
	glTexCoord2f(1, 0);	glVertex2f(x + size, y);
	glTexCoord2f(1, 1);	glVertex2f(x + size, y + size);
	glTexCoord2f(0, 1);	glVertex2f(x, y + size);
	glEnd();
	if (flag_texture)
		glDisable(GL_TEXTURE_2D);
}

void drawCircle(int x0, int y0, int RR, color Color)
{
	int x;
	int Radius;
	int y;
	int Z;
	x = 0;
	Radius = RR - RR%size;
	y = Radius;
	Z = 0;
	int tmp = 0;
	while (x <= y)
	{
		tmp = y;
		if (x != 0)
		{
			if (x != y)
			{
				drawpixel(x0 + x, y0 - y, Color);
				drawpixel(x0 - y, y0 + x, Color);
			}
			drawpixel(x0 - x, y0 + y, Color);
			drawpixel(x0 + y, y0 - x, Color);
		}
		if (x != y)
		{
			drawpixel(x0 + x, y0 + y, Color);
			drawpixel(x0 - y, y0 - x, Color);
		}
		drawpixel(x0 - x, y0 - y, Color);
		drawpixel(x0 + y, y0 + x, Color);

		for (int i = y - size; i >= x; i -= size)
		{
			if (x != 0)
			{
				drawpixel(x0+x, y0 + i, Color);
				drawpixel(x0+x, y0 - i, Color);
				if (i != x)
				{
					drawpixel(x0 + i, y0 + x, Color);
					drawpixel(x0 - i, y0 + x, Color);
				}
			}
			drawpixel(x0 - x, y0 + i, Color);
			drawpixel(x0 - x, y0 - i, Color);
			if (i != x)
			{
				drawpixel(x0 + i, y0 - x, Color);
				drawpixel(x0 - i, y0 - x, Color);
			}

		}

		if (Z > 0)
		{
			y -= size;
			Z -= y;
			Z -= y;
		}
		x += size;
		Z += x;
		Z += x;
	}

	if (!background)
		glColor3f(0, 0, 0);
	else
		glColor3f(1, 1, 1);
	glTranslatef(x0 + size/2, 0, 0);
	glTranslatef(0, y0 + size/2, 0);
	glBegin(GL_LINE_LOOP);
	float a, RADIUS = Radius;
	for (int i = 0; i < 50; i++)
	{
		a = (float)i / 50 * 3.1415f * 2.0f;
		glVertex2f(cos(a) * RADIUS, sin(a) * RADIUS);
	}
	glEnd();
	glTranslatef(-x0 - size/2, 0, 0);
	glTranslatef(0, -y0 - size/2, 0);
}

void Display(void)
{
	if (background)
		glClearColor(0, 0, 0, 1);
	else
		glClearColor(1, 1, 1, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glShadeModel(GL_SMOOTH);
	glLineWidth(1.0);
	glPointSize(2.0);
	int x0;
	int y0;
	if (background)
	glColor3f(1, 1, 1);
	else
		glColor3f(0, 0, 0);
	
	//Сетка
	int n = (int)Height / size;
	for (int a = size; a<Height; a += size)
	{
		glBegin(GL_LINES);
		glVertex2f(0, a);
		glVertex2f(Widht, a);
		glEnd();
	}
	n = (int)Widht / size;
	for (int a = size; a<Widht; a += size)
		{
		glBegin(GL_LINES);
		glVertex2f(a, 0);
		glVertex2f(a, Height);
		glEnd();
	}
	glFinish();
	if (log_oper)
	{
		glEnable(GL_COLOR_LOGIC_OP);
		glLogicOp(GL_XOR);
	}
	for (unsigned int k = 0; k < Circle.size(); k++)
	{
		if (Circle[k].is_texture)
			flag_texture = 1;
		x0 = Circle[k].P.x - Circle[k].P.x%size;
		y0 = -Circle[k].P.y + Height + (Circle[k].P.y - Height) % size;
		//движения
		glTranslated(Circle[k].translate_x, Circle[k].translate_y, 0);
		//вращения
		glTranslated(x0 + size / 2, y0 + size / 2, 0);
			glRotated(Circle[k].rotate_angle, 0, 0, 1);
		glTranslated(-x0 - size / 2, -y0 - size / 2, 0);

		drawpixel(x0, y0, Circle[k].P.Color);
		drawCircle(x0, y0, Circle[k].R, Circle[k].P.Color);
		//восстановление после движения
		//восстановление после вращания
		glTranslated(x0 + size / 2, y0 + size / 2, 0);
			glRotated(-Circle[k].rotate_angle, 0, 0, 1);
		glTranslated(-x0 - size / 2, -y0 - size / 2, 0);
		glTranslated(-Circle[k].translate_x, -Circle[k].translate_y, 0);
		flag_texture = 0;
	}
	//texture_draw(300, 300);
	if(log_oper)
		glDisable(GL_COLOR_LOGIC_OP);
	glFinish();
	flag_texture = 0;
};
void Reshape(GLint w, GLint h)
{
	Widht = w; Height = h;
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, w, 0, h, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
};
void Keyboard(unsigned char key, int x, int y)
{
#define ESCAPE 27
#define enter 13
#define  backspace 8
	switch (key)
	{
	case ESCAPE: exit(0);
	case '1':	
		TCircle.P.Color.R = 255;
		TCircle.P.Color.G = 0;
		TCircle.P.Color.B = 0;
		cout << "Красный\n";
		break;
	case '2':	
		TCircle.P.Color.R = 0;
		TCircle.P.Color.G = 255;
		TCircle.P.Color.B = 0;
		cout << "Зелёный\n";
		break;
	case '3':	
		TCircle.P.Color.R = 0;
		TCircle.P.Color.G = 0;
		TCircle.P.Color.B = 255;
		cout << "Синий\n";
		break;
	case '+':
		R +=size;
		cout <<"R = "<< R/size << endl;
		break;
	case '-':
		if (R/size>1)
			R-=size;
		cout << "R = " << R/size << endl;
		break;
	case 'q':
		if (log_oper)
		{
			log_oper = 0;
			cout << "NOT" << endl;
		}
		else
		{
			log_oper = 1;
			cout << "XOR" << endl;
		}
		break;
	case 'z':
		size++;
		break;
	case 'x':
		if (size>1)
			size--;
		break;
	case 'w':
		if (background)
			background = 0;
		else 
			background = 1;
		break;
	case enter:
		Circle[Circle.size()-1].is_texture = 1;
		break;
	};
	glutPostRedisplay();
}

int num_circle;
void Keyboard_Spec(int key, int x, int y)
{
	if (first&&(flag_translate||flag_rotate))
	{
		for (unsigned int i = 0; i < Circle.size(); i++)
		{
			if (x <= Circle[i].P.x + Circle[i].R + Circle[i].translate_x &&
				x >= Circle[i].P.x - Circle[i].R + Circle[i].translate_x &&
				y <= Circle[i].P.y + Circle[i].R + Circle[i].translate_y &&
				y >= Circle[i].P.y - Circle[i].R + Circle[i].translate_y)
				num_circle = i;
		}
		first = 0;
	}
		switch (key) {
		case GLUT_KEY_DOWN:
			{
					if (flag_translate)
					{
						Circle[num_circle].translate_y -= size;
					}
			}
			break;
		case GLUT_KEY_UP:
			{
					if (flag_translate)
					{
						Circle[num_circle].translate_y += size;
					}
			}
			break;
		case GLUT_KEY_LEFT:
			{
					if (flag_translate)
					{
						Circle[num_circle].translate_x -= size;
					}
					else
						if (flag_rotate)
						{
							Circle[num_circle].rotate_angle -= 90;
						}
			}
			break;
		case GLUT_KEY_RIGHT:
			{
					if (flag_translate)
					{
						Circle[num_circle].translate_x += size;
					}
					else
						if (flag_rotate)
						{
							Circle[num_circle].rotate_angle += 90;
						}
			}
			break;
		case GLUT_ENTERED:
			cout << "enter" << endl;
			break;
		}
	glutPostRedisplay();
}
void processMenuEvents(int option)
{

	switch (option) {
	case Translate_circle:
		flag_rotate = 0;
		flag_translate = 1;
		first = 1;
		flag_texture = 0;
		break;
	case Rotate_circle:
		flag_rotate = 1;
		flag_translate = 0;
		first = 1;
		flag_texture = 0;
		break;
	}
}
void createGLUTMenus() {
	int menu;
	menu = glutCreateMenu(processMenuEvents);
	glutAddMenuEntry("Двигать", Translate_circle);
	glutAddMenuEntry("Поворачивать", Rotate_circle);
	glutAttachMenu(GLUT_RIGHT_BUTTON);
}

void Mouse(int button, int state, int x, int y)
{
	if (state == GLUT_UP&&button == GLUT_LEFT_BUTTON)
	{
		TCircle.P.x = x;
		TCircle.P.y = y;
		TCircle.R = R;
		Circle.push_back(TCircle);
		TCircle.P.x = 0;
		TCircle.P.y = 0;
		flag_rotate = 0;
		flag_translate = 0;
	}
	createGLUTMenus();
	glutPostRedisplay();
};

int wmain(int argc, char* argv[])
{
	setlocale(LC_ALL, "Russian");
	cout << "Горячие клавиши:\n";
	cout << "1 - красный\n2 - зеленый\n3 - синий\n";
	TCircle.P.Color.R = 255;
	TCircle.P.Color.G = 0;
	TCircle.P.Color.B = 0;
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB);
	glutInitWindowSize(Widht, Height);
	glutInitWindowPosition(400, 3);
	glutCreateWindow("Компьютерная графика");
	TexInit();
	glutDisplayFunc(Display);
	glutReshapeFunc(Reshape);
	glutKeyboardFunc(Keyboard);
	glutSpecialFunc(Keyboard_Spec);
	glutMouseFunc(Mouse);
	glutMainLoop();
	return 0;
}