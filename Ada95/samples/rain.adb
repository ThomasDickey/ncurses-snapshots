------------------------------------------------------------------------------
--                                                                          --
--                       GNAT ncurses Binding Samples                       --
--                                                                          --
--                                   Rain                                   --
--                                                                          --
--                                 B O D Y                                  --
--                                                                          --
--  Version 00.93                                                           --
--                                                                          --
--  The ncurses Ada95 binding is copyrighted 1996 by                        --
--  Juergen Pfeifer, Email: Juergen.Pfeifer@T-Online.de                     --
--                                                                          --
--  Permission is hereby granted to reproduce and distribute this           --
--  binding by any means and for any fee, whether alone or as part          --
--  of a larger distribution, in source or in binary form, PROVIDED         --
--  this notice is included with any such distribution, and is not          --
--  removed from any of its header files. Mention of ncurses and the        --
--  author of this binding in any applications linked with it is            --
--  highly appreciated.                                                     --
--                                                                          --
--  This binding comes AS IS with no warranty, implied or expressed.        --
------------------------------------------------------------------------------
--  Version Control
--  $Revision: 1.1 $
------------------------------------------------------------------------------
--  This package has been contributed by Laurent Pautet <pautet@gnat.com>   --
--                                                                          --
with Ada.Numerics.Float_Random; use Ada.Numerics.Float_Random;
with Status; use Status;
with Terminal_Interface.Curses; use Terminal_Interface.Curses;

procedure Rain is

   Visibility : Cursor_Visibility;

   subtype X_Position is Line_Position;
   subtype Y_Position is Column_Position;

   Xpos    : array (1 .. 5) of X_Position;
   Ypos    : array (1 .. 5) of Y_Position;

   N : Integer;

   G : Generator;

   Max_X, X : X_Position;
   Max_Y, Y : Y_Position;

   procedure Next (J : in out Integer);
   procedure Cursor (X : X_Position; Y : Y_Position);

   procedure Next (J : in out Integer) is
   begin
      if J = 5 then
         J := 1;
      else
         J := J + 1;
      end if;
   end Next;

   procedure Cursor (X : X_Position; Y : Y_Position) is
   begin
      Move_Cursor (Line => X, Column => Y);
   end Cursor;
   pragma Inline (Cursor);

begin

   Init_Screen;
   Set_NL_Mode;
   Set_Echo_Mode (False);

   Visibility := Invisible;
   Set_Cursor_Visibility (Visibility);

   Max_X := Lines - 5;
   Max_Y := Columns - 5;

   for I in Xpos'Range loop
      Xpos (I) := X_Position (Float (Max_X) * Random (G)) + 2;
      Ypos (I) := Y_Position (Float (Max_Y) * Random (G)) + 2;
   end loop;

   N := 1;
   while Process.Continue loop

      X := X_Position (Float (Max_X) * Random (G)) + 2;
      Y := Y_Position (Float (Max_Y) * Random (G)) + 2;

      Cursor (X, Y);
      Add (Ch => '.');

      Cursor (Xpos (N), Ypos (N));
      Add (Ch => 'o');

      --
      Next (N);
      Cursor (Xpos (N), Ypos (N));
      Add (Ch => 'O');

      --
      Next (N);
      Cursor (Xpos (N) - 1, Ypos (N));
      Add (Ch => '-');
      Cursor (Xpos (N), Ypos (N) - 1);
      Add (Str => "|.|");
      Cursor (Xpos (N) + 1, Ypos (N));
      Add (Ch => '-');

      --
      Next (N);
      Cursor (Xpos (N) - 2, Ypos (N));
      Add (Ch => '-');
      Cursor (Xpos (N) - 1, Ypos (N) - 1);
      Add (Str => "/\\");
      Cursor (Xpos (N), Ypos (N) - 2);
      Add (Str => "| O |");
      Cursor (Xpos (N) + 1, Ypos (N) - 1);
      Add (Str => "\\/");
      Cursor (Xpos (N) + 2, Ypos (N));
      Add (Ch => '-');

      --
      Next (N);
      Cursor (Xpos (N) - 2, Ypos (N));
      Add (Ch => ' ');
      Cursor (Xpos (N) - 1, Ypos (N) - 1);
      Add (Str => "   ");
      Cursor (Xpos (N), Ypos (N) - 2);
      Add (Str => "     ");
      Cursor (Xpos (N) + 1, Ypos (N) - 1);
      Add (Str => "   ");
      Cursor (Xpos (N) + 2, Ypos (N));
      Add (Ch => ' ');

      Xpos (N) := X;
      Ypos (N) := Y;

      Refresh;
      Nap_Milli_Seconds (50);
   end loop;

   Visibility := Normal;
   Set_Cursor_Visibility (Visibility);
   End_Windows;

end Rain;
