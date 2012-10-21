
#include <stdio.h> // printf
#include <stdlib.h> // exit
#include <string.h> // strlen

#if defined __APPLE_CC__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

static int window_width = 300;
static int window_height = 300;
static int cursor_x = window_width/2;
static int cursor_y = window_height/2;

static GLint glyphs_display_list = 0;

static void glutInitGlyphs()
{
    //just doing 7-bit ascii
    glyphs_display_list = glGenLists(256);

    for (int i = 0; i < 256; i++) {
        glNewList(glyphs_display_list + i, GL_COMPILE);
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, i);
        glEndList();
    }
}

static void glutDrawString(int x, int y, const char * text)
{
    glListBase(glyphs_display_list);
    glRasterPos2i(x+1, y + 4);
    glCallLists((GLsizei)strlen(text), GL_UNSIGNED_BYTE, text);
}

static void glutKeyboardCallback(unsigned char key, int x, int y)
{
    //printf( "keydn=%i\n", key );
    
    int modifiers = glutGetModifiers();

    if (modifiers & GLUT_ACTIVE_SHIFT) {

    }
    if (modifiers & GLUT_ACTIVE_CTRL) {

    }
    if (modifiers & GLUT_ACTIVE_ALT) {

    }

    if (key == 27) {
        exit(0);
    }
}

static void glutKeyboardUpCallback(unsigned char key, int x, int y)
{
    //printf( "keyup=%i\n", key );
}

static void glutMouseCallback(int button, int state, int x, int y)
{
    cursor_x = x;
    cursor_y = y;

    if (button == GLUT_LEFT_BUTTON) {
    }
    else if (button == GLUT_RIGHT_BUTTON) {
    }
    else if (button == GLUT_MIDDLE_BUTTON) {
    }

    glutPostRedisplay();
}

static void glutMotionCallback(int x, int y) 
{
    cursor_x = x;
    cursor_y = y;

    glutPostRedisplay();
}

static void glutDisplayCallback(void)
{
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBegin(GL_LINES);
    glVertex2f(cursor_x, cursor_y);
    glVertex2f(window_width/2, window_height/2);
    glEnd();

    glutDrawString(16, 16, "Hello GLUT");
    
    glutSwapBuffers();
}

static void glutReshapeCallback(int width, int height)
{   
    window_width = width;
    window_height = height;

    glViewport(0, 0, window_width, window_height);
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity(); 
    glOrtho(0, window_width, window_height, 0, /*near=*/0, /*far=*/1);
    
    glutPostRedisplay();
}

/*static void glutIdleCallback(void)
{
    glutPostRedisplay();
}*/

/*static void glutTimerFunc(int value)
{
    glutPostRedisplay();
}*/

int main(int argc, char *argv[])
{
    glutInit(&argc, argv);

    //glutInitDisplayString("rgb depth double samples=4");
    glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH );
    glutInitWindowSize( window_width, window_height );
    glutCreateWindow( "glut-template" );
    glutKeyboardFunc( glutKeyboardCallback );
    glutKeyboardUpFunc( glutKeyboardUpCallback );
    glutMouseFunc( glutMouseCallback );
    glutMotionFunc(glutMotionCallback);
    //glutPassiveMotionFunc(glutMotionCallback);
    glutDisplayFunc( glutDisplayCallback );
    glutReshapeFunc( glutReshapeCallback );
    //glutIdleFunc( glutIdleCallback );
    //glutTimerFunc( 100, glutTimerFunc, 0 );

    glutInitGlyphs();

    glutPostRedisplay();

    glutMainLoop();

    return 0;
}

