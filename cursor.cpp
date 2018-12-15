using namespace std;
#include "cursor.h"

void set_cursor_position( int column, int line )
{// pre-conditie:
    assert (column >= 0 && column < 80 && line >= 0 && line < 80) ;
//  post-conditie: de cursor is geplaatst op kolom column en rij line (beide geteld vanaf 0)
    COORD coord;
    coord.X = column;
    coord.Y = line;
    SetConsoleCursorPosition( GetStdHandle( STD_OUTPUT_HANDLE ), coord );
}

/*  bron onderstaande code:
    http://www.cplusplus.com/forum/beginner/1988/3/
*/
void cls( )
{// pre-conditie:
    assert (true) ;
/*  post-conditie: het console window is gevuld met spaties en de cursor staat op (0,0) - linksboven.
*/
    COORD coordScreen = { 0, 0 };    // home for the cursor
    DWORD cCharsWritten;
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    DWORD dwConSize;
    HANDLE hConsole = GetStdHandle ( STD_OUTPUT_HANDLE ) ;

    // Get the number of character cells in the current buffer.
    if( !GetConsoleScreenBufferInfo( hConsole, &csbi ))
        return;
    dwConSize = csbi.dwSize.X * csbi.dwSize.Y;

    // Fill the entire screen with blanks.
    if( !FillConsoleOutputCharacter( hConsole, (TCHAR) ' ', dwConSize, coordScreen, &cCharsWritten ))
       return;

    // Get the current text attribute.
    if( !GetConsoleScreenBufferInfo( hConsole, &csbi ))
       return;

    // Set the buffer's attributes accordingly.
    if( !FillConsoleOutputAttribute( hConsole, csbi.wAttributes, dwConSize, coordScreen, &cCharsWritten ))
       return;

   // Put the cursor at its home coordinates.
   SetConsoleCursorPosition( hConsole, coordScreen );
}
