
// ------------------------------------------------------------------------------
// H E A D E R
// ------------------------------------------------------------------------------

#ifndef REMOXLY_RENDER_GL_H
#define REMOXLY_RENDER_GL_H

// -------------------------------------------

#define RENDER_GL2 1                         /* Use GL2, legacy support */
#define RENDER_GL3 2                         /* Use GL3+ */

#if !defined(RENDER_GL)
#  define RENDER_GL RENDER_GL3
#endif

#if RENDER_GL == RENDER_GL3
#  define BITMAP_FONT_GL BITMAP_FONT_GL3
#elif RENDER_GL == RENDER_GL2
#  define BITMAP_FONT_GL BITMAP_FONT_GL2
#endif

#if defined(__APPLE__) && RENDER_GL == RENDER_GL2
#  define glGenVertexArrays glGenVertexArraysAPPLE
#  define glBindVertexArray glBindVertexArrayAPPLE
#endif

// -------------------------------------------

#define TEXT_INPUT_IMPLEMENTATION
#define BITMAP_FONT_IMPLEMENTATION
#include <bitmapfont/BitmapFont.h>
#include <textinput/TextInput.h>

/* @todo - pick a font in RenderGL header */
#include <gui/fonts/FreePixel.h>
#include <gui/fonts/FontAwesome.h>
#include <gui/fonts/SourceCode.h>
#include <gui/fonts/Arial.h>

#include <gui/Render.h>
#include <assert.h>

// -------------------------------------------

#if RENDER_GL == RENDER_GL2
static const char* GUI_RENDER_VS = ""
  "#version 110\n"
  "uniform mat4 u_pm;"
  "attribute vec4 a_pos;"
  "attribute vec4 a_color;"
  "varying vec4 v_color;"
  "void main() {"
  "  gl_Position = u_pm * a_pos;"
  "  v_color = a_color;"
  "}"
  "";

static const char* GUI_RENDER_FS = ""
  "#version 110\n"
  "varying vec4 v_color;"
  "void main() {"
  "  gl_FragColor = v_color;"
  "}"
  "";

// Position + TexCoord 
// -------------------------------------------
static const char* GUI_RENDER_PT_VS = ""
  "#version 110\n"
  "uniform mat4 u_pm;"
  "attribute vec4 a_pos;"
  "attribute vec2 a_tex;"
  "varying vec2 v_tex;"
  ""
  "void main() {"
  "  gl_Position = u_pm * a_pos;"
  "  v_tex = a_tex;"
  "}"
  "";

static const char* GUI_RENDER_PT_FS = ""
  "#version 110\n"
  "uniform sampler2D u_tex;"
  "varying vec2 v_tex;"
  ""
  "void main() {"
  "  gl_FragColor = texture2D(u_tex, v_tex);"
  "}"
  "";

#endif

// -------------------------------------------
#if RENDER_GL == RENDER_GL3
static const char* GUI_RENDER_VS = ""
  "#version 150\n"
  "uniform mat4 u_pm;"
  "in vec4 a_pos;"
  "in vec4 a_color;"
  "out vec4 v_color;"
  "void main() {"
  "  gl_Position = u_pm * a_pos;"
  "  v_color = a_color;"
  "}"
  "";

static const char* GUI_RENDER_FS = ""
  "#version 150\n"
  "in vec4 v_color;"
  "out vec4 fragcolor;"
  "void main() {"
  "  fragcolor = v_color;"
  "}"
  "";

// Position + TexCoord 
// -------------------------------------------
static const char* GUI_RENDER_PT_VS = ""
  "#version 150\n"
  "uniform mat4 u_pm;"
  "in vec4 a_pos;"
  "in vec2 a_tex;"
  "out vec2 v_tex;"
  ""
  "void main() {"
  "  gl_Position = u_pm * a_pos;"
  "  v_tex = a_tex;"
  "}"
  "";

static const char* GUI_RENDER_PT_FS = ""
  "#version 150\n"
  "uniform sampler2D u_tex;"
  "in vec2 v_tex;"
  "out vec4 fragcolor;"
  ""
  "void main() {"
  "  fragcolor = vec4(1.0, 0.0, 0.0, 1.0);"
  "  fragcolor = texture(u_tex, v_tex);"
  "}"
  "";
#endif
// -------------------------------------------

// position + colors
struct GuiVertexPC {

public:
  GuiVertexPC();
  GuiVertexPC(float x, float y, float r, float g, float b, float a);
  GuiVertexPC(float x, float y, float* col);
  void setPos(float x, float y);
  void setColor(float r, float g, float b, float a);
  const float* ptr() { return &pos[0]; } 
  void print();

public:
  float pos[2];
  float color[4];
};

// -------------------------------------------

// position + texcoords
struct GuiVertexPT {

public:
  GuiVertexPT();
  GuiVertexPT(float x, float y, float u, float v);
  void setPos(float x, float y);
  void setTexCoord(float u, float v);
  const float* ptr() { return &pos[0]; } 

public:
  float pos[2];
  float tex[2];
};

// -------------------------------------------

class TextureInfoGL : public TextureInfo {
 public:
  TextureInfoGL(GLenum type, int id);
  int getWidth();
  int getHeight();

 public:
  GLuint id;
  GLenum type;
  int tex_w;
  int tex_h;
};

// -------------------------------------------

struct TextureDrawInfo {

  TextureDrawInfo();

  int offset;
  int count;
  TextureInfoGL* info;
};

// -------------------------------------------

GLuint gui_create_shader(GLenum type, const char* src);
GLuint gui_create_program(GLuint vert, GLuint frag, int natts = 0, const char** atts = NULL);
void gui_print_program_link_info(GLuint prog);
void gui_print_shader_compile_info(GLuint shader);
void gui_ortho(float l, float r, float b, float t, float n, float f, float* dest);

// -------------------------------------------

class RenderGL : public Render {

 public:
  RenderGL(int gl = RENDER_GL3);

  void update();
  void draw();
  void resize(int w, int h);

  void getWindowSize(int& ww, int& wh);
  void beginScissor();
  void scissor(int sx, int sy, int sw, int sh);
  void endScissor();

  void clear();                                                                             /* removes all vertices, and offset data */
  void writeText(float x, float y, std::string text, float* color);                         /* write some text */
  void writeNumber(float x, float y, std::string number, float* color);                     /* write a number value; we right align it */
  void writeIcon(float x, float y, unsigned int icon, float* color);                        /* write an icon. see Types.h for the available icons */
  void addRectangle(float x, float y, float w, float h, float* color, bool filled = true);  /* draw a rectangle at x/y with w/h and given color; must have 4 elements */
  void addRectangle(float x, float y, float w, float h, TextureInfo* texinfo);              /* draw a rectangle at x/y with w/h for the given texture */

  void enableTextInput(float x, float y, float maxw, std::string value, float* color); 
  void enableNumberInput(float x, float y, float maxw, std::string value, float* color);
  void disableNumberInput();
  void disableTextInput();
  void getNumberInputValue(std::string& result);
  void getTextInputValue(std::string& result);
  bool getIconSize(unsigned int id, int& w, int& h);

  void onCharPress(unsigned int key); /* gets called when a key is pressed (called by an editable widget) */
  void onKeyPress(int key, int mods); /* gets called for special keys (called by an editable widget) */

 private: 
  void updatePositionColorBuffers();
  void updatePositionTexCoordBuffers();

 public:

  /* opengl */
  int gl_version;                                              /* passed into the constructor; either RENDER_GL2 or RENDER_GL3 */
  GLint viewport[4];                                           /* contains the viewport size; is e.g. used in getWindowSize() */
  static bool is_initialized;                                  /* static member, is set to true when the shaders/prog has been created */

  /* GuiVertexPC: lines + background */
  GLuint vbo_pc;                                               /* the vbo that keeps the vertices for the position + colors */
  GLuint vao_pc;                                               /* vao to keep state of the vbo for position + colors */
  static GLuint prog_pc;                                       /* the shader program that we use to render everything */
  static GLuint vert_pc;                                       /* the vertex shader for the gui */
  static GLuint frag_pc;                                       /* the fragment shader for the gui */

  /* GuiVertexPT: textures */
  GLuint vbo_pt;                                               /* vbo for the position + texcoord buffers */
  GLuint vao_pt;                                               /* vao for the position + textcoord buffers */
  static GLuint prog_pt;                                       /* shader program that renders GuiVertexPT */
  static GLuint vert_pt;                                       /* vertex shader that renders GuiVertexPT */
  static GLuint frag_pt;                                       /* fragment shader that renders GuiVertexPT */

  /* GuiVertexPC buffer info */
  bool needs_update_pc;                                        /* set to true whenever we need to update the vbo for the position + color type*/
  size_t bytes_allocated_pc;                                   /* how many bytes we've allocated in the vbo for the position + color type */
  std::vector<GuiVertexPC> vertices_pc;                        /* vertices for that make up the gui (for color + position) */
  std::vector<GLint> bg_offsets;                               /* offsets of the different background elements */
  std::vector<GLsizei> bg_counts;                              /* vertex counts for the background elements */
  std::vector<GLint> fg_offsets;                               /* offsets of the foreground elements */
  std::vector<GLsizei> fg_counts;                              /* vetex counts for the foreground elements */

  /* GuiVertexPT buffer info */
  bool needs_update_pt;                                        /* is set to true whenever we need t update the pos/tex vertices */
  size_t bytes_allocated_pt;                                   /* the number of bytes allocated in the GuiVertexPT buffer */
  std::vector<GuiVertexPT> vertices_pt;                        /* the vertices that we use to draw a texture */
  std::vector<TextureDrawInfo> texture_draws;                  /* keeps information about the textures we need to draw */

  /* fonts */
  Arial text_font;
  SourceCode number_font;
  FontAwesome icon_font;
  Arial text_input_font;                                       /* we need to use another font object for the text input because the text input clears all vertices, so it cannot share the text_font. */
  SourceCode number_input_font;                                /* we need to use another font object for the number font because the number font clears all vertices, so it cannot share the number_font. */
  TextInput text_input;
  TextInput number_input;
};

#endif

// ------------------------------------------------------------------------------
// I M P L E M E N T A T I O N
// ------------------------------------------------------------------------------

#if defined(REMOXLY_IMPLEMENTATION)

// -------------------------------------------

GLuint RenderGL::prog_pc = 0;
GLuint RenderGL::vert_pc = 0;
GLuint RenderGL::frag_pc = 0;
GLuint RenderGL::prog_pt = 0;
GLuint RenderGL::vert_pt = 0;
GLuint RenderGL::frag_pt = 0;
bool RenderGL::is_initialized = false;

// -------------------------------------------

RenderGL::RenderGL(int gl) 
  :gl_version(gl)
  ,vbo_pc(0)
  ,vao_pc(0)
  ,bytes_allocated_pc(0)
  ,bytes_allocated_pt(0)
  ,needs_update_pc(false)
  ,needs_update_pt(false)
  ,text_input(0.0f, 0.0f, 0.0f, text_input_font)
  ,number_input(0.0f, 0.0f, 0.0f, number_input_font)
{
    
  // ortho projection 
  float pm[16];
  viewport[0] = viewport[1] = viewport[2] = viewport[3] = 0;
  glGetIntegerv(GL_VIEWPORT, viewport);
  gui_ortho(0.0f, viewport[2], viewport[3], 0.0f, 0.0f, 100.0f, pm);

  if(!is_initialized) {

    // shader for pos + color
    const char* atts_pc[] = { "a_pos", "a_color" };
    vert_pc = gui_create_shader(GL_VERTEX_SHADER, GUI_RENDER_VS);
    frag_pc = gui_create_shader(GL_FRAGMENT_SHADER, GUI_RENDER_FS);
    prog_pc = gui_create_program(vert_pc, frag_pc, 2, atts_pc);

    // shader for pos + texcoord
    const char* atts_pt[] = { "a_pos", "a_tex" } ;
    vert_pt = gui_create_shader(GL_VERTEX_SHADER, GUI_RENDER_PT_VS);
    frag_pt = gui_create_shader(GL_FRAGMENT_SHADER, GUI_RENDER_PT_FS);
    prog_pt = gui_create_program(vert_pt, frag_pt, 2, atts_pt);

    glUseProgram(prog_pc);
    glUniformMatrix4fv(glGetUniformLocation(prog_pc, "u_pm"), 1, GL_FALSE, pm);

    glUseProgram(prog_pt);
    glUniformMatrix4fv(glGetUniformLocation(prog_pt, "u_pm"), 1, GL_FALSE, pm);
    glUniform1i(glGetUniformLocation(prog_pt, "u_tex"), 0);

    is_initialized = true;
  }

  // pos + color vao,vbo
  glGenVertexArrays(1, &vao_pc);
  glBindVertexArray(vao_pc);

  glGenBuffers(1, &vbo_pc);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_pc);

  glEnableVertexAttribArray(0); // pos
  glEnableVertexAttribArray(1); // color

  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(GuiVertexPC), (GLvoid*)0); // pos
  glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(GuiVertexPC), (GLvoid*)8); // col

  // pos + texcoord vao,vbo
  glGenVertexArrays(1, &vao_pt);
  glBindVertexArray(vao_pt);

  glGenBuffers(1, &vbo_pt);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_pt);

  glEnableVertexAttribArray(0); // pos
  glEnableVertexAttribArray(1); // texcoord

  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(GuiVertexPT), (GLvoid*)0); // pos
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(GuiVertexPT), (GLvoid*)8); // texcoord

  if(!text_font.setup()) {
    printf("Error: cannot setup text font.\n");
    return;
  }

  if(!icon_font.setup()) {
    printf("Error: cannot setup icon font.\n");
    return;
  }

  if(!number_font.setup()) {
    printf("Error: cannot setup number font.\n");
    return;
  }

  if(!text_input_font.setup()) {
    printf("Error: cannot setup the text input font.\n");
    return;
  }

  if(!number_input_font.setup()) {
    printf("Error: cannot setup the number input font.\n");
    return;
  }

  number_input.align = BITMAP_FONT_ALIGN_RIGHT;
  text_input.align = BITMAP_FONT_ALIGN_LEFT;

  //  num_instances++;
}

void RenderGL::getWindowSize(int& ww, int& wh) {
  ww = viewport[2];
  wh = viewport[3];
}

void RenderGL::update() {
  
  updatePositionColorBuffers();
  updatePositionTexCoordBuffers();
}

void RenderGL::updatePositionColorBuffers() {

 if(!vertices_pc.size()) {
    return;
  }
  
  if(!needs_update_pc) {
    return;
  }

  glBindBuffer(GL_ARRAY_BUFFER, vbo_pc);

  size_t needed = vertices_pc.size() * sizeof(GuiVertexPC);

  if(needed > bytes_allocated_pc) {
    glBufferData(GL_ARRAY_BUFFER, needed, vertices_pc[0].ptr(), GL_STREAM_DRAW);
    bytes_allocated_pc = needed;
  }
  else {
    glBufferSubData(GL_ARRAY_BUFFER, 0, needed, vertices_pc[0].ptr());
  }

  needs_update_pc = false;
}

void RenderGL::updatePositionTexCoordBuffers() {

  if(!vertices_pt.size()) {
    return;
  }

  if(!needs_update_pt) {
    return;
  }

  glBindBuffer(GL_ARRAY_BUFFER, vbo_pt);

  size_t needed = vertices_pt.size() * sizeof(GuiVertexPT);

  if(needed > bytes_allocated_pt) {
    glBufferData(GL_ARRAY_BUFFER, needed, vertices_pt[0].ptr(), GL_STREAM_DRAW);
    bytes_allocated_pt = needed;
  }
  else {
    glBufferSubData(GL_ARRAY_BUFFER, 0, needed, vertices_pt[0].ptr());
  }

  needs_update_pt = false;
}

void RenderGL::draw() {

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  if(vertices_pc.size()) {

    glUseProgram(prog_pc);
    glBindVertexArray(vao_pc);
    glMultiDrawArrays(GL_TRIANGLES, &bg_offsets[0], &bg_counts[0], bg_counts.size());
    glMultiDrawArrays(GL_LINE_STRIP, &fg_offsets[0], &fg_counts[0], fg_counts.size());
  }

  if(texture_draws.size()) {

    glUseProgram(prog_pt);
    glBindVertexArray(vao_pt);
    glActiveTexture(GL_TEXTURE0);

    for(std::vector<TextureDrawInfo>::iterator it = texture_draws.begin(); it != texture_draws.end(); ++it) {
      TextureDrawInfo& tex = *it;
      glBindTexture(tex.info->type, tex.info->id);
      glDrawArrays(GL_TRIANGLES, tex.offset, tex.count);
    }
  }

  glBlendFunc(GL_ONE, GL_ONE);
  icon_font.draw();
  text_font.draw();
  number_font.draw();
  text_input.draw();
  number_input.draw();

  glDisable(GL_BLEND);
}

void RenderGL::resize(int w, int h) {

  float pm[16];
  gui_ortho(0.0f, w, h, 0.0f, 0.0f, 100.0f, pm);

  glUseProgram(prog_pc);
  glUniformMatrix4fv(glGetUniformLocation(prog_pc, "u_pm"), 1, GL_FALSE, pm);

  glUseProgram(prog_pt);
  glUniformMatrix4fv(glGetUniformLocation(prog_pt, "u_pm"), 1, GL_FALSE, pm);
  
  icon_font.resize(w, h);
  text_font.resize(w, h);
  number_font.resize(w, h);
  text_input_font.resize(w, h);
  number_input_font.resize(w, h);

  viewport[2] = w;
  viewport[3] = h;
}

void RenderGL::beginScissor() {
  glEnable(GL_SCISSOR_TEST);
}

void RenderGL::scissor(int sx, int sy, int sw, int sh) {
  glScissor(sx, sy, sw, sh);
}

void RenderGL::endScissor() {
  glDisable(GL_SCISSOR_TEST);
}

void RenderGL::clear() {

  vertices_pc.clear();
  bg_offsets.clear();
  bg_counts.clear();
  fg_offsets.clear();
  fg_counts.clear();

  vertices_pt.clear();
  texture_draws.clear();
  
  icon_font.clear();
  text_font.clear();
  number_font.clear();
  text_input_font.clear();
  number_input_font.clear();
}

void RenderGL::onCharPress(unsigned int key) {

  if(text_input.mode != TI_MODE_DISABLED) {
    text_input.onCharPress(key);
  }

  if(number_input.mode != TI_MODE_DISABLED) {
    number_input.onCharPress(key);
  }
}

void RenderGL::onKeyPress(int key, int mods) {

  if(text_input.mode != TI_MODE_DISABLED) {
    text_input.onKeyPress(key, mods);
  }

  if(number_input.mode != TI_MODE_DISABLED) {
    number_input.onKeyPress(key, mods);
  }
}

void RenderGL::writeText(float x, float y, std::string text, float* color) {

  text_font.setColor(color[0], color[1], color[2], color[3]);
  text_font.write(x, y, text, BITMAP_FONT_ALIGN_LEFT);
}

void RenderGL::writeNumber(float x, float y, std::string number, float* color) {

  number_font.setColor(color[0], color[1], color[2], color[3]);
  number_font.write(x, y, number, BITMAP_FONT_ALIGN_RIGHT);
}

void RenderGL::writeIcon(float x, float y, unsigned int icon, float* color) {
  
  icon_font.setColor(color[0], color[1], color[2], color[3]);
  icon_font.write(x, y, icon);
}

void RenderGL::enableTextInput(float x, float y, float maxw, std::string value, float* color) {

  text_input.x = x;
  text_input.y = y;
  text_input.w = maxw;

  text_input.clear(); // @todo this might be a duplicate call, when a widget makes its state editable RenderGL::clear() will be called too.
  text_input.enable();
  text_input.setValue(value);
  text_input.select();
}

void RenderGL::disableTextInput() {
  text_input.clear();
  text_input.disable();
}

void RenderGL::getTextInputValue(std::string& result) {
  result = text_input.getValue();
}

void RenderGL::disableNumberInput() {
  number_input.clear();
  number_input.disable();
}

void RenderGL::getNumberInputValue(std::string& result) {
  result = number_input.getValue();
}

void RenderGL::enableNumberInput(float x, float y, float maxw, std::string value, float* color) {
  number_input.x = x;
  number_input.y = y;
  number_input.w = maxw;
  
  number_input.clear();
  number_input.enable();
  number_input.setValue(value);
  number_input.select();
}

bool RenderGL::getIconSize(unsigned int id, int& ww, int& hh) {

  Character result;
  ww = 0;
  hh = 0;

  if(!icon_font.getChar(id, result)) {
    return false;
  }

  ww = result.width;
  hh = result.height;

  return true;
}

void RenderGL::addRectangle(float x, float y, float w, float h, float* color, bool filled) {

  GuiVertexPC a(x, y + h, color);     // bottom left
  GuiVertexPC b(x + w, y + h, color); // bottom right
  GuiVertexPC c(x + w, y, color);     // top right 
  GuiVertexPC d(x, y, color);         // top left
  
  if(filled) { 
    bg_offsets.push_back(vertices_pc.size());

    vertices_pc.push_back(a);
    vertices_pc.push_back(b);
    vertices_pc.push_back(c);

    vertices_pc.push_back(a);
    vertices_pc.push_back(c);
    vertices_pc.push_back(d);

    bg_counts.push_back(vertices_pc.size() - bg_offsets.back());
  }
  else {
    fg_offsets.push_back(vertices_pc.size());
    vertices_pc.push_back(a);
    vertices_pc.push_back(b);
    vertices_pc.push_back(c);
    vertices_pc.push_back(d);
    vertices_pc.push_back(a);
    fg_counts.push_back(vertices_pc.size() - fg_offsets.back());
  }

  needs_update_pc = true;
}

void RenderGL::addRectangle(float x, float y, float w, float h, TextureInfo* tex) {

  GuiVertexPT a(x, y + h, 0.0f, 1.0f);     // bottom left
  GuiVertexPT b(x + w, y + h, 1.0f, 1.0f); // bottom right
  GuiVertexPT c(x + w, y, 1.0f, 0.0f);     // top right
  GuiVertexPT d(x, y, 0.0f, 0.0f);         // top left;

  TextureDrawInfo draw_info;
  draw_info.offset = vertices_pt.size();
  draw_info.info = static_cast<TextureInfoGL*>(tex);


  vertices_pt.push_back(a);
  vertices_pt.push_back(b);
  vertices_pt.push_back(c);
  vertices_pt.push_back(a);
  vertices_pt.push_back(c);
  vertices_pt.push_back(d);

  draw_info.count = vertices_pt.size() - draw_info.offset;

  texture_draws.push_back(draw_info);

  needs_update_pt = true;
}

// -------------------------------------------

GuiVertexPC::GuiVertexPC() {
  setPos(0.0f, 0.0f);
  setColor(1.0f, 1.0f, 1.0f, 1.0f);
}

GuiVertexPC::GuiVertexPC(float x, float y, float* col) {
  setPos(x, y);
  setColor(col[0], col[1], col[2], col[3]);
}

GuiVertexPC::GuiVertexPC(float x, float y, float r, float g, float b, float a) {
  setPos(x, y);
  setColor(r,g,b,a);
}

void GuiVertexPC::setPos(float x, float y) {
  pos[0] = x;
  pos[1] = y;
}

void GuiVertexPC::setColor(float r, float g, float b, float a) {
  color[0] = r;
  color[1] = g;
  color[2] = b;
  color[3] = a;
}

void GuiVertexPC::print() {
  printf("x: %3.2f, y: %3.2f, r: %3.2f, g: %3.2f, b: %3.2f a: %3.2f\n", pos[0], pos[1], color[0], color[1], color[2], color[3]);
}

// -------------------------------------------

GuiVertexPT::GuiVertexPT() {
  setPos(0.0f, 0.0f);
  setTexCoord(0.0f, 0.0f);
}

GuiVertexPT::GuiVertexPT(float x, float y, float u, float v) {
  setPos(x,y);
  setTexCoord(u,v);
}

void GuiVertexPT::setPos(float x, float y) {
  pos[0] = x;
  pos[1] = y;
}

void GuiVertexPT::setTexCoord(float u, float v) {
  tex[0] = u;
  tex[1] = v;
}

// -------------------------------------------

TextureDrawInfo::TextureDrawInfo()
  :offset(0)
  ,count(0)
  ,info(NULL)
{
}

// -------------------------------------------

TextureInfoGL::TextureInfoGL(GLenum type, int id)
  :id(id)
  ,type(type)
  ,tex_w(0)
  ,tex_h(0)
{

  glBindTexture(type, id);
  glGetTexLevelParameteriv(type, 0, GL_TEXTURE_WIDTH, &tex_w);
  glGetTexLevelParameteriv(type, 0, GL_TEXTURE_HEIGHT, &tex_h);
}

int TextureInfoGL::getWidth() {
  return tex_w;
}

int TextureInfoGL::getHeight() {
  return tex_h;
}

// Utils
// -------------------------------------------

GLuint gui_create_shader(GLenum type, const char* src) {
  GLuint s = glCreateShader(type);
  glShaderSource(s, 1, &src,  NULL);
  glCompileShader(s);
  gui_print_shader_compile_info(s);
  return s;
}

extern GLuint gui_create_program(GLuint vert, GLuint frag, int natts, const char** atts) {
  GLuint prog = glCreateProgram();
  glAttachShader(prog, vert);
  glAttachShader(prog, frag);

  for(int i = 0; i < natts; ++i) {
    glBindAttribLocation(prog, i, atts[i]);
  }

  glLinkProgram(prog);

  gui_print_program_link_info(prog);

  return prog;
}

extern void gui_print_program_link_info(GLuint program) {
  GLint status = 0;
  GLint count = 0;
  GLchar* error = NULL;
  GLsizei nchars = 0;
  glGetProgramiv(program, GL_LINK_STATUS, &status);
  if(!status) {
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &count);
    if(count > 0) {
      error = new GLchar[count];
      glGetProgramInfoLog(program, 2048, &nchars, error);
      printf("------\n");
      printf("%s\n", error);
      printf("------\n");
      delete[] error;
      error = NULL;
      assert(0);
    }
  }
}

extern void gui_print_shader_compile_info(GLuint shader) {
  GLint status = 0;
  GLint count = 0;
  GLchar* error = NULL;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
  if(!status) {
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &count);
    if(count > 0) {
      error = new GLchar[count];
      glGetShaderInfoLog(shader, count, NULL, error);
      printf("------\n");
      printf("%s\n", error);
      printf("------\n");
      delete[] error;
      error = NULL;
      assert(0);
    }
  }
}

void gui_ortho(float l, float r, float b, float t, float n, float f, float* dest) {
  dest[0] = (2.0f / (r - l));
  dest[1] = 0.0f;
  dest[2] = 0.0f;
  dest[3] = 0.0f;

  dest[4] = 0.0f;
  dest[5] = (2.0f / (t - b));
  dest[6] = 0.0f;
  dest[7] = 0.0f;
  
  dest[8] = 0.0f; 
  dest[9] = 0.0f;
  dest[10] = (-2.0f / (f - n));
  dest[11] = 0.0f;

  dest[12] = - ((r + l) / (r - l));
  dest[13] = - ((t + b) / (t - b));
  dest[14] = - ((f + n) / (f - n));
  dest[15] = 1.0f;
}

#endif
