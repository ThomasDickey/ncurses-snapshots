------------------------------------------------------------------------------
--                                                                          --
--                       GNAT ncurses Binding Samples                       --
--                                                                          --
--                           Sample.My_Field_Type                           --
--                                                                          --
--                                 S P E C                                  --
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
--  $Revision: 1.4 $
------------------------------------------------------------------------------
with Terminal_Interface.Curses.Forms; use Terminal_Interface.Curses.Forms;
with Terminal_Interface.Curses.Forms.Field_Types.User;
use Terminal_Interface.Curses.Forms.Field_Types.User;

--  This is a very simple user defined field type. It accepts only a
--  defined character as input into the field.
--
package Sample.My_Field_Type is

   type My_Data is new User_Defined_Field_Type with
      record
        Ch : Character;
      end record;

   function Field_Check (Fld  : Field;
                         Typ  : My_Data) return Boolean;

   function Character_Check (Ch  : Character;
                             Typ : My_Data) return Boolean;

end Sample.My_Field_Type;

