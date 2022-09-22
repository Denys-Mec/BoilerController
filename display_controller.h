#ifndef DISPLAY_CONTROLLER 
#define DISPLAY_CONTROLLER 

#include "GyverButton.h"

struct DisplayController
{
	GButton	*top;
	GButton *bottom;
	GButton *left;
	GButton *right;
	GButton *select;

  DisplayController(int8_t Top, int8_t Bottom, int8_t Left, int8_t Right, int8_t Select)
  {
    top = new GButton(Top);
    bottom = new GButton(Bottom);
    left = new GButton(Left);
    right = new GButton(Right);
    select = new GButton(Select);
  }
};


#endif
