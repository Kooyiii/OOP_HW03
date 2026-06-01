//********************************************
// Student Name		: LinGuiNa
// Student ID		: 114550010
// Student Email Address: Coryaquinolim07@gmail.com
//********************************************
//
// Instructor: Sai-Keung WONG
// Email:	cswingo@cs.nctu.edu.tw
//			wingo.wong@gmail.com
//
// National Yang Ming Chiao Tung University, Taiwan
// Computer Science
// Date: 2026/02
//

#include <iostream>
#include "mySystem_GraphSystem.h"
#include <time.h>
#include <cmath>
#include <climits>
#include <cfloat>

using namespace std;

namespace SYS_CONSTANTS {
    int max_int = INT_MAX;
    double max_double = DBL_MAX;
}

int Param::GRAPH_MAX_NUM_NODES = 10000;
int Param::GRAPH_MAX_NUM_EDGES = 10000;

// -------------------------------------------------------
// Constructor
// -------------------------------------------------------
GRAPH_SYSTEM::GRAPH_SYSTEM( )
{
    mFlgAutoNodeDeletion = false;
    mFlgShowNodeDepth    = false;
    mOperation           = GRAPH_OPERATION::GRAPH_OPERATION_SHORTEST_PATH;
    mOperationName       = "SHORTEST_PATH";
    mMaxNodeDepth        = 0.0;
    mStartNode           = nullptr;
    mDestinationNode     = nullptr;

    initMemoryPool();
    createDefaultGraph();
}

// -------------------------------------------------------
// Memory pool
// -------------------------------------------------------
void GRAPH_SYSTEM::initMemoryPool( )
{
    mMaxNumNodes = Param::GRAPH_MAX_NUM_NODES;
    mMaxNumEdges = Param::GRAPH_MAX_NUM_EDGES;

    mNodeArr_Pool = new GRAPH_NODE[mMaxNumNodes];
    mEdgeArr_Pool = new GRAPH_EDGE[mMaxNumEdges];

    mCurNumOfActiveNodes = 0;
    mCurNumOfActiveEdges = 0;

    mActiveNodeArr = new int[mMaxNumNodes];
    mActiveEdgeArr = new int[mMaxNumEdges];
    mFreeNodeArr   = new int[mMaxNumNodes];
    mFreeEdgeArr   = new int[mMaxNumEdges];

    for ( int i = 0; i < mMaxNumNodes; ++i ) {
        mNodeArr_Pool[ i ].id = i;
    }
    for ( int i = 0; i < mMaxNumEdges; ++i ) {
        mEdgeArr_Pool[ i ].id = i;
    }

    reset( );
}

void GRAPH_SYSTEM::reset( )
{
    stopAutoNodeDeletion();

    mCurNumOfActiveNodes = 0;
    mCurNumOfActiveEdges = 0;
    mCurNumOfFreeNodes   = mMaxNumNodes;
    mCurNumOfFreeEdges   = mMaxNumEdges;

    for ( int i = 0; i < mCurNumOfFreeNodes; ++i ) {
        mFreeNodeArr[ i ] = i;
    }
    for ( int i = 0; i < mCurNumOfFreeEdges; ++i ) {
        mFreeEdgeArr[ i ] = i;
    }

    // Clear edge lists on all pool nodes
    for ( int i = 0; i < mMaxNumNodes; ++i ) {
        mNodeArr_Pool[ i ].edgeID.clear();
        mNodeArr_Pool[ i ].visited    = false;
        mNodeArr_Pool[ i ].depth      = 0;
        mNodeArr_Pool[ i ].path_cost  = 0.0;
        mNodeArr_Pool[ i ].path_parent = nullptr;
    }

    mPassiveSelectedNode = nullptr;
    mSelectedNode        = nullptr;

    mFlgAutoNodeDeletion = false;
    mFlgShowNodeDepth    = false;
    mMaxNodeDepth        = 0.0;
    mStartNode           = nullptr;
    mDestinationNode     = nullptr;
    mPaths.clear();
}

// -------------------------------------------------------
// Pool helpers
// -------------------------------------------------------
GRAPH_NODE *GRAPH_SYSTEM::getFreeNode( )
{
    if ( mCurNumOfFreeNodes == 0 ) return nullptr;
    --mCurNumOfFreeNodes;
    int id = mFreeNodeArr[ mCurNumOfFreeNodes ];
    GRAPH_NODE *n = &mNodeArr_Pool[ id ];
    mActiveNodeArr[ mCurNumOfActiveNodes ] = id;
    n->dynamicID = mCurNumOfActiveNodes;
    ++mCurNumOfActiveNodes;
    return n;
}

GRAPH_EDGE *GRAPH_SYSTEM::getFreeEdge( )
{
    if ( mCurNumOfFreeEdges == 0 ) return nullptr;
    --mCurNumOfFreeEdges;
    int id = mFreeEdgeArr[ mCurNumOfFreeEdges ];
    GRAPH_EDGE *e = &mEdgeArr_Pool[ id ];
    mActiveEdgeArr[ mCurNumOfActiveEdges ] = id;
    e->dynamicID = mCurNumOfActiveEdges;
    ++mCurNumOfActiveEdges;
    return e;
}

// -------------------------------------------------------
// addNode  – returns nodeID, or -1 on failure
// -------------------------------------------------------
int GRAPH_SYSTEM::addNode( float x, float y, float z, float r )
{
    GRAPH_NODE *g = getFreeNode();
    if ( g == nullptr ) return -1;

    g->p        = vector3( x, y, z );
    g->r        = r;
    g->visited  = false;
    g->depth    = 0;
    g->path_cost  = 0.0;
    g->path_parent = nullptr;
    g->edgeID.clear();

    return g->id;
}

// -------------------------------------------------------
// addEdge  – returns edgeID, or -1 on failure
// -------------------------------------------------------
int GRAPH_SYSTEM::addEdge( int nodeID_0, int nodeID_1 )
{
    GRAPH_EDGE *e = getFreeEdge();
    if ( e == nullptr ) return -1;

    e->nodeID[0] = nodeID_0;
    e->nodeID[1] = nodeID_1;

    mNodeArr_Pool[ nodeID_0 ].edgeID.push_back( e->id );
    mNodeArr_Pool[ nodeID_1 ].edgeID.push_back( e->id );

    return e->id;
}

// -------------------------------------------------------
// findNearestNode
// -------------------------------------------------------
GRAPH_NODE *GRAPH_SYSTEM::findNearestNode( double x, double z, double &cur_distance2 ) const
{
    GRAPH_NODE *nearest = nullptr;
    cur_distance2 = DBL_MAX;

    for ( int i = 0; i < mCurNumOfActiveNodes; ++i ) {
        int id = mActiveNodeArr[ i ];
        const GRAPH_NODE *n = &mNodeArr_Pool[ id ];
        double dx = n->p.x - x;
        double dz = n->p.z - z;
        double d2 = dx*dx + dz*dz;
        if ( d2 < cur_distance2 ) {
            cur_distance2 = d2;
            nearest = const_cast<GRAPH_NODE*>(n);
        }
    }
    return nearest;
}

// -------------------------------------------------------
// Delete helpers
// -------------------------------------------------------
void GRAPH_SYSTEM::removeEdgeFromNode( const GRAPH_EDGE *e, int nodeID )
{
    GRAPH_NODE *n = &mNodeArr_Pool[ nodeID ];
    for ( int i = 0; i < (int)n->edgeID.size(); ++i ) {
        if ( n->edgeID[ i ] == e->id ) {
            n->edgeID.erase( n->edgeID.begin() + i );
            return;
        }
    }
}

void GRAPH_SYSTEM::deleteEdge( int edgeID )
{
    GRAPH_EDGE *e = &mEdgeArr_Pool[ edgeID ];

    // Remove the edge reference from both endpoint nodes
    removeEdgeFromNode( e, e->nodeID[0] );
    removeEdgeFromNode( e, e->nodeID[1] );

    // Swap with last active edge
    int dynamicID = e->dynamicID;
    int lastID    = mActiveEdgeArr[ mCurNumOfActiveEdges - 1 ];
    mActiveEdgeArr[ dynamicID ] = lastID;
    mEdgeArr_Pool[ lastID ].dynamicID = dynamicID;

    --mCurNumOfActiveEdges;

    // Return to free pool
    mFreeEdgeArr[ mCurNumOfFreeEdges ] = edgeID;
    ++mCurNumOfFreeEdges;
}

void GRAPH_SYSTEM::deleteEdgesOfNode( int nodeID )
{
    GRAPH_NODE *n = &mNodeArr_Pool[ nodeID ];
    // Copy the list because deleteEdge modifies it
    vector<int> edgesToDelete = n->edgeID;
    for ( int i = 0; i < (int)edgesToDelete.size(); ++i ) {
        deleteEdge( edgesToDelete[ i ] );
    }
}

void GRAPH_SYSTEM::deleteNode( int nodeID )
{
    if ( mCurNumOfActiveNodes <= 0 ) return;

    GRAPH_NODE *n = &mNodeArr_Pool[ nodeID ];

    // Delete all incident edges first
    deleteEdgesOfNode( nodeID );

    // Swap with last active node
    int dynamicID = n->dynamicID;
    int lastID    = mActiveNodeArr[ mCurNumOfActiveNodes - 1 ];
    mActiveNodeArr[ dynamicID ] = lastID;
    mNodeArr_Pool[ lastID ].dynamicID = dynamicID;

    --mCurNumOfActiveNodes;

    // Return to free pool
    mFreeNodeArr[ mCurNumOfFreeNodes ] = nodeID;
    ++mCurNumOfFreeNodes;
}

void GRAPH_SYSTEM::deleteSelectedNode( )
{
    if ( mSelectedNode == nullptr ) return;
    int id = mSelectedNode->id;
    mSelectedNode        = nullptr;
    mPassiveSelectedNode = nullptr;
    mStartNode           = nullptr;
    mDestinationNode     = nullptr;
    deleteNode( id );
}

// -------------------------------------------------------
// Graph creation helpers
// -------------------------------------------------------
void GRAPH_SYSTEM::createDefaultGraph( )
{
    reset( );

    // A simple triangle with a center hub
    float ox = 90.f, oz = 15.f;

    int n0 = addNode( ox + 0.0f,  0.f, oz + 0.0f,  2.0f );   // center
    int n1 = addNode( ox + 10.0f, 0.f, oz - 10.0f, 2.0f );
    int n2 = addNode( ox + 15.0f, 0.f, oz + 5.0f,  2.0f );
    int n3 = addNode( ox + 0.0f,  0.f, oz + 12.0f, 2.0f );
    int n4 = addNode( ox - 15.0f, 0.f, oz + 5.0f,  2.0f );
    int n5 = addNode( ox - 10.0f, 0.f, oz - 10.0f, 2.0f );

    addEdge( n0, n1 );
    addEdge( n0, n2 );
    addEdge( n0, n3 );
    addEdge( n0, n4 );
    addEdge( n0, n5 );
    addEdge( n1, n2 );
    addEdge( n2, n3 );
    addEdge( n3, n4 );
    addEdge( n4, n5 );
    addEdge( n5, n1 );
}

// Circular rings: n nodes per layer, num_layers layers
void GRAPH_SYSTEM::createNet_Circular( int n, int num_layers )
{
    reset( );

    float ox = 90.f, oz = 30.f;
    float base_r = 5.f;
    float layer_dist = 5.f;

    const float PI = 3.14159265f;

    // Keep track of node IDs per layer
    vector< vector<int> > layers( num_layers );

    for ( int layer = 0; layer < num_layers; ++layer ) {
        float radius = base_r + layer * layer_dist;
        layers[ layer ].resize( n );
        for ( int i = 0; i < n; ++i ) {
            float angle = 2.f * PI * i / n;
            float x = ox + radius * cosf( angle );
            float z = oz + radius * sinf( angle );
            layers[ layer ][ i ] = addNode( x, 0.f, z );
        }
        // Connect ring
        for ( int i = 0; i < n; ++i ) {
            addEdge( layers[ layer ][ i ], layers[ layer ][ (i+1) % n ] );
        }
        // Connect to previous layer
        if ( layer > 0 ) {
            for ( int i = 0; i < n; ++i ) {
                addEdge( layers[ layer ][ i ], layers[ layer-1 ][ i ] );
            }
        }
    }
}

// Square grid: n columns, num_layers rows
void GRAPH_SYSTEM::createNet_Square( int n, int num_layers )
{
    reset( );

    float ox = 5.f, oz = 5.f;
    float dx = 5.f, dz = 5.f;

    vector< vector<int> > grid( num_layers, vector<int>(n) );

    for ( int row = 0; row < num_layers; ++row ) {
        for ( int col = 0; col < n; ++col ) {
            float x = ox + col * dx;
            float z = oz + row * dz;
            grid[ row ][ col ] = addNode( x, 0.f, z );
        }
    }

    // Horizontal edges
    for ( int row = 0; row < num_layers; ++row ) {
        for ( int col = 0; col < n-1; ++col ) {
            addEdge( grid[row][col], grid[row][col+1] );
        }
    }
    // Vertical edges
    for ( int row = 0; row < num_layers-1; ++row ) {
        for ( int col = 0; col < n; ++col ) {
            addEdge( grid[row][col], grid[row+1][col] );
        }
    }
}

// Radical circular: n nodes on a ring + spokes to center
void GRAPH_SYSTEM::createNet_RadicalCircular( int n )
{
    reset( );

    float ox = 90.f, oz = 15.f;
    float r  = 15.f;
    const float PI = 3.14159265f;

    int center = addNode( ox, 0.f, oz );

    int first = -1, prev = -1;
    for ( int i = 0; i < n; ++i ) {
        float angle = 2.f * PI * i / n;
        float x = ox + r * cosf( angle );
        float z = oz + r * sinf( angle );
        int id = addNode( x, 0.f, z );
        addEdge( center, id );
        if ( first == -1 ) first = id;
        if ( prev  != -1 ) addEdge( prev, id );
        prev = id;
    }
    if ( first != -1 && prev != -1 ) addEdge( prev, first );
}

// Double circles with random cross-connections
void GRAPH_SYSTEM::createRandomGraph_DoubleCircles( int n )
{
    reset( );

    float ox = 90.f, oz = 15.f;
    float r1 = 15.f, r2 = 28.f;
    const float PI = 3.14159265f;

    srand( (unsigned)time(nullptr) );

    vector<int> inner(n), outer(n);

    for ( int i = 0; i < n; ++i ) {
        float angle = 2.f * PI * i / n;
        inner[i] = addNode( ox + r1*cosf(angle), 0.f, oz + r1*sinf(angle) );
        outer[i] = addNode( ox + r2*cosf(angle), 0.f, oz + r2*sinf(angle) );
    }

    for ( int i = 0; i < n; ++i ) {
        addEdge( inner[i], inner[(i+1)%n] );
        addEdge( outer[i], outer[(i+1)%n] );
        // Spoke every other node
        if ( i % 2 == 0 ) addEdge( inner[i], outer[i] );
    }

    // A few random cross edges between rings
    int extras = n / 3;
    for ( int k = 0; k < extras; ++k ) {
        int a = rand() % n;
        int b = rand() % n;
        if ( a != b ) addEdge( inner[a], outer[b] );
    }
}

// -------------------------------------------------------
// Depth computation (DFS)
// -------------------------------------------------------
void GRAPH_SYSTEM::resetDepthOfAllNodes( )
{
    mMaxNodeDepth = 0;
    int numNodes = getNumOfNodes();
    for ( int i = 0; i < numNodes; ++i ) {
        int nodeID = mActiveNodeArr[ i ];
        mNodeArr_Pool[ nodeID ].depth   = 0;
        mNodeArr_Pool[ nodeID ].visited = false;
    }
}

void GRAPH_SYSTEM::computeDepthOfAllNodesFromSelectedNode( GRAPH_NODE *node, int depth )
{
    if ( node == nullptr ) return;
    if ( node->visited ) return;

    node->visited = true;
    node->depth   = depth;

    for ( int i = 0; i < (int)node->edgeID.size(); ++i ) {
        int edgeID = node->edgeID[ i ];
        GRAPH_EDGE *e  = &mEdgeArr_Pool[ edgeID ];
        GRAPH_NODE *n0 = &mNodeArr_Pool[ e->nodeID[0] ];
        GRAPH_NODE *n1 = &mNodeArr_Pool[ e->nodeID[1] ];
        GRAPH_NODE *next = ( n0 == node ) ? n1 : n0;
        if ( !next->visited ) {
            computeDepthOfAllNodesFromSelectedNode( next, depth + 1 );
        }
    }
}

void GRAPH_SYSTEM::computeDepthOfAllNodesFromSelectedNode( )
{
    resetDepthOfAllNodes();
    if ( mSelectedNode == nullptr ) return;

    mMaxNodeDepth = 0;
    mSelectedNode->depth = 0;

    computeDepthOfAllNodesFromSelectedNode( mSelectedNode, 0 );

    // Find max depth
    int numNodes = getNumOfNodes();
    for ( int i = 0; i < numNodes; ++i ) {
        int nodeID = mActiveNodeArr[ i ];
        GRAPH_NODE *n = &mNodeArr_Pool[ nodeID ];
        if ( n->depth > (int)mMaxNodeDepth ) {
            mMaxNodeDepth = (float)n->depth;
        }
    }
}

float GRAPH_SYSTEM::getNodeDepthFromSelectedNode( int nodeIndex ) const
{
    int nodeID = mActiveNodeArr[ nodeIndex ];
    return (float)mNodeArr_Pool[ nodeID ].depth;
}

// -------------------------------------------------------
// Shortest path (Dijkstra-style recursive)
// -------------------------------------------------------
void GRAPH_SYSTEM::resetPathInformationOfAllNodes( )
{
    int numNodes = getNumOfNodes();
    for ( int i = 0; i < numNodes; ++i ) {
        int nodeID = mActiveNodeArr[ i ];
        GRAPH_NODE *n = &mNodeArr_Pool[ nodeID ];
        n->path_cost   = DBL_MAX;
        n->path_parent = nullptr;
        n->visited     = false;
    }
}

void GRAPH_SYSTEM::computeShortestPath( GRAPH_NODE *node )
{
    if ( node == nullptr ) return;

    for ( int i = 0; i < (int)node->edgeID.size(); ++i ) {
        int edgeID     = node->edgeID[ i ];
        GRAPH_EDGE *e  = &mEdgeArr_Pool[ edgeID ];
        GRAPH_NODE *n0 = &mNodeArr_Pool[ e->nodeID[0] ];
        GRAPH_NODE *n1 = &mNodeArr_Pool[ e->nodeID[1] ];

        GRAPH_NODE *next = ( n0 == node ) ? n1 : n0;

        // Euclidean distance between node and next
        double d = node->p.distance( next->p );
        double newCost = node->path_cost + d;

        if ( newCost < next->path_cost ) {
            next->path_cost   = newCost;
            next->path_parent = node;
            computeShortestPath( next );
        }
    }
}

void GRAPH_SYSTEM::computeShortestPath( )
{
    resetPathInformationOfAllNodes();
    if ( mStartNode == nullptr || mDestinationNode == nullptr ) return;

    mStartNode->path_cost   = 0.0;
    mStartNode->path_parent = nullptr;

    computeShortestPath( mStartNode );
}

// -------------------------------------------------------
// Interaction
// -------------------------------------------------------
void GRAPH_SYSTEM::askForInput( )
{
    cout << "GRAPH_SYSTEM -- LinGuiNa (114550010)" << endl;
    cout << "Key usage:" << endl;
    cout << "1: create a default graph" << endl;
    cout << "2: create circular graph" << endl;
    cout << "3: create square grid graph" << endl;
    cout << "4: create radical circular graph" << endl;
    cout << "5: create double-circles graph" << endl;
    cout << "Delete: delete selected node and all incident edges" << endl;
    cout << "Spacebar: Change operation" << endl;
    cout << "d/D: Toggle automatic node deletion" << endl;
    cout << "Use mouse left button to select/unselect or add edge" << endl;
}

void GRAPH_SYSTEM::moveTo( double x, double y )
{
    if ( !mSelectedNode ) return;
    mSelectedNode->p.x = x;
    mSelectedNode->p.z = y;
}

void GRAPH_SYSTEM::clickAt( double x, double z )
{
    double cur_d2;
    GRAPH_NODE *nearest_node = findNearestNode( x, z, cur_d2 );

    if ( nearest_node == nullptr ) {
        mSelectedNode = nullptr;
        return;
    }
    // Use a generous click radius: max of node radius or 3.0 units, times 4
    double clickRadius = nearest_node->r < 3.0 ? 3.0 : nearest_node->r;
    if ( cur_d2 > clickRadius * clickRadius * 4.0 ) {
        mSelectedNode = nullptr;
        return;
    }
    if ( mSelectedNode == nearest_node ) {
        mSelectedNode = nullptr;
        return;
    }
    performOperation( nearest_node );
}

void GRAPH_SYSTEM::performOperation( GRAPH_NODE *node )
{
    switch ( mOperation ) {
    case GRAPH_OPERATION::GRAPH_OPERATION_NODE_DEPTH:
        mSelectedNode     = node;
        mFlgShowNodeDepth = true;
        computeDepthOfAllNodesFromSelectedNode();
        break;

    case GRAPH_OPERATION::GRAPH_OPERATION_ADD_EDGE:
        if ( mSelectedNode != nullptr ) {
            addEdge( mSelectedNode->id, node->id );
            mSelectedNode = nullptr;
            return;
        }
        mSelectedNode = node;
        break;

    case GRAPH_OPERATION::GRAPH_OPERATION_SHORTEST_PATH:
        if ( mSelectedNode != nullptr ) {
            mStartNode       = mSelectedNode;
            mDestinationNode = node;
            computeShortestPath();
            mSelectedNode = nullptr;
            return;
        }
        mSelectedNode = node;
        break;
    }
}

void GRAPH_SYSTEM::changeOperation( )
{
    switch ( mOperation ) {
    case GRAPH_OPERATION::GRAPH_OPERATION_NODE_DEPTH:
        mOperation     = GRAPH_OPERATION::GRAPH_OPERATION_ADD_EDGE;
        mOperationName = "ADD_EDGE";
        break;
    case GRAPH_OPERATION::GRAPH_OPERATION_ADD_EDGE:
        mOperation     = GRAPH_OPERATION::GRAPH_OPERATION_SHORTEST_PATH;
        mOperationName = "SHORTEST_PATH";
        break;
    case GRAPH_OPERATION::GRAPH_OPERATION_SHORTEST_PATH:
        mOperation     = GRAPH_OPERATION::GRAPH_OPERATION_NODE_DEPTH;
        mOperationName = "NODE_DEPTH";
        break;
    }
}

void GRAPH_SYSTEM::handleKeyPressedEvent( unsigned char key )
{
    switch ( key ) {
    case 127: // Delete
        mFlgAutoNodeDeletion = false;
        deleteSelectedNode();
        break;
    case '1':
        mFlgAutoNodeDeletion = false;
        createDefaultGraph();
        mSelectedNode = nullptr;
        break;
    case '2':
        mFlgAutoNodeDeletion = false;
        createNet_Circular( 12, 6 );
        mSelectedNode = nullptr;
        break;
    case '3':
        mFlgAutoNodeDeletion = false;
        createNet_Square( 15, 4 );
        mSelectedNode = nullptr;
        break;
    case '4':
        mFlgAutoNodeDeletion = false;
        createNet_RadicalCircular( 24 );
        mSelectedNode = nullptr;
        break;
    case '5':
        mNumPoints_DoubleCircles = 24;
        mFlgAutoNodeDeletion = false;
        createRandomGraph_DoubleCircles( mNumPoints_DoubleCircles );
        mSelectedNode = nullptr;
        break;
    case '<':
        mNumPoints_DoubleCircles--;
        if ( mNumPoints_DoubleCircles < 3 ) mNumPoints_DoubleCircles = 3;
        mFlgAutoNodeDeletion = false;
        createRandomGraph_DoubleCircles( mNumPoints_DoubleCircles );
        mSelectedNode = nullptr;
        break;
    case '>':
        mNumPoints_DoubleCircles++;
        if ( mNumPoints_DoubleCircles > 36 ) mNumPoints_DoubleCircles = 36;
        mFlgAutoNodeDeletion = false;
        createRandomGraph_DoubleCircles( mNumPoints_DoubleCircles );
        mSelectedNode = nullptr;
        break;
    case 'r':
    case 'R':
        mFlgAutoNodeDeletion = false;
        mSelectedNode = nullptr;
        break;
    case ' ':
        changeOperation();
        break;
    case 'd':
    case 'D':
        mFlgAutoNodeDeletion = !mFlgAutoNodeDeletion;
        break;
    }
}

void GRAPH_SYSTEM::handlePassiveMouseEvent( double x, double z )
{
    double cur_d2;
    GRAPH_NODE *n = findNearestNode( x, z, cur_d2 );
    if ( n == nullptr ) return;
    double clickRadius = n->r < 3.0 ? 3.0 : n->r;
    if ( cur_d2 > clickRadius * clickRadius * 4.0 ) {
        mPassiveSelectedNode = nullptr;
        return;
    }
    mPassiveSelectedNode = n;
}

// -------------------------------------------------------
// Getters
// -------------------------------------------------------
int GRAPH_SYSTEM::getNumOfNodes( ) const
{
    return mCurNumOfActiveNodes;
}

int GRAPH_SYSTEM::getNumOfEdges( ) const
{
    return mCurNumOfActiveEdges;
}

void GRAPH_SYSTEM::getNodeInfo( int nodeIndex, double &r, vector3 &p ) const
{
    int nodeID = mActiveNodeArr[ nodeIndex ];
    GRAPH_NODE *n = &mNodeArr_Pool[ nodeID ];
    r = n->r;
    p = n->p;
}

void GRAPH_SYSTEM::getNodeInfo( int nodeIndex, double &r, vector3 &p, float &depth ) const
{
    int nodeID = mActiveNodeArr[ nodeIndex ];
    GRAPH_NODE *n = &mNodeArr_Pool[ nodeID ];
    r     = n->r;
    p     = n->p;
    depth = (float)n->depth;
}

vector3 GRAPH_SYSTEM::getNodePositionOfEdge( int edgeIndex, int nodeIndex ) const
{
    int edgeID = mActiveEdgeArr[ edgeIndex ];
    GRAPH_EDGE *e = &mEdgeArr_Pool[ edgeID ];
    return mNodeArr_Pool[ e->nodeID[ nodeIndex ] ].p;
}

bool GRAPH_SYSTEM::isSelectedNode( ) const
{
    return mSelectedNode != nullptr;
}

void GRAPH_SYSTEM::getInfoOfSelectedPoint( double &r, vector3 &p ) const
{
    if ( !isSelectedNode() ) return;
    r = mSelectedNode->r;
    p = mSelectedNode->p;
}

void GRAPH_SYSTEM::stopAutoNodeDeletion( )
{
    mFlgAutoNodeDeletion = false;
}

// -------------------------------------------------------
// Update  (auto node deletion)
// -------------------------------------------------------
void GRAPH_SYSTEM::update( )
{
    if ( !mFlgAutoNodeDeletion ) return;
    if ( mCurNumOfActiveNodes <= 0 ) {
        mFlgAutoNodeDeletion = false;
        return;
    }

    Sleep( 250 );

    // Delete a random active node each frame
    int idx    = rand() % mCurNumOfActiveNodes;
    int nodeID = mActiveNodeArr[ idx ];
    GRAPH_NODE *n = &mNodeArr_Pool[ nodeID ];

    if ( mSelectedNode        == n ) mSelectedNode        = nullptr;
    if ( mPassiveSelectedNode == n ) mPassiveSelectedNode = nullptr;
    if ( mStartNode           == n ) mStartNode           = nullptr;
    if ( mDestinationNode     == n ) mDestinationNode     = nullptr;

    deleteNode( nodeID );

    mSelectedNode        = nullptr;
    mPassiveSelectedNode = nullptr;
}
