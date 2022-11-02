//
//  main.cpp
//  DigitalFilters
//
//  Created by Jignesh Panchal on 11/01/22.
//
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <math.h>
#include <string.h>

#ifdef __APPLE__
// Defined before OpenGL and GLUT includes to avoid deprecation messages
#define GL_SILENCE_DEPRECATION
#define GLFW_INCLUDE_NONE
#define GLEW_STATIC

//#include <glad/glad.h>
//GLEW
#include <GL/glew.h>
//GLFW
#include <GLFW/glfw3.h>
//GLUT
#include <GL/glut.h>
#include <GL/glx.h>
#endif

#define SCALE1 100
#define SCALE 250
#define N 513
#define L 1
#define NO 256
#define LE 43

typedef int BOOL;
#define TRUE 1
#define FALSE 0

enum {
    MENU_RECTANGLE = 1, // 1
    MENU_BARTLETT,      // 2
    MENU_HANNING,       // 3
    MENU_HAMMING,       // 4
    MENU_BLACKMAN,      // 5
    MENU_FILTER,        // f
    MENU_LOWPASS,       // l
    MENU_HIGHPASS,      // h
    MENU_BANDPASS,      // b
    MENU_BANDSTOP,      // p
    MENU_INPUT,         // i
    MENU_SINE,          // s
    MENU_SQUARE,        // q
    MENU_NOISE,         // n
    MENU_NOISYSINE,     // ns
    MENU_NOISYSQUARE,   // nq
    MENU_OUTPUT,        // o
    MENU_EXIT,          // Esc
    MENU_EMPTY
};

static BOOL g_bButton1Down = FALSE;
static int g_yClick = 0;

static int g_Width = 1000;                          // Initial window width
static int g_Height = 800;

int x_offset = ((g_Width/2)/10);
int y_offset = ((g_Height/4)/10);

int x1_start_4F = -((g_Width/2) - x_offset);
int y1_start_4F = (g_Height/4);

int x2_start_4F = x_offset;
int y2_start_4F = (g_Height/4) - y_offset;

int x3_start_4F = -((g_Width/2) - x_offset);
int y3_start_4F = -(g_Height/4);

int x4_start_4F = x_offset;
int y4_start_4F = -(g_Height/4);

int n = 512; // number of samples - should be less than N-1 (513-1=512)
int no = 200; // filter length
int m,si;

float a[N], b[N];
float w[N];
float h[N];
float ha[N], HA_amp[N], HA_phase[N];
float x[N], X_amp[N], X_phase[N];
float Y_amp[N], Y_phase[N];

float fs = 4000; //Hz
float fc1 = 0; //Hz
float delta_fc1 = 10;
float fc2 = 100; //Hz
float min_fc = fc1; //Hz
float max_fc = (fs/2); //Hz
float t = (1/fs);

float fin = 100; //Hz

char filter_type = 'l';
char window_type = '1';
char signal_type = 'q';

void display_trial(void)
{
    //Clear information from last draw
    glClear(GL_COLOR_BUFFER_BIT| GL_DEPTH_BUFFER_BIT);
    
    //Switch to the drawing perspective
    glMatrixMode(GL_MODELVIEW);
    
    //Start with identity matrix
    glLoadIdentity();

    //draw
    glBegin(GL_LINE_STRIP);
    //glBegin(GL_LINE);
    //glBegin(GL_LINES);
    //glBegin(GL_POLYGON);
        glVertex2f(2.0, 4.0);
        glVertex2f(8.0, 4.0);
        glVertex2f(8.0, 6.0);
        glVertex2f(2.0, 6.0);
    glEnd();
    
    glFlush();
}

const char* GetSignalType(char ch)
{
    switch (ch)
    {
        case 's':
            return "SINE";
            break;
        case 'q':
            return "SQUARE";
            break;
        case 'n':
            return "NOISE";
            break;
    }
    return "";
}

const char* GetWindowType(char ch)
{
    switch (ch)
    {
        case '1':
            return "RECTANGLE";
            break;
        case '2':
            return "BARTLETT";
            break;
        case '3':
            return "HANNING";
            break;
        case '4':
            return "HAMMING";
            break;
        case '5':
            return "BLACKMAN";
            break;
    }
    return "";
}

const char* GetFilterType(char ch)
{
    switch (ch)
    {
        case 'l':
            return "LOW PASS";
            break;
        case 'h':
            return "HIGH PASS";
            break;
        case 'b':
            return "BAND PASS";
            break;
        case 'p':
            return "BAND STOP";
            break;
    }
    
    return "";
}

//Rectangular Window = 'r'
void GenerateWindow(char ch)
{
    int i;
    
    for(i=0;i<=N;i++)
    {
        w[i]=0;
    }
    
    switch(ch)
    {
        case '1':
            for(i=1;i<=no;i++)
            {
                w[i]=1;
            }
            //window_type[] = "Rectangle";
            break;
            
        case '4':
            for(i=1;i<=no;i++)
            {
                w[i]=0.54 - 0.46*cos((float)(2*M_PI*i/(no-1)));
            }
            //window_type[] = "Hamming";
            break;
            
        case '3':
            for(i=1;i<=no;i++)
            {
                w[i]=0.5 - 0.5*cos((float)(2*M_PI*i/(no-1)));
            }
            //window_type[] = "Hanning";
            break;
            
        case '5':
            for(i=1;i<=no;i++)
            {
                w[i]=0.42 - 0.5*cos((float)(2*M_PI*i/(no-1)))+0.08*cos((float)(4*M_PI*i/(no-1)));
            }
            //window_type[] = "Blackman";
            break;
            
        case '2':
            for(i=1;i<no/2;i++)
            {
                w[i]=(float)2.00*i/(no-1);
            }
            for(i=no/2;i<=no;i++)
            {
                w[i]=2.00-(float)2.00*i/(no-1);
            }
            //window_type[] = "Bartlett";
            break;
    }
    
}

void GenerateFilterCoeff(char ch)
{
    int i, j;
    
    //---------------------------------------------------
    // Filter Cutoff Frequencies fc1 and fc2 range check
    //---------------------------------------------------
    if (fc1 < 0)
    {
        fc1 = 0;
    }
    if (fc2 < 0)
    {
        fc2 = 0;
        printf("Error: Filter Design");
    }
    if( fc2 > max_fc )
    {
        fc2 = max_fc;
    }
    if( fc1 > max_fc )
    {
        fc1 = max_fc;
        printf("Error: Filter Design");
    }
    if (fc1 >= fc2)
    {
        printf("Error: Filter Design");
    }
    //-------------------------
    
    for(i=0;i<=N;i++)
    {
        h[i]=0;
    }

    if (ch == 'p') //Generate Band Stop Filter Coefficients
    {
        //filter_type[] = "Band Stop";
        for(j=-(no-1)/2,i=1;i<=no;i++,j++)
        {
           h[i]=0;
           if(j!=0)
           {
               h[i]= (float)(1/(j*M_PI))*(sin((float)(2*M_PI*fc1*j*t))) + (float)(1/(j*M_PI))*(sin((float)(M_PI*fs*j*t)) - sin((float)(2*M_PI*fc2*j*t)));
           }
           else
           {
               h[i]=(float)((2/fs)*fc1) + (float)((2/fs)*((fs/2)-fc2));
           }
        }
    }
    else //Generate Low Pass, High Pass and Band Pass Filter Coefficients
    {
        switch (ch)
        {
            case 'l':
                fc1=0;
                //filter_type[] = "Low Pass";
                break;
            case 'h':
                fc2=max_fc;
                //filter_type[] = "High Pass";
                break;
            case 'b':
                //filter_type[] = "Band Pass";
                break;
        }
        
        for(j=-(no-1)/2,i=1;i<=no;i++,j++)
        {
           h[i]=0;
           if(j!=0)
           {
               h[i] = (float) (1/(j * M_PI)) * (sin((float)(2 * M_PI * fc2 * j * t)) - sin((float)(2 * M_PI * fc1 * j * t)));
           }
           else
           {
               h[i] = (float) ((2/fs)*(fc2-fc1));
           }
       }
    }
}

void UpdateFilterCoeff()
{
    int i;
    
    for(i=0;i<=N;i++)
    {
        ha[i]=0;
    }
    
    for(i=1;i<=no;i++)
    {
        ha[i]=h[i]*w[i];
    }
}

//Square = 'q'
void GenerateSignal(char ch)
{
    int i,j;
    
    for(i=0;i<=N;i++)
    {
        x[i]=0;
    }
    
    if(ch=='s')
    {
        if (fin < 0)
        {
            fin = 0;
        }
        if( fin > max_fc )
        {
            fin = max_fc;
        }
        for(i=1;i<=n;i++)
        {
            x[i]=sin(2*M_PI*(float)(fin*i/fs));
        }
    }
    else if (ch == 'n')
    {
        if (fin < 0)
        {
            fin = 0;
        }
        if( fin > max_fc )
        {
            fin = max_fc;
        }
        j=0;
        for(i=1;i<=n;i++)
        {
            x[i]=(float)(0.1*(rand()%10))+sin(2*M_PI*(float)(fin*i/fs));
            if( (rand()%10) >= 5)
            {
                j=-1;
            }
            else
            {
                j=1;
            }
            x[i]=(x[i]*(float)j);
        }
    }
    else if (ch == 'q')
    {
        for(i=1;i<n/4;i++)
        {
            x[i]=0;
        }
        for(i=n/4;i<=n/2;i++)
        {
            x[i]=1;
        }
        for(i=n/2;i<3*n/4;i++)
        {
            x[i]=-1;
        }
        for(i=3*n/4;i<=n;i++)
        {
            x[i]=0;
        }
    }
    
    
}

void bitrev(void)
{
    int n2,n1,i,j,k;
    float ta,tb;

    n2=(int)(n/2);
    
    n1=n-1;
    j=1;
    
    for(i=1;i<=n1;i++)
    {
        if(i<j)
        {
            ta=a[j];
            a[j]=a[i];
            a[i]=ta;
            tb=b[j];
            b[j]=b[i];
            b[i]=tb;
        }
        
        k=n2;
           
        while(k<j)
        {
            j-=k;
            k=(int)(k/2);
        }

        j+=k;
    }
}


void fft(void)
{
    int l,le,l2,g,h,ip;
    float wa,wb,va,vb,ua[N],ub[N];

    for(l=1;l<=m;l++)
    {
        le=(int)pow(2,(float)l);
        l2=(int)(le/2);
        ua[1]=1.0;
        ub[1]=0.0;
        wa=cos(M_PI/(float)l2);
        wb=(float)si*sin(M_PI/(float)l2);
        
        for(g=1;g<=l2;g++)
        {
            for(h=g;h<=n;h+=le)
            {
                ip = h + l2;
                va=(a[ip]*ua[g]) - (b[ip]*ub[g]);
                vb=(b[ip]*ua[g]) + (a[ip]*ub[g]);
                a[ip]=a[h]-va;
                b[ip]=b[h]-vb;
                a[h]=a[h]+va;
                b[h]=b[h]+vb;
            }
            ua[g+1]=(ua[g]*wa) - (ub[g]*wb);
            ub[g+1]=(ub[g]*wa) + (ua[g]*wb);
        }

    }
}

void GenerateFilterOutput(void)
{
    int i;
    float ax[N], bx[N];

    for(i=0;i<N;i++)
    {
        a[i]=0;
        b[i]=0;
        ax[i]=0;
        bx[i]=0;
    }

    m=0;
    i=1;
    while(n>i)
    {
        m++;
        i=pow(2,(float)m);
    }

    
    //--------------------------
    // FFT of h(n) in ax and bx
    //--------------------------
    for(i=1;i<=n;i++)
    {
        a[i]=ha[i];
        b[i]=0;
    }
    bitrev();
    si=-1;
    fft();
    for(i=1;i<=n;i++)
    {
        ax[i]=a[i];
        bx[i]=b[i];
        HA_amp[i]=0;
        HA_amp[i]=(sqrt(pow(ax[i],2) + pow(bx[i],2)));
        HA_phase[i]=0;
        HA_phase[i]=-atan(bx[i]/ax[i]);
    }
    //--------------------
    
    
    //------------------------
    // FFT of x(n) in a and b
    //------------------------
    for(i=1;i<=n;i++)
    {
        a[i]=0;
        a[i]=x[i];
        b[i]=0;
    }
    bitrev();
    si=-1;
    fft();
    for(i=1;i<=n;i++)
    {
        X_amp[i]=0;
        X_amp[i]=(sqrt(pow(a[i],2) + pow(b[i],2)));
        X_phase[i]=0;
        X_phase[i]=-atan(b[i]/a[i]);
    }
    //--------------------
    
    
    //--------------------
    // Y = X * H
    //--------------------
    for(i=1;i<=n;i++)
    {
        b[i] = (a[i]*bx[i]) + (ax[i]*b[i]);
        a[i] = (a[i]*ax[i]) - (b[i]*bx[i]);
        b[i] = b[i]/n;
        a[i] = a[i]/n;
        
        Y_amp[i]=0;
        Y_amp[i]=(sqrt(pow(a[i],2) + pow(b[i],2)));
        Y_phase[i]=0;
        Y_phase[i]=-atan(b[i]/a[i]);
    }
    //---------------------
    
    //--------------------
    // Y(n) to y(t)
    //--------------------
    bitrev();
    si=1;
    fft();
    //display2();
    //--------------------
}

/**
* Draw a character string.
*
* @param x        The x position
* @param y        The y position
* @param string   The character string
*/
void drawString(int x, int y, char *string)
{
    glRasterPos2f(x, y);

    for (char* c = string; *c != '\0'; c++)
    {
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_10, *c);  // Updates the position
    }
}

void displayOutput(void)
{
    float ma,mha,mw,mb,mx,mha_amp,mha_phase,mx_amp,my_amp;
    int c,i;
    
    int max_points_4F = fmin(n, (g_Width/2)-(2*x_offset));
    
    //Clear information from last draw
    glClear(GL_COLOR_BUFFER_BIT| GL_DEPTH_BUFFER_BIT);
    
    //Switch to the drawing perspective
    glMatrixMode(GL_MODELVIEW);
    
    //Start with identity matrix
    glLoadIdentity();

    ma=0;
    mw=0;
    mha=0;
    mb=0;
    mx=0;
    mha_amp=0;
    mha_phase=0;
    mx_amp=0;
    my_amp=0;

    // Find Maximum -ve or +ve value of each array
    for(c=1;c<=no;c++)
    {
        if(fabs(a[c])>ma) ma=fabs(a[c]);
        if(fabs(b[c])>mb) mb=fabs(b[c]);
        if(fabs(w[c])>mw) mw=fabs(w[c]);
        if(fabs(ha[c])>mha) mha=fabs(ha[c]);
        if(fabs(x[c])>mx) mx=fabs(x[c]);
        if(fabs(HA_amp[c])>mha_amp) mha_amp=fabs(HA_amp[c]);
        if(fabs(HA_phase[c])>mha_phase) mha_phase=fabs(HA_phase[c]);
        if(fabs(X_amp[c])>mx_amp) mx_amp=fabs(X_amp[c]);
        if(fabs(Y_amp[c])>my_amp) my_amp=fabs(Y_amp[c]);
    }

    for(c=1;c<=no;c++)
    {
        a[c]=(a[c]/ma)*((g_Height/4)-(y_offset*2));
        b[c]=(b[c]/mb)*((g_Height/4)-(y_offset*2));
        w[c]=(w[c]/mw)*((g_Height/4)-(y_offset*2));
        ha[c]=(ha[c]/mha)*((g_Height/4)-(y_offset*2));
        x[c]=(x[c]/mx)*((g_Height/4)-(y_offset*2));
        HA_amp[c]=(HA_amp[c]/mha_amp)*((g_Height/4)-(y_offset*2));
        HA_phase[c]=(HA_phase[c]/mha_phase)*((g_Height/4)-(y_offset*2));
        X_amp[c]=(X_amp[c]/mx_amp)*((g_Height/4)-(y_offset*2));
        Y_amp[c]=(Y_amp[c]/my_amp)*((g_Height/4)-(y_offset*2));
    }
    
    //------------------
    //Figure 1
    //------------------
    glColor3f(1.0,0.0,0.0);
    glLineWidth(2);
    glRasterPos2f(x1_start_4F,y1_start_4F);
    glBegin(GL_LINE_STRIP);
        glVertex2d(x1_start_4F,y1_start_4F);
        for(i=0,c=1;c<=max_points_4F;i=i+1,c+=L)
        {
            glVertex2d(x1_start_4F+i, y1_start_4F+x[c]);
        }
    glEnd();
    char signal_string[100];
    sprintf(signal_string, "%s Input Signal (Time Domain)\n", GetSignalType(signal_type));
    drawString(x1_start_4F,(g_Height/2)-y_offset,signal_string);
    //------------------
    
    
    //------------------
    //Figure 2
    //------------------
    glColor3f(0.0,1.0,0.0);
    glRasterPos2f(x2_start_4F,y2_start_4F);
    glBegin(GL_LINE_STRIP);
    glVertex2d(x2_start_4F,y2_start_4F);
        for(i=0,c=1;c<=max_points_4F;i=i+1,c+=L)
        {
            glVertex2d(x2_start_4F+i, y2_start_4F+X_amp[c]);
        }
    glEnd();
    //char signal_string[100];
    sprintf(signal_string, "%s Input Signal (Frequency Domain)\n", GetSignalType(signal_type));
    drawString(x2_start_4F,(g_Height/2)-y_offset,signal_string);
    //------------------
    
    //------------------
    //Figure 3
    //------------------
    glColor3f(1.0,1.0,0.0);
    glRasterPos2f(x3_start_4F,y3_start_4F);
    glBegin(GL_LINE_STRIP);
        glVertex2d(x3_start_4F,y3_start_4F);
        for(i=0,c=1;c<=max_points_4F;i=i+1,c+=L)
        {
            glVertex2d(x3_start_4F+i, y3_start_4F+a[c]);
        }
    glEnd();
    drawString(x3_start_4F,-y_offset,"Output (Time Domain)");
    //------------------
    
    //------------------
    //Figure 4
    //------------------
    glColor3f(1.0,0.0,1.0);
    glRasterPos2f(x4_start_4F,y4_start_4F);
    glBegin(GL_LINE_STRIP);
        glVertex2d(x4_start_4F,y4_start_4F);
        for(i=0,c=1;c<=max_points_4F;i=i+1,c+=L)
        {
            glVertex2d(x4_start_4F+i, y4_start_4F+Y_amp[c]);
        }
    glEnd();
    drawString(x4_start_4F,-y_offset,"Output (Frequency Domain)");
    //------------------
    
    //outtextxy(10,35," Input to Filter ");
    //outtextxy(10,10+ym/2," Filter Output ");
    
    glFlush();
}

void displayInput(void)
{
    float ma,mha,mw,mb,mx,mha_amp,mha_phase,mx_amp,my_amp;
    int c,i;
    
    int max_points_4F = fmin(n, (g_Width/2)-(2*x_offset));
    
    //Clear information from last draw
    glClear(GL_COLOR_BUFFER_BIT| GL_DEPTH_BUFFER_BIT);
    
    //Switch to the drawing perspective
    glMatrixMode(GL_MODELVIEW);
    
    //Start with identity matrix
    glLoadIdentity();

    ma=0;
    mw=0;
    mha=0;
    mb=0;
    mx=0;
    mha_amp=0;
    mha_phase=0;
    mx_amp=0;
    my_amp=0;

    // Find Maximum -ve or +ve value of each array
    for(c=1;c<=no;c++)
    {
        if(fabs(a[c])>ma) ma=fabs(a[c]);
        if(fabs(b[c])>mb) mb=fabs(b[c]);
        if(fabs(w[c])>mw) mw=fabs(w[c]);
        if(fabs(ha[c])>mha) mha=fabs(ha[c]);
        if(fabs(x[c])>mx) mx=fabs(x[c]);
        if(fabs(HA_amp[c])>mha_amp) mha_amp=fabs(HA_amp[c]);
        if(fabs(HA_phase[c])>mha_phase) mha_phase=fabs(HA_phase[c]);
        if(fabs(X_amp[c])>mx_amp) mx_amp=fabs(X_amp[c]);
        if(fabs(Y_amp[c])>my_amp) my_amp=fabs(Y_amp[c]);
    }

    for(c=1;c<=no;c++)
    {
        a[c]=(a[c]/ma)*((g_Height/4)-(y_offset*2));
        b[c]=(b[c]/mb)*((g_Height/4)-(y_offset*2));
        w[c]=(w[c]/mw)*((g_Height/4)-(y_offset*2));
        ha[c]=(ha[c]/mha)*((g_Height/4)-(y_offset*2));
        x[c]=(x[c]/mx)*((g_Height/4)-(y_offset*2));
        HA_amp[c]=(HA_amp[c]/mha_amp)*((g_Height/4)-(y_offset*2));
        HA_phase[c]=(HA_phase[c]/mha_phase)*((g_Height/4)-(y_offset*2));
        X_amp[c]=(X_amp[c]/mx_amp)*((g_Height/4)-(y_offset*2));
        Y_amp[c]=(Y_amp[c]/my_amp)*((g_Height/4)-(y_offset*2));
    }
    
    //------------------
    //Figure 1
    //------------------
    glColor3f(1.0,0.0,0.0);
    glLineWidth(2);
    glRasterPos2f(x1_start_4F,y1_start_4F);
    glBegin(GL_LINE_STRIP);
        glVertex2d(x1_start_4F,y1_start_4F);
        for(i=0,c=1;c<=max_points_4F;i=i+1,c+=L)
        {
            glVertex2d(x1_start_4F+i, y1_start_4F+x[c]);
        }
    glEnd();
    char signal_string[100];
    sprintf(signal_string, "%s Input Signal (Time Domain)\n", GetSignalType(signal_type));
    drawString(x1_start_4F,(g_Height/2)-y_offset,signal_string);
    //------------------
    
    
    //------------------
    //Figure 2
    //------------------
    glColor3f(0.0,1.0,0.0);
    glRasterPos2f(x2_start_4F,y2_start_4F);
    glBegin(GL_LINE_STRIP);
    glVertex2d(x2_start_4F,y2_start_4F);
        for(i=0,c=1;c<=max_points_4F;i=i+1,c+=L)
        {
            glVertex2d(x2_start_4F+i, y2_start_4F+X_amp[c]);
        }
    glEnd();
    //char signal_string[100];
    sprintf(signal_string, "%s Input Signal (Frequency Domain)\n", GetSignalType(signal_type));
    drawString(x2_start_4F,(g_Height/2)-y_offset,signal_string);
    //------------------
    
    //------------------
    //Figure 3
    //------------------
    glColor3f(1.0,1.0,0.0);
    glRasterPos2f(x3_start_4F,y3_start_4F);
    glBegin(GL_LINE_STRIP);
        glVertex2d(x3_start_4F,y3_start_4F);
        for(i=0,c=1;c<=max_points_4F;i=i+1,c+=L)
        {
            glVertex2d(x3_start_4F+i, y3_start_4F+a[c]);
        }
    glEnd();
    drawString(x3_start_4F,-y_offset,"Output (Time Domain)");
    //------------------
    
    //------------------
    //Figure 4
    //------------------
    glColor3f(1.0,0.0,1.0);
    glRasterPos2f(x4_start_4F,y4_start_4F);
    glBegin(GL_LINE_STRIP);
        glVertex2d(x4_start_4F,y4_start_4F);
        for(i=0,c=1;c<=max_points_4F;i=i+1,c+=L)
        {
            glVertex2d(x4_start_4F+i, y4_start_4F+Y_amp[c]);
        }
    glEnd();
    drawString(x4_start_4F,-y_offset,"Output (Frequency Domain)");
    //------------------
    
    //outtextxy(10,35," Input to Filter ");
    //outtextxy(10,10+ym/2," Filter Output ");
    
    glFlush();
}

void displayFilter(void)
{
    float ma,mha,mw,mb,mx,mha_amp,mha_phase,mx_amp,my_amp;
    int c,i;
    
    int max_points_4F = fmin(n, (g_Width/2)-(2*x_offset));
    
    //Clear information from last draw
    glClear(GL_COLOR_BUFFER_BIT| GL_DEPTH_BUFFER_BIT);
    
    //Switch to the drawing perspective
    glMatrixMode(GL_MODELVIEW);
    
    //Start with identity matrix
    glLoadIdentity();

    ma=0;
    mw=0;
    mha=0;
    mb=0;
    mx=0;
    mha_amp=0;
    mha_phase=0;
    mx_amp=0;
    my_amp=0;

    // Find Maximum -ve or +ve value of each array
    for(c=1;c<=no;c++)
    {
        if(fabs(a[c])>ma) ma=fabs(a[c]);
        if(fabs(b[c])>mb) mb=fabs(b[c]);
        if(fabs(w[c])>mw) mw=fabs(w[c]);
        if(fabs(ha[c])>mha) mha=fabs(ha[c]);
        if(fabs(x[c])>mx) mx=fabs(x[c]);
        if(fabs(HA_amp[c])>mha_amp) mha_amp=fabs(HA_amp[c]);
        if(fabs(HA_phase[c])>mha_phase) mha_phase=fabs(HA_phase[c]);
        if(fabs(X_amp[c])>mx_amp) mx_amp=fabs(X_amp[c]);
        if(fabs(Y_amp[c])>my_amp) my_amp=fabs(Y_amp[c]);
    }

    for(c=1;c<=no;c++)
    {
        a[c]=(a[c]/ma)*((g_Height/4)-(y_offset*2));
        b[c]=(b[c]/mb)*((g_Height/4)-(y_offset*2));
        w[c]=(w[c]/mw)*((g_Height/4)-(y_offset*2));
        ha[c]=(ha[c]/mha)*((g_Height/4)-(y_offset*2));
        x[c]=(x[c]/mx)*((g_Height/4)-(y_offset*2));
        HA_amp[c]=(HA_amp[c]/mha_amp)*((g_Height/4)-(y_offset*2));
        HA_phase[c]=(HA_phase[c]/mha_phase)*((g_Height/4)-(y_offset*2));
        X_amp[c]=(X_amp[c]/mx_amp)*((g_Height/4)-(y_offset*2));
        Y_amp[c]=(Y_amp[c]/my_amp)*((g_Height/4)-(y_offset*2));
    }
    
    //------------------
    //Figure 1
    //------------------
    glColor3f(1.0,0.0,0.0);
    glLineWidth(2);
    glRasterPos2f(x1_start_4F,y1_start_4F);
    glBegin(GL_LINE_STRIP);
        glVertex2d(x1_start_4F,y1_start_4F);
        for(i=0,c=1;c<=max_points_4F;i=i+1,c+=L)
        {
            glVertex2d(x1_start_4F+i, y1_start_4F+ha[c]);
        }
    glEnd();
    
    char filter_string[100];
    sprintf(filter_string, "%s Filter Impluse Response [%.2fHz,%.2fHz] (Time Domain)\n", GetFilterType(filter_type), fc1, fc2);
    drawString(x1_start_4F,(g_Height/2)-y_offset,filter_string);
    //------------------
    
    //------------------
    //Figure 2
    //------------------
    glColor3f(0.0,1.0,0.0);
    glRasterPos2f(x2_start_4F,y2_start_4F);
    glBegin(GL_LINE_STRIP);
    glVertex2d(x2_start_4F,y2_start_4F);
        for(i=0,c=1;c<=max_points_4F;i=i+1,c+=L)
        {
            glVertex2d(x2_start_4F+i, y2_start_4F+w[c]);
        }
    glEnd();
    char window_string[100];
    sprintf(window_string, "%s Window (Time Domain)\n", GetWindowType(window_type));
    drawString(x2_start_4F,(g_Height/2)-y_offset,window_string);
    //------------------
    
    //drawString(x1_start_4F,(g_Height/2)-y_offset,"Filter Impluse Response (Time Domain)");
    
    //------------------
    //Figure 3
    //------------------
    glColor3f(1.0,1.0,0.0);
    glRasterPos2f(x3_start_4F,y3_start_4F);
    glBegin(GL_LINE_STRIP);
        glVertex2d(x3_start_4F,y3_start_4F);
        for(i=0,c=1;c<=max_points_4F;i=i+1,c+=L)
        {
            glVertex2d(x3_start_4F+i, y3_start_4F+HA_amp[c]);
        }
    glEnd();
    drawString(x3_start_4F,-y_offset,"Magnitude vs. Frequency");
    //------------------
    
    //------------------
    //Figure 4
    //------------------
    glColor3f(1.0,0.0,1.0);
    glRasterPos2f(x4_start_4F,y4_start_4F);
    glBegin(GL_LINE_STRIP);
        glVertex2d(x4_start_4F,y4_start_4F);
        for(i=0,c=1;c<=max_points_4F;i=i+1,c+=L)
        {
            glVertex2d(x4_start_4F+i, y4_start_4F+HA_phase[c]);
        }
    glEnd();
    drawString(x4_start_4F,-y_offset,"Phase vs. Frequency");
    //------------------
    
    //outtextxy(10,35," Input to Filter ");
    //outtextxy(10,10+ym/2," Filter Output ");
    
    glFlush();
}



void reshape(GLint width, GLint height)
{
    //Viewport: area within drawing are displayed
    glViewport(0, 0, (GLsizei)width, (GLsizei)height);
    
    //Setup viewing projection
    glMatrixMode(GL_PROJECTION);
    
    //Start with identity matrix
    glLoadIdentity();
    
    gluOrtho2D(-g_Width/2,g_Width/2,-g_Height/2,g_Height/2);
    
    //Switch to the drawing perspective
    glMatrixMode(GL_MODELVIEW);
}


void SelectFromMenu(int idCommand)
{
    switch (idCommand)
    {
            // r b h m l
        case MENU_RECTANGLE:
            window_type = '1';
            GenerateWindow(window_type);
            UpdateFilterCoeff();
            GenerateFilterOutput();
            glutDisplayFunc(displayFilter);
            break;
        case MENU_BARTLETT:
            window_type = '2';
            GenerateWindow(window_type);
            UpdateFilterCoeff();
            GenerateFilterOutput();
            glutDisplayFunc(displayFilter);
            break;
        case MENU_HANNING:
            window_type = '3';
            GenerateWindow(window_type);
            UpdateFilterCoeff();
            GenerateFilterOutput();
            glutDisplayFunc(displayFilter);
            break;
        case MENU_HAMMING:
            window_type = '4';
            GenerateWindow(window_type);
            UpdateFilterCoeff();
            GenerateFilterOutput();
            glutDisplayFunc(displayFilter);
            break;
        case MENU_BLACKMAN:
            window_type = '5';
            GenerateWindow(window_type);
            UpdateFilterCoeff();
            GenerateFilterOutput();
            glutDisplayFunc(displayFilter);
            break;
        case MENU_FILTER:
            glutDisplayFunc(displayFilter);
            break;
        case MENU_LOWPASS:
            filter_type = 'l';
            GenerateFilterCoeff(filter_type);
            UpdateFilterCoeff();
            GenerateFilterOutput();
            glutDisplayFunc(displayFilter);
            break;
        case MENU_HIGHPASS:
            filter_type = 'h';
            GenerateFilterCoeff(filter_type);
            UpdateFilterCoeff();
            GenerateFilterOutput();
            glutDisplayFunc(displayFilter);
            break;
        case MENU_BANDPASS:
            filter_type = 'b';
            GenerateFilterCoeff(filter_type);
            UpdateFilterCoeff();
            GenerateFilterOutput();
            glutDisplayFunc(displayFilter);
            break;
        case MENU_BANDSTOP:
            filter_type = 'p';
            GenerateFilterCoeff(filter_type);
            UpdateFilterCoeff();
            GenerateFilterOutput();
            glutDisplayFunc(displayFilter);
            break;
        case MENU_INPUT:
            glutDisplayFunc(displayInput);
            break;
        case MENU_SINE:
            signal_type = 's';
            GenerateSignal(signal_type);
            GenerateFilterOutput();
            glutDisplayFunc(displayInput);
            break;
        case MENU_SQUARE:
            signal_type = 'q';
            GenerateSignal(signal_type);
            GenerateFilterOutput();
            glutDisplayFunc(displayInput);
            break;
        case MENU_NOISE:
            signal_type = 'n';
            GenerateSignal(signal_type);
            GenerateFilterOutput();
            glutDisplayFunc(displayInput);
            break;
        case MENU_OUTPUT:
            glutDisplayFunc(displayOutput);
            break;
        case MENU_EXIT:
            exit (0);
            break;
    }

    // Almost any menu selection requires a redraw
    glutPostRedisplay();
}

void Keyboard(unsigned char key, int x, int y)
{
    //printf("%c\n",key);
    
    switch (key)
    {
        case '+':
            fc1+=delta_fc1;
            if (fc1 < 0) fc1 = 0;
            if (fc1 > max_fc) fc1 = max_fc;
            if (fc1 > fc2) fc1 = fc2-1;
            GenerateFilterCoeff(filter_type);
            UpdateFilterCoeff();
            GenerateFilterOutput();
            glutDisplayFunc(displayFilter);
            glutPostRedisplay();
            //printf("fc1 = %f, fc2=%f\n",fc1,fc2);
            break;
        case '_':
            fc1-=delta_fc1;
            if (fc1 < 0) fc1 = 0;
            if (fc1 > max_fc) fc1 = max_fc;
            if (fc1 > fc2) fc1 = fc2-1;
            GenerateFilterCoeff(filter_type);
            UpdateFilterCoeff();
            GenerateFilterOutput();
            glutDisplayFunc(displayFilter);
            glutPostRedisplay();
            //printf("fc1 = %f, fc2=%f\n",fc1,fc2);
            break;
        case '1':
            SelectFromMenu(MENU_RECTANGLE);
            break;
        case '2':
            SelectFromMenu(MENU_BARTLETT);
            break;
        case '3':
            SelectFromMenu(MENU_HANNING);
            break;
        case '4':
            SelectFromMenu(MENU_HAMMING);
            break;
        case '5':
            SelectFromMenu(MENU_BLACKMAN);
            break;
        case 'f':
            SelectFromMenu(MENU_FILTER);
            break;
        case 'l':
            SelectFromMenu(MENU_LOWPASS);
            break;
        case 'h':
            SelectFromMenu(MENU_HIGHPASS);
            break;
        case 'b':
            SelectFromMenu(MENU_BANDPASS);
            break;
        case 'p':
            SelectFromMenu(MENU_BANDSTOP);
            break;
        case 'i':
            SelectFromMenu(MENU_INPUT);
            break;
        case 's':
            SelectFromMenu(MENU_SINE);
            break;
        case 'q':
            SelectFromMenu(MENU_SQUARE);
            break;
        case 'n':
            SelectFromMenu(MENU_NOISE);
            break;
        case 'o':
            SelectFromMenu(MENU_OUTPUT);
            break;
        case 27: // ESCAPE key
            exit(0);
            break;
      }
}

void MouseButton(int button, int state, int x, int y)
{
    // Respond to mouse button presses.
    // If button1 pressed, mark this state so we know in motion function.
    if (button == GLUT_LEFT_BUTTON)
    {
        g_bButton1Down = (state == GLUT_DOWN) ? TRUE : FALSE;
        g_yClick = y - fc2;
    }
    
    //printf("%d %d \n",button,state);
}

void MouseMotion(int x, int y)
{
    // If button1 pressed, zoom in/out if mouse is moved up/down.
    if (g_bButton1Down)
    {
        fc2 = (y - g_yClick);
        
        //printf("fc1 = %f, fc2=%f\n",fc1,fc2);
        
        if (fc2 < 0) fc2 = 0;
        if (fc2 < fc1) fc2 = fc1 + 1;
        if (fc2 > max_fc) fc2 = max_fc;
        
        //printf("fc1 = %f, fc2=%f\n",fc1,fc2);
        
        GenerateFilterCoeff(filter_type);
        UpdateFilterCoeff();
        GenerateFilterOutput();
      
        glutPostRedisplay();
    }
    else
    {
        printf("Mouse motion without button pressed");
    }
}

int BuildPopupMenu (void)
{
    int menu;
    
    menu = glutCreateMenu(SelectFromMenu);
  
    glutAddMenuEntry ("Select Window",MENU_EMPTY);
    glutAddMenuEntry ("-------------",MENU_EMPTY);
    glutAddMenuEntry ("RECTANGULAR: \t (PRESS '1')", MENU_RECTANGLE);
    glutAddMenuEntry ("BARTLETT: \t \t (PRESS '2')", MENU_BARTLETT);
    glutAddMenuEntry ("HANNING: \t \t (PRESS '3')", MENU_HANNING);
    glutAddMenuEntry ("HAMMING: \t \t (PRESS '4')", MENU_HAMMING);
    glutAddMenuEntry ("BLACKMAN: \t \t (PRESS '5')", MENU_BLACKMAN);
    glutAddMenuEntry ("",MENU_EMPTY);
    glutAddMenuEntry ("Select Filter",MENU_EMPTY);
    glutAddMenuEntry ("-------------",MENU_EMPTY);
    glutAddMenuEntry ("Low Pass: \t \t (PRESS 'l')", MENU_LOWPASS);
    glutAddMenuEntry ("High Pass: \t \t (PRESS 'h')", MENU_HIGHPASS);
    glutAddMenuEntry ("Band Pass: \t \t (PRESS 'b')", MENU_BANDPASS);
    glutAddMenuEntry ("Band Stop: \t \t (PRESS 'p')", MENU_BANDSTOP);
    glutAddMenuEntry ("",MENU_EMPTY);
    glutAddMenuEntry ("Select Input Signal",MENU_EMPTY);
    glutAddMenuEntry ("-------------------",MENU_EMPTY);
    glutAddMenuEntry ("Sine: \t \t \t (PRESS 's')", MENU_SINE);
    glutAddMenuEntry ("Square: \t \t \t (PRESS 'q')", MENU_SQUARE);
    glutAddMenuEntry ("Noise: \t \t \t (PRESS 'n')", MENU_NOISE);
    glutAddMenuEntry ("",MENU_EMPTY);
    glutAddMenuEntry ("Filter Window: \t (PRESS 'f')", MENU_FILTER);
    glutAddMenuEntry ("Input Window: \t (PRESS 'i')", MENU_INPUT);
    glutAddMenuEntry ("Output Window: \t (PRESS 'o')", MENU_OUTPUT);
    glutAddMenuEntry ("",MENU_EMPTY);
    glutAddMenuEntry ("Exit demo: \t \t (PRESS 'Esc')", MENU_EXIT);
  
    return menu;
}

int main(int argc, char **argv)
{
    
    //---------------------------------------------------
    // n and N range check
    //---------------------------------------------------
    if (n > N)
    {
        printf("Error!!!");
    }
    //---------------------------------------------------
    
    //Create Data
    GenerateWindow(window_type);
    GenerateFilterCoeff(filter_type);
    UpdateFilterCoeff();
    GenerateSignal(signal_type);
    GenerateFilterOutput();
    
    // GLUT Window Initialization:
    glutInit (&argc, argv);
    
    glutInitDisplayMode ( GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH);
    
    //Position of Window appears on the terminal screen
    glutInitWindowPosition(0,0);
    
    //Size of the Window
    glutInitWindowSize(g_Width,g_Height);
    
    glutCreateWindow ("Digital Filter Demo");
    
    glClearColor(0.0, 0.0, 0.0, 1.0);         // BLACK background

    // Register callbacks:
    glutDisplayFunc(displayFilter);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(Keyboard);
    glutMouseFunc(MouseButton);
    glutMotionFunc(MouseMotion);
    //glutIdleFunc (AnimateScene);
    
    // Create our popup menu
    BuildPopupMenu();
    glutAttachMenu(GLUT_RIGHT_BUTTON);
    
    // Turn the flow of control over to GLUT
    glutMainLoop ();
    
    exit(EXIT_SUCCESS);
}
