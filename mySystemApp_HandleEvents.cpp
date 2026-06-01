//********************************************
// Student Name		: LinGuiNa
// Student ID		: 114550010
// Student Email Address: Coryaquinolim07@gmail.com
//********************************************
// Instructor: Sai-Keung WONG
// Email:	cswingo@cs.nctu.edu.tw
//			wingo.wong@gmail.com
// National Yang Ming Chiao Tung University, Taiwan
// Computer Science
// Date: 2026/02

#include <sstream>
#include "mySystemApp.h"
using namespace std;

#define cn_StudentInfo "OOP --- Student Name:LinGuiNa. ID:114550010."

void MY_SYSTEM_APP::askForInput( )
{
    switch( mSystemType ) {
    case SYSTEM_TYPE_MONTE_CARLO_SIMULATION:
        //mMonteCarloSystem.askForInput( );
        break;
    case SYSTEM_TYPE_GALAXY_SYSTEM:
        //mGalaxySystem.askForInput( );
        break;
    case SYSTEM_TYPE_SIMPLE_PAINTER:
        //mSimplePainter.askForInput( );
        break;
    case SYSTEM_TYPE_SORTING_VISUALIZATION:
        //mSortingVisualization.askForInput( );
        break;
    case SYSTEM_TYPE_SIMPLE_FILE_EDITOR:
        //mSimpleFileEditor.askForInput( );
        break;
    }
}

void MY_SYSTEM_APP::updateMonteCarloTitle( )
{
    /*
    string title;
    string result;
    ostringstream strs;
    ostringstream strs_int;
    double area;
    int numSamples;
    title = "SYSTEM_TYPE_MONTE_CARLO_SIMULATION";
    mMonteCarloSystem.reset( );
    area = mMonteCarloSystem.computeArea( );
    strs << area;
    title = title + "  Area:";
    title = title + strs.str();
    numSamples = mMonteCarloSystem.getNumSamples();
    strs_int << numSamples;
    title = title + "  #Samples:";
    title = title + strs_int.str();
    glutSetWindowTitle(title.data());
    */
}

void MY_SYSTEM_APP::update_GraphSystemTitle( )
{
    string title;
    title = cn_StudentInfo;
    title = title + "  " + "Graph System. Spring 2026";
    glutSetWindowTitle(title.data());
}

void MY_SYSTEM_APP::update_2048SystemTitle( const char *msg )
{
    string title;
    title = cn_StudentInfo;
    title = title + "  " + "2048 System. Spring 2026";
    title = title + "   " + msg;
    glutSetWindowTitle(title.data());
}

void MY_SYSTEM_APP::update_ImageSystemTitle( )
{
    string title;
    title = cn_StudentInfo;
    title = title + "  " + "Image System. Spring 2026";
    glutSetWindowTitle(title.data());
}

void MY_SYSTEM_APP::reset( )
{
    /*
    switch( mSystemType ) {
    case SYSTEM_TYPE_MONTE_CARLO_SIMULATION:
        updateMonteCarloTitle( );
        break;
    case SYSTEM_TYPE_GALAXY_SYSTEM:
        mGalaxySystem.reset( );
        break;
    case SYSTEM_TYPE_SIMPLE_PAINTER:
        mSimplePainter.reset( );
        break;
    case SYSTEM_TYPE_SORTING_VISUALIZATION:
        mSortingVisualization.reset( );
        break;
    case SYSTEM_TYPE_SIMPLE_FILE_EDITOR:
        mSimpleFileEditor.reset( );
        break;
    }
    */
}

void MY_SYSTEM_APP::moveTo( float x, float y )
{
    if ( mCurSystem ) {
        mCurSystem->moveTo( x, y );
    }
}

void MY_SYSTEM_APP::clickAt( float x, float y )
{
    if ( mCurSystem ) {
        mCurSystem->clickAt( x, y );
    }
}

bool MY_SYSTEM_APP::specialFunc( int key, int x, int y )
{
    bool flgHandled = false;
    if ( mCurSystem == mGraphSystem ) {
        mGraphSystem->stopAutoNodeDeletion();
    }
    switch( key ) {
    case GLUT_KEY_F1:
        update_GraphSystemTitle();
        flgHandled = true;
        mCurSystem = mGraphSystem;
        mCamera->setZoom(-50.0);
        mCamera->set(vec3(-54.8, -55.4, 0.0));
        break;
    case GLUT_KEY_F2:
        update_2048SystemTitle("Dimension=4x4");
        flgHandled = true;
        mMy2048->setPosition( 30.0, 0 );
        mMy2048->setDimension(4, 4);
        mPrev_2048->setDimension(4, 4);
        mCurSystem = mMy2048;
        mCamera->setZoom(69.0);
        mCamera->set( vec3(-23.8, -11.4, 0.0) );
        break;
    case GLUT_KEY_F3:
        update_2048SystemTitle("Dimension=8x8");
        flgHandled = true;
        mMy2048->setPosition( 50.0, 0 );
        mMy2048->setDimension(8, 8);
        mPrev_2048->setDimension(8, 8);
        mCurSystem = mMy2048;
        mCamera->setZoom(46.0);
        mCamera->set( vec3(-43.8, -20.0, 0.0) );
        break;
    case GLUT_KEY_F4:
        break;
    case GLUT_KEY_F5:
        glutSetWindowTitle(cn_StudentInfo);
        flgHandled = true;
        break;
    }

    if ( flgHandled == false && mCurSystem == mMy2048 ) {
        mCurSystem->handleSpecialKeyPressedEvent( key );
        return true;
    }
    return true;
}

bool MY_SYSTEM_APP::handleKeyEvent( unsigned char key )
{
    bool flgHandled = false;
    cout << "key:" << (int) key << endl;

    if ( key == 's' || key == 'S') {
        showStudentInfo();
        return true;
    }
    if ( key == 'i' || key == 'I') {
        mCurSystem->askForInput( );
        return true;
    }
    if ( mCurSystem ) {
        mCurSystem->handleKeyPressedEvent( key );
        return true;
    }

    switch ( key ) {
    case ' ':
        break;
    case 'm':
    case 'M':
        showMenu( );
        break;
    case 's':
    case 'S':
        showStudentInfo( );
        break;
    case 'i':
    case 'I':
        askForInput( );
        break;
    case 'g':
    case 'G':
        mFlgShow_Grid = !mFlgShow_Grid;
        break;
    case 'r':
    case 'R':
        reset( );
        flgHandled = true;
        break;
    case 'z':
    case 'Z':
        mCamera->setZoom(-44.0);
        break;
    case 'c':
    case 'C':
        mCamera->setZoom(1.0);
        break;
    }
    return flgHandled;
}

bool MY_SYSTEM_APP::mouseMotionFunc( int mx, int my )
{
    static vector3 color( 1.0, 0.0, 0.0 );
    float x, y, z;
    mCamera->getCoordinates( x, y, z, (float) mx, (float) my );
    mMouseX_2DSpace = x;
    mMouseY_2DSpace = z;
    if ( mMouseButton == GLUT_LEFT_BUTTON ) {
        float rf = (rand()%1000)/1000.0f*0.5f;
        color += vector3(0.1f, 0.07f, 0.11f*rf);
        moveTo(x, z);
        return true;
    } else {
        return FREE_CANVAS_2D::mouseMotionFunc( mx, my );
    }
    return true;
}

bool MY_SYSTEM_APP::mouseFunc( int button, int state, int mx, int my )
{
    float x, y, z;
    mCamera->getCoordinates( x, y, z, (float) mx, (float) my );
    vector3 color( 1.0, 0.0, 0.0 );
    mMouseButton = button;
    mMouseX = mx;
    mMouseY = mCanvasHeight - my;
    if ( button == GLUT_LEFT_BUTTON ) {
        if ( state == GLUT_DOWN ) {
            clickAt( x, z );
        }
    } else {
        return FREE_CANVAS_2D::mouseFunc( button, state, mx, my );
    }
    return true;
}

bool MY_SYSTEM_APP::passiveMouseFunc( int mx, int my )
{
    float x, y, z;
    mCamera->getCoordinates( x, y, z, (float) mx, (float) my );
    mMouseX_2DSpace = x;
    mMouseY_2DSpace = z;
    if ( mCurSystem ) {
        mCurSystem->handlePassiveMouseEvent( x, z );
    }
    return true;
}
