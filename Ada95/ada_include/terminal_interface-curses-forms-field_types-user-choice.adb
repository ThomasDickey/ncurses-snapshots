------------------------------------------------------------------------------
--                                                                          --
--                           GNAT ncurses Binding                           --
--                                                                          --
--          Terminal_Interface.Curses.Forms.Field_Types.User.Choice         --
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
with Ada.Unchecked_Conversion;
with Interfaces.C;

package body Terminal_Interface.Curses.Forms.Field_Types.User.Choice is

   use type Interfaces.C.Int;

   function To_Argument_Access is new Ada.Unchecked_Conversion
     (System.Address, Argument_Access);

   function Generic_Next (Fld : Field;
                          Usr : System.Address) return C_Int
   is
      Result : Boolean;
      Udf    : User_Defined_Field_Type_With_Choice_Access :=
        User_Defined_Field_Type_With_Choice_Access
        (To_Argument_Access (Usr).Typ);
   begin
      Result := Next (Fld, Udf.all);
      return C_Int (Boolean'Pos (Result));
   end Generic_Next;

   function Generic_Prev (Fld : Field;
                          Usr : System.Address) return C_Int
   is
      Result : Boolean;
      Udf    : User_Defined_Field_Type_With_Choice_Access :=
        User_Defined_Field_Type_With_Choice_Access
        (To_Argument_Access (Usr).Typ);
   begin
      Result := Previous (Fld, Udf.all);
      return C_Int (Boolean'Pos (Result));
   end Generic_Prev;


begin
   C_Generic_Choice := New_Fieldtype (Generic_Field_Check'Access,
                                      Generic_Char_Check'Access);
   if C_Generic_Choice = Null_Field_Type then
      Eti_Exception (E_System_Error);
   end if;

   declare
      Res : Eti_Error;
   begin
      Res := Set_Fieldtype_Arg (C_Generic_Choice,
                                Make_Arg'Access,
                                Copy_Arg'Access,
                                Free_Arg'Access);
      if Res /= E_Ok then
         Eti_Exception (Res);
      end if;

      Res := Set_Fieldtype_Choice (C_Generic_Choice,
                                   Generic_Next'Access,
                                   Generic_Prev'Access);
      if Res /= E_Ok then
         Eti_Exception (Res);
      end if;
   end;

end Terminal_Interface.Curses.Forms.Field_Types.User.Choice;
