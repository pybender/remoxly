/*

  Serializer
  -----------
  
  Serializes the Guis and Panels. You can add Panels that have 
  Groups contained or only just Groups, or a combination. Because the json
  will be transferred over the network we try to keep it as short
  as posislbe. The generated  JSON has to following structure:


 */
#ifndef REMOXLY_GUI_REMOTE_SERIALIZER_H
#define REMOXLY_GUI_REMOTE_SERIALIZER_H

#include <gui/Remoxly.h>
#include <vector>
#include <string>

extern "C" { 
#  include <jansson.h>
}

namespace rx { 

class Serializer {

 public:
  void addGroup(Group* g);                                           /* adds a group that we need to serialize */
  void addPanel(Panel* p);                                           /* adds a panel that we need to serialize */
  bool serialize(std::string& result);                               /* serializes all added groups and panels and stores the json result in the given param. returns true on success else false. */
  bool canSerialize();                                               /* returns true when groups or panels are added and we can serialize */
  void clear();                                                      /* removes the added panels and groups */

  std::string serializeTask(int task, std::string& value, int id);   /* generates the json for the given task and connection/gui id. */
  bool serializeValueChanged(Widget* w, std::string& json);          /* generates the json string that represents the value for the given widget. */

  /* serializing the values */
  bool serializeValues(std::string& json);                           /* serialize the value of the added groups/panels; we assume that all elements have an unique ID */
  bool serializeValuesForGroup(Group* g, json_t* array);             /* serialize the values for the given group and append them to the array */
  bool serializeValuesForPanel(Panel* p, json_t* array);             /* serialize the values for the given panel and append them ot the array */
  json_t* serializeValueWidget(Widget* w);                           /* serialize the widget */
  json_t* serializeValueSliderInt(Slider<int>* slider);              /* serialize the integer slider value */
  json_t* serializeValueSliderFloat(Slider<float>* slider);          /* serialize the float slider value */
  json_t* serializeValueToggle(Toggle* toggle);                      /* serialize the toggle value */
  json_t* serializeValueColorRGB(ColorRGB* col);                     /* serialize the color value */
  json_t* serializeValueButton(Button* button);                      /* serialize the button value; only used to notify a click */
  json_t* serializeValueText(Text* text);                            /* serialize the text value */

 private:

  /* serialization */
  json_t* serializeGroups(std::vector<Group*>& all);                 /* serializes the given vector with groups */
  json_t* serializeGroup(Group* group);                              /* serializes a specific group */
  json_t* serializePanels(std::vector<Panel*>& all);                 /* serializes all panels and groups inside the given vector */
  json_t* serializePanel(Panel* panel);                              /* serialize the given panel, and the groups contained */
  json_t* serializeSlider(Slider<int>* slider);                      /* serializes a slider of type int */
  json_t* serializeSlider(Slider<float>* slider);                    /* serializes a slider of type float */
  json_t* serializeToggle(Toggle* toggle);                           /* serializes a toggle */
  json_t* serializeColorRGB(ColorRGB* color);                        /* serializes a color rgb */
  json_t* serializeButton(Button* button);                           /* serializes a button */
  json_t* serializeText(Text* text);                                 /* serializes a text field */

  /* utils */
  bool appendToArray(json_t* parent, json_t* child);

 public:
  std::vector<Panel*> panels;
  std::vector<Group*> groups;
};

} // namespace rx 

#endif
