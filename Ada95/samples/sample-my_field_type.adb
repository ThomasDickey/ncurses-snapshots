------------------------------------------------------------------------------
--                                                                          --
--                       GNAT ncurses Binding Samples                       --
--                                                                          --
--                           Sample.My_Field_Type                           --
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
--  $Revision: 1.3 $
------------------------------------------------------------------------------
with Terminal_Interface.Curses.Forms; use Terminal_Interface.Curses.Forms;

--  This is a very simple user defined field type. It accepts only a
--  defined character as input into the field.
--
package body Sample.My_Field_Type is

   --  That's simple. There are no field validity checks.
   function Field_Check (Fld : Field;
                         Typ : My_Data) return Boolean
   is
   begin
      return True;
   end Field_Check;

   --  Check exactly against the specified character.
   function Character_Check (Ch  : Character;
                             Typ : My_Data) return Boolean
   is
      C : constant Character := Typ.Ch;
   begin
      return Ch = C;
   end Character_Check;

end Sample.My_Field_Type;
