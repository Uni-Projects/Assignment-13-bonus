#include <iostream>
#include <cassert>
#include <windows.h>

/* set_cursor_position (column, line)
   Move the cursor to the given location in which column is the number
   of columns and line the number of lines (both starting at 0, from the left-top of
   the console window.)

   preconditions:
   assert (column >= 0 && column < 80 && line >= 0 && line < 80)

   postconditions:
   the cursor has moved to (column,line)
*/
void set_cursor_position ( int column, int line ) ;

/* cls ( )
   Wipe the console window 'clean' with space characters and move the cursor to (0,0) - left top.

   preconditions:
   assert (true)

   postconditions:
   the console window contains only space characters and the cursor is at (0,0).
*/
void cls ( ) ;
