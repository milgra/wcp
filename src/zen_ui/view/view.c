#ifndef view_h
#define view_h

#include "wm_event.c"
#include "zc_bm_rgba.c"
#include "zc_util2.c"
#include "zc_vec2.c"
#include "zc_vector.c"

typedef enum _laypos_t // layout position
{
    LP_STATIC = 0,
    LP_FIXED,
    LP_ABSOLUTE,
} laypos_t;

typedef enum _laydis_t // layout display
{
    LD_NONE = 0,
    LD_FLEX,
} laydis_t;

typedef enum _flexdir_t // flexdir
{
    FD_ROW = 0,
    FD_COL,
} flexdir_t;

typedef enum _itemalign_t // flexdir
{
    IA_NONE = 0,
    IA_CENTER,
} itemalign_t;

typedef enum _cjustify_t // justify content
{
    JC_NONE = 0,
    JC_CENTER,
} cjustify_t;

typedef struct _vstyle_t vstyle_t; // view style
struct _vstyle_t
{
    /* css dimension */

    int width;
    int height;

    float w_per;
    float h_per;

    /* css position */

    int top;
    int left;
    int right;
    int bottom;

    int margin;
    int margin_top;
    int margin_left;
    int margin_right;
    int margin_bottom;

    /* css decoration */

    int border_radius;

    int shadow_h;
    int shadow_w;
    int shadow_blur;
    int shadow_color;

    uint32_t background_color;
    char*    background_image;

    /* css layout */

    laypos_t    position;
    laydis_t    display;
    flexdir_t   flexdir;
    itemalign_t itemalign;
    cjustify_t  cjustify;

    /* css text */

    char*    font_family;
    float    font_size;
    uint32_t color;
    int      text_align;
    float    line_height;
    int      word_wrap;

    /* non-css */

    char masked; /* view should be used as mask for subviews? OVERFLOW */
    char unmask; /* masking should be stopped, helper variable */
};

typedef enum _texst_t // texture loading state
{
    TS_BLANK,   /* texture is empty */
    TS_PENDING, /* texture is under generation */
    TS_READY,   /* texture is generated */
    TS_STORED,  /* texture is stored in texmap */
} texst_t;

typedef enum _textype_t
{
    TT_MANAGED,
    TT_EXTERNAL
} textype_t;

typedef struct _texture_t
{
    textype_t type; /* managed or external */
    uint32_t  page; /* texture page */
    float     alpha;
    char      resizable; /* if view is resized initiate texture re-rendering */

    // internal texture

    texst_t    state;         /* render state of texture */
    bm_rgba_t* bitmap;        /* texture bitmap */
    char       changed;       /* texture is changed */
    char       alpha_changed; /* alpha channel is changed */

    // decoration

    char full;
    char blur;
} texture_t;

typedef struct _frame_t
{
    r2_t local;  // local position
    r2_t global; // global position
    r2_t region; // region to show
    char pos_changed;
    char dim_changed;
    char reg_changed;
} frame_t;

typedef struct _view_t view_t;
struct _view_t
{
    char exclude; /* view should be displayed? */

    char needs_key;   /* accepts key events */
    char needs_text;  /* accepts text events */
    char needs_time;  /* accepts time events */
    char needs_touch; /* accepts touch events */

    char blocks_key;    /* blocks key events */
    char blocks_touch;  /* blocks touch events */
    char blocks_scroll; /* blocks scroll events */

    char* id;        /* identifier for handling view */
    char* class;     /* css class(es) */
    char*    script; /* script */
    char*    type;   /* html type (button,checkbox) */
    vec_t*   views;  /* subviews */
    view_t*  parent; /* parent view */
    uint32_t index;  /* depth */

    frame_t   frame;
    vstyle_t  style;
    texture_t texture;

    void (*handler)(view_t*, ev_t); /* view handler for view */
    void (*tex_gen)(view_t*);       /* texture generator for view */
    void* handler_data;             /* data for event handler */
    void* tex_gen_data;             /* data for texture generator */
};

view_t* view_new(char* id, r2_t frame);
void    view_set_type(view_t* view, char* type);
void    view_set_class(view_t* view, char* class);
void    view_set_script(view_t* view, char* script);
void    view_add_subview(view_t* view, view_t* subview);
void    view_remove_subview(view_t* view, view_t* subview);
void    view_insert_subview(view_t* view, view_t* subview, uint32_t index);
void    view_remove_from_parent(view_t* view);
void    view_set_parent(view_t* view, view_t* parent);

void    view_evt(view_t* view, ev_t ev); /* general event, sending to all views */
void    view_coll_touched(view_t* view, ev_t ev, vec_t* queue);
view_t* view_get_subview(view_t* view, char* id);
void    view_gen_texture(view_t* view);
void    view_set_masked(view_t* view, char masked);
void    view_set_frame(view_t* view, r2_t frame);
void    view_set_region(view_t* view, r2_t frame);
void    view_set_style(view_t* view, vstyle_t style);
void    view_set_block_touch(view_t* view, char block, char recursive);
void    view_set_texture_bmp(view_t* view, bm_rgba_t* tex);
void    view_set_texture_page(view_t* view, uint32_t page);
void    view_set_texture_type(view_t* view, textype_t type);
void    view_set_texture_alpha(view_t* view, float alpha, char recur);
void    view_invalidate_texture(view_t* view);

void view_describe(void* pointer, int level);
void view_desc(void* pointer, int level);
void view_desc_style(vstyle_t l);
void view_calc_global(view_t* view);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "views.c"
#include "zc_cstring.c"
#include "zc_log.c"
#include "zc_memory.c"
#include <limits.h>

int view_cnt = 0;

void view_del(void* pointer)
{
    view_t* view = (view_t*) pointer;

    MDEL(views.names, view->id);

    if (view->style.background_image != NULL) REL(view->style.background_image);
    if (view->style.font_family != NULL) REL(view->style.font_family);

    if (view->handler_data) REL(view->handler_data);
    if (view->tex_gen_data) REL(view->tex_gen_data);

    if (view->texture.bitmap) REL(view->texture.bitmap); // not all views has texture
    if (view->class) REL(view->class);
    if (view->type) REL(view->type);
    if (view->script) REL(view->script);

    REL(view->id);
    REL(view->views);
}

view_t* view_new(char* id, r2_t frame)
{
    if (MGET(views.names, id))
    {
	zc_log_error("VIEW NAMES MUST BE UNIQUE!!! : %s\n", id);
	abort();
    }

    view_t* view            = CAL(sizeof(view_t), view_del, view_desc);
    view->id                = cstr_new_cstring(id);
    view->views             = VNEW();
    view->frame.local       = frame;
    view->frame.global      = frame;
    view->texture.page      = -1;
    view->texture.alpha     = 1.0;
    view->texture.resizable = 1;
    view->needs_touch       = 1;
    // view->blocks_touch      = 1;
    view->exclude = 1;

    // reset margins

    view->style.margin_top    = INT_MAX;
    view->style.margin_left   = INT_MAX;
    view->style.margin_right  = INT_MAX;
    view->style.margin_bottom = INT_MAX;
    view->style.top           = INT_MAX;
    view->style.left          = INT_MAX;
    view->style.right         = INT_MAX;
    view->style.bottom        = INT_MAX;
    view->style.shadow_color  = 0x00000033;

    // store and release

    MPUT(views.names, id, view->id);

    return view;
}

void view_set_type(view_t* view, char* type)
{
    if (view->type) REL(view->type);
    if (type) view->type = RET(type);
}

void view_set_class(view_t* view, char* class)
{
    if (view->class) REL(view->class);
    if (class) view->class = RET(class);
}

void view_set_script(view_t* view, char* script)
{
    if (view->script) REL(view->script);
    if (script) view->script = RET(script);
}

void view_set_masked(view_t* view, char masked)
{
    view->style.masked = 1;
}

void view_add_subview(view_t* view, view_t* subview)
{
    for (int i = 0; i < view->views->length; i++)
    {
	view_t* sview = view->views->data[i];
	if (strcmp(sview->id, subview->id) == 0)
	{
	    printf("DUPLICATE SUBVIEW %s\n", subview->id);
	    return;
	}
    }

    views.arrange = 1;

    view_set_parent(subview, view);

    VADD(view->views, subview);

    view_calc_global(view);
}

void view_insert_subview(view_t* view, view_t* subview, uint32_t index)
{
    views.arrange = 1;

    for (int i = 0; i < view->views->length; i++)
    {
	view_t* sview = view->views->data[i];
	if (strcmp(sview->id, subview->id) == 0)
	{
	    printf("DUPLICATE SUBVIEW %s\n", subview->id);
	    return;
	}
    }

    vec_ins(view->views, subview, index);

    view_set_parent(subview, view);

    view_calc_global(view);
}

void view_remove_subview(view_t* view, view_t* subview)
{
    char success = VREM(view->views, subview);

    if (success == 1)
    {
	views.arrange = 1;
	view_set_parent(subview, NULL);
    }
}

void view_remove_from_parent(view_t* view)
{
    if (view->parent) view_remove_subview(view->parent, view);
}

void view_set_parent(view_t* view, view_t* parent)
{
    view->parent = parent;
}

void view_coll_touched(view_t* view, ev_t ev, vec_t* queue)
{
    if (ev.x <= view->frame.global.x + view->frame.global.w &&
	ev.x >= view->frame.global.x &&
	ev.y <= view->frame.global.y + view->frame.global.h &&
	ev.y >= view->frame.global.y)
    {
	vec_add_unique_data(queue, view);
	for (int i = 0; i < view->views->length; i++)
	{
	    view_t* v = view->views->data[i];
	    view_coll_touched(v, ev, queue);
	}
    }
}

view_t* view_get_subview(view_t* view, char* id)
{
    if (strcmp(view->id, id) == 0) return view;
    for (int i = 0; i < view->views->length; i++)
    {
	view_t* sv = view->views->data[i];
	view_t* re = view_get_subview(sv, id);
	if (re) return re;
    }
    return NULL;
}

void view_evt(view_t* view, ev_t ev)
{
    for (int i = 0; i < view->views->length; i++)
    {
	view_t* v = view->views->data[i];
	view_evt(v, ev);
    }

    if (view->handler) (*view->handler)(view, ev);
}

void view_calc_global(view_t* view)
{
    r2_t frame_parent = {0};
    if (view->parent != NULL) frame_parent = view->parent->frame.global;

    r2_t frame_global = view->frame.local;
    r2_t old_global   = view->frame.global;

    frame_global.x = roundf(frame_parent.x) + roundf(frame_global.x);
    frame_global.y = roundf(frame_parent.y) + roundf(frame_global.y);

    if (frame_global.x != old_global.x || frame_global.y != old_global.y) view->frame.pos_changed = 1;

    view->frame.global = frame_global;

    for (int i = 0; i < view->views->length; i++)
    {
	view_t* v = view->views->data[i];
	view_calc_global(v);
    }
}

void view_set_frame(view_t* view, r2_t frame)
{
    // check if texture needs rerendering

    if (view->frame.local.w != frame.w ||
	view->frame.local.h != frame.h)
    {
	view->frame.dim_changed = 1;
	if (frame.w >= 1.0 && frame.h >= 1.0)
	{
	    if (view->texture.type == TT_MANAGED && view->texture.resizable == 1)
	    {
		view->texture.state = TS_BLANK;
	    }
	}
    }

    view->frame.local = frame;

    view_calc_global(view);
}

void view_set_region(view_t* view, r2_t region)
{
    view->frame.region      = region;
    view->frame.reg_changed = 1;
}

void view_set_block_touch(view_t* view, char block, char recursive)
{
    view->blocks_touch = block;

    if (recursive)
    {
	for (int i = 0; i < view->views->length; i++)
	{
	    view_t* v = view->views->data[i];
	    view_set_block_touch(v, block, recursive);
	}
    }
}

void view_set_texture_bmp(view_t* view, bm_rgba_t* bitmap)
{
    if (view->texture.bitmap) REL(view->texture.bitmap);
    view->texture.bitmap  = RET(bitmap);
    view->texture.state   = TS_READY;
    view->texture.changed = 1;
}

void view_set_texture_page(view_t* view, uint32_t page)
{
    view->texture.page = page;
}

void view_set_texture_type(view_t* view, textype_t type)
{
    view->texture.type = type;
}

void view_set_texture_alpha(view_t* view, float alpha, char recur)
{
    view->texture.alpha         = alpha;
    view->texture.alpha_changed = 1;

    if (recur)
    {
	for (int i = 0; i < view->views->length; i++)
	{
	    view_t* v = view->views->data[i];
	    view_set_texture_alpha(v, alpha, recur);
	}
    }
}

void view_invalidate_texture(view_t* view)
{
    view->texture.state = TS_BLANK;
}

void view_set_style(view_t* view, vstyle_t style)
{
    view->style = style;
}

void view_gen_texture(view_t* view)
{
    if (view->tex_gen) (*view->tex_gen)(view);
}

void view_draw_delimiter(view_t* view)
{
    if (view->parent)
    {
	view_draw_delimiter(view->parent);
	uint32_t index = vec_index_of_data(view->parent->views, view);
	if (index == view->parent->views->length - 1)
	    printf("    ");
	else
	    printf("│   ");
    }
}

void view_desc(void* pointer, int level)
{
    view_t* view = (view_t*) pointer;
    printf("view %s", view->id);
}

void view_describe(void* pointer, int level)
{
    view_t* view = (view_t*) pointer;

    char* arrow = "├── ";
    if (view->parent)
    {
	view_draw_delimiter(view->parent);
	if (vec_index_of_data(view->parent->views, view) == view->parent->views->length - 1) arrow = "└── ";
	printf("%s", arrow);
    }

    printf("%s [x:%.0f y:%.0f w:%.0f h:%.0f tx:%i eh:%i tg:%i rc:%zu]\n", view->id, view->frame.local.x, view->frame.local.y, view->frame.local.w, view->frame.local.h, view->texture.page, view->handler != NULL, view->tex_gen != NULL, mem_retaincount(view));

    for (int i = 0; i < view->views->length; i++) view_describe(view->views->data[i], level + 1);
}

void view_desc_style(vstyle_t l)
{
    printf(
	"position %i\n"
	"display %i\n"
	"flexdir %i\n"
	"itemalign %i\n"
	"cjustify %i\n"
	"w_per %f\n"
	"h_per %f\n"
	"width %i\n"
	"height %i\n"
	"margin %i\n"
	"margin_top %i\n"
	"margin_left %i\n"
	"margin_right %i\n"
	"margin_bottom %i\n"
	"top %i\n"
	"left %i\n"
	"right %i\n"
	"bottom %i\n"
	"border_radius %i\n"
	"background_color %x\n"
	"background_image %s\n"
	"font_family %s\n"
	"font_size %f\n"
	"color %x\n"
	"text_align %i\n"
	"line_height %f\n"
	"word_wrap %i\n",
	l.position,
	l.display,
	l.flexdir,
	l.itemalign,
	l.cjustify,
	l.w_per,
	l.h_per,
	l.width,
	l.height,
	l.margin,
	l.margin_top,
	l.margin_left,
	l.margin_right,
	l.margin_bottom,
	l.top,
	l.left,
	l.right,
	l.bottom,
	l.border_radius,
	l.background_color,
	l.background_image == NULL ? "" : l.background_image,
	l.font_family == NULL ? "" : l.font_family,
	l.font_size,
	l.color,
	l.text_align,
	l.line_height,
	l.word_wrap);
}

#endif
