/* content view body */

#ifndef vh_cv_body_h
#define vh_cv_body_h

#include "view.c"

typedef struct _vh_cv_body_t
{
    void* userdata;

    view_t* content;
    float   cw; // content width
    float   ch; // content height
    float   px;
    float   py;
    float   scale;
} vh_cv_body_t;

void vh_cv_body_attach(
    view_t* view,
    void*   userdata);

void vh_cv_body_set_content_size(
    view_t* view,
    int     cw,
    int     ch);

void vh_cv_body_move(
    view_t* view,
    float   dx,
    float   dy);

void vh_cv_body_zoom(
    view_t* view,
    float   s,
    int     x,
    int     y);

void vh_cv_body_reset(
    view_t* view);

void vh_cv_body_hjump(
    view_t* view,
    float   dx);

void vh_cv_body_vjump(
    view_t* view,
    int     topindex);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "tg_scaledimg.c"
#include "zc_log.c"

void vh_cv_body_del(void* p)
{
}

void vh_cv_body_desc(void* p, int level)
{
    printf("vh_cv_body");
}

void vh_cv_body_attach(
    view_t* view,
    void*   userdata)
{
    assert(view->handler == NULL && view->handler_data == NULL);

    vh_cv_body_t* vh = CAL(sizeof(vh_cv_body_t), vh_cv_body_del, vh_cv_body_desc);
    vh->userdata     = userdata;
    vh->content      = view->views->data[0];
    vh->scale        = 1.0;

    vh->cw = 1;
    vh->ch = 1;

    view->handler_data = vh;
}

void vh_cv_body_set_content_size(
    view_t* view,
    int     cw,
    int     ch)
{
    vh_cv_body_t* vh = view->handler_data;

    vh->cw = cw;
    vh->ch = ch;

    r2_t lf = view->frame.local; // local frame

    float cr = (float) ch / (float) cw; // content aspect ratio

    /* fit width first */

    float nw = lf.w;
    float nh = nw * cr;

    vh->scale = (float) nw / cw;

    if (nh > lf.h)
    {
	cr = lf.h / nh;

	nh = lf.h;
	nw *= cr;
	vh->scale = (float) nh / ch;
    }

    r2_t frame = vh->content->frame.local;
    frame.x    = (lf.w - nw) / 2.0;
    frame.y    = (lf.h - nh) / 2.0;
    frame.w    = nw;
    frame.h    = nh;

    view_set_frame(vh->content, frame);

    tg_scaledimg_set_content_size(vh->content, cw, ch);
    tg_scaledimg_gen(vh->content);
}

void vh_cv_body_move(
    view_t* view,
    float   dx,
    float   dy)
{
    vh_cv_body_t* vh = view->handler_data;

    r2_t frame = vh->content->frame.local;
    frame.x += dx;
    frame.y += dy;
    view_set_frame(vh->content, frame);
}

void vh_cv_body_zoom(
    view_t* view,
    float   s,
    int     x,
    int     y)
{
    vh_cv_body_t* vh = view->handler_data;

    r2_t gf = vh->content->frame.global;
    r2_t lf = vh->content->frame.local;

    /* partial width and height from mouse position */

    float pw = (float) x - gf.x;
    float ph = (float) y - gf.y;

    /* ratios */

    float rw = pw / gf.w;
    float rh = ph / gf.h;

    /* new dimensions */

    float ds = s / 100.0;
    if (ds > 0.5) ds = 0.5;
    if (ds < -0.5) ds = -0.5;

    if (vh->scale + ds > 10.0) vh->scale = 10.0;
    else if (vh->scale + ds < 0.1) vh->scale = 0.1;
    else vh->scale += ds;

    float nw = vh->cw * vh->scale;
    float nh = vh->ch * vh->scale;

    lf.x = (float) x - rw * nw - view->frame.global.x;
    lf.y = (float) y - rh * nh - view->frame.global.y;
    lf.w = nw;
    lf.h = nh;

    view_set_frame(vh->content, lf);
}

void vh_cv_body_reset(
    view_t* view)
{
}

void vh_cv_body_hjump(
    view_t* view,
    float   x)
{
}

void vh_cv_body_vjump(
    view_t* view,
    int     topindex)
{
}

#endif
