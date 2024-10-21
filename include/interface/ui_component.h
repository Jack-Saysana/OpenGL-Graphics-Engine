#ifndef __ENGINE_UI_COMPONENT_H__
#define __ENGINE_UI_COMPONENT_H__

#include "./structs/ui_component_str.h"
#include "./structs/font_str.h"

int init_ui(char *quad_path, char *ui_vs, char *ui_fs, char *text_vs,
            char *text_fs);
int free_ui();

int import_font(char *bin_path, char *tex_path, F_GLYPH **dest);

UI_COMP *add_ui_comp(UI_COMP *parent, vec2 pos, float width, float height,
                     int options);
int render_ui();

void set_ui_pos(UI_COMP *comp, vec2 pos);
void set_ui_width(UI_COMP *comp, float width);
void set_ui_height(UI_COMP *comp, float height);
void set_manual_layer(UI_COMP *comp, float layer);
void disable_manual_layer(UI_COMP *comp);
void set_ui_pivot(UI_COMP *comp, PIVOT pivot);
void set_ui_display(UI_COMP *comp, int display);
void set_ui_text(UI_COMP *comp, char *str, float line_height,
                 TEXT_ANCHOR txt_anc, F_GLYPH *font, vec3 col);
void update_ui_text(UI_COMP *comp, char *text);
void set_ui_text_col(UI_COMP *comp, vec3 col);
void set_ui_texture(UI_COMP *comp, char *path);
void set_ui_texture_unit(UI_COMP *comp, unsigned int tex);
void set_ui_options(UI_COMP *compc, int options);
void set_ui_enabled(UI_COMP *comp, int enabled);
void set_ui_on_click(UI_COMP *comp, void (*cb)(UI_COMP *, void *), void *);
void set_ui_on_release(UI_COMP *comp, void (*cb)(UI_COMP *, void *), void *);
void set_ui_on_hover(UI_COMP *comp, void (*cb)(UI_COMP *, void *), void *);
void set_ui_no_hover(UI_COMP *comp, void (*cb)(UI_COMP *, void *), void *);

#endif
