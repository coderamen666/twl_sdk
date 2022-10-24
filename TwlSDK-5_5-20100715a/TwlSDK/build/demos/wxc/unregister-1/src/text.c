/*---------------------------------------------------------------------------*
  Project:  TwlSDK - WXC - demos - unregister-1
  File:     text.c

  Copyright 2005-2009 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2007-11-15#$
  $Rev: 2414 $
  $Author: hatamoto_minoru $
 *---------------------------------------------------------------------------*/


#include <nitro.h>
#include "font.h"
#include "text.h"

#define TAB_SIZE 8
/*****************************************************/

static LINE_BUF *lb_free_ptr;
static int x_size = X_LINE_MAX;
static int y_size = Y_LINE_MAX;

static void link_line_buf(LINE_BUF * m, LINE_BUF * s)
{
    if (m->next != NULL)
    {
        s->next = m->next;
        (s->next)->prev = s;
    }
    else
        s->next = NULL;

    m->next = s;
    s->prev = m;
}

static void unlink_line_buf(LINE_BUF * s)
{
    if ((s->prev != NULL) && (s->next != NULL))
    {
        (s->prev)->next = s->next;
        (s->next)->prev = s->prev;
        s->next = NULL;
        s->prev = NULL;
    }
    else if (s->next != NULL)
    {
        s->prev = NULL;
        (s->next)->prev = NULL;
        s->next = NULL;
    }
    else if (s->prev != NULL)
    {
        s->next = NULL;
        (s->prev)->next = NULL;
        s->prev = NULL;
    }
}

static LINE_BUF *alloc_line_buf(void)
{
    LINE_BUF *tmp;
    tmp = lb_free_ptr;
    lb_free_ptr = lb_free_ptr->next;
    unlink_line_buf(tmp);
    tmp->last_count = 0;
    tmp->cur_count = 0;
#if 0
    for (i = 0; i < LINE_BUF_X_SIZE; i++)
        tmp->buf[i] = 0x20;
#endif
    return (tmp);
}

static void free_line_buf(LINE_BUF * lb)
{
    lb->next = lb_free_ptr;
    lb->prev = NULL;
    (lb->next)->prev = lb;
    lb_free_ptr = lb;
}

static int init_lb_heap(void *heap_start, void *heap_end)
{
    int     i;
    LINE_BUF *lb_heap = (LINE_BUF *) heap_start;
    lb_free_ptr = (LINE_BUF *) heap_start;
    lb_heap[0].prev = NULL;

    i = 0;
    while ((u32)&(lb_heap[i + 1]) < (u32)heap_end)
    {
        link_line_buf(&(lb_heap[i]), &(lb_heap[i + 1]));
        i++;
    }
    lb_heap[i].next = NULL;
    return i;                          /* Num of line buffer */
}

static void init_text_buf(TEXT_BUF * tb)
{
    tb->num_y = &y_size;
    tb->num_x = &x_size;
    tb->start = tb->cur = alloc_line_buf();
}


void init_text(TEXT_CTRL * tc, u16 *screen, u16 palette)
{
    tc->screen = screen;
    tc->x_line = 0;
    tc->y_line = 0;
    tc->palette = palette;
    init_text_buf(&(tc->text_buf));
}



void m_set_palette(TEXT_CTRL * tc, u16 num)
{
    tc->palette = (u16)(0x0f & num);
}

u16 m_get_palette(TEXT_CTRL * tc)
{
    return tc->palette;
}

static void clear_line(TEXT_CTRL * tc)
{
    int     i;
    i = tc->x_line;

    for (; i < X_LINE_MAX; i++)
        tc->screen[(tc->y_line * X_LINE_MAX) + i] = 0x20;

}

static void add_y_line(TEXT_CTRL * tc, u16 num)
{
    tc->y_line += num;
    if (tc->y_line > Y_LINE_MAX - 1)
    {
        tc->y_line = 0;
    }
}

static void add_x_line(TEXT_CTRL * tc, u16 num)
{
    tc->x_line += num;
    if (tc->x_line > X_LINE_MAX - 1)
    {
        tc->x_line = 0;
        add_y_line(tc, 1);
    }
}


/**********************/

void m_putchar(TEXT_CTRL * tc, int c)
{
    int     i;
    LINE_BUF *tmp;
    TEXT_BUF *tb = &(tc->text_buf);

    switch (c)
    {
    case '\0':
        break;
    case '\n':
        if (tb->cur->next == NULL)
        {
            tmp = tb->cur;             /* Recalculate the display start buffer */
            for (i = 0; i < *(tb->num_y) - 1 && i < LINE_BUF_Y_SIZE; i++)
            {
                if (tmp->prev == NULL)
                    break;
                tmp = tmp->prev;
            }
            tb->start = tmp;
            while (tmp->prev != NULL)
            {                          /* Deallocate unnecessary buffers */
                tmp = tmp->prev;
                unlink_line_buf(tmp);
                free_line_buf(tmp);
            }
            tmp = alloc_line_buf();    /* Get one buffer */

            link_line_buf(tb->cur, tmp);        /* Link the obtained buffer as the next buffer to the current one
                                                    */
            tb->cur = tmp;             /* Update the current buffer */
            tb->cur->cur_count = 0;
            tb->cur->last_count = 0;
        }
        else
        {
            tb->cur = tb->cur->next;
            tb->cur->cur_count = 0;
            tb->cur->last_count = 0;
        }
        break;
    case '\r':
        tb->cur->cur_count = 0;
        break;
    case '\t':
        tb->cur->cur_count += TAB_SIZE;
        if (tb->cur->cur_count > LINE_BUF_X_SIZE)
            tb->cur->cur_count = LINE_BUF_X_SIZE;
        if (tb->cur->cur_count > tb->cur->last_count)
            tb->cur->last_count = tb->cur->cur_count;
        break;
    case '\f':
        tb->cur = tb->start;
        tb->cur->cur_count = 0;
        tb->cur->last_count = 0;
        break;
    case '\a':                        /* BELL */
        break;
    case '\b':                        /* Backspace */
        if (tb->cur->cur_count > 0)
            tb->cur->cur_count--;
        break;
    case '\v':                        /* Vertical Tab */
        break;
    default:
        tb->cur->buf[tb->cur->cur_count] = c;
        tb->cur->col[tb->cur->cur_count] = tc->palette;
        tb->cur->cur_count++;
        if (tb->cur->cur_count > LINE_BUF_X_SIZE)
        {
            tb->cur->cur_count = LINE_BUF_X_SIZE;
            break;
        }
        if (tb->cur->cur_count > tb->cur->last_count)
        {
            tb->cur->last_count = tb->cur->cur_count;
        }
        break;
    }
}


int init_text_buf_sys(void *heap_start, void *heap_end)
{
    return init_lb_heap(heap_start, heap_end);
}


static void put_char_vram(TEXT_CTRL * tc, int c, u16 col)
{

    switch (c)
    {
    case '\0':
        break;
    case '\n':
        clear_line(tc);
        tc->x_line = 0;
        add_y_line(tc, 1);
        break;
    case '\r':
        tc->x_line = 0;
        break;
    case '\t':
        add_x_line(tc, TAB_SIZE);
        break;
    case '\f':
        tc->x_line = 0;
        tc->y_line = 0;
        clear_line(tc);
        break;
    case '\b':                        /* Backspace */
    case '\v':                        /* Vertical Tab */
        break;
    default:
        if ((c < 0x20) || (0x7f < c))
            c = 0x20;                  /* Whitespace */
        tc->screen[(tc->y_line * X_LINE_MAX) + tc->x_line] = (u16)((col << 12) | (0x00ff & c));
        add_x_line(tc, 1);
    }
}

void text_buf_to_vram(TEXT_CTRL * tc)
{
    int     x_line, line_no;
    int     c;
    u16     col;
    LINE_BUF *lb;
    TEXT_BUF *tb = &(tc->text_buf);


    lb = tb->start;
    put_char_vram(tc, '\f', (u16)0);

    for (line_no = 0; line_no < (*tb->num_y + 1); line_no++)
    {
        x_line = 0;
        if (lb == NULL)
            break;
        while (x_line < lb->last_count)
        {
            if (x_line >= (*tb->num_x + 1))
            {
                break;
            }
            else
            {
                c = lb->buf[x_line];
                col = lb->col[x_line];
                put_char_vram(tc, c, col);
            }
            x_line++;
        }

#if 1
        if (x_line <= *tb->num_x)
        {
            put_char_vram(tc, '\n', (u16)0);
        }
#endif
        lb = lb->next;
    }

    for (; line_no <= *tb->num_y; line_no++)
    {
        put_char_vram(tc, '\n', (u16)0);
    }
}


void mprintf(const char *text, ...)
{
    va_list vlist;
    char    temp[32];
    s32     i = 0;
    OSIntrMode enabled;

    enabled = OS_DisableInterrupts();

    va_start(vlist, text);
    (void)OS_VSNPrintf(temp, 32, text, vlist);
    va_end(vlist);

    for (i = 0; i < 32; i++)
    {
        m_putchar(tc[0], (int)temp[i]);
        if (temp[i] == NULL)
        {
            break;
        }
    }
#if 1
    if (i >= 31)
        m_putchar(tc[0], '\n');
#endif
    (void)OS_RestoreInterrupts(enabled);


}

#if 0
0x42 0x6C 0x6F 0x63 0x6B 0x49 0x64 0x3D
    0x31 0x30 0x30 0x30 0x31 0x20 0x73 0x69
    0x7A 0x65 0x3D 0x36 0x35 0x35 0x33 0x36
    0x0A 0x00
    0x20 0x20 0x55 0x73 0x65 0x72 0x49 0x64
    0x20 0x42 0x54 0x20 0x74 0x65 0x78 0x74
    0x20 0x69 0x6E 0x66 0x6F 0x72 0x6D 0x61
    0x74 0x69 0x6F 0x6E 0x20 0x61 0x72 0x00
    0x20 0x20 0x00
    0x42 0x6C 0x6F 0x63 0x6B 0x49 0x64 0x3D
    0x31 0x30 0x30 0x30 0x32 0x20 0x73 0x69
    0x7A 0x65 0x3D 0x36 0x35 0x35 0x33 0x36
    0x0A 0x00
    0x20 0x20 0x55 0x73 0x65 0x72 0x49 0x64
    0x20 0x42 0x54 0x20 0x74 0x65 0x78 0x74
    0x20 0x69 0x6E 0x66 0x6F 0x72 0x6D 0x61 0x74 0x69 0x6F 0x6E 0x20 0x61 0x72 0x00 0x3D 0x3E 0x00
#endif
void mfprintf(TEXT_CTRL * tc, const char *text, ...)
{
    va_list vlist;

    va_start(vlist, text);
    mfvprintf(tc, text, vlist);
    va_end(vlist);
}

void mfvprintf(TEXT_CTRL * tc, const char *text, va_list va)
{
    OSIntrMode enabled;
    char    temp[32];
    s32     i = 0;

    enabled = OS_DisableInterrupts();

    (void)OS_VSNPrintf(temp, 32, text, va);
    for (i = 0; i < 32; i++)
    {
        m_putchar(tc, (int)temp[i]);
        if (temp[i] == '\0')
        {
            break;
        }
    }
    if (i >= 31)
        m_putchar(tc, '\n');
    (void)OS_RestoreInterrupts(enabled);
}
