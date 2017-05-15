#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/time.h>
#include "address_map_arm_brl4.h"

typedef unsigned int fix_3_29; // 3:29 fixed point

// returns a float f to fixed point representation
#define float2fix(a) ((fix_3_29)((a)*536870912.0) & 0xFFFFFFFF)

// Graphics macros
#define SLIDER_X_INDENT         50
#define SLIDER_Y_INDENT         38
#define SLIDER_SPACE            10
#define SLIDER_WIDTH           509
#define SLIDER_BUTTON_WIDTH     10
#define SLIDER_COLOR          0xff
#define BOTTOM_SLIDER_Y_OFFSET  (22 + SLIDER_BUTTON_WIDTH)
#define SLIDER_START (SLIDER_X_INDENT + SLIDER_BUTTON_WIDTH + SLIDER_SPACE)
#define SLIDER_END (SLIDER_X_INDENT + SLIDER_BUTTON_WIDTH + SLIDER_SPACE + SLIDER_WIDTH)
#define SLIDER_BAR_WIDTH         5
#define SLIDER_BAR_COLOR      0xe0
#define BUTTON_STEP             21
#define INIT_SLIDER_BAR         (SLIDER_START + 12*BUTTON_STEP)
#define BLACK                 0x00
#define HALF_STEP_UP     (1.0594630944)
#define HALF_STEP_DOWN   (0.9438743127)
#define TRIANGLE_STEP    (4)

#define TL_BUTTON_X_START    (SLIDER_X_INDENT)
#define TL_BUTTON_X_END      (SLIDER_X_INDENT + SLIDER_BUTTON_WIDTH)
#define TL_BUTTON_Y_START    (SLIDER_Y_INDENT)
#define TL_BUTTON_Y_END      (SLIDER_Y_INDENT + SLIDER_BUTTON_WIDTH)

#define TR_BUTTON_X_START    (SLIDER_X_INDENT + SLIDER_BUTTON_WIDTH + SLIDER_SPACE * 2 + SLIDER_WIDTH)
#define TR_BUTTON_Y_START    (SLIDER_Y_INDENT)
#define TR_BUTTON_X_END      (SLIDER_X_INDENT + 2 * SLIDER_BUTTON_WIDTH + SLIDER_SPACE * 2 + SLIDER_WIDTH)
#define TR_BUTTON_Y_END      (SLIDER_Y_INDENT + SLIDER_BUTTON_WIDTH)

#define BL_BUTTON_X_START    (SLIDER_X_INDENT)
#define BL_BUTTON_Y_START    (SLIDER_Y_INDENT + BOTTOM_SLIDER_Y_OFFSET)
#define BL_BUTTON_X_END      (SLIDER_X_INDENT + SLIDER_BUTTON_WIDTH)
#define BL_BUTTON_Y_END      (SLIDER_Y_INDENT + BOTTOM_SLIDER_Y_OFFSET + SLIDER_BUTTON_WIDTH)

#define BR_BUTTON_X_START    (SLIDER_X_INDENT + SLIDER_BUTTON_WIDTH + SLIDER_SPACE * 2 + SLIDER_WIDTH)
#define BR_BUTTON_Y_START    (SLIDER_Y_INDENT + BOTTOM_SLIDER_Y_OFFSET)
#define BR_BUTTON_X_END      (SLIDER_X_INDENT + 2 * SLIDER_BUTTON_WIDTH + SLIDER_SPACE * 2 + SLIDER_WIDTH)
#define BR_BUTTON_Y_END      (SLIDER_Y_INDENT + BOTTOM_SLIDER_Y_OFFSET + SLIDER_BUTTON_WIDTH)

#define BUTTON_ROW_Y          100
#define BUTTON_ROW_X          183 
#define BUTTON_WIDTH          60
#define TRIANGLE_OFFSET       397
#define TRIANGLE_MODE_COLOR   0xe0

#define TRIANGLE_LEFT_X_START        (BUTTON_ROW_X)
#define TRIANGLE_LEFT_X_END          (BUTTON_ROW_X + BUTTON_WIDTH)
#define TRIANGLE_LEFT_Y_START        (BUTTON_ROW_Y)
#define TRIANGLE_LEFT_Y_END          (BUTTON_ROW_Y + BUTTON_WIDTH)
#define TRIANGLE_LEFT_COLOR          0x03

#define TRIANGLE_RIGHT_X_START     (TRIANGLE_OFFSET)
#define TRIANGLE_RIGHT_X_END       (BUTTON_WIDTH + TRIANGLE_OFFSET)
#define TRIANGLE_RIGHT_Y_START     (BUTTON_ROW_Y)
#define TRIANGLE_RIGHT_Y_END       (BUTTON_ROW_Y + BUTTON_WIDTH)
#define TRIANGLE_RIGHT_COLOR       0x03

#define RESET_X_START    290 
#define RESET_X_END      350
#define RESET_Y_START    240
#define RESET_Y_END      300
#define RESET_COLOR      0x1c //Green

// function prototypes
void VGA_text(int, int, char*);
void VGA_text_clear();
void VGA_box(int, int, int, int, short);
void VGA_line(int, int, int, int, short);
void VGA_disc(int, int, int, short);
char VGA_PIXEL_GET_COLOR(int, int);
void delay(int x) {
  int i,j;
  for (i = 0; i < 256; i++)
    for (j = 0; j < x; j++) {}
}

// pixel macro
#define VGA_PIXEL(x,y,color) do{                        \
    char* pixel_ptr;                                    \
    pixel_ptr = (char*)vga_pixel_ptr + ((y)<<10) + (x); \
    *(char*)pixel_ptr = (color);                        \
  } while(0)

// the light weight bus base
void *h2p_lw_virtual_base;

volatile unsigned int* h2p_lw_reset            = NULL;
volatile unsigned int* h2p_lw_delta_left       = NULL;
volatile unsigned int* h2p_lw_delta_right      = NULL;
volatile unsigned int* h2p_lw_triangle_right   = NULL;
volatile unsigned int* h2p_lw_triangle_left    = NULL;
volatile unsigned int* h2p_lw_delta_mode_right = NULL;
volatile unsigned int* h2p_lw_delta_mode_left  = NULL;

// pixel buffer
volatile unsigned int* vga_pixel_ptr = NULL;
void* vga_pixel_virtual_base;

// character buffer
volatile unsigned int* vga_char_ptr = NULL;
void* vga_char_virtual_base;

// /dev/mem file id
int fd;

// mouse variables
int fmouse; // /dev/input/mice
int bytes; // bytes read from mouse
unsigned char mouse_data[3]; // mouse clicking and movement data
const char* pDevice = "/dev/input/mice"; // mouse location

int main(void){

  //////////////////////////////////////////////////////////////////////////////
  // SETUP I/O
  //////////////////////////////////////////////////////////////////////////////

  // get FPGA addresses
  fd = open("/dev/mem", (O_RDWR | O_SYNC));
  if (fd == -1){
    printf( "ERROR: could not open \"/dev/mem\"...\n" );
    return(1);
  }

  // get virtual addr that maps to physical
  h2p_lw_virtual_base = mmap(NULL, HW_REGS_SPAN, (PROT_READ | PROT_WRITE),
                             MAP_SHARED, fd, HW_REGS_BASE);
  if (h2p_lw_virtual_base == MAP_FAILED){
    printf("ERROR: mmap() failed...\n");
    close(fd);
    return(1);
  }

  // get virtual addr that maps to physical
  vga_pixel_virtual_base = mmap(NULL, FPGA_ONCHIP_SPAN, (PROT_READ | PROT_WRITE), MAP_SHARED, fd, FPGA_ONCHIP_BASE);
  if (vga_pixel_virtual_base == MAP_FAILED) {
    printf("ERROR: mmap3() failed...\n");
    close(fd);
    return(1);
  }

  vga_pixel_ptr = (unsigned int*)(vga_pixel_virtual_base);
  // === get VGA char addr =====================
  // get virtual addr that maps to physical
  vga_char_virtual_base = mmap(NULL, FPGA_CHAR_SPAN, (PROT_READ | PROT_WRITE),
                               MAP_SHARED, fd, FPGA_CHAR_BASE);
  if( vga_char_virtual_base == MAP_FAILED ){
    printf( "ERROR: mmap2() failed...\n" );
    close( fd );
    return(1);
  }

  // Get the address that maps to the char buffer control
  vga_char_ptr =(unsigned int*)(vga_char_virtual_base);

  // get pointers
  h2p_lw_reset           = (unsigned int*)(h2p_lw_virtual_base + (RESET_BASE));
  h2p_lw_delta_left      = (unsigned int*)(h2p_lw_virtual_base + (DELTA_LEFT_BASE));
  h2p_lw_delta_right     = (unsigned int*)(h2p_lw_virtual_base + (DELTA_RIGHT_BASE));
  h2p_lw_triangle_right  = (unsigned int*)(h2p_lw_virtual_base + (TRIANGLE_RIGHT_BASE));
  h2p_lw_triangle_left   = (unsigned int*)(h2p_lw_virtual_base + (TRIANGLE_LEFT_BASE));
  h2p_lw_delta_mode_left = (unsigned int*)(h2p_lw_virtual_base + (DELTA_MODE_LEFT_BASE));
  h2p_lw_delta_mode_right = (unsigned int*)(h2p_lw_virtual_base + (DELTA_MODE_RIGHT_BASE));

  // Open the mouse
  fmouse = open(pDevice, O_RDWR | O_NONBLOCK);
  if (fmouse == -1){
    printf("ERROR Opening %s\n", pDevice);
    close(fmouse);
    return -1;
  }

  // Read mouse
  int left, middle, right;
  int cursor_x, cursor_y;
  signed char x, y;
  char old_color[4];

  // initial mouse to the top left corner
  cursor_x = 1;
  cursor_y = 1;
  old_color[0] = VGA_PIXEL_GET_COLOR(cursor_x, cursor_y);
  old_color[1] = VGA_PIXEL_GET_COLOR(cursor_x+1, cursor_y);
  old_color[2] = VGA_PIXEL_GET_COLOR(cursor_x, cursor_y+1);
  old_color[3] = VGA_PIXEL_GET_COLOR(cursor_x+1, cursor_y+1);

  //////////////////////////////////////////////////////////////////////////////
  // MAIN LOOP
  //////////////////////////////////////////////////////////////////////////////

  int i;

  char text_top_row[40] = "DE1-SoC ARM/FPGA\0";
  char text_bottom_row[40] = "Cornell ece5760 swc63 jmt329 gzm3\0";

  char reset_text[40] = "RESET\0";
  char left_text[40] = "LEFT\0";
  char right_text[40] = "RIGHT\0";
  char left_delta_text[40] = "\0";
  char right_delta_text[40] = "\0";
  char left_triangle_text[40] = "\0";
  char right_triangle_text[40] = "\0";

  char triangle_mode[40] = "Modulation Mode \0";
  char delta_mode[40]    = "Pitch Shift Mode\0";

  VGA_text_clear();
  VGA_box(0, 0, 639, 479, 0x00);
  VGA_text (1, 1, text_top_row);
  VGA_text (1, 2, text_bottom_row);
  VGA_text (38, 33, reset_text);
  VGA_text (1, 5, left_text);
  VGA_text (1, 9, right_text);
  VGA_text (25, 16, left_text);
  VGA_text (51, 16, right_text);

  // Triangle Left Button
  VGA_box(TRIANGLE_LEFT_X_START,
          TRIANGLE_LEFT_Y_START,
          TRIANGLE_LEFT_X_END,
          TRIANGLE_LEFT_Y_END,
          TRIANGLE_LEFT_COLOR);

  // Triangle Right Button
  VGA_box(TRIANGLE_RIGHT_X_START,
          TRIANGLE_RIGHT_Y_START,
          TRIANGLE_RIGHT_X_END,
          TRIANGLE_RIGHT_Y_END,
          TRIANGLE_RIGHT_COLOR);

  // Reset Button
  VGA_box(RESET_X_START,
          RESET_Y_START,
          RESET_X_END,
          RESET_Y_END,
          RESET_COLOR);

  // Top slider left button
  VGA_box(TL_BUTTON_X_START,
          TL_BUTTON_Y_START,
          TL_BUTTON_X_END,
          TL_BUTTON_Y_END,
          SLIDER_COLOR);
  // Top slider right button
  VGA_box(TR_BUTTON_X_START,
          TR_BUTTON_Y_START,
          TR_BUTTON_X_END,
          TR_BUTTON_Y_END,
          SLIDER_COLOR);
  // Bottom slider left button
  VGA_box(BL_BUTTON_X_START,
          BL_BUTTON_Y_START,
          BL_BUTTON_X_END,
          BL_BUTTON_Y_END,
          SLIDER_COLOR);
  // Bottom slider right button
  VGA_box(BR_BUTTON_X_START,
          BR_BUTTON_Y_START,
          BR_BUTTON_X_END,
          BR_BUTTON_Y_END,
          SLIDER_COLOR);
  // Top Slider
  VGA_box(SLIDER_START,
          SLIDER_Y_INDENT,
          SLIDER_END,
          SLIDER_Y_INDENT + SLIDER_BUTTON_WIDTH,
          SLIDER_COLOR);
  // Bottom Slider
  VGA_box(SLIDER_START,
          SLIDER_Y_INDENT + BOTTOM_SLIDER_Y_OFFSET,
          SLIDER_END,
          SLIDER_Y_INDENT + BOTTOM_SLIDER_Y_OFFSET + SLIDER_BUTTON_WIDTH,
          SLIDER_COLOR);

  // Initialize slider bar for top
  int top_slider_x = INIT_SLIDER_BAR;
  VGA_box(top_slider_x,
          SLIDER_Y_INDENT,
          top_slider_x + SLIDER_BAR_WIDTH,
          SLIDER_Y_INDENT + SLIDER_BUTTON_WIDTH,
          SLIDER_BAR_COLOR);
  // Initialize slider bar for bottom
  int bottom_slider_x = INIT_SLIDER_BAR;
  VGA_box(bottom_slider_x,
          SLIDER_Y_INDENT + BOTTOM_SLIDER_Y_OFFSET,
          bottom_slider_x + SLIDER_BAR_WIDTH,
          SLIDER_Y_INDENT + BOTTOM_SLIDER_Y_OFFSET + SLIDER_BUTTON_WIDTH,
          SLIDER_BAR_COLOR);


  *h2p_lw_reset            = 0;
  *h2p_lw_delta_left       = (float2fix(1.0));
  double delta_left        = 1.0;
  *h2p_lw_delta_right      = (float2fix(1.0));
  double delta_right       = 1.0;
  *h2p_lw_delta_mode_right = 1;
  *h2p_lw_delta_mode_left  = 1;
  *h2p_lw_triangle_right   = 188;
  *h2p_lw_triangle_left    = 188;

  if (*h2p_lw_delta_mode_left){
    sprintf(left_delta_text, "%.5f\0", delta_left);
    VGA_text(20, 21, delta_mode);
  }
  else{
    sprintf(left_delta_text, "%.5f\0", 513.0*((float)(*h2p_lw_triangle_left))/(96000.0));
    VGA_text(20, 21, triangle_mode);
  }
  VGA_text(36, 7, "                                      \0");
  VGA_text(36, 7, left_delta_text);

  if (*h2p_lw_delta_mode_right){
    sprintf(right_delta_text, "%.5f\0", delta_right);
    VGA_text(45, 21, delta_mode);
  }
  else{
    sprintf(right_delta_text, "%.5f\0", 513.0*((float)(*h2p_lw_triangle_right))/(96000.0));
    VGA_text(45, 21, triangle_mode);
  }
  VGA_text(36, 11, "                                      \0");
  VGA_text(36, 11, right_delta_text);
  while(1){
    bytes = read(fmouse, mouse_data, sizeof(mouse_data));
    if (bytes > 0){
      left   = mouse_data[0] & 0x1;
      right  = mouse_data[0] & 0x2;
      middle = mouse_data[0] & 0x4;

      x = mouse_data[1];
      y = mouse_data[2];

      // reset pixel where cursor was
      //VGA_box(cursor_x, cursor_y, cursor_x+2, cursor_y+2, 0xff);
      VGA_PIXEL(cursor_x, cursor_y, old_color[0]);
      VGA_PIXEL(cursor_x+1, cursor_y, old_color[1]);
      VGA_PIXEL(cursor_x, cursor_y+1, old_color[2]);
      VGA_PIXEL(cursor_x+1, cursor_y+1, old_color[3]);

      // update cursor position
      cursor_x += x;
      cursor_y -= y;
      if(cursor_x < 0)    cursor_x = 0;
      if(cursor_x >= 639) cursor_x = 638;
      if(cursor_y < 0)    cursor_y = 0;
      if(cursor_y >= 479) cursor_y = 478;
      //VGA_box(cursor_x, cursor_y, cursor_x+2, cursor_y+2, 0x1c);
      old_color[0] = VGA_PIXEL_GET_COLOR(cursor_x, cursor_y);
      old_color[1] = VGA_PIXEL_GET_COLOR(cursor_x+1, cursor_y);
      old_color[2] = VGA_PIXEL_GET_COLOR(cursor_x, cursor_y+1);
      old_color[3] = VGA_PIXEL_GET_COLOR(cursor_x+1, cursor_y+1);
      VGA_PIXEL(cursor_x, cursor_y, 0xe0);
      VGA_PIXEL(cursor_x+1, cursor_y, 0xe0);
      VGA_PIXEL(cursor_x, cursor_y+1, 0xe0);
      VGA_PIXEL(cursor_x+1, cursor_y+1, 0xe0);

      // check mouse clicks
      if (left){
        // on left click
        // If the mouse is on one of the slider buttons, redraw slider bar, set new delta

        // Top Slider
        // Top left slider button
        if (cursor_x > TL_BUTTON_X_START && cursor_x < TL_BUTTON_X_END &&
            cursor_y > TL_BUTTON_Y_START && cursor_y < TL_BUTTON_Y_END){
          // Move down if possible
          if (top_slider_x >= SLIDER_START + BUTTON_STEP){
            // Draw over old position
            VGA_box(top_slider_x,
                    SLIDER_Y_INDENT,
                    top_slider_x + SLIDER_BAR_WIDTH,
                    SLIDER_Y_INDENT + SLIDER_BUTTON_WIDTH,
                    SLIDER_COLOR);
            // Update variables, delta
            top_slider_x -= BUTTON_STEP;
            if (*h2p_lw_delta_mode_left){
              delta_left = delta_left * (HALF_STEP_DOWN);
              *h2p_lw_delta_left  = (float2fix(delta_left));
            }
            else{
              *h2p_lw_triangle_left += TRIANGLE_STEP;
            }
            // Draw new position
            VGA_box(top_slider_x,
                    SLIDER_Y_INDENT,
                    top_slider_x + SLIDER_BAR_WIDTH,
                    SLIDER_Y_INDENT + SLIDER_BUTTON_WIDTH,
                    SLIDER_BAR_COLOR);
          }
        }
        // Top right slider button
        if (cursor_x > TR_BUTTON_X_START && cursor_x < TR_BUTTON_X_END &&
            cursor_y > TR_BUTTON_Y_START && cursor_y < TR_BUTTON_Y_END){
          if ((top_slider_x + SLIDER_BAR_WIDTH) <= (SLIDER_END - BUTTON_STEP)){
            // Draw over old position
            VGA_box(top_slider_x,
                    SLIDER_Y_INDENT,
                    top_slider_x + SLIDER_BAR_WIDTH,
                    SLIDER_Y_INDENT + SLIDER_BUTTON_WIDTH,
                    SLIDER_COLOR);
            // Update variables, delta
            top_slider_x += BUTTON_STEP;
            if (*h2p_lw_delta_mode_left){
              delta_left = delta_left * (HALF_STEP_UP);
              *h2p_lw_delta_left  = (float2fix(delta_left));
            }
            else{
              *h2p_lw_triangle_left -= TRIANGLE_STEP;
            }
            // Draw new position
            VGA_box(top_slider_x,
                    SLIDER_Y_INDENT,
                    top_slider_x + SLIDER_BAR_WIDTH,
                    SLIDER_Y_INDENT + SLIDER_BUTTON_WIDTH,
                    SLIDER_BAR_COLOR);
          }

        }
        printf("LEFT:  %f\n", delta_left);
        printf("LEFT:  %08X\n", (*h2p_lw_delta_left));

        // Bottom Slider
        // Bottom left slider button
        if (cursor_x > BL_BUTTON_X_START && cursor_x < BL_BUTTON_X_END &&
            cursor_y > BL_BUTTON_Y_START && cursor_y < BL_BUTTON_Y_END){
          // Move down if possible
          if (bottom_slider_x >= SLIDER_START + BUTTON_STEP){
            // Draw over old position
            VGA_box(bottom_slider_x,
                    SLIDER_Y_INDENT + BOTTOM_SLIDER_Y_OFFSET,
                    bottom_slider_x + SLIDER_BAR_WIDTH,
                    SLIDER_Y_INDENT + BOTTOM_SLIDER_Y_OFFSET + SLIDER_BUTTON_WIDTH,
                    SLIDER_COLOR);
            // Update variables, delta
            bottom_slider_x -= BUTTON_STEP;
            if (*h2p_lw_delta_mode_right){
              delta_right = delta_right * (HALF_STEP_DOWN);
              *h2p_lw_delta_right  = (float2fix(delta_right));
            }
            else{
              *h2p_lw_triangle_right += TRIANGLE_STEP;
            }
            // Draw new position
            VGA_box(bottom_slider_x,
                    SLIDER_Y_INDENT + BOTTOM_SLIDER_Y_OFFSET,
                    bottom_slider_x + SLIDER_BAR_WIDTH,
                    SLIDER_Y_INDENT + BOTTOM_SLIDER_Y_OFFSET + SLIDER_BUTTON_WIDTH,
                    SLIDER_BAR_COLOR);
          }
        }
        // Bottom right slider button
        if (cursor_x > BR_BUTTON_X_START && cursor_x < BR_BUTTON_X_END &&
            cursor_y > BR_BUTTON_Y_START && cursor_y < BR_BUTTON_Y_END){
          if ((bottom_slider_x + SLIDER_BAR_WIDTH) <= (SLIDER_END - BUTTON_STEP)){
            // Draw over old position
            VGA_box(bottom_slider_x,
                    SLIDER_Y_INDENT + BOTTOM_SLIDER_Y_OFFSET,
                    bottom_slider_x + SLIDER_BAR_WIDTH,
                    SLIDER_Y_INDENT + BOTTOM_SLIDER_Y_OFFSET + SLIDER_BUTTON_WIDTH,
                    SLIDER_COLOR);
            // Update variables, delta
            bottom_slider_x += BUTTON_STEP;
            if (*h2p_lw_delta_mode_right){
              delta_right = delta_right * (HALF_STEP_UP);
              *h2p_lw_delta_right  = (float2fix(delta_right));
            }
            else{
              *h2p_lw_triangle_right -= TRIANGLE_STEP;
            }
            // Draw new position
            VGA_box(bottom_slider_x,
                    SLIDER_Y_INDENT + BOTTOM_SLIDER_Y_OFFSET,
                    bottom_slider_x + SLIDER_BAR_WIDTH,
                    SLIDER_Y_INDENT + BOTTOM_SLIDER_Y_OFFSET + SLIDER_BUTTON_WIDTH,
                    SLIDER_BAR_COLOR);
          }
        }
        printf("RIGHT: %f\n", delta_right);
        printf("RIGHT: %08X\n", (*h2p_lw_delta_right));


        // Triangle Left button
        // Put sliders back into middle, starting position, reset deltas/triangle settings
        if (cursor_x > TRIANGLE_LEFT_X_START && cursor_x < TRIANGLE_LEFT_X_END &&
            cursor_y > TRIANGLE_LEFT_Y_START && cursor_y < TRIANGLE_LEFT_Y_END){
          // Toggle the left triangle/delta mode
          if (*h2p_lw_delta_mode_left){
            // Change to triangle mode
            *h2p_lw_delta_mode_left = !(*h2p_lw_delta_mode_left);
            // Draw new color, indicating triangle mode
            // Triangle Left Button
            VGA_box(TRIANGLE_LEFT_X_START,
                    TRIANGLE_LEFT_Y_START,
                    TRIANGLE_LEFT_X_END,
                    TRIANGLE_LEFT_Y_END,
                    TRIANGLE_MODE_COLOR);
          }
          else{
            // Change to delta mode
            *h2p_lw_delta_mode_left = !(*h2p_lw_delta_mode_left);
            // Draw new color, indicating delta mode
            // Triangle Left Button
            VGA_box(TRIANGLE_LEFT_X_START,
                    TRIANGLE_LEFT_Y_START,
                    TRIANGLE_LEFT_X_END,
                    TRIANGLE_LEFT_Y_END,
                    TRIANGLE_LEFT_COLOR);
          }
          // Reset the sliders
          // Top Slider
          // Draw over old position
          VGA_box(top_slider_x,
                  SLIDER_Y_INDENT,
                  top_slider_x + SLIDER_BAR_WIDTH,
                  SLIDER_Y_INDENT + SLIDER_BUTTON_WIDTH,
                  SLIDER_COLOR);
          // Update variables, delta
          top_slider_x = INIT_SLIDER_BAR;
          delta_left = 1.0;
          *h2p_lw_delta_left  = (float2fix(delta_left));
          *h2p_lw_triangle_left = 188;
          // Draw new position
          VGA_box(top_slider_x,
                  SLIDER_Y_INDENT,
                  top_slider_x + SLIDER_BAR_WIDTH,
                  SLIDER_Y_INDENT + SLIDER_BUTTON_WIDTH,
                  SLIDER_BAR_COLOR);
        }

        // Triangle Right button
        // Put sliders back into middle, starting position, reset deltas/triangle settings
        if (cursor_x > TRIANGLE_RIGHT_X_START && cursor_x < TRIANGLE_RIGHT_X_END &&
            cursor_y > TRIANGLE_RIGHT_Y_START && cursor_y < TRIANGLE_RIGHT_Y_END){
          // Toggle the right triangle/delta mode
          if (*h2p_lw_delta_mode_right){
            // Change to triangle mode
            *h2p_lw_delta_mode_right = !(*h2p_lw_delta_mode_right);
            // Draw new color, indicating triangle mode
            // Triangle Right Button
            VGA_box(TRIANGLE_RIGHT_X_START,
                    TRIANGLE_RIGHT_Y_START,
                    TRIANGLE_RIGHT_X_END,
                    TRIANGLE_RIGHT_Y_END,
                    TRIANGLE_MODE_COLOR);
          }
          else{
            // Change to delta mode
            *h2p_lw_delta_mode_right = !(*h2p_lw_delta_mode_right);
            // Draw new color, indicating delta mode
            // Triangle Right Button
            VGA_box(TRIANGLE_RIGHT_X_START,
                    TRIANGLE_RIGHT_Y_START,
                    TRIANGLE_RIGHT_X_END,
                    TRIANGLE_RIGHT_Y_END,
                    TRIANGLE_RIGHT_COLOR);
          }
          // Reset the sliders
          // Bottom Slider
          // Draw over old position
          VGA_box(bottom_slider_x,
                  SLIDER_Y_INDENT + BOTTOM_SLIDER_Y_OFFSET,
                  bottom_slider_x + SLIDER_BAR_WIDTH,
                  SLIDER_Y_INDENT + BOTTOM_SLIDER_Y_OFFSET + SLIDER_BUTTON_WIDTH,
                  SLIDER_COLOR);
          // Update variables, delta
          bottom_slider_x = INIT_SLIDER_BAR;
          delta_right = 1.0;
          *h2p_lw_delta_right  = (float2fix(delta_right));
          *h2p_lw_triangle_right = 188;
          // Draw new position
          VGA_box(bottom_slider_x,
                  SLIDER_Y_INDENT + BOTTOM_SLIDER_Y_OFFSET,
                  bottom_slider_x + SLIDER_BAR_WIDTH,
                  SLIDER_Y_INDENT + BOTTOM_SLIDER_Y_OFFSET + SLIDER_BUTTON_WIDTH,
                  SLIDER_BAR_COLOR);
        }

        // Reset button
        // Put sliders back into middle, starting position, reset deltas/triangle settings
        if (cursor_x > RESET_X_START && cursor_x < RESET_X_END &&
            cursor_y > RESET_Y_START && cursor_y < RESET_Y_END){
          // Place into delta mode
          *h2p_lw_delta_mode_right = 1;
          // Draw new color, indicating delta mode
          // Triangle Right Button
          VGA_box(TRIANGLE_RIGHT_X_START,
                  TRIANGLE_RIGHT_Y_START,
                  TRIANGLE_RIGHT_X_END,
                  TRIANGLE_RIGHT_Y_END,
                  TRIANGLE_RIGHT_COLOR);
          *h2p_lw_delta_mode_left = 1;
          // Draw new color, indicating delta mode
          // Triangle Left Button
          VGA_box(TRIANGLE_LEFT_X_START,
                  TRIANGLE_LEFT_Y_START,
                  TRIANGLE_LEFT_X_END,
                  TRIANGLE_LEFT_Y_END,
                  TRIANGLE_LEFT_COLOR);
          // Reset the sliders
          // Top Slider
          // Draw over old position
          VGA_box(top_slider_x,
                  SLIDER_Y_INDENT,
                  top_slider_x + SLIDER_BAR_WIDTH,
                  SLIDER_Y_INDENT + SLIDER_BUTTON_WIDTH,
                  SLIDER_COLOR);
          // Update variables, delta
          top_slider_x = INIT_SLIDER_BAR;
          delta_left = 1.0;
          *h2p_lw_delta_left  = (float2fix(delta_left));
          *h2p_lw_triangle_left = 188;
          // Draw new position
          VGA_box(top_slider_x,
                  SLIDER_Y_INDENT,
                  top_slider_x + SLIDER_BAR_WIDTH,
                  SLIDER_Y_INDENT + SLIDER_BUTTON_WIDTH,
                  SLIDER_BAR_COLOR);
          // Bottom Slider
          // Draw over old position
          VGA_box(bottom_slider_x,
                  SLIDER_Y_INDENT + BOTTOM_SLIDER_Y_OFFSET,
                  bottom_slider_x + SLIDER_BAR_WIDTH,
                  SLIDER_Y_INDENT + BOTTOM_SLIDER_Y_OFFSET + SLIDER_BUTTON_WIDTH,
                  SLIDER_COLOR);
          // Update variables, delta
          bottom_slider_x = INIT_SLIDER_BAR;
          delta_right = 1.0;
          *h2p_lw_delta_right  = (float2fix(delta_right));
          *h2p_lw_triangle_right = 188;
          // Draw new position
          VGA_box(bottom_slider_x,
                  SLIDER_Y_INDENT + BOTTOM_SLIDER_Y_OFFSET,
                  bottom_slider_x + SLIDER_BAR_WIDTH,
                  SLIDER_Y_INDENT + BOTTOM_SLIDER_Y_OFFSET + SLIDER_BUTTON_WIDTH,
                  SLIDER_BAR_COLOR);
        }

        if (*h2p_lw_delta_mode_left){
          sprintf(left_delta_text, "%.5f\0", delta_left);
          VGA_text(20, 21, delta_mode);
        }
        else{
          sprintf(left_delta_text, "%.5f\0", 513.0*((float)(*h2p_lw_triangle_left))/(96000.0));
          VGA_text(20, 21, triangle_mode);
        }
        VGA_text(36, 7, "                                      \0");
        VGA_text(36, 7, left_delta_text);

        if (*h2p_lw_delta_mode_right){
          sprintf(right_delta_text, "%.5f\0", delta_right);
          VGA_text(45, 21, delta_mode);
        }
        else{
          sprintf(right_delta_text, "%.5f\0", 513.0*((float)(*h2p_lw_triangle_right))/(96000.0));
          VGA_text(45, 21, triangle_mode);
        }
        VGA_text(36, 11, "                                      \0");
        VGA_text(36, 11, right_delta_text);
        
      }

    } // end mouse checks

  }
  close(fd);
  close(fmouse);

  return 0;
}

char VGA_PIXEL_GET_COLOR(int x, int y) {
  char* pixel_ptr;
  pixel_ptr = (char*)vga_pixel_ptr + ((y)<<10) + (x);
  return *(char*)pixel_ptr;
}


/****************************************************************************************
 * Draw a filled rectangle on the VGA monitor
 ****************************************************************************************/
#define SWAP(X,Y) do{int temp=X; X=Y; Y=temp;}while(0)

// When mode is 0, draw the pixel with the given color
// When mode is non zero, draw the pixel with the current life colors
void VGA_box(int x1, int y1, int x2, int y2, short pixel_color){
  char* pixel_ptr;
  int   row, col;

  /* check and fix box coordinates to be valid */
  if (x1>639) x1 = 639;
  if (y1>479) y1 = 479;
  if (x2>639) x2 = 639;
  if (y2>479) y2 = 479;
  if (x1<0) x1 = 0;
  if (y1<0) y1 = 0;
  if (x2<0) x2 = 0;
  if (y2<0) y2 = 0;
  if (x1>x2) SWAP(x1,x2);
  if (y1>y2) SWAP(y1,y2);
  for (row = y1; row <= y2; row++){
    for (col = x1; col <= x2; ++col){
      //640x480
      pixel_ptr = (char*)vga_pixel_ptr + (row<<10) + col;
      // set pixel color
      *(char*)pixel_ptr = pixel_color;
    }
  }
}

/****************************************************************************************
 * Subroutine to send a string of text to the VGA monitor
 ****************************************************************************************/
void VGA_text(int x, int y, char * text_ptr){
  volatile char * character_buffer = (char *) vga_char_ptr; // VGA character buffer
  int offset;
  /* assume that the text string fits on one line */
  offset = (y << 7) + x;
  while ( *(text_ptr) ){
    // write to the character buffer
    *(character_buffer + offset) = *(text_ptr);
    ++text_ptr;
    ++offset;
  }
}

/****************************************************************************************
 * Subroutine to clear text to the VGA monitor
 ****************************************************************************************/
void VGA_text_clear(){
  volatile char* character_buffer = (char*) vga_char_ptr; // VGA character buffer
  int offset, x, y;
  for (x=0; x<70; x++){
    for (y=0; y<50; y++){
      // assume that the text string fits on one line
      offset = (y << 7) + x;
      // write to the character buffer
      *(character_buffer + offset) = ' ';
    }
  }
}

/****************************************************************************************
 * Draw a filled circle on the VGA monitor
 ****************************************************************************************/
void VGA_disc(int x, int y, int r, short pixel_color){
  char* pixel_ptr;
  int   row, col, rsqr, xc, yc;

  rsqr = r*r;

  for (yc = -r; yc <= r; yc++){
    for (xc = -r; xc <= r; xc++){
      col = xc;
      row = yc;
      // add the r to make the edge smoother
      if(col*col+row*row <= rsqr+r){
        col += x; // add the center point
        row += y; // add the center point
        //check for valid 640x480
        if (col>639) col = 639;
        if (row>479) row = 479;
        if (col<0) col = 0;
        if (row<0) row = 0;
        pixel_ptr = (char*)vga_pixel_ptr + (row<<10) + col;
        // set pixel color
        *(char*)pixel_ptr = pixel_color;
      }
    }
  }
}

// =============================================
// VGA_line
// =============================================
// plot a line
// at x1,y1 to x2,y2 with color
// Code is from David Rodgers,
// "Procedural Elements of Computer Graphics",1985
void VGA_line(int x1, int y1, int x2, int y2, short c){
  int e;
  signed int dx,dy,j, temp;
  signed int s1,s2, xchange;
  signed int x,y;
  char*      pixel_ptr;

  // check and fix line coordinates to be valid
  if (x1>639) x1 = 639;
  if (y1>479) y1 = 479;
  if (x2>639) x2 = 639;
  if (y2>479) y2 = 479;
  if (x1<0) x1 = 0;
  if (y1<0) y1 = 0;
  if (x2<0) x2 = 0;
  if (y2<0) y2 = 0;

  x = x1;
  y = y1;

  //take absolute value
  if (x2 < x1){
    dx = x1 - x2;
    s1 = -1;
  }
  else if (x2 == x1){
    dx = 0;
    s1 = 0;
  }
  else{
    dx = x2 - x1;
    s1 = 1;
  }

  if (y2 < y1){
    dy = y1 - y2;
    s2 = -1;
  }
  else if (y2 == y1){
    dy = 0;
    s2 = 0;
  }
  else{
    dy = y2 - y1;
    s2 = 1;
  }

  xchange = 0;

  if (dy>dx){
    temp = dx;
    dx = dy;
    dy = temp;
    xchange = 1;
  }

  e = ((int)dy<<1) - dx;

  for (j=0; j<=dx; j++){
    //video_pt(x,y,c); //640x480
    pixel_ptr = (char*)vga_pixel_ptr + (y<<10)+ x;
    // set pixel color
    *(char*)pixel_ptr = c;

    if (e>=0){
      if (xchange==1) x = x + s1;
      else y = y + s2;
      e = e - ((int)dx<<1);
    }

    if (xchange==1) y = y + s2;
    else x = x + s1;

    e = e + ((int)dy<<1);
  }
}
