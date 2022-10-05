#ifndef view_html_h
#define view_html_h

#include "zc_vector.c"

void viewgen_html_parse(char* htmlpath, vec_t* views);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "cstr_util.c"
#include "html.c"
#include "view.c"

void viewgen_html_parse(char* htmlpath, vec_t* views)
{
    char*  html = cstr_new_file(htmlpath); // REL 0
    tag_t* tags = html_new(html);          // REL 1
    tag_t* head = tags;

    while ((*tags).len > 0)
    {
	tag_t t = *tags;
	if (t.id.len > 0)
	{
	    // extract id
	    char* id = CAL(sizeof(char) * t.id.len + 1, NULL, cstr_describe); // REL 0
	    memcpy(id, html + t.id.pos + 1, t.id.len);
	    view_t* view = view_new(id, (r2_t){0}); // REL 1

	    if (t.level > 0)
	    {
		// add to parent
		view_t* parent = views->data[t.parent];
		view_add_subview(parent, view);
	    }

	    if (t.class.len > 0)
	    {
		// store css classes
		char* class = CAL(sizeof(char) * t.class.len + 1, NULL, cstr_describe); // REL 0
		memcpy(class, html + t.class.pos + 1, t.class.len);
		view_set_class(view, class);
		REL(class);
	    }

	    if (t.type.len > 0)
	    {
		// store html stype
		char* type = CAL(sizeof(char) * t.type.len + 1, NULL, cstr_describe); // REL 2
		memcpy(type, html + t.type.pos + 1, t.type.len);
		view_set_type(view, type);
		REL(type); // REL 2
	    }

	    if (t.script.len > 0)
	    {
		// store html stype
		char* script = CAL(sizeof(char) * t.script.len + 1, NULL, cstr_describe); // REL 2
		memcpy(script, html + t.script.pos + 1, t.script.len);
		view_set_script(view, script);
		REL(script); // REL 2
	    }

	    VADD(views, view);

	    REL(id);   // REL 0
	    REL(view); // REL 1
	}
	else
	{
	    static int divcnt = 0;
	    char*      divid  = cstr_new_format(10, "div%i", divcnt++);
	    // idless view, probably </div>
	    view_t* view = view_new(divid, (r2_t){0});
	    VADD(views, view);
	    REL(view);
	    REL(divid);
	}
	tags += 1;
    }

    // cleanup

    REL(head); // REL 1
    REL(html); // REL 0
}

#endif
