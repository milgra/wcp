#ifndef ui_table_h
#define ui_table_h

#include "view.c"
#include "zc_text.c"
#include "zc_vector.c"
#include <stdint.h>

typedef struct _ui_table_t ui_table_t;

enum ui_table_event_id
{
    UI_TABLE_EVENT_SELECT,
    UI_TABLE_EVENT_OPEN,
    UI_TABLE_EVENT_CONTEXT,
    UI_TABLE_EVENT_DRAG,
    UI_TABLE_EVENT_DROP,
    UI_TABLE_EVENT_KEY,
    UI_TABLE_EVENT_FIELDS_UPDATE,
    UI_TABLE_EVENT_FIELD_SELECT
};

typedef struct _ui_table_event_t
{
    enum ui_table_event_id id;
    ui_table_t*            table;
    char*                  field;
    vec_t*                 fields;
    vec_t*                 selected_items;
    int32_t                selected_index;
    view_t*                rowview;
    ev_t                   ev;
} ui_table_event_t;

struct _ui_table_t
{
    char*       id;             // unique id for item generation
    uint32_t    cnt;            // item count for item generation
    vec_t*      items;          // data items
    vec_t*      cache;          // item cache
    vec_t*      fields;         // field name field size interleaved vector
    vec_t*      selected_items; // selected items
    int32_t     selected_index; // index of last selected
    view_t*     body_v;
    view_t*     evnt_v;
    view_t*     scrl_v;
    textstyle_t textstyle;
    void (*on_event)(ui_table_event_t event);
};

ui_table_t* ui_table_create(
    char*   id,
    view_t* body,
    view_t* scrl,
    view_t* evnt,
    view_t* head,
    vec_t*  fields,
    void (*on_event)(ui_table_event_t event));

void ui_table_select(
    ui_table_t* table,
    int32_t     index);

void ui_table_set_data(
    ui_table_t* uit, vec_t* data);

vec_t* ui_table_get_fields(ui_table_t* uit);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "config.c"
#include "tg_css.c"
#include "tg_text.c"
#include "ui_util.c"
#include "vh_tbl_body.c"
#include "vh_tbl_evnt.c"
#include "vh_tbl_head.c"
#include "vh_tbl_scrl.c"
#include "zc_cstring.c"
#include "zc_log.c"
#include "zc_memory.c"
#include "zc_number.c"

void ui_table_head_align(ui_table_t* uit, int fixed_index, int fixed_pos)
{
    for (int ri = 0; ri < uit->body_v->views->length; ri++)
    {
	view_t* rowview = uit->body_v->views->data[ri];
	float   wth     = 0;

	for (int ci = 0; ci < rowview->views->length; ci++)
	{
	    view_t* cellview = rowview->views->data[ci];
	    r2_t    frame    = cellview->frame.local;
	    num_t*  sizep    = uit->fields->data[ci * 2 + 1];
	    frame.x          = ci == fixed_index ? (float) fixed_pos : wth;
	    frame.w          = (float) sizep->intv;
	    view_set_frame(cellview, frame);
	    wth += frame.w + 2;
	}

	r2_t frame = rowview->frame.local;
	frame.w    = wth;
	view_set_frame(rowview, frame);
    }
}

void ui_table_head_move(view_t* hview, int index, int pos, void* userdata)
{
    ui_table_t* uit = (ui_table_t*) userdata;

    ui_table_head_align(uit, index, pos);
}

void ui_table_head_resize(view_t* hview, int index, int size, void* userdata)
{
    ui_table_t* uit = (ui_table_t*) userdata;

    if (index > -1)
    {
	num_t* sizep = uit->fields->data[index * 2 + 1];
	sizep->intv  = size;

	ui_table_head_align(uit, -1, 0);
    }
    else
    {
	ui_table_event_t event = {.table = uit, .id = UI_TABLE_EVENT_FIELDS_UPDATE, .fields = uit->fields};
	(*uit->on_event)(event);
    }
}

void ui_table_head_reorder(view_t* hview, int ind1, int ind2, void* userdata)
{
    ui_table_t* uit = (ui_table_t*) userdata;

    if (ind1 == -1)
    {
	// self click, dispatch event
	char*            field = uit->fields->data[ind2 * 2];
	ui_table_event_t event = {.id = UI_TABLE_EVENT_FIELD_SELECT, .field = field};
	(*uit->on_event)(event);
    }
    else
    {
	char*  field1 = uit->fields->data[ind1 * 2];
	num_t* size1  = uit->fields->data[ind1 * 2 + 1];
	char*  field2 = uit->fields->data[ind2 * 2];
	num_t* size2  = uit->fields->data[ind2 * 2 + 1];

	uit->fields->data[ind1 * 2]     = field2;
	uit->fields->data[ind1 * 2 + 1] = size2;

	uit->fields->data[ind2 * 2]     = field1;
	uit->fields->data[ind2 * 2 + 1] = size1;

	for (int ri = 0; ri < uit->body_v->views->length; ri++)
	{
	    view_t* rowview = uit->body_v->views->data[ri];

	    view_t* cell1 = RET(rowview->views->data[ind1]);
	    view_t* cell2 = RET(rowview->views->data[ind2]);

	    view_remove_subview(rowview, cell1);
	    view_insert_subview(rowview, cell1, ind2);
	    view_remove_subview(rowview, cell2);
	    view_insert_subview(rowview, cell2, ind1);

	    REL(cell1);
	    REL(cell2);
	}

	ui_table_head_align(uit, -1, 0);

	ui_table_event_t event = {.table = uit, .id = UI_TABLE_EVENT_FIELDS_UPDATE, .fields = uit->fields};
	(*uit->on_event)(event);
    }
}

view_t* ui_table_head_create(
    view_t* head_v,
    void*   userdata)
{
    ui_table_t* uit = (ui_table_t*) userdata;

    char*   headid   = cstr_new_format(100, "%s_header", uit->id); // REL 0
    view_t* headview = view_new(headid, (r2_t){0, 0, 100, uit->textstyle.line_height});
    REL(headid); // REL 0

    int         wth = 0;
    textstyle_t ts  = uit->textstyle;

    for (int i = 0; i < uit->fields->length; i += 2)
    {
	char*   field    = uit->fields->data[i];
	num_t*  size     = uit->fields->data[i + 1];
	char*   cellid   = cstr_new_format(100, "%s_cell_%s", headview->id, field);      // REL 2
	view_t* cellview = view_new(cellid, (r2_t){wth, 0, size->intv, ts.line_height}); // REL 3

	wth += size->intv + 2;

	ts.backcolor = 0x454545FF;

	tg_text_add(cellview);
	tg_text_set(cellview, field, ts);

	view_add_subview(headview, cellview);

	REL(cellid);   // REL 2
	REL(cellview); // REL 3
    }

    view_set_frame(headview, (r2_t){0, 0, wth, ts.line_height});

    return headview;
}

view_t* ui_table_item_create(
    view_t* table_v,
    int     index,
    void*   userdata)
{
    ui_table_t* uit = (ui_table_t*) userdata;

    view_t* rowview = NULL;

    if (uit->items)
    {
	if (index > -1 && index < uit->items->length)
	{
	    map_t* data = uit->items->data[index];

	    textstyle_t ts = uit->textstyle;

	    if (uit->cache->length > 0)
	    {
		rowview = RET(uit->cache->data[0]);
		vec_rem_at_index(uit->cache, 0);
	    }
	    else
	    {
		char* rowid = cstr_new_format(100, "%s_rowitem_%i", uit->id, uit->cnt++); // REL 0
		rowview     = view_new(rowid, (r2_t){0, 0, table_v->frame.local.w, ts.line_height});
		REL(rowid); // REL 0

		tg_css_add(rowview);

		int wth = 0;

		for (int i = 0; i < uit->fields->length; i += 2)
		{
		    char*  field = uit->fields->data[i];
		    num_t* size  = uit->fields->data[i + 1];
		    // char*   value    = MGET(data, field);
		    char*   cellid   = cstr_new_format(100, "%s_cell_%s", rowview->id, field);       // REL 2
		    view_t* cellview = view_new(cellid, (r2_t){wth, 0, size->intv, ts.line_height}); // REL 3

		    wth += size->intv + 2;

		    tg_text_add(cellview);

		    view_add_subview(rowview, cellview);

		    REL(cellid);   // REL 2
		    REL(cellview); // REL 3
		}
	    }

	    rowview->style.background_color = 0x000000FF;

	    if (uit->selected_items->length > 0)
	    {
		uint32_t pos = vec_index_of_data(uit->selected_items, data);

		if (pos < UINT32_MAX)
		{
		    rowview->style.background_color = 0x006600FF;
		}
	    }

	    view_invalidate_texture(rowview);

	    uint32_t color = index % 2 != 0 ? 0x35353588 : 0x45454588;

	    int wth = 0;

	    for (int i = 0; i < uit->fields->length; i += 2)
	    {
		char*   field    = uit->fields->data[i];
		num_t*  size     = uit->fields->data[i + 1];
		char*   value    = MGET(data, field);
		view_t* cellview = rowview->views->data[i / 2];
		r2_t    frame    = cellview->frame.local;

		frame.x = wth;
		frame.w = size->intv;
		view_set_frame(cellview, frame);

		wth += size->intv + 2;

		ts.backcolor = color;

		if (value) tg_text_set(cellview, value, ts);
		else tg_text_set(cellview, "", ts); // reset old value
	    }

	    view_set_frame(rowview, (r2_t){0, 0, wth, ts.line_height});
	}
    }

    return rowview;
}

void ui_table_item_recycle(
    view_t* table_v,
    view_t* item_v,
    void*   userdata)
{
    ui_table_t* uit = (ui_table_t*) userdata;
    VADD(uit->cache, item_v);
}

void ui_table_evnt_event(vh_tbl_evnt_event_t event)
{
    ui_table_t* uit = (ui_table_t*) event.userdata;

    if (event.id == VH_TBL_EVENT_SELECT)
    {
	uit->selected_index = event.index;

	map_t* data = uit->items->data[event.index];

	uint32_t pos = vec_index_of_data(uit->selected_items, data);

	if (pos == UINT32_MAX)
	{
	    // reset selected if control is not down
	    if (!event.ev.ctrl_down)
	    {
		vec_reset(uit->selected_items);
		vh_tbl_body_t* bvh = uit->body_v->handler_data;

		for (int index = 0; index < bvh->items->length; index++)
		{
		    view_t* item = bvh->items->data[index];
		    if (item->style.background_color == 0x006600FF)
		    {
			item->style.background_color = 0x000000FF;
			view_invalidate_texture(item);
		    }
		}
	    }

	    VADD(uit->selected_items, data);
	    event.rowview->style.background_color = 0x006600FF;
	    view_invalidate_texture(event.rowview);
	}
	else
	{
	    VREM(uit->selected_items, data);
	    event.rowview->style.background_color = 0x000000FF;
	    view_invalidate_texture(event.rowview);
	}

	ui_table_event_t tevent = {.table = uit, .id = UI_TABLE_EVENT_SELECT, .selected_items = uit->selected_items, .selected_index = event.index, .rowview = event.rowview};
	(*uit->on_event)(tevent);
    }
    else if (event.id == VH_TBL_EVENT_CONTEXT)
    {
	uit->selected_index = event.index;

	map_t* data = uit->items->data[event.index];

	uint32_t pos = vec_index_of_data(uit->selected_items, data);

	if (pos == UINT32_MAX)
	{
	    // reset selected if control is not down
	    if (!event.ev.ctrl_down)
	    {
		vec_reset(uit->selected_items);
		vh_tbl_body_t* bvh = uit->body_v->handler_data;

		for (int index = 0; index < bvh->items->length; index++)
		{
		    view_t* item = bvh->items->data[index];
		    if (item->style.background_color == 0x006600FF)
		    {
			item->style.background_color = 0x000000FF;
			view_invalidate_texture(item);
		    }
		}
	    }

	    VADD(uit->selected_items, data);

	    if (event.rowview)
	    {
		event.rowview->style.background_color = 0x006600FF;
		view_invalidate_texture(event.rowview);
	    }
	}

	ui_table_event_t tevent = {.table = uit, .id = UI_TABLE_EVENT_CONTEXT, .selected_items = uit->selected_items, .selected_index = event.index, .rowview = event.rowview, .ev = event.ev};
	(*uit->on_event)(tevent);
    }
    else if (event.id == VH_TBL_EVENT_OPEN)
    {
	ui_table_event_t tevent = {.table = uit, .id = UI_TABLE_EVENT_OPEN, .selected_items = uit->selected_items, .selected_index = event.index, .rowview = event.rowview};
	(*uit->on_event)(tevent);
    }
    else if (event.id == VH_TBL_EVENT_DRAG)
    {
	ui_table_event_t tevent = {.table = uit, .id = UI_TABLE_EVENT_DRAG, .selected_items = uit->selected_items, .selected_index = event.index, .rowview = event.rowview};
	(*uit->on_event)(tevent);
    }
    else if (event.id == VH_TBL_EVENT_DROP)
    {
	ui_table_event_t tevent = {.table = uit, .id = UI_TABLE_EVENT_DROP, .selected_items = uit->selected_items, .selected_index = event.index, .rowview = event.rowview};
	(*uit->on_event)(tevent);
    }
    else if (event.id == VH_TBL_EVENT_KEY)
    {
	ui_table_event_t tevent = {.table = uit, .id = UI_TABLE_EVENT_KEY, .selected_items = uit->selected_items, .selected_index = uit->selected_index, .rowview = event.rowview, .ev = event.ev};
	(*uit->on_event)(tevent);
    }
}

void ui_table_del(
    void* p)
{
    ui_table_t* uit = p;

    // remove items from view
    REL(uit->id);             // REL S0
    REL(uit->cache);          // REL S1
    REL(uit->fields);         // REL S2
    REL(uit->selected_items); // REL S3

    if (uit->items) REL(uit->items);

    if (uit->body_v) REL(uit->body_v);
    if (uit->evnt_v) REL(uit->evnt_v);
    if (uit->scrl_v) REL(uit->scrl_v);
}

void ui_table_desc(
    void* p,
    int   level)
{
    printf("ui_table");
}

ui_table_t* ui_table_create(
    char*   id, // id has to be unique
    view_t* body,
    view_t* scrl,
    view_t* evnt,
    view_t* head,
    vec_t*  fields,
    void (*on_event)(ui_table_event_t event))
{
    assert(id != NULL);
    assert(body != NULL);

    ui_table_t* uit     = CAL(sizeof(ui_table_t), ui_table_del, ui_table_desc);
    uit->id             = cstr_new_cstring(id); // REL S0
    uit->cache          = VNEW();               // REL S1
    uit->fields         = RET(fields);          // REL S2
    uit->selected_items = VNEW();               // REL S3
    uit->on_event       = on_event;

    uit->body_v = RET(body);

    vh_tbl_body_attach(
	body,
	ui_table_item_create,
	ui_table_item_recycle,
	uit);

    uit->textstyle             = ui_util_gen_textstyle(body);
    uit->textstyle.margin_left = 5;
    if (uit->textstyle.line_height == 0) uit->textstyle.line_height = (int) uit->textstyle.size;

    if (head)
    {
	vh_tbl_head_attach(
	    head,
	    ui_table_head_create,
	    ui_table_head_move,
	    ui_table_head_resize,
	    ui_table_head_reorder,
	    uit);
    }

    if (scrl)
    {
	uit->scrl_v = RET(scrl);

	vh_tbl_scrl_attach(
	    scrl,
	    body,
	    head,
	    uit);
    }

    if (evnt)
    {
	uit->evnt_v = RET(evnt);
	vh_tbl_evnt_attach(
	    evnt,
	    body,
	    scrl,
	    head,
	    ui_table_evnt_event,
	    uit);
    }

    return uit;
}

/* data items have to be maps containing the same keys */

void ui_table_set_data(
    ui_table_t* uit,
    vec_t*      data)
{
    if (uit->items) REL(uit->items);
    uit->items = RET(data);

    uit->selected_index = 0;

    vec_reset(uit->selected_items);
    if (uit->selected_index < uit->items->length)
    {
	map_t* sel = uit->items->data[uit->selected_index];
	VADD(uit->selected_items, sel);
    }

    vh_tbl_body_reset(uit->body_v);
    vh_tbl_body_move(uit->body_v, 0, 0);

    if (uit->scrl_v) vh_tbl_scrl_set_item_count(uit->scrl_v, data->length);
}

/* select index */

void ui_table_select(
    ui_table_t* uit,
    int32_t     index)
{
    vh_tbl_body_t* bvh = uit->body_v->handler_data;

    uit->selected_index = index;
    if (uit->selected_index < 0) uit->selected_index = 0;
    if (uit->selected_index > uit->items->length - 1) uit->selected_index = uit->items->length - 1;

    if (bvh->bot_index <= uit->selected_index)
    {
	vh_tbl_body_vjump(uit->body_v, uit->selected_index + 1, 0);

	if (bvh->tail_index == bvh->bot_index)
	{
	    // check if bottom item is out of bounds
	    view_t* lastitem = vec_tail(bvh->items);
	    r2_t    iframe   = lastitem->frame.local;
	    r2_t    vframe   = uit->body_v->frame.local;

	    if (iframe.y + iframe.h > vframe.h)
	    {
		vh_tbl_body_move(uit->body_v, 0, iframe.y + iframe.h - vframe.h);
	    }
	}
    }

    if (uit->selected_index <= bvh->top_index)
    {
	vh_tbl_body_vjump(uit->body_v, uit->selected_index - 1, 1);
    }

    vec_reset(uit->selected_items);
    map_t* sel = uit->items->data[uit->selected_index];
    VADD(uit->selected_items, sel);

    /* color item */

    for (int i = 0; i < bvh->items->length; i++)
    {
	int     realindex = bvh->head_index + i;
	view_t* item      = bvh->items->data[i];

	if (item->style.background_color == 0x006600FF)
	{
	    item->style.background_color = 0x000000FF;
	    view_invalidate_texture(item);
	}

	if (realindex == uit->selected_index)
	{
	    item->style.background_color = 0x006600FF;
	    view_invalidate_texture(item);
	}
    }

    /* ui_table_event_t event = {.table = uit, .id = UI_TABLE_EVENT_SELECT, .selected_items = uit->selected_items, .selected_index = index, .rowview = NULL}; */
    /* (*uit->on_event)(event); */
}

vec_t* ui_table_get_fields(ui_table_t* uit)
{
    return uit->fields;
}

#endif
