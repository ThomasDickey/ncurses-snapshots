------------------------------------------------------------------------------
--                                                                          --
--                           GNAT ncurses Binding                           --
--                                                                          --
--               Terminal_Interface.Curses.Forms.Field_Types.User           --
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
--  Version Control:
--  $Revision: 1.2 $
------------------------------------------------------------------------------
with Ada.Unchecked_Deallocation;
with Terminal_Interface.Curses.Aux;

package Terminal_Interface.Curses.Forms.Field_Types.User is
   pragma Preelaborate (User);

   type User_Defined_Field_Type is abstract new Field_Type with null record;
   --  This is the root of the mechanism we use to create field types in
   --  Ada95. You should your own type derive from this one and implement
   --  the Field_Check and Character_Check functions for your own type.

   type User_Defined_Field_Type_Access is access all
     User_Defined_Field_Type'Class;

   function Field_Check
     (Fld : Field;
      Typ : User_Defined_Field_Type) return Boolean
      is abstract;
   --  If True is returned, the field is considered valid, otherwise it is
   --  invalid.

   function Character_Check
     (Ch  : Character;
      Typ : User_Defined_Field_Type) return Boolean
      is abstract;
   --  If True is returned, the character is considered as valid for the
   --  field, otherwise as invalid.

   procedure Set_Field_Type (Fld : in Field;
                             Typ : in User_Defined_Field_Type);
   --  This should work for all types derived from User_Defined_Field_Type.
   --  No need to reimplement it for your derived type.

   --  +----------------------------------------------------------------------
   --  | Private Part.
   --  | Used by the Choice child package.
private
   use type Interfaces.C.Int;

   function C_Generic_Type   return C_Field_Type;

   function Generic_Field_Check (Fld : Field;
                                 Usr : System.Address) return C_Int;
   pragma Convention (C, Generic_Field_Check);
   --  This is the generic Field_Check_Function for the low-level fieldtype
   --  representing all the User_Defined_Field_Type derivates. It routes
   --  the call to the Field_Check implementation for the type.

   function Generic_Char_Check (Ch  : C_Int;
                                Usr : System.Address) return C_Int;
   pragma Convention (C, Generic_Char_Check);
   --  This is the generic Char_Check_Function for the low-level fieldtype
   --  representing all the User_Defined_Field_Type derivates. It routes
   --  the call to the Character_Check implementation for the type.

end Terminal_Interface.Curses.Forms.Field_Types.User;
