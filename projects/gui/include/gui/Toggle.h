/*

  Toggle 
  ------

  A toggle is a set the given value to true or false when the user
  clicks on the widget. 

 */
#ifndef REMOXLY_GUI_TOGGLE_H
#define REMOXLY_GUI_TOGGLE_H

#include <gui/Widget.h>
#include <gui/Types.h>

namespace rx { 

class Toggle : public Widget {

 public:
  Toggle(std::string label, bool& value);
  void create();
  void onMousePress(float mx, float my, int button, int modkeys);
  void onMouseRelease(float mx, float my, int button, int modkeys);
  void setValue(bool v);

 public:
  bool& value;
};

} // namespace rx 

#endif
