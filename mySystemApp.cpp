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

#include "mySystemApp.h"
using namespace std;

void MY_SYSTEM_APP::showStudentInfo( ) const
{
	cout << "****************************************************************" << endl;
    cout << "****************************************************************" << endl;
    cout << "Data Structures and Object-Oriented Programming" << endl;
    cout << "Instructor: Prof. Sai-Keung Wong" << endl;
    cout << "----------------------------------------------------------------" << endl;
    cout << "Date: 2026/05/01" << endl;
	cout << "Student ID:\t" << "114550010" << endl;
	cout << "Student Name:\t" << "LinGuiNa" << endl;
	cout << "Student Email:\t" << "Coryaquinolim07@gmail.com" << endl;
    cout << "****************************************************************" << endl;
    cout << "****************************************************************" << endl;
}

void MY_SYSTEM_APP::showMenu( ) const
{
    cout << "Welcome to our mini-galaxy!" << endl;
    cout << "Menu----------------------" << endl;
    cout << "Key usage:" << endl;
    cout << "F1: Graph System" << endl;
    cout << "F2: 2048 (4x4)" << endl;
    cout << "F3: 2048 (8x8)" << endl;
    cout << endl;
    cout << "r: reset" << endl;
    cout << "--------------------------" << endl;
}

MY_SYSTEM_APP::MY_SYSTEM_APP( )
{
	mSystemType   = SYSTEM_TYPE_GALAXY_SYSTEM;
    mColorPalette = new COLOR_PALETTE;
    //
    mCurSystem = 0;
}

void MY_SYSTEM_APP::initApp( )
{
	mFlgShow_Grid = false;
    showMenu( );
    //
    mGraphSystem = new GRAPH_SYSTEM;
    mMy2048      = new MY_2048;
    //
    mCurSystem = mMy2048;
    ////////////////////////////////////////////
    ((MY_2048*)mMy2048)->setPosition( 30.0, 0 );
    mPrev_2048 = new MY_2048;
    mPrev_2048->setPosition( 0.0, 0 );
    mPrev_2048->setFocus( false );
    //
    specialFunc( GLUT_KEY_F1, 0, 0 );
}

void MY_SYSTEM_APP::update( )
{
    if ( mCurSystem == mMy2048 ) {
        if ( mCurSystem->isAutoPlay() ) {
            mCurSystem->update( );
        }
        return;
    }
    mCurSystem->update( );
}
