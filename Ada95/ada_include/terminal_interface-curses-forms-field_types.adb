------------------------------------------------------------------------------
--                                                                          --
--                           GNAT ncurses Binding                           --
--                                                                          --
--                 Terminal_Interface.Curses.Forms.Field_Types              --
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
--  $Revision: 1.5 $
------------------------------------------------------------------------------
with Interfaces.C;
with Terminal_Interface.Curses.Aux; use Terminal_Interface.Curses.Aux;
with Ada.Unchecked_Deallocation;
with Ada.Unchecked_Conversion;
--  |
--  |=====================================================================
--  | man page form_fieldtype.3x
--  |=====================================================================
--  |
package body Terminal_Interface.Curses.Forms.Field_Types is

   use type Interfaces.C.int;
   use type System.Address;

   function To_Argument_Access is new Ada.Unchecked_Conversion
     (System.Address, Argument_Access);

   function Get_Fieldtype (F : Field) return C_Field_Type;
   pragma Import (C, Get_Fieldtype, "field_type");

   function Get_Arg (F : Field) return System.Address;
   pragma Import (C, Get_Arg, "field_arg");
   --  |
   --  |=====================================================================
   --  | man page form_field_validation.3x
   --  |=====================================================================
   --  |
   --  |
   --  |
   function Get_Type (Fld : in Field) return Field_Type_Access
   is
      Low_Level : constant C_Field_Type := Get_Fieldtype (Fld);
      Arg : Argument_Access;
   begin
      if Low_Level = Null_Field_Type then
         return null;
      else
         if Low_Level = C_Builtin_Router or else
           Low_Level = C_Generic_Type or else
           Low_Level = C_Choice_Router or else
           Low_Level = C_Generic_Choice then
            Arg := To_Argument_Access (Get_Arg (Fld));
            if Arg = null then
               raise Form_Exception;
            else
               return Arg.Typ;
            end if;
         else
            raise Form_Exception;
         end if;
      end if;
   end Get_Type;

   function Make_Arg (Args : System.Address) return System.Address
   is
      function Getarg (Arg : System.Address := Args)
        return System.Address;
      pragma Import (C, Getarg, "_nc_ada_getvarg");
   begin
      return Getarg;
   end Make_Arg;

   function Copy_Arg (Usr : System.Address) return System.Address
   is
   begin
      return Usr;
   end Copy_Arg;

   procedure Free_Arg (Usr : in System.Address)
   is
      procedure Free_Type is new Ada.Unchecked_Deallocation
        (Field_Type'Class, Field_Type_Access);
      procedure Freeargs is new Ada.Unchecked_Deallocation
        (Argument, Argument_Access);

      To_Be_Free : Argument_Access := To_Argument_Access (Usr);
      Low_Level  : C_Field_Type;
   begin
      if To_Be_Free /= null then
         if To_Be_Free.Usr /= System.Null_Address then
            Low_Level := To_Be_Free.Cft;
            if Low_Level.Freearg /= null then
               Low_Level.Freearg (To_Be_Free.Usr);
            end if;
         end if;
         if To_Be_Free.Typ /= null then
            Free_Type (To_Be_Free.Typ);
         end if;
         Freeargs (To_Be_Free);
      end if;
   end Free_Arg;


   procedure Wrap_Builtin (Fld : Field;
                           Typ : Field_Type'Class;
                           Cft : C_Field_Type := C_Builtin_Router)
   is
      Usr_Arg   : System.Address := Get_Arg (Fld);
      Low_Level : constant C_Field_Type := Get_Fieldtype (Fld);
      Arg : Argument_Access;
      Res : Eti_Error;
      function Set_Fld_Type (F    : Field := Fld;
                             Cf   : C_Field_Type := Cft;
                             Arg1 : Argument_Access) return C_Int;
      pragma Import (C, Set_Fld_Type, "set_field_type");

   begin
      pragma Assert (Low_Level /= Null_Field_Type);
      if Cft /= C_Builtin_Router and then Cft /= C_Choice_Router then
         raise Form_Exception;
      else
         Arg := new Argument'(Usr => System.Null_Address,
                              Typ => new Field_Type'Class'(Typ),
                              Cft => Get_Fieldtype (Fld));
         if Usr_Arg /= System.Null_Address then
            if Low_Level.Copyarg /= null then
               Arg.Usr := Low_Level.Copyarg (Usr_Arg);
            else
               Arg.Usr := Usr_Arg;
            end if;
         end if;

         Res := Set_Fld_Type (Arg1 => Arg);
         if Res /= E_Ok then
            Eti_Exception (Res);
         end if;
      end if;
   end Wrap_Builtin;

   function Field_Check_Router (Fld : Field;
                                Usr : System.Address) return C_Int
   is
      Arg  : constant Argument_Access := To_Argument_Access (Usr);
   begin
      pragma Assert (Arg /= null and then Arg.Cft /= Null_Field_Type
                     and then Arg.Typ /= null);
      if Arg.Cft.Fcheck /= null then
         return Arg.Cft.Fcheck (Fld, Arg.Usr);
      else
         return 1;
      end if;
   end Field_Check_Router;

   function Char_Check_Router (Ch  : C_Int;
                               Usr : System.Address) return C_Int
   is
      Arg  : constant Argument_Access := To_Argument_Access (Usr);
   begin
      pragma Assert (Arg /= null and then Arg.Cft /= Null_Field_Type
                     and then Arg.Typ /= null);
      if Arg.Cft.Ccheck /= null then
         return Arg.Cft.Ccheck (Ch, Arg.Usr);
      else
         return 1;
      end if;
   end Char_Check_Router;

   function Next_Router (Fld : Field;
                         Usr : System.Address) return C_Int
   is
      Arg  : constant Argument_Access := To_Argument_Access (Usr);
   begin
      pragma Assert (Arg /= null and then Arg.Cft /= Null_Field_Type
                     and then Arg.Typ /= null);
      if Arg.Cft.Next /= null then
         return Arg.Cft.Next (Fld, Arg.Usr);
      else
         return 1;
      end if;
   end Next_Router;

   function Prev_Router (Fld : Field;
                         Usr : System.Address) return C_Int
   is
      Arg  : constant Argument_Access := To_Argument_Access (Usr);
   begin
      pragma Assert (Arg /= null and then Arg.Cft /= Null_Field_Type
                     and then Arg.Typ /= null);
      if Arg.Cft.Prev /= null then
         return Arg.Cft.Prev (Fld, Arg.Usr);
      else
         return 1;
      end if;
   end Prev_Router;

begin
   C_Builtin_Router := New_Fieldtype (Field_Check_Router'Access,
                                      Char_Check_Router'Access);
   C_Choice_Router := New_Fieldtype (Field_Check_Router'Access,
                                      Char_Check_Router'Access);
   if C_Builtin_Router = Null_Field_Type
     or else C_Choice_Router = Null_Field_Type then
      raise Form_Exception;
   end if;

   declare
      Res : Eti_Error;
   begin
      Res := Set_Fieldtype_Arg (C_Builtin_Router,
                                Make_Arg'Access,
                                Copy_Arg'Access,
                                Free_Arg'Access);
      if Res /= E_Ok then
         Eti_Exception (Res);
      end if;

      Res := Set_Fieldtype_Arg (C_Choice_Router,
                                Make_Arg'Access,
                                Copy_Arg'Access,
                                Free_Arg'Access);
      if Res /= E_Ok then
         Eti_Exception (Res);
      end if;

      Res := Set_Fieldtype_Choice (C_Choice_Router,
                                   Next_Router'Access,
                                   Prev_Router'Access);
      if Res /= E_Ok then
         Eti_Exception (Res);
      end if;
   end;

end Terminal_Interface.Curses.Forms.Field_Types;