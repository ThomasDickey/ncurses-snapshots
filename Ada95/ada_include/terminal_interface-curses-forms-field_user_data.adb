------------------------------------------------------------------------------
--                                                                          --
--                           GNAT ncurses Binding                           --
--                                                                          --
--               Terminal_Interface.Curses.Forms.Field_User_Data            --
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
--  $Revision: 1.4 $
------------------------------------------------------------------------------
with Terminal_Interface.Curses.Aux; use  Terminal_Interface.Curses.Aux;

--  |
--  |=====================================================================
--  | man page form_field_userptr.3x
--  |=====================================================================
--  |
package body Terminal_Interface.Curses.Forms.Field_User_Data is
   --  |
   --  |
   --  |
   use type Interfaces.C.Int;

   procedure Set_User_Data (Fld  : in Field;
                            Data : in User_Access)
   is
      function Set_Field_Userptr (Fld : Field;
                                  Usr : User_Access) return C_Int;
      pragma Import (C, Set_Field_Userptr, "set_field_userptr");

      Res : constant Eti_Error := Set_Field_Userptr (Fld, Data);
   begin
      if Res /= E_Ok then
         Eti_Exception (Res);
      end if;
   end Set_User_Data;
   --  |
   --  |
   --  |
   procedure Get_User_Data (Fld  : in  Field;
                            Data : out User_Access)
   is
      function Field_Userptr (Fld : Field) return User_Access;
      pragma Import (C, Field_Userptr, "field_userptr");
   begin
      Data := Field_Userptr (Fld);
   end Get_User_Data;

end Terminal_Interface.Curses.Forms.Field_User_Data;
