#include "console.h"
#include "gl.h"
#include "mailbox.h"
#include "fb.h"
#include <stdarg.h>
#include "malloc.h"
#include "printf.h"

#define MAX_OUTPUT_LEN 1024
static int numrows;
static int numcols;
static int cursorrow;
static int cursorcol;
static int textlength;
static int charcount;
static char* buf;


void console_init(unsigned int nrows, unsigned int ncols)
{
    buf = malloc(nrows * ncols);
    gl_init(ncols * gl_get_char_width(), nrows * gl_get_char_height(), FB_DOUBLEBUFFER);
    numrows = nrows;
    numcols = ncols;
    cursorrow = 0;
    cursorcol = 0;
    textlength = 0;
    charcount = 0;
}

void console_clear(void)
{
    gl_clear(GL_BLACK);
}

void print_buffer(void)
{
    int row = 0;
    int col = 0;
    for (int i = 0; i < textlength; i++ ){
        if (buf[i] == '\n') {
            row = 0;
            col++;
        } else if (buf[i] == '\f') {
            console_clear();
            row = 0;
            col = 0;
        } else if (buf[i] == '\b') {
            if (row == 0) {
                row = numcols - 1;
                col--;
                gl_draw_rect(row * gl_get_char_width(), col * gl_get_char_height(), gl_get_char_width() * 1.5, gl_get_char_height(), GL_BLACK);
            } else {
                row--;
                gl_draw_rect(row * gl_get_char_width(), col * gl_get_char_height(), gl_get_char_width() * 1.5, gl_get_char_height(), GL_BLACK);
            }
        } else if (row == numcols) {
            row = 0;
            col++;
            gl_draw_char(row * gl_get_char_width(), col * gl_get_char_height(), buf[i], GL_GREEN);
            row++;
        } else {
            gl_draw_char(row * gl_get_char_width(), col * gl_get_char_height(), buf[i], GL_GREEN);
            row++;
        }
    }
}

int console_printf(const char *format, ...)
{
    char textbuf[MAX_OUTPUT_LEN];
    va_list args;
    va_start(args, format);
    
    int length = vsnprintf(textbuf, MAX_OUTPUT_LEN, format, args);
    //    printf("length = %d \n", length);
    //    printf("textlength = %d \n", textlength);
    //    printf("charcount = %d \n", charcount);
    //    printf("numcols*numrows = %d\n", (numrows * numcols));
    for (int i = 0; i < length; i++) {
        // new line
        if (textbuf[i] == '\n') {
            buf[textlength + i] = textbuf[i];
            charcount += (numcols - 1 - cursorrow);
            cursorrow = 0;
            cursorcol++;
        } else if (textbuf[i] == '\f'){
            buf[textlength + i] = textbuf[i];
            console_clear();
            charcount = 0;
            textlength -= (textlength + i + 1);
            cursorrow = 0;
            cursorcol = 0;
            //backspace
        } else if (textbuf[i] == '\b') {
            buf[textlength + i] = textbuf[i];
            charcount--;
            if (cursorrow == 0) {
                cursorrow = numcols - 1;
                cursorcol--;
            } else {
                cursorrow--;
            }
            // scroll
        } else if (charcount >= (numrows * numcols)) {
            for (int n = 0; n < ((numcols)*(numrows-1)); n++) {
                buf[n] = buf[n+numcols];
            }
            cursorcol = numrows-1;
            cursorrow = 0;
            textlength -= numcols;
            charcount -= numcols;
            buf[textlength + i] = textbuf[i];
            cursorrow++;
            charcount++;
        } else {
            buf[textlength + i] = textbuf[i];
            charcount++;
            if (cursorrow == (numcols - 1)) {
                cursorrow = 0;
                cursorcol++;
            } else {
                cursorrow++;
            }
        }
    }
    
    textlength += length;
    console_clear();
    print_buffer();
    va_end(args);
    gl_swap_buffer();
    
    return textlength;
}
}
