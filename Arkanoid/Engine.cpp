#include "Engine.h"

#define _USE_MATH_DEFINES
#include <math.h>

enum ELetter_Type
{
   ELT_None,
   ELT_O
};

enum EBrick_Type
{
   EBT_None,
   EBT_Yellow,
   EBT_Purple
};

HWND HWnd;
HPEN Letter_Pen, BG_Pen, White_Pen, Brick_Yellow_Pen, Brick_Purple_Pen, Platform_Blue_Pen, Platform_Grey_Pen;
HBRUSH BG_Brush, White_Brush, Brick_Yellow_Brush, Brick_Purple_Brush, Platform_Blue_Brush, Platform_Grey_Brush;
RECT Level_Rect;
RECT Platform_Rect, Prev_Platform_Rect;

const int Brick_Width = 15;
const int Brick_Height = 7;
const int Cell_Width = Brick_Width + 1;
const int Cell_Height = Brick_Height + 1;
const int Brick_Rounding_Scaled = int(2.5f * (float)Global_Scale);
const int Level_X_Offset = 8;
const int Level_Y_Offset = 6;
const int Level_Width = 12; // Level width in cells
const int Level_Height = 14; // Level height in cells
const int Platform_Height = 12;
const int Platform_Height_Full = 12 + 3; // Platform height with all elements
const int Platform_Y_Pos = 185;

int Platform_Width = 28;
int Platform_X_Pos = 0 + Level_X_Offset + Level_Width * Cell_Width / 2 - Platform_Width / 2; // Set a start position of the platform
int Platform_X_Step = Global_Scale * 2;

char Level_01[Level_Height][Level_Width] =
{
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
   2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
   2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
   2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
   2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};
//------------------------------------------------------------------------------------------------------------

void Create_New_Pen_Brush(unsigned char r, unsigned char g, unsigned char b, HPEN &pen, HBRUSH &brush)
{
   pen = CreatePen(PS_SOLID, 0, RGB(r, g, b));
   brush = CreateSolidBrush(RGB(r, g, b));
};
//------------------------------------------------------------------------------------------------------------

void Redraw_Platform()
{
   Prev_Platform_Rect = Platform_Rect;

   Platform_Rect.left = Platform_X_Pos * Global_Scale;
   Platform_Rect.top = Platform_Y_Pos * Global_Scale;
   Platform_Rect.right = Platform_Rect.left + Platform_Width * Global_Scale;
   Platform_Rect.bottom = Platform_Rect.top + Platform_Height_Full * Global_Scale;

   UnionRect(&Platform_Rect, &Prev_Platform_Rect, &Platform_Rect); // A new optimized method (experimental)
   //InvalidateRect(HWnd, &Prev_Platform_Rect, FALSE); // The old method of removing the previous platform
   InvalidateRect(HWnd, &Platform_Rect, FALSE);
}
//------------------------------------------------------------------------------------------------------------

void Init_Engine(HWND hWnd)
{
   HWnd = hWnd;

   Letter_Pen = CreatePen(PS_SOLID, Global_Scale, RGB(255, 255, 255));

   Create_New_Pen_Brush(25, 25, 25, BG_Pen, BG_Brush);
   Create_New_Pen_Brush(255, 255, 255, White_Pen, White_Brush);
   Create_New_Pen_Brush(240, 240, 130, Brick_Yellow_Pen, Brick_Yellow_Brush);
   Create_New_Pen_Brush(240, 130, 240, Brick_Purple_Pen, Brick_Purple_Brush);
   Create_New_Pen_Brush(160, 230, 250, Platform_Blue_Pen, Platform_Blue_Brush);
   Create_New_Pen_Brush(110, 110, 110, Platform_Grey_Pen, Platform_Grey_Brush);

   Level_Rect.left = Level_X_Offset * Global_Scale;
   Level_Rect.top = Level_Y_Offset * Global_Scale;
   Level_Rect.right = Level_Rect.left + Cell_Width * Level_Width * Global_Scale;
   Level_Rect.bottom = Level_Rect.top + Cell_Height * Level_Height * Global_Scale;

   Redraw_Platform();
}
//------------------------------------------------------------------------------------------------------------

void Set_Brick_Letter_Colors(bool is_switch_color, EBrick_Type brick_type, HPEN &front_pen, HBRUSH &front_brush, HPEN& back_pen, HBRUSH& back_brush)
{
   if (is_switch_color)
   {
      front_pen = Brick_Purple_Pen;
      front_brush = Brick_Purple_Brush;

      back_pen = Brick_Yellow_Pen;
      back_brush = Brick_Yellow_Brush;
   }
   else
   {
      front_pen = Brick_Yellow_Pen;
      front_brush = Brick_Yellow_Brush;

      back_pen = Brick_Purple_Pen;
      back_brush = Brick_Purple_Brush;
   }
}
//------------------------------------------------------------------------------------------------------------

void Draw_Brick_Letter(HDC hdc, int x, int y, EBrick_Type brick_type, ELetter_Type letter_type, int rotation_step)
{
   bool switch_color;
   double rotation_angle, offset;
   int back_part_offset;
   int brick_half_height = Brick_Height * Global_Scale / 2;
   HPEN front_pen, back_pen;
   HBRUSH front_brush, back_brush;
   XFORM xform{}, old_xform{};
   
   if (!(brick_type == EBT_Yellow || brick_type == EBT_Purple))
      return; // A falling brick with a letter must be only one of these colors

   // Convert a rotation step to angle
   rotation_step = rotation_step % 16;
   
   if (rotation_step < 8)
      rotation_angle = 2.0 * M_PI / 16.0 * (double)rotation_step;
   else
      rotation_angle = 2.0 * M_PI / 16.0 * (double)(8 - rotation_step);

   // Choose the brick colors
   if (rotation_step > 4 && rotation_step <= 12)
      switch_color = brick_type == EBT_Yellow;
   else
      switch_color = brick_type == EBT_Purple;
   
   Set_Brick_Letter_Colors(switch_color, brick_type, front_pen, front_brush, back_pen, back_brush);

   // Draw the brick with a letter
   if (rotation_step == 4 || rotation_step == 12) // Actions for special cases
   {
      // Draw the back part of the brick
      SelectObject(hdc, back_pen);
      SelectObject(hdc, back_brush);
      RoundRect(hdc, x, y + brick_half_height - Global_Scale, x + Brick_Width * Global_Scale, y + brick_half_height + Global_Scale - 1, Brick_Rounding_Scaled, Brick_Rounding_Scaled);

      // Draw the front part of the brick
      SelectObject(hdc, front_pen);
      SelectObject(hdc, front_brush);
      RoundRect(hdc, x, y + brick_half_height, x + Brick_Width * Global_Scale, y + brick_half_height + Global_Scale - 1, Brick_Rounding_Scaled, Brick_Rounding_Scaled);
   }
   else // Actions for standard cases
   {
      SetGraphicsMode(hdc, GM_ADVANCED);

      // Set parameters of the rotation matrix
      xform.eM11 = 1.0f;
      xform.eM12 = 0.0f;
      xform.eM21 = 0.0f;
      xform.eM22 = (float)cos(rotation_angle);
      xform.eDx = (float)x;
      xform.eDy = (float)(y + brick_half_height);
      GetWorldTransform(hdc, &old_xform);
      SetWorldTransform(hdc, &xform);

      // Draw the back part of the brick
      SelectObject(hdc, back_pen);
      SelectObject(hdc, back_brush);

      offset = 3.0 * (1.0 - fabs(xform.eM22)) * (double)Global_Scale;
      back_part_offset = (int)round(offset);
      RoundRect(hdc, 0, -brick_half_height - back_part_offset, Brick_Width * Global_Scale, brick_half_height - back_part_offset, Brick_Rounding_Scaled, Brick_Rounding_Scaled);

      // Draw the front part of the brick
      SelectObject(hdc, front_pen);
      SelectObject(hdc, front_brush);
      RoundRect(hdc, 0, -brick_half_height, Brick_Width * Global_Scale, brick_half_height, Brick_Rounding_Scaled, Brick_Rounding_Scaled);

      // Draw a letter
      if (rotation_step > 4 && rotation_step <= 12)
      {
         if (letter_type == ELT_O)
         {
            SelectObject(hdc, Letter_Pen);
            Ellipse(hdc, 5 * Global_Scale, -5 * Global_Scale / 2, 10 * Global_Scale, 5 * Global_Scale / 2);
         }
      }

      SetWorldTransform(hdc, &old_xform);
   }
}
//------------------------------------------------------------------------------------------------------------

void Draw_Brick(HDC hdc, int x, int y, EBrick_Type brick_type)
{
   HPEN pen;
   HBRUSH brush;

   switch (brick_type)
   {
   case EBT_None:
      return;

   case EBT_Yellow:
      pen = Brick_Yellow_Pen;
      brush = Brick_Yellow_Brush;
      break;

   case EBT_Purple:
      pen = Brick_Purple_Pen;
      brush = Brick_Purple_Brush;
      break;

   default:
      return;
   }

   SelectObject(hdc, pen);
   SelectObject(hdc, brush);
   RoundRect(hdc, x * Global_Scale, y * Global_Scale, (x + Brick_Width) * Global_Scale, (y + Brick_Height) * Global_Scale, Brick_Rounding_Scaled, Brick_Rounding_Scaled);
}
//------------------------------------------------------------------------------------------------------------

void Draw_Level(HDC hdc)
{
   int i, j;

   for (i = 0; i < 14; i++)
      for (j = 0; j < 12; j++)
         Draw_Brick(hdc, Level_X_Offset + j * Cell_Width, Level_Y_Offset + i * Cell_Height, (EBrick_Type)Level_01[i][j]);
}
//------------------------------------------------------------------------------------------------------------

void Draw_Platform(HDC hdc, int x, int y)
{
   // Remove the previous platform
   SelectObject(hdc, BG_Pen);
   SelectObject(hdc, BG_Brush);
   Rectangle(hdc, Prev_Platform_Rect.left, Prev_Platform_Rect.top, Prev_Platform_Rect.right, Prev_Platform_Rect.bottom);

   // Create a new platform:
   // Base
   SelectObject(hdc, Platform_Grey_Pen);
   SelectObject(hdc, Platform_Grey_Brush);
   RoundRect(hdc, x * Global_Scale, y * Global_Scale, (x + Platform_Width) * Global_Scale, (y + Platform_Height) * Global_Scale, 3 * Global_Scale, 3 * Global_Scale);
   // Legs
   RoundRect(hdc, x * Global_Scale, y * Global_Scale, (x + 7) * Global_Scale, (y + Platform_Height_Full) * Global_Scale, 4 * Global_Scale, 4 * Global_Scale);
   RoundRect(hdc, (x + Platform_Width - 7) * Global_Scale, y * Global_Scale, (x + Platform_Width) * Global_Scale, (y + Platform_Height_Full) * Global_Scale, 4 * Global_Scale, 4 * Global_Scale);
   // Face
   SelectObject(hdc, Platform_Blue_Pen);
   SelectObject(hdc, Platform_Blue_Brush);
   RoundRect(hdc, (x + 1) * Global_Scale, (y + 1) * Global_Scale, (x + Platform_Width - 1) * Global_Scale, (y + Platform_Height - 1) * Global_Scale, 3 * Global_Scale, 3 * Global_Scale);
   // Eyes and mouth
   SelectObject(hdc, Platform_Grey_Pen);
   SelectObject(hdc, Platform_Grey_Brush);
   RoundRect(hdc, (x + 2) * Global_Scale, (y + 2) * Global_Scale, (x + 2 + 4) * Global_Scale, (y + 2 + 5) * Global_Scale, 2 * Global_Scale, 2 * Global_Scale);
   RoundRect(hdc, (x + Platform_Width - 2 - 4) * Global_Scale, (y + 2) * Global_Scale, (x + Platform_Width - 2) * Global_Scale, (y + 2 + 5) * Global_Scale, 2 * Global_Scale, 2 * Global_Scale);
   RoundRect(hdc, (x + (Platform_Width / 2) - 4) * Global_Scale, (y + Platform_Height - 2 - 2) * Global_Scale, (x + (Platform_Width / 2) + 4) * Global_Scale, (y + Platform_Height - 2) * Global_Scale, 2 * Global_Scale, 2 * Global_Scale);
   // Iris
   SelectObject(hdc, White_Pen);
   SelectObject(hdc, White_Brush);
   Rectangle(hdc, (x + 2 + 1) * Global_Scale, (y + 2 + 1) * Global_Scale, (x + 2 + 2) * Global_Scale, (y + 2 + 2) * Global_Scale);
   Rectangle(hdc, (x + Platform_Width - 2 - 4 + 1) * Global_Scale, (y + 2 + 1) * Global_Scale, (x + Platform_Width - 2 - 2) * Global_Scale, (y + 2 + 2) * Global_Scale);
}
//------------------------------------------------------------------------------------------------------------

void Draw_Frame(HDC hdc, RECT &paint_area)
{
   RECT intersection_rect;

   if (IntersectRect(&intersection_rect, &paint_area, &Level_Rect))
      Draw_Level(hdc);

   if (IntersectRect(&intersection_rect, &paint_area, &Platform_Rect))
      Draw_Platform(hdc, Platform_X_Pos, Platform_Y_Pos);

   int i;
   for (i = 0; i < 16; i++) 
   {
      Draw_Brick_Letter(hdc, 20 + i * Cell_Width * Global_Scale, 350, EBT_Yellow, ELT_O, i);
      Draw_Brick_Letter(hdc, 20 + i * Cell_Width * Global_Scale, 400, EBT_Purple, ELT_O, i);
   }
}
//------------------------------------------------------------------------------------------------------------

int On_Key_Down(EKey_Type key_type)
{
   switch (key_type)
   {
   case EKT_Left:
      Platform_X_Pos -= Platform_X_Step;
      Redraw_Platform();
      break;

   case EKT_Right:
      Platform_X_Pos += Platform_X_Step;
      Redraw_Platform();
      break;

   case EKT_Space:
      Beep(1000, 2);
      break;
   }

   return 0;
}