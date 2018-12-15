/*
    Paolo Scattolin s1023775
    Johan Urban     s1024726
    Assignment 13 bonus
*/
#include <iostream>
#include <fstream>
#include <string>
#include <cassert>
#include <vector>

using namespace std;

/************************************************************************
*  This section contains data structures, constants and global variables.
************************************************************************/

enum Cell {empty=0, wall, destination, worker, box,worker_on_dest,box_on_dest};
enum Direction {north=0, south, east, west};

const int HEIGHT = 15;
const int WIDTH = 15;

struct Pos
{
    int row;
    int col;
};
struct State
{
    Cell layout [HEIGHT][WIDTH];
    Pos player;
    vector <Pos> boxes;
};
struct Node
{
    State config ;
    int parent ;
};

int a_height;
int a_width;

Cell world [HEIGHT][WIDTH];
vector <State> path;

void solve2( vector<State>& attempt, vector<State>& shortest, int max_depth);

/************************************************************************
*  Some useful operators.
************************************************************************/

fstream& operator>> (fstream& file, Cell world[HEIGHT][WIDTH])
{
    // PRE:
    assert(true);
    // POST: the world design of the text file is scanned and saved in an array,
    // which can be used to be modified and printed.
    char c;

    for (int i = 0 ; i < HEIGHT ; i++)
    {
        for(int j = 0 ; j < WIDTH ; j++)
        {
            file.get(c);

            if (j == 0 && c != '*')
            {
                a_height = i;
                j = WIDTH;
                i = HEIGHT;
            }
            if (c == '\n')
            {
                 if (i == 0)
                    a_width = j;
                 j = WIDTH;
            }
            if (c == '*')
                world[i][j] = wall;
            if (c == ' ')
                world[i][j] = empty;
            if (c == '.')
                world[i][j] = destination;
            if (c == 'w')
                world[i][j] = worker;
            if (c == 'b')
                world[i][j] = box;
            if (c == 'W')
                world[i][j] = worker_on_dest;
            if (c == 'B')
                world[i][j] = box_on_dest;
        }
    }

}

bool operator== (Pos p1, Pos p2)
{
    // PRE:
    assert(p1.col > 0 && p1.row > 0 && p2.col > 0 && p2.row > 0);
    // POST: return true if the positions are equivalent.
    return (p1.col == p2.col && p1.row == p2.row) ;
}
bool operator== (vector<Pos> b1, vector<Pos> b2)
{
    // PRE:
    assert(true);
    // POST: returns true if the vectors are equivalent.
    for (int i = 0 ; i < b1.size() ; i ++)
    {
        if (!(b1[i] == b2[i]) )
            return false;
    }
    return true;
}
bool operator== (State& s1, State& s2)
{
    // PRE:
    assert(true);
    // POST: returns true if two instances of the world are the same. We only consider boxes and the worker,
    // becase the rest of the world remains static. This helps to avoid some unnecessary duplicate computations.
    return (s1.player == s2.player && s1.boxes == s2.boxes);
}

/************************************************************************
*  These 2 function should be in I/O but the program need them here :/
************************************************************************/

void show_world (Cell world[HEIGHT][WIDTH])
{
    // PRE:
    assert (true) ;
    // POST: Displays the world layout into the console.
    for (int i = 0 ; i < a_height ; i++)
    {
        for(int j = 0 ; j < a_width ; j++)
        {
            if (world[i][j] == wall)
                cout << "*";
            if ( world[i][j] == empty)
                cout << " ";
            if (world[i][j] == destination)
                cout << ".";
            if (world[i][j] == worker)
                cout << "w";
            if (world[i][j] == box)
                cout << "b";
            if (world[i][j] == worker_on_dest)
                cout << "W";
            if (world[i][j] == box_on_dest)
                cout << "B";
        }
        cout << endl ;
    }
}

State set_up (Cell w [HEIGHT][WIDTH])
{
    // PRE:
    assert(true);
    // POST: Returns a valid state based on the world.
    State config;

    for (int i = 0 ; i < a_height ; i++)
    {
        for(int j = 0 ; j < a_width ; j++)
        {
            Pos temp;
            config.layout[i][j] = w [i][j];

            if (config.layout[i][j] == worker || config.layout[i][j] == worker_on_dest )
            {
                config.player.row = i ;
                config.player.col = j ;
            }

            if (config.layout[i][j] == box || config.layout[i][j] == box_on_dest )
            {
                temp.row = i ;
                temp.col = j ;
                config.boxes.push_back(temp) ;
            }
        }
    }
    return config ;
}

/************************************************************************
*  This are the functions needed for world navigation of the worker + box
*  movements. Also they update the map and check for its completion.
*  finally since a box in a corner is a very easy deadlock to find we
*  implemented a function that recognise that situation.
************************************************************************/

bool box_in_corner (State start)
{
    // PRE:
    assert(true);
    // POST: returns true if there is at least one box in a corner.
    for (int i = 1 ; i < a_height-1 ; i++)
    {
        for (int j = 1 ; j < a_width-1 ; j++)
        {
            if (start.layout[i][j] == box)
            {
               if (start.layout[i+1][j] == wall || start.layout[i-1][j] == wall)
                {
                  if (start.layout[i][j+1] == wall || start.layout[i][j-1] == wall)
                    return true;
                }
            }
        }
    }
    return false;
}

bool facing_box (State s, Direction d)
{
    // PRE:
    assert(static_cast<int> (d) >= 0 && static_cast<int> (d) < 4);
    // POST: returns true if player faces a box for the given direction d. False otherwise.
    switch (d)
        {

            case north: return ((s.layout[s.player.row - 1] [s.player.col] == box
                              || s.layout[s.player.row - 1] [s.player.col] == box_on_dest));

            case south: return ((s.layout[s.player.row + 1] [s.player.col] == box
                              || s.layout[s.player.row + 1] [s.player.col] == box_on_dest));

            case east:  return ((s.layout[s.player.row] [s.player.col + 1] == box
                              || s.layout[s.player.row] [s.player.col + 1] == box_on_dest));

            case west:  return ((s.layout[s.player.row] [s.player.col - 1] == box
                              || s.layout[s.player.row] [s.player.col - 1] == box_on_dest));
        }

}

bool is_movable (State s, Direction d)
{
    // PRE:
    assert(static_cast<int> (d) >= 0 && static_cast<int> (d) < 4);
    // POST: returns true if, for the direction the worker intends to push the box,
    // there is a free cell / destination cell for the box to be moved to.
    switch (d)
    {
        case north:
            return (s.layout[s.player.row - 2][s.player.col] == empty || s.layout[s.player.row - 2][s.player.col] == destination);
            break;
        case south:
            return (s.layout[s.player.row + 2][s.player.col] == empty || s.layout[s.player.row + 2][s.player.col] == destination);
            break;
        case east:
            return (s.layout[s.player.row][s.player.col + 2] == empty || s.layout[s.player.row][s.player.col + 2] == destination);
            break;
        case west:
            return (s.layout[s.player.row][s.player.col - 2] == empty || s.layout[s.player.row][s.player.col - 2] == destination);
            break;
    }
}

bool can_go_north(State s)
{
    // PRE:
    assert(true);
    //POST: returns true if the player may go north, i.e. there is no object in the way.
     return (s.layout[s.player.row - 1][s.player.col] == empty || s.layout[s.player.row - 1][s.player.col] == destination);
}

bool can_go_south(State s)
{
    // PRE:
    assert(true);
    //POST: returns true if the player may go south, i.e. there is no object in the way.
    return (s.layout[s.player.row + 1][s.player.col] == empty || s.layout[s.player.row + 1][s.player.col] == destination);
}

bool can_go_east(State s)
{
    // PRE:
    assert(true);
    //POST: returns true if the player may go east, i.e. there is no object in the way.
    return (s.layout[s.player.row][s.player.col + 1] == empty || s.layout[s.player.row][s.player.col + 1] == destination);
}

bool can_go_west(State s)
{
    // PRE:
    assert(true);
    //POST: returns true if the player may go west, i.e. there is no object in the way.
    return (s.layout[s.player.row][s.player.col - 1] == empty || s.layout[s.player.row][s.player.col - 1] == destination);
}

bool is_solved (State s)
{
    // PRE:
    assert(true);
    // POST: returns true if the challenged is solved, i.e. in a solution state.
    for (int i = 0 ; i < a_height ; i++)
    {
        for(int j = 0 ; j < a_width ; j++)
        {
           if (s.layout[i][j] == box)
            return false ;
        }
    }
    return true;
}

/************************************************************************
*  This are the functions needed if the user select breadth first search.
************************************************************************/

bool new_config (vector<Node>& n, State s)
{
    // PRE:
    assert(n.size() > 0);
    // POST: checks if this scenario has been explored before. If not, true is returned.
    for(int j = 0 ; j < n.size(); j++)
    {
        if (n[j].config == s)
          return false;
    }
    return true;
}

void move_player (vector<Node>& n, int i, Direction d, State s)
{
    // PRE:
    assert(static_cast<int> (d) >=0 && static_cast<int> (d) < 4);
    assert(n.size() > 0);
    // POST: this function handles worker movement by swapping worker position with the next cell (if legal)
    // in the indicated direction. Also the state of the worker (on destination, not on destination) is respected.
    // Based on this a new state is generated and saved in the vector of all possible states.
    State newState = s;
    Node newNode;

    switch (d)
     {
            case north:
                if(newState.layout[newState.player.row][newState.player.col] == worker)
                {
                        if(newState.layout[newState.player.row - 1][newState.player.col] == empty)
                        {
                            newState.layout[newState.player.row - 1][newState.player.col] = worker;
                            newState.layout[newState.player.row][newState.player.col] = empty ;
                        }
                        if(newState.layout[newState.player.row - 1][newState.player.col] == destination)
                        {
                            newState.layout[newState.player.row - 1][newState.player.col] = worker_on_dest;
                            newState.layout[newState.player.row][newState.player.col] = empty ;
                        }
                }
                if(newState.layout[newState.player.row][newState.player.col] == worker_on_dest)
                {
                        if(newState.layout[newState.player.row - 1][newState.player.col] == empty)
                        {
                            newState.layout[newState.player.row - 1][newState.player.col] = worker;
                            newState.layout[newState.player.row][newState.player.col] = destination;
                        }
                        if(newState.layout[newState.player.row - 1][newState.player.col] == destination)
                        {
                            newState.layout[newState.player.row - 1][newState.player.col] = worker_on_dest;
                            newState.layout[newState.player.row][newState.player.col] = destination;
                        }
                }
                break;

            case south:
                if(newState.layout[newState.player.row][newState.player.col] == worker)
                {
                        if(newState.layout[newState.player.row + 1][newState.player.col] == empty)
                        {
                            newState.layout[newState.player.row + 1][newState.player.col] = worker;
                            newState.layout[newState.player.row][newState.player.col] = empty ;
                        }
                        if(newState.layout[newState.player.row + 1][newState.player.col] == destination)
                        {
                            newState.layout[newState.player.row + 1][newState.player.col] = worker_on_dest;
                            newState.layout[newState.player.row][newState.player.col] = empty ;
                        }
                }
                if(newState.layout[newState.player.row][newState.player.col] == worker_on_dest)
                {
                        if(newState.layout[newState.player.row + 1][newState.player.col] == empty)
                        {
                            newState.layout[newState.player.row + 1][newState.player.col] = worker;
                            newState.layout[newState.player.row][newState.player.col] = destination;
                        }
                        if(newState.layout[newState.player.row + 1][newState.player.col] == destination)
                        {
                            newState.layout[newState.player.row + 1][newState.player.col] = worker_on_dest;
                            newState.layout[newState.player.row][newState.player.col] = destination;
                        }
                }
                break;

            case east:
                if(newState.layout[newState.player.row][newState.player.col] == worker)
                {
                        if(newState.layout[newState.player.row][newState.player.col + 1] == empty)
                        {
                            newState.layout[newState.player.row][newState.player.col + 1] = worker;
                            newState.layout[newState.player.row][newState.player.col] = empty ;
                        }
                        if(newState.layout[newState.player.row][newState.player.col + 1] == destination)
                        {
                            newState.layout[newState.player.row][newState.player.col + 1] = worker_on_dest;
                            newState.layout[newState.player.row][newState.player.col] = empty ;
                        }
                }
                if(newState.layout[newState.player.row][newState.player.col] == worker_on_dest)
                {
                        if(newState.layout[newState.player.row][newState.player.col + 1] == empty)
                        {
                            newState.layout[newState.player.row][newState.player.col + 1] = worker;
                            newState.layout[newState.player.row][newState.player.col] = destination;
                        }
                        if(newState.layout[newState.player.row][newState.player.col + 1] == destination)
                        {
                            newState.layout[newState.player.row][newState.player.col + 1] = worker_on_dest;
                            newState.layout[newState.player.row][newState.player.col] = destination;
                        }
                }
                break;

            case west:
                if(newState.layout[newState.player.row][newState.player.col] == worker)
                {
                        if(newState.layout[newState.player.row][newState.player.col - 1] == empty)
                        {
                            newState.layout[newState.player.row][newState.player.col - 1] = worker;
                            newState.layout[newState.player.row][newState.player.col] = empty ;
                        }
                        if(newState.layout[newState.player.row][newState.player.col - 1] == destination)
                        {
                            newState.layout[newState.player.row][newState.player.col - 1] = worker_on_dest;
                            newState.layout[newState.player.row][newState.player.col] = empty ;
                        }
                }
                if(newState.layout[newState.player.row][newState.player.col] == worker_on_dest)
                {
                        if(newState.layout[newState.player.row][newState.player.col - 1] == empty)
                        {
                            newState.layout[newState.player.row][newState.player.col - 1] = worker;
                            newState.layout[newState.player.row][newState.player.col] = destination;
                        }
                        if(newState.layout[newState.player.row][newState.player.col - 1] == destination)
                        {
                            newState.layout[newState.player.row][newState.player.col - 1] = worker_on_dest;
                            newState.layout[newState.player.row][newState.player.col] = destination;
                        }
                }
                break;
     }

    newState = set_up(newState.layout);

    if (new_config(n, newState) && !box_in_corner(newState))
    {
        newNode.config = newState;
        newNode.parent = i;
        n.push_back(newNode) ;
    }
}

void move_box (vector<Node>& n, int i, Direction d)
{
    // PRE:
    assert(static_cast<int> (d) >=0 && static_cast<int> (d) < 4);
    assert(n.size() > 0);
    // POST: From perspective of the worker position and considering the indicated direction
    // it is checked whether there is a box in front of worker and that this box is allowed to be
    // moved in the same direction as the worker is going. If the criteria are met, the box is moved.
    // After, the function move_player is called.
    State newState = n[i].config;
    switch (d)
     {
            case north:
                if(newState.layout[newState.player.row - 1][newState.player.col] == box)
                {
                        if(newState.layout[newState.player.row - 2][newState.player.col] == empty)
                        {
                            newState.layout[newState.player.row - 2][newState.player.col] = box;
                            newState.layout[newState.player.row - 1][newState.player.col] = empty ;
                        }
                        if(newState.layout[newState.player.row - 2][newState.player.col] == destination)
                        {
                            newState.layout[newState.player.row - 2][newState.player.col] = box_on_dest;
                            newState.layout[newState.player.row - 1][newState.player.col] = empty ;
                        }
                }
                if(newState.layout[newState.player.row - 1][newState.player.col] == box_on_dest)
                {
                        if(newState.layout[newState.player.row - 2][newState.player.col] == empty)
                        {
                            newState.layout[newState.player.row - 2][newState.player.col] = box;
                            newState.layout[newState.player.row - 1][newState.player.col] = destination;
                        }
                        if(newState.layout[newState.player.row - 2][newState.player.col] == destination)
                        {
                            newState.layout[newState.player.row - 2][newState.player.col] = box_on_dest;
                            newState.layout[newState.player.row - 1][newState.player.col] = destination;
                        }
                }
                break ;

            case south:
                if(newState.layout[newState.player.row + 1][newState.player.col] == box)
                {
                        if(newState.layout[newState.player.row + 2][newState.player.col] == empty)
                        {
                            newState.layout[newState.player.row + 2][newState.player.col] = box;
                            newState.layout[newState.player.row + 1][newState.player.col] = empty ;
                        }
                        if(newState.layout[newState.player.row + 2][newState.player.col] == destination)
                        {
                            newState.layout[newState.player.row + 2][newState.player.col] = box_on_dest;
                            newState.layout[newState.player.row + 1][newState.player.col] = empty ;
                        }
                }
                if(newState.layout[newState.player.row + 1][newState.player.col] == box_on_dest)
                {
                        if(newState.layout[newState.player.row + 2][newState.player.col] == empty)
                        {
                            newState.layout[newState.player.row + 2][newState.player.col] = box;
                            newState.layout[newState.player.row + 1][newState.player.col] = destination;
                        }
                        if(newState.layout[newState.player.row + 2][newState.player.col] == destination)
                        {
                            newState.layout[newState.player.row + 2][newState.player.col] = box_on_dest;
                            newState.layout[newState.player.row + 1][newState.player.col] = destination;
                        }
                }
                break;

            case east:
                if(newState.layout[newState.player.row][newState.player.col + 1] == box)
                {
                        if(newState.layout[newState.player.row][newState.player.col + 2] == empty)
                        {
                            newState.layout[newState.player.row][newState.player.col + 2] = box;
                            newState.layout[newState.player.row][newState.player.col + 1] = empty ;
                        }
                        if(newState.layout[newState.player.row][newState.player.col + 2] == destination)
                        {
                            newState.layout[newState.player.row][newState.player.col + 2] = box_on_dest;
                            newState.layout[newState.player.row][newState.player.col + 1] = empty ;
                        }
                }
                if(newState.layout[newState.player.row][newState.player.col + 1] == box_on_dest)
                {
                        if(newState.layout[newState.player.row][newState.player.col + 2] == empty)
                        {
                            newState.layout[newState.player.row][newState.player.col + 2] = box;
                            newState.layout[newState.player.row][newState.player.col + 1] = destination;
                        }
                        if(newState.layout[newState.player.row][newState.player.col + 2] == destination)
                        {
                            newState.layout[newState.player.row][newState.player.col + 2] = box_on_dest;
                            newState.layout[newState.player.row][newState.player.col + 1] = destination;
                        }
                }
                break;

            case west:
                if(newState.layout[newState.player.row][newState.player.col - 1] == box)
                {
                        if(newState.layout[newState.player.row][newState.player.col - 2] == empty)
                        {
                            newState.layout[newState.player.row][newState.player.col - 2] = box;
                            newState.layout[newState.player.row][newState.player.col - 1] = empty ;
                        }
                        if(newState.layout[newState.player.row][newState.player.col - 2] == destination)
                        {
                            newState.layout[newState.player.row][newState.player.col - 2] = box_on_dest;
                            newState.layout[newState.player.row][newState.player.col - 1] = empty ;
                        }
                }
                if(newState.layout[newState.player.row][newState.player.col - 1] == box_on_dest)
                {
                        if(newState.layout[newState.player.row][newState.player.col - 2] == empty)
                        {
                            newState.layout[newState.player.row][newState.player.col - 2] = box;
                            newState.layout[newState.player.row][newState.player.col - 1] = destination;
                        }
                        if(newState.layout[newState.player.row][newState.player.col - 2] == destination)
                        {
                            newState.layout[newState.player.row][newState.player.col - 2] = box_on_dest;
                            newState.layout[newState.player.row][newState.player.col - 1] = destination;
                        }
                }
                break;
     }

    newState = set_up(newState.layout);
    move_player(n,i,d, newState);
}

void show_path (vector <Node>& n ,int i)
{
    // PRE:
    assert(n.size() > 0);
    // POST: recursively fills the path vector with the nodes that lead to a solution.
    if (i >= 0)
    {
        path.push_back(n[i].config);
        show_path(n, n[i].parent);
    }
}

void solve (State start)
{
    // PRE:
    assert(true);
    // POST: Computes the optimal solution, i.e. using least number of moves, to the level in question using breadth-first search.
    vector<Node> n = {{start,-1}} ;
    int i = 0 ;

    while(i < n.size() && !is_solved(n[i].config))
    {
        State s = n[i].config;

        if (facing_box(s,north) && is_movable(s,north))
            move_box(n,i,north);

        if (can_go_north(s))
            move_player(n,i,north,n[i].config);

        if (facing_box(s,south) && is_movable(s,south))
            move_box(n,i,south);

        if (can_go_south(s))
            move_player(n,i,south,n[i].config);

        if (facing_box(s,east) && is_movable(s,east))
            move_box(n,i,east);

        if (can_go_east(s))
            move_player(n,i,east,n[i].config);

        if (facing_box(s,west) && is_movable(s,west))
            move_box(n,i,west);

        if (can_go_west(s))
            move_player(n,i,west,n[i].config);

        i++ ;
     }

      if(i < n.size())
        show_path(n, i) ;
}

/************************************************************************
*  This are the functions needed if the user select depth first search.
*  p.s the giant switches both in move_player2 and move_box2 are the same
*      as the ones in move_player and move_box.
************************************************************************/

bool new_config2 (vector<State>& attempt, State s)
{
    // PRE:
    assert(attempt.size() > 0);
    // POST: checks if this scenario has been explored before. If not, true is returned.
    for(int i = 0 ; i < attempt.size(); i++)
    {
        if (attempt[i] == s)
          return false;
    }
    return true;
}

void move_player2 (vector<State>& attempt, vector<State>& shortest, Direction d, State s, int max_depth)
{
    // PRE:
    assert(static_cast<int> (d) >=0 && static_cast<int> (d) < 4);
    assert(attempt.size() > 0);
    // POST: this function handles worker movement by swapping worker position with the next cell (if legal)
    // in the indicated direction. Also the state of the worker (on destination, not on destination) is respected.
    // Based on this a new state is generated and saved in the vector of all possible states.
    // Same as before, but this time recursively.
    State newState = s;

    switch (d)
     {
            case north:
                if(newState.layout[newState.player.row][newState.player.col] == worker)
                {
                        if(newState.layout[newState.player.row - 1][newState.player.col] == empty)
                        {
                            newState.layout[newState.player.row - 1][newState.player.col] = worker;
                            newState.layout[newState.player.row][newState.player.col] = empty ;
                        }
                        if(newState.layout[newState.player.row - 1][newState.player.col] == destination)
                        {
                            newState.layout[newState.player.row - 1][newState.player.col] = worker_on_dest;
                            newState.layout[newState.player.row][newState.player.col] = empty ;
                        }
                }
                if(newState.layout[newState.player.row][newState.player.col] == worker_on_dest)
                {
                        if(newState.layout[newState.player.row - 1][newState.player.col] == empty)
                        {
                            newState.layout[newState.player.row - 1][newState.player.col] = worker;
                            newState.layout[newState.player.row][newState.player.col] = destination;
                        }
                        if(newState.layout[newState.player.row - 1][newState.player.col] == destination)
                        {
                            newState.layout[newState.player.row - 1][newState.player.col] = worker_on_dest;
                            newState.layout[newState.player.row][newState.player.col] = destination;
                        }
                }
                break;

            case south:
                if(newState.layout[newState.player.row][newState.player.col] == worker)
                {
                        if(newState.layout[newState.player.row + 1][newState.player.col] == empty)
                        {
                            newState.layout[newState.player.row + 1][newState.player.col] = worker;
                            newState.layout[newState.player.row][newState.player.col] = empty ;
                        }
                        if(newState.layout[newState.player.row + 1][newState.player.col] == destination)
                        {
                            newState.layout[newState.player.row + 1][newState.player.col] = worker_on_dest;
                            newState.layout[newState.player.row][newState.player.col] = empty ;
                        }
                }
                if(newState.layout[newState.player.row][newState.player.col] == worker_on_dest)
                {
                        if(newState.layout[newState.player.row + 1][newState.player.col] == empty)
                        {
                            newState.layout[newState.player.row + 1][newState.player.col] = worker;
                            newState.layout[newState.player.row][newState.player.col] = destination;
                        }
                        if(newState.layout[newState.player.row + 1][newState.player.col] == destination)
                        {
                            newState.layout[newState.player.row + 1][newState.player.col] = worker_on_dest;
                            newState.layout[newState.player.row][newState.player.col] = destination;
                        }
                }
                break;

            case east:
                if(newState.layout[newState.player.row][newState.player.col] == worker)
                {
                        if(newState.layout[newState.player.row][newState.player.col + 1] == empty)
                        {
                            newState.layout[newState.player.row][newState.player.col + 1] = worker;
                            newState.layout[newState.player.row][newState.player.col] = empty ;
                        }
                        if(newState.layout[newState.player.row][newState.player.col + 1] == destination)
                        {
                            newState.layout[newState.player.row][newState.player.col + 1] = worker_on_dest;
                            newState.layout[newState.player.row][newState.player.col] = empty ;
                        }
                }
                if(newState.layout[newState.player.row][newState.player.col] == worker_on_dest)
                {
                        if(newState.layout[newState.player.row][newState.player.col + 1] == empty)
                        {
                            newState.layout[newState.player.row][newState.player.col + 1] = worker;
                            newState.layout[newState.player.row][newState.player.col] = destination;
                        }
                        if(newState.layout[newState.player.row][newState.player.col + 1] == destination)
                        {
                            newState.layout[newState.player.row][newState.player.col + 1] = worker_on_dest;
                            newState.layout[newState.player.row][newState.player.col] = destination;
                        }
                }
                break;

            case west:
                if(newState.layout[newState.player.row][newState.player.col] == worker)
                {
                        if(newState.layout[newState.player.row][newState.player.col - 1] == empty)
                        {
                            newState.layout[newState.player.row][newState.player.col - 1] = worker;
                            newState.layout[newState.player.row][newState.player.col] = empty ;
                        }
                        if(newState.layout[newState.player.row][newState.player.col - 1] == destination)
                        {
                            newState.layout[newState.player.row][newState.player.col - 1] = worker_on_dest;
                            newState.layout[newState.player.row][newState.player.col] = empty ;
                        }
                }
                if(newState.layout[newState.player.row][newState.player.col] == worker_on_dest)
                {
                        if(newState.layout[newState.player.row][newState.player.col - 1] == empty)
                        {
                            newState.layout[newState.player.row][newState.player.col - 1] = worker;
                            newState.layout[newState.player.row][newState.player.col] = destination;
                        }
                        if(newState.layout[newState.player.row][newState.player.col - 1] == destination)
                        {
                            newState.layout[newState.player.row][newState.player.col - 1] = worker_on_dest;
                            newState.layout[newState.player.row][newState.player.col] = destination;
                        }
                }
                break;
     }

    newState = set_up(newState.layout);

    if (new_config2(attempt, newState) && !box_in_corner(newState))
    {
        attempt.push_back(newState) ;
        solve2(attempt, shortest, max_depth) ;
        attempt.pop_back() ;
    }
}

void move_box2 (vector<State>& attempt, vector<State>& shortest, Direction d, int max_depth)
{
    // PRE:
    assert(static_cast<int> (d) >=0 && static_cast<int> (d) < 4);
    assert(attempt.size() > 0);
    // POST: From perspective of the worker position and considering the indicated direction
    // it is checked whether there is a box in front of worker and that this box is allowed to be
    // moved in the same direction as the worker is going. If the criteria are met, the box is moved.
    // After, the function move_player2 is called.
    State newState = attempt[attempt.size()-1];
    switch (d)
     {
            case north:
                if(newState.layout[newState.player.row - 1][newState.player.col] == box)
                {
                        if(newState.layout[newState.player.row - 2][newState.player.col] == empty)
                        {
                            newState.layout[newState.player.row - 2][newState.player.col] = box;
                            newState.layout[newState.player.row - 1][newState.player.col] = empty ;
                        }
                        if(newState.layout[newState.player.row - 2][newState.player.col] == destination)
                        {
                            newState.layout[newState.player.row - 2][newState.player.col] = box_on_dest;
                            newState.layout[newState.player.row - 1][newState.player.col] = empty ;
                        }
                }
                if(newState.layout[newState.player.row - 1][newState.player.col] == box_on_dest)
                {
                        if(newState.layout[newState.player.row - 2][newState.player.col] == empty)
                        {
                            newState.layout[newState.player.row - 2][newState.player.col] = box;
                            newState.layout[newState.player.row - 1][newState.player.col] = destination;
                        }
                        if(newState.layout[newState.player.row - 2][newState.player.col] == destination)
                        {
                            newState.layout[newState.player.row - 2][newState.player.col] = box_on_dest;
                            newState.layout[newState.player.row - 1][newState.player.col] = destination;
                        }
                }
                break;

            case south:
                if(newState.layout[newState.player.row + 1][newState.player.col] == box)
                {
                        if(newState.layout[newState.player.row + 2][newState.player.col] == empty)
                        {
                            newState.layout[newState.player.row + 2][newState.player.col] = box;
                            newState.layout[newState.player.row + 1][newState.player.col] = empty ;
                        }
                        if(newState.layout[newState.player.row + 2][newState.player.col] == destination)
                        {
                            newState.layout[newState.player.row + 2][newState.player.col] = box_on_dest;
                            newState.layout[newState.player.row + 1][newState.player.col] = empty ;
                        }
                }
                if(newState.layout[newState.player.row + 1][newState.player.col] == box_on_dest)
                {
                        if(newState.layout[newState.player.row + 2][newState.player.col] == empty)
                        {
                            newState.layout[newState.player.row + 2][newState.player.col] = box;
                            newState.layout[newState.player.row + 1][newState.player.col] = destination;
                        }
                        if(newState.layout[newState.player.row + 2][newState.player.col] == destination)
                        {
                            newState.layout[newState.player.row + 2][newState.player.col] = box_on_dest;
                            newState.layout[newState.player.row + 1][newState.player.col] = destination;
                        }
                }
                break;

            case east:
                if(newState.layout[newState.player.row][newState.player.col + 1] == box)
                {
                        if(newState.layout[newState.player.row][newState.player.col + 2] == empty)
                        {
                            newState.layout[newState.player.row][newState.player.col + 2] = box;
                            newState.layout[newState.player.row][newState.player.col + 1] = empty ;
                        }
                        if(newState.layout[newState.player.row][newState.player.col + 2] == destination)
                        {
                            newState.layout[newState.player.row][newState.player.col + 2] = box_on_dest;
                            newState.layout[newState.player.row][newState.player.col + 1] = empty ;
                        }
                }
                if(newState.layout[newState.player.row][newState.player.col + 1] == box_on_dest)
                {
                        if(newState.layout[newState.player.row][newState.player.col + 2] == empty)
                        {
                            newState.layout[newState.player.row][newState.player.col + 2] = box;
                            newState.layout[newState.player.row][newState.player.col + 1] = destination;
                        }
                        if(newState.layout[newState.player.row][newState.player.col + 2] == destination)
                        {
                            newState.layout[newState.player.row][newState.player.col + 2] = box_on_dest;
                            newState.layout[newState.player.row][newState.player.col + 1] = destination;
                        }
                }
                break;

            case west:
                if(newState.layout[newState.player.row][newState.player.col - 1] == box)
                {
                        if(newState.layout[newState.player.row][newState.player.col - 2] == empty)
                        {
                            newState.layout[newState.player.row][newState.player.col - 2] = box;
                            newState.layout[newState.player.row][newState.player.col - 1] = empty ;
                        }
                        if(newState.layout[newState.player.row][newState.player.col - 2] == destination)
                        {
                            newState.layout[newState.player.row][newState.player.col - 2] = box_on_dest;
                            newState.layout[newState.player.row][newState.player.col - 1] = empty ;
                        }
                }
                if(newState.layout[newState.player.row][newState.player.col - 1] == box_on_dest)
                {
                        if(newState.layout[newState.player.row][newState.player.col - 2] == empty)
                        {
                            newState.layout[newState.player.row][newState.player.col - 2] = box;
                            newState.layout[newState.player.row][newState.player.col - 1] = destination;
                        }
                        if(newState.layout[newState.player.row][newState.player.col - 2] == destination)
                        {
                            newState.layout[newState.player.row][newState.player.col - 2] = box_on_dest;
                            newState.layout[newState.player.row][newState.player.col - 1] = destination;
                        }
                }
                break;
     }

    newState = set_up(newState.layout);
    move_player2(attempt,shortest,d, newState,max_depth);
}

void solve2( vector<State>& attempt, vector<State>& shortest, int max_depth)
{
    // PRE:
    assert (attempt.size() > 0 && max_depth > 0);
    // POST: Computes the optimal solution, i.e. using least number of moves, to the level in question
    // using depth-first search recursively.

    const int CURRENT = attempt.size() ;
    const int BEST = shortest.size() ;
    State s = attempt[CURRENT-1] ;

    if (BEST > 0 && CURRENT >= BEST)
    {
         return;
    }
    else if (CURRENT > max_depth + 1)
    {
        return;
    }
    else if (is_solved(s))
    {
        shortest = attempt;
        return;
    }
    if (facing_box(s,north) && is_movable(s,north))
            move_box2(attempt,shortest,north,max_depth);

        if (can_go_north(s))
            move_player2(attempt,shortest,north,s,max_depth);

        if (facing_box(s,south) && is_movable(s,south))
            move_box2(attempt,shortest,south,max_depth);

        if (can_go_south(s))
            move_player2(attempt,shortest,south,s,max_depth);

        if (facing_box(s,east) && is_movable(s,east))
            move_box2(attempt,shortest,east,max_depth);

        if (can_go_east(s))
            move_player2(attempt,shortest,east,s,max_depth);

        if (facing_box(s,west) && is_movable(s,west))
            move_box2(attempt,shortest,west,max_depth);

        if (can_go_west(s))
            move_player2(attempt,shortest,west,s,max_depth);
}

/************************************************************************
*  Main + I/O functions
************************************************************************/

void open_file (fstream& file, string file_name)
{
    // PRE:
    assert(file_name.length() > 4);
    // POST: takes care of the file opening giving feedback.
   file.open(file_name.c_str());
   if (file.is_open())
   {
       cout << "File loaded correctly" << endl ;
   }
   else cout << "File not loaded!" << endl ;
}

void read_file(State& start,vector <State>& attempt,fstream& file)
{
    //PRE:
    assert(attempt.size() > 0 && file.is_open());
    //POST:
    // read the file, prints the loaded challenge and set the variables needed for the
    // challenge solving.

    file >> world;
    file.close();
    cout << endl;
    cout << "CHALLENGE:" << endl;
    show_world(world);
    start = set_up(world);
    attempt.push_back(start);
}

void selection_and_print (State& start, vector<State>& attempt, vector<State>& shortest)
{
    //PRE:
    assert(attempt.size() > 0);
    //POST:
    //it takes care of the user choice for solving the challenge and prints the solution with a nice format.

    int max_depth;
    int c = 1;
    char choice;

    cout << endl;
    cout << "How do you want to solve this challenge?" << endl;
    cout << "'a' --> breadth first search" << endl;
    cout << "'b' --> depth first search" << endl;

    cin >> choice;
    cout << endl ;

    switch(choice)
    {
       case 'a':
           cout << "SOLUTION WITH BREADTH FIRST SEARCH:" << endl;
           cout << endl;
           solve(start);
           for(int j = path.size() - 2 ; j >= 0 ; j--)
           {
             cout << "MOVE " << c << endl;
             show_world(path[j].layout);
             c++;
           }
           break;
       case 'b':
           cout << "What should the max depth be?" << endl;
           cin >> max_depth;
           cout << "SOLUTION WITH DEPTH FIRST SEARCH:" << endl;
           cout << endl;
           solve2(attempt,shortest,max_depth); // function names ending with 2 are part 3.

           for (int i = 1 ; i < shortest.size(); i++)
           {
              cout << "MOVE " << i << endl;
              show_world(shortest[i].layout);
           }
           break;
       default:
           cout << "Please, select a valid option!"<< endl ;
           break;
    }
}

int main()
{
    // PRE:
    assert(true);
    //POST: open the file, copies the configuration in the file and ask the user how to solve the challenge.

    fstream file;
    string file_name;
    vector<State> attempt, shortest;

    State start;

    cout << "Insert challenge name: ";

    getline(cin,file_name);

    open_file(file, file_name);
    read_file(start, attempt, file);
    selection_and_print(start, attempt, shortest);

    return 0;
}
