------------------------------------------------------------------------------
--                                                                          --
--                           GNAT ncurses Binding                           --
--                                                                          --
--             Terminal_Interface.Curses.Forms.Field_Types.RegExp           --
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
--  Version Control:
--  $Revision: 1.1 $
------------------------------------------------------------------------------
with Interfaces.C; use Interfaces.C;
with Terminal_Interface.Curses.Aux; use Terminal_Interface.Curses.Aux;

package body Terminal_Interface.Curses.Forms.Field_Types.RegExp is

   procedure Set_Field_Type (Fld : in Field;
                             Typ : in Regular_Expression_Field)
   is
      type Char_Ptr is access all Interfaces.C.Char;

      C_Regexp_Field_Type : C_Field_Type;
      pragma Import (C, C_Regexp_Field_Type, "TYPE_REGEXP");

      function Set_Ftyp (F    : Field := Fld;
                         Cft  : C_Field_Type := C_Regexp_Field_Type;
                         Arg1 : Char_Ptr) return C_Int;
      pragma Import (C, Set_Ftyp, "set_field_type");

      Txt : char_array (0 .. Typ.Regular_Expression.all'Length);
      Len : size_t;
      Res : Eti_Error;
   begin
      To_C (Typ.Regular_Expression.all, Txt, Len);
      Res := Set_Ftyp (Arg1 => Txt (Txt'First)'Access);
      if Res /= E_Ok then
         Eti_Exception (Res);
      end if;
      Wrap_Builtin (Fld, Typ);
   end Set_Field_Type;

end Terminal_Interface.Curses.Forms.Field_Types.RegExp;
