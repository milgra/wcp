#ifndef vh_anim_h
#define vh_anim_h

#include "view.c"
#include "zc_util2.c"
#include "zc_vec2.c"

typedef enum _animtype_t
{
    AT_LINEAR,
    AT_EASE,
    AT_EASE_IN,
    AT_EASE_OUT
} animtype_t;

enum vh_anim_event_id
{
    VH_ANIM_END
};

typedef struct _vh_anim_event_t
{
    enum vh_anim_event_id id;
    view_t*               view;
    void*                 userdata;
} vh_anim_event_t;

typedef struct _vh_anim_t
{
    animtype_t type;

    r2_t sf; // starting frame
    r2_t ef; // ending frame

    r2_t sr; // starting region
    r2_t er; // ending region

    float sa; // starting alpha
    float ea; // ending alpha

    char anim_alpha;
    char anim_frame;
    char anim_region;

    int astep;
    int asteps;
    int rstep;
    int rsteps;
    int fstep;
    int fsteps;

    void* userdata;
    void (*on_event)(vh_anim_event_t event);
} vh_anim_t;

void vh_anim_frame(view_t* view, r2_t sf, r2_t ef, int steps, animtype_t type);

void vh_anim_alpha(view_t* view, float sa, float ea, int steps, animtype_t type);

void vh_anim_region(view_t* view, r2_t sr, r2_t er, int steps, animtype_t type);

void vh_anim_finish(view_t* view);

void vh_anim_add(view_t* view, void (*on_event)(vh_anim_event_t), void* userdata);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "zc_log.c"

void vh_anim_evt(view_t* view, ev_t ev)
{
    if (ev.type == EV_TIME)
    {
	vh_anim_t* vh = view->handler_data;

	if (vh->anim_frame)
	{
	    if (vh->fstep < vh->fsteps)
	    {
		r2_t sf = vh->sf;
		r2_t cf = sf;
		r2_t ef = vh->ef;

		if (vh->type == AT_LINEAR)
		{
		    // just increase current with delta
		    cf.x = sf.x + ((ef.x - sf.x) / vh->fsteps) * vh->fstep;
		    cf.y = sf.y + ((ef.y - sf.y) / vh->fsteps) * vh->fstep;
		    cf.w = sf.w + ((ef.w - sf.w) / vh->fsteps) * vh->fstep;
		    cf.h = sf.h + ((ef.h - sf.h) / vh->fsteps) * vh->fstep;
		}
		else if (vh->type == AT_EASE)
		{
		    // speed function based on cosine ( half circle )
		    float angle = 3.14 + (3.14 / vh->fsteps) * vh->fstep;
		    float delta = (cos(angle) + 1.0) / 2.0;

		    cf.x = sf.x + (ef.x - sf.x) * delta;
		    cf.y = sf.y + (ef.y - sf.y) * delta;
		    cf.w = sf.w + (ef.w - sf.w) * delta;
		    cf.h = sf.h + (ef.h - sf.h) * delta;
		}

		if (vh->fstep == vh->fsteps - 1) cf = ef;

		view_set_frame(view, cf);

		vh->fstep += 1;

		if (vh->fstep == vh->fsteps)
		{
		    vh->anim_frame        = 0;
		    vh_anim_event_t event = {.id = VH_ANIM_END, .view = view, .userdata = vh->userdata};
		    if (vh->on_event) (*vh->on_event)(event);
		}
	    }
	}

	if (vh->anim_region)
	{
	    if (vh->rstep < vh->rsteps)
	    {
		r2_t sr = vh->sr;
		r2_t cr = sr;
		r2_t er = vh->er;

		if (vh->type == AT_LINEAR)
		{
		    // just increase current with delta
		    cr.x = sr.x + ((er.x - sr.x) / vh->rsteps) * vh->rstep;
		    cr.y = sr.y + ((er.y - sr.y) / vh->rsteps) * vh->rstep;
		    cr.w = sr.w + ((er.w - sr.w) / vh->rsteps) * vh->rstep;
		    cr.h = sr.h + ((er.h - sr.h) / vh->rsteps) * vh->rstep;
		}
		else if (vh->type == AT_EASE)
		{
		    // speed function based on cosine ( half circle )
		    float angle = 3.14 + (3.14 / vh->rsteps) * vh->rstep;
		    float delta = (cos(angle) + 1.0) / 2.0;

		    cr.x = sr.x + (er.x - sr.x) * delta;
		    cr.y = sr.y + (er.y - sr.y) * delta;
		    cr.w = sr.w + (er.w - sr.w) * delta;
		    cr.h = sr.h + (er.h - sr.h) * delta;
		}

		if (vh->rstep == vh->rsteps - 1) cr = er;

		view_set_region(view, cr);

		vh->rstep += 1;

		if (vh->rstep == vh->rsteps)
		{
		    vh->anim_region       = 0;
		    vh_anim_event_t event = {.id = VH_ANIM_END, .view = view, .userdata = vh->userdata};
		    if (vh->on_event) (*vh->on_event)(event);
		}
	    }
	}

	if (vh->anim_alpha)
	{
	    if (vh->astep < vh->asteps)
	    {
		float sa = vh->sa;
		float ea = vh->ea;
		float ca = sa;

		if (vh->type == AT_LINEAR)
		{
		    // just increase current with delta
		    ca = sa + ((ea - sa) / vh->asteps) * vh->astep;
		}
		else if (vh->type == AT_EASE)
		{
		    // speed function based on cosine ( half circle )
		    float angle = 3.14 + (3.14 / vh->asteps) * vh->astep;
		    float delta = (cos(angle) + 1.0) / 2.0;
		    ca          = sa + (ea - sa) * delta;
		}

		if (vh->astep == vh->asteps - 1) ca = ea;

		view_set_texture_alpha(view, ca, 1);

		vh->astep += 1;

		if (vh->astep == vh->asteps)
		{
		    vh->anim_alpha        = 0;
		    vh_anim_event_t event = {.id = VH_ANIM_END, .view = view, .userdata = vh->userdata};
		    if (vh->on_event) (*vh->on_event)(event);
		}
	    }
	}
    }
}

void vh_anim_frame(view_t* view, r2_t sf, r2_t ef, int steps, animtype_t type)
{
    vh_anim_t* vh = view->handler_data;
    if (vh->fstep == vh->fsteps)
    {
	vh->sf         = sf;
	vh->ef         = ef;
	vh->fstep      = 0;
	vh->type       = type;
	vh->fsteps     = steps;
	vh->anim_frame = 1;
    }
}

void vh_anim_region(view_t* view, r2_t sr, r2_t er, int steps, animtype_t type)
{
    vh_anim_t* vh = view->handler_data;
    if (vh->rstep == vh->rsteps)
    {
	vh->sr          = sr;
	vh->er          = er;
	vh->rstep       = 0;
	vh->type        = type;
	vh->rsteps      = steps;
	vh->anim_region = 1;
    }
}

void vh_anim_alpha(view_t* view, float sa, float ea, int steps, animtype_t type)
{
    vh_anim_t* vh = view->handler_data;

    if (vh->astep == vh->asteps)
    {
	vh->sa         = sa;
	vh->ea         = ea;
	vh->astep      = 0;
	vh->type       = type;
	vh->asteps     = steps;
	vh->anim_alpha = 1;
    }
}

void vh_anim_finish(view_t* view)
{
    vh_anim_t* vh = view->handler_data;
    vh->astep     = vh->asteps;
    vh->fstep     = vh->fsteps;
    vh->rstep     = vh->rsteps;

    if (vh->anim_frame) view_set_frame(view, vh->ef);
    if (vh->anim_region) view_set_region(view, vh->er);
    if (vh->anim_alpha) view_set_texture_alpha(view, vh->ea, 1);

    vh->anim_frame  = 0;
    vh->anim_region = 0;
    vh->anim_alpha  = 0;
}

void vh_anim_desc(void* p, int level)
{
    printf("vh_anim");
}

void vh_anim_add(view_t* view, void (*on_event)(vh_anim_event_t), void* userdata)
{
    assert(view->handler == NULL && view->handler_data == NULL);

    vh_anim_t* vh = CAL(sizeof(vh_anim_t), NULL, vh_anim_desc);
    vh->on_event  = on_event;
    vh->userdata  = userdata;

    view->handler      = vh_anim_evt;
    view->handler_data = vh;
}

#endif
