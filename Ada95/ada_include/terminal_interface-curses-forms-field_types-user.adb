------------------------------------------------------------------------------
--                                                                          --
--                           GNAT ncurses Binding                           --
--                                                                          --
--              Terminal_Interface.Curses.Forms.Field_Types.User            --
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
--  $Revision: 1.2 $
------------------------------------------------------------------------------
with Ada.Unchecked_Conversion;
with Interfaces.C;

package body Terminal_Interface.Curses.Forms.Field_Types.User is

   use type Interfaces.C.Int;

   procedure Set_Field_Type (Fld : in Field;
                             Typ : in User_Defined_Field_Type)
   is
      function Allocate_Arg (T : User_Defined_Field_Type'Class)
                             return Argument_Access;

      function Set_Fld_Type (F    : Field := Fld;
                             Cft  : C_Field_Type := C_Generic_Type;
                             Arg1 : Argument_Access)
                             return C_Int;
      pragma Import (C, Set_Fld_Type, "set_field_type");

      Res : Eti_Error;

      function Allocate_Arg (T : User_Defined_Field_Type'Class)
                             return Argument_Access
      is
         Ptr : Field_Type_Access := new User_Defined_Field_Type'Class'(T);
      begin
         return new Argument'(Usr => System.Null_Address,
                              Typ => Ptr,
                              Cft => Null_Field_Type);
      end Allocate_Arg;

   begin
      Res := Set_Fld_Type (Arg1 => Allocate_Arg (Typ));
      if Res /= E_Ok then
         Eti_Exception (Res);
      end if;
   end Set_Field_Type;

   function To_Argument_Access is new Ada.Unchecked_Conversion
     (System.Address, Argument_Access);

   function Generic_Field_Check (Fld : Field;
                                 Usr : System.Address) return C_Int
   is
      Result : Boolean;
      Udf    : User_Defined_Field_Type_Access :=
        User_Defined_Field_Type_Access (To_Argument_Access (Usr).Typ);
   begin
      Result := Field_Check (Fld, Udf.all);
      return C_Int (Boolean'Pos (Result));
   end Generic_Field_Check;

   function Generic_Char_Check (Ch  : C_Int;
                                Usr : System.Address) return C_Int
   is
      Result : Boolean;
      Udf    : User_Defined_Field_Type_Access :=
        User_Defined_Field_Type_Access (To_Argument_Access (Usr).Typ);
   begin
      Result := Character_Check (Character'Val (Ch), Udf.all);
      return C_Int (Boolean'Pos (Result));
   end Generic_Char_Check;

   --  -----------------------------------------------------------------------
   --
   function C_Generic_Type return C_Field_Type
   is
      Res : Eti_Error;
      T   : C_Field_Type;
   begin
      if M_Generic_Type = Null_Field_Type then
         T := New_Fieldtype (Generic_Field_Check'Access,
                             Generic_Char_Check'Access);
         if T = Null_Field_Type then
            raise Form_Exception;
         else
            Res := Set_Fieldtype_Arg (T,
                                      Make_Arg'Access,
                                      Copy_Arg'Access,
                                      Free_Arg'Access);
            if Res /= E_Ok then
               Eti_Exception (Res);
            end if;
         end if;
         M_Generic_Type := T;
      end if;
      pragma Assert (M_Generic_Type /= Null_Field_Type);
      return M_Generic_Type;
   end C_Generic_Type;

end Terminal_Interface.Curses.Forms.Field_Types.User;
