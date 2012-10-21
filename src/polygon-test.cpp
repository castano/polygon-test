
#include <stdio.h> // printf
#include <stdlib.h> // exit
#include <string.h> // strlen
#include <math.h> // strlen

#if defined __APPLE_CC__
#include <GLUT/glut.h>
#include <OpenGL/glu.h>
#else
#include <GL/glut.h>
#include <GL/glu.h>
#endif


// Polygon 

const int max_vertex_count = 65536;
int x_array[max_vertex_count];
int y_array[max_vertex_count];
int vertex_count = 0;
bool done;

void polygon_add_vertex(int x, int y) {
    if (vertex_count == 0) {
        x_array[vertex_count] = x;
        y_array[vertex_count] = y;
        vertex_count++;
    }
    x_array[vertex_count] = x;
    y_array[vertex_count] = y;
    vertex_count++;
    done = false;
}

void polygon_undo_add() {
    vertex_count--;
    if (vertex_count < 0) vertex_count = 0;
    done = false;
}

void polygon_set_vertex(int x, int y) {
    if (!done) {
        x_array[vertex_count-1] = x;
        y_array[vertex_count-1] = y;
    }
}

void polygon_done() {
    polygon_undo_add();
    done = true;
}

void polygon_reset() {
    vertex_count = 0;
    done = false;
}

void polygon_save() {
    FILE * fp = fopen("polygon.txt", "wb");
    if (fp != NULL) {
        for (int i = 0; i < vertex_count; i++) {
            fprintf(fp, "%d %d\n", x_array[i], y_array[i]);
        }
        fclose(fp);
    }
}

void polygon_open(const char * name) {
    FILE * fp = fopen(name, "rb");
    if (fp != NULL) {
        vertex_count = 0;
        int read;
        do {
            read = fscanf(fp, "%d %d\n", &x_array[vertex_count], &y_array[vertex_count]);
            vertex_count++;
        } while(read == 2);
        vertex_count--;

        done = true;

        fclose(fp);        
    }
}


// Polygon rendering

int render_mode = 0;
const int render_mode_count = 4;
const char * render_mode_name[render_mode_count] = {
    "Boundary",
    "GLU",
    "Stencil Naive",
    "Stencil Ear Clip",
};

void draw_polygon_boundary() {
    if (vertex_count == 0) return;

    glColor3f(1, 1, 1);

    glBegin(GL_LINES);
    for (int i = 0, j = vertex_count-1; i < vertex_count; j=i, i++) {
        glVertex2i(x_array[j], y_array[j]);
        glVertex2i(x_array[i], y_array[i]);
    }
    glEnd();
}

void tcbBegin (GLenum prim) {
   glBegin (prim);
}

void tcbVertex (void *data) {
   glVertex3dv ((GLdouble *)data);
}

void tcbEnd () {
   glEnd ();
}

void tcbCombine (GLdouble c[3], void *d[4], GLfloat w[4], void **out) {
    GLdouble * nv = (GLdouble *) malloc(sizeof(GLdouble)*3);
    nv[0] = c[0];
    nv[1] = c[1];
    nv[2] = c[2];
    *out = nv; 
}

void draw_polygon_glu() {
    if (vertex_count < 3) return;

    GLUtriangulatorObj * t = gluNewTess();
    gluTessCallback(t, GLU_TESS_BEGIN, (GLvoid (*)())tcbBegin);
    gluTessCallback(t, GLU_TESS_VERTEX, (GLvoid (*)())tcbVertex);
    gluTessCallback(t, GLU_TESS_END, (GLvoid (*)())tcbEnd);
    gluTessCallback(t, GLU_TESS_COMBINE, (GLvoid (*)())tcbCombine);

    double * v = (double *)malloc(3 * vertex_count * sizeof(double));

    glColor3f(0, 0, 0.5);

    gluBeginPolygon(t);
    gluTessBeginContour(t);

    for (int i = 0; i < vertex_count; i++) {
        v[3*i+0] = double(x_array[i]);
        v[3*i+1] = double(y_array[i]);
        v[3*i+2] = 0.0f;
        gluTessVertex(t, &v[3*i+0], &v[3*i+0]);
    }

    gluTessEndContour(t);
    gluEndPolygon(t);

    free(v);

    gluDeleteTess(t);
}

void setup_stencil_pass() {
    glEnable(GL_STENCIL_TEST);
    glStencilMask(1);
    glStencilOp(GL_KEEP, GL_KEEP, GL_INVERT);
    glStencilFunc(GL_ALWAYS, 0, ~0);
    
    if (1) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);
        glColorMask(1, 1, 1, 1);
    }
    else {
        glDisable(GL_BLEND);
        glColorMask(0, 0, 0, 0);
    }
}

/*void setup_overdraw_pass() {
    glDisable(GL_STENCIL_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    glColorMask(1, 1, 1, 1);
}*/

void setup_shading_pass() {
    glEnable(GL_STENCIL_TEST);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    glStencilFunc(GL_EQUAL, 1, ~0);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    glColorMask(1, 1, 1, 1);
}
void reset_state() {
    glDisable(GL_STENCIL_TEST);
    glDisable(GL_BLEND);
    glColorMask(1, 1, 1, 1);
}

void draw_polygon_stencil() {
    if (vertex_count < 3) return;

    // Naive implemenation. One triangle for each edge in fan order.

    setup_stencil_pass();

    glColor3f(0.1, 0, 0);

    glBegin(GL_TRIANGLES);

    extern int window_width;    // From below.
    extern int window_height;

    int x_min = window_width;
    int y_min = window_height;
    int x_max = 0;
    int y_max = 0;

    for (int i = 0, j = vertex_count-1; i < vertex_count; j=i, i++) {
        glVertex2i(x_array[0], y_array[0]);
        glVertex2i(x_array[j], y_array[j]);
        glVertex2i(x_array[i], y_array[i]);

        if (x_array[i] < x_min) x_min = x_array[i];
        if (y_array[i] < y_min) y_min = y_array[i];
        if (x_array[i] > x_max) x_max = x_array[i];
        if (y_array[i] > y_max) y_max = y_array[i];
    }

    glEnd();

    setup_shading_pass();

    glColor3f(0, 0, 0.5);

    glBegin(GL_QUADS);

    glVertex2i(x_min, y_min);
    glVertex2i(x_max, y_min);
    glVertex2i(x_max, y_max);
    glVertex2i(x_min, y_max);

    glEnd();

    reset_state();
}


float get_cos_angle(int i, int j, int k) {
    float v0x = x_array[i] - x_array[j];
    float v0y = y_array[i] - y_array[j];
    float v1x = x_array[k] - x_array[j];
    float v1y = y_array[k] - y_array[j];

    float v0il = 1.0f / sqrt(v0x * v0x + v0y * v0y);
    v0x *= v0il;
    v0y *= v0il;

    float v1il = 1.0f / sqrt(v1x * v1x + v1y * v1y);
    v1x *= v1il;
    v1y *= v1il;

    return v0x * v1x + v0y * v1y; // dot(v0, v1)
}


void draw_polygon_stencil_ear_clip() {
    if (vertex_count < 3) return;

    // Init links
    int * next = new int[vertex_count];
    int * prev = new int[vertex_count];

    for (int i = 0, j = vertex_count-1, k = vertex_count-2; i < vertex_count; k=j, j=i, i++) {
        next[j] = i;
        prev[j] = k;
    }

    // This brute-froce implementation is o(n^2) and we re-evaluate them every time.

    // We should put the angles in a heap. When a vertex is removed, then update the neighboring angles.

    setup_stencil_pass();

    glColor3f(0.1, 0, 0);

    glBegin(GL_TRIANGLES);

    int first = 0;

    for (int i = 0; i < vertex_count-2; i++) {
        
        // Find best ear.
        int best_j = -1;
        float best_c = -1;
        for (int j = first; next[j] != first; j = next[j]) {
            float c = get_cos_angle(prev[j], j, next[j]);
            if (c > best_c) {
                best_c = c;
                best_j = j;
            }
        }

        // Render triangle.
        int best_i = prev[best_j];
        int best_k = next[best_j];

        glVertex2i(x_array[best_i], y_array[best_i]);
        glVertex2i(x_array[best_j], y_array[best_j]);
        glVertex2i(x_array[best_k], y_array[best_k]);

        // Remove vertex.
        if (first == best_j) first = best_k;
        next[best_i] = best_k;
        prev[best_k] = best_i;
    }

    glEnd();

    // Compute bounds.
    extern int window_width;    // From below.
    extern int window_height;

    int x_min = window_width;
    int y_min = window_height;
    int x_max = 0;
    int y_max = 0;

    for (int i = 0; i < vertex_count; i++) {
        if (x_array[i] < x_min) x_min = x_array[i];
        if (y_array[i] < y_min) y_min = y_array[i];
        if (x_array[i] > x_max) x_max = x_array[i];
        if (y_array[i] > y_max) y_max = y_array[i];
    }

    setup_shading_pass();

    glColor3f(0, 0, 0.5);

    glBegin(GL_QUADS);

    glVertex2i(x_min, y_min);
    glVertex2i(x_max, y_min);
    glVertex2i(x_max, y_max);
    glVertex2i(x_min, y_max);

    glEnd();

    reset_state();

    delete next;
    delete prev;
}

void draw_polygon_overdraw() {
    if (vertex_count == 0) return;

    glBegin(GL_LINES);
    for (int i = 0, j = vertex_count-1; i < vertex_count; j=i, i++) {
        glVertex2i(x_array[j], y_array[j]);
        glVertex2i(x_array[i], y_array[i]);
    }
    glEnd();
}


// Text rendering

GLint glyphs_display_list = 0;

void glutInitGlyphs()
{
    glyphs_display_list = glGenLists(256);

    for (int i = 0; i < 256; i++) {
        glNewList(glyphs_display_list + i, GL_COMPILE);
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, i);
        glEndList();
    }
}

void glutDrawString(int x, int y, const char * text)
{
    glListBase(glyphs_display_list);
    glRasterPos2i(x+1, y + 4);
    glCallLists((GLsizei)strlen(text), GL_UNSIGNED_BYTE, text);
}


// GLUT stuff

int window_width = 512;
int window_height = 512;

void glutKeyboardCallback(unsigned char key, int x, int y)
{
    int modifiers = glutGetModifiers();

    if (modifiers & GLUT_ACTIVE_SHIFT) {

    }
    if (modifiers & GLUT_ACTIVE_CTRL) {

    }
    if (modifiers & GLUT_ACTIVE_ALT) {

    }

    if (key == '`') {
        render_mode = 0;
    }
    if (key > '0' && key < '0' + render_mode_count) {
        render_mode = key - '0';
    }
    if (key == '-') {
        render_mode--;
        if (render_mode == -1) render_mode = render_mode_count-1;
    }
    if (key == '+' || key == '=') {
        render_mode++;
        if (render_mode == render_mode_count) render_mode = 0;
    }

    if (key == ' ') {
        polygon_reset();
    }

    if (key == 27) {
        polygon_done();
        polygon_save();
    }

    glutPostRedisplay();
}

void glutKeyboardUpCallback(unsigned char key, int x, int y)
{
}

void glutMouseCallback(int button, int state, int x, int y)
{
    if (state == GLUT_UP) {
        if (button == GLUT_LEFT_BUTTON) {
                polygon_add_vertex(x, y);
        }
        else if (button == GLUT_RIGHT_BUTTON) {
            polygon_undo_add();
        }
        else if (button == GLUT_MIDDLE_BUTTON) {
        }
    }

    glutPostRedisplay();
}

void glutMotionCallback(int x, int y) 
{
    polygon_set_vertex(x, y);

    glutPostRedisplay();
}

void glutDisplayCallback(void)
{
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    if (render_mode == 1) {
        draw_polygon_glu();
    }
    else if (render_mode == 2) {
        draw_polygon_stencil();
    }
    else if (render_mode == 3) {
        draw_polygon_stencil_ear_clip();
    }

    draw_polygon_boundary();

    glutDrawString(16, 16, render_mode_name[render_mode]);
    
    glutSwapBuffers();
}

void glutReshapeCallback(int width, int height)
{   
    window_width = width;
    window_height = height;

    glViewport(0, 0, window_width, window_height);
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity(); 
    glOrtho(0, window_width, window_height, 0, /*near=*/0, /*far=*/1);
    
    glutPostRedisplay();
}

/*void glutIdleCallback(void)
{
    glutPostRedisplay();
}*/

/*void glutTimerFunc(int value)
{
    glutPostRedisplay();
}*/

int main(int argc, char *argv[])
{
    const char * file_name = "polygon.txt";
    if (argc == 2) {
        file_name = argv[1];
    }
    polygon_open(file_name);

    glutInit(&argc, argv);

    glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_STENCIL );
    glutInitWindowSize( window_width, window_height );
    glutCreateWindow( argv[0] );
    glutKeyboardFunc( glutKeyboardCallback );
    glutKeyboardUpFunc( glutKeyboardUpCallback );
    glutMouseFunc( glutMouseCallback );
    glutMotionFunc(glutMotionCallback);
    glutPassiveMotionFunc(glutMotionCallback);
    glutDisplayFunc( glutDisplayCallback );
    glutReshapeFunc( glutReshapeCallback );
    //glutIdleFunc( glutIdleCallback );
    //glutTimerFunc( 100, glutTimerFunc, 0 );

    glutInitGlyphs();

    glutPostRedisplay();

    glutMainLoop();

    printf("Yay!");

    return 0;
}

    