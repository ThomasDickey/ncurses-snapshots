------------------------------------------------------------------------------
--                                                                          --
--                           GNAT ncurses Binding                           --
--                                                                          --
--           Terminal_Interface.Curses.Forms.Field_Types.User.Choice        --
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

package Terminal_Interface.Curses.Forms.Field_Types.User.Choice is
   pragma Preelaborate (Choice);

   type User_Defined_Field_Type_With_Choice is abstract new
     User_Defined_Field_Type with null record;
   --  This is the root of the mechanism we use to create field types in
   --  Ada95 that allow the prev/next mechanism. You should your own type
   --  derive from this one and implement the Field_Check, Character_Check
   --  Next and Previous functions for your own type.

   type User_Defined_Field_Type_With_Choice_Access is access all
     User_Defined_Field_Type_With_Choice'Class;

   function Next
     (Fld : Field;
      Typ : User_Defined_Field_Type_With_Choice) return Boolean
      is abstract;
   --  If True is returned, the function successfully generated a next
   --  value into the fields buffer.

   function Previous
     (Fld : Field;
      Typ : User_Defined_Field_Type_With_Choice) return Boolean
      is abstract;
   --  If True is returned, the function successfully generated a previous
   --  value into the fields buffer.

   --  +----------------------------------------------------------------------
   --  | Private Part.
   --  |
private
   use type Interfaces.C.Int;

   function C_Generic_Choice return C_Field_Type;

   function Generic_Next (Fld : Field;
                          Usr : System.Address) return C_Int;
   pragma Convention (C, Generic_Next);
   --  This is the generic next Choice_Function for the low-level fieldtype
   --  representing all the User_Defined_Field_Type derivates. It routes
   --  the call to the Next implementation for the type.

   function Generic_Prev (Fld : Field;
                          Usr : System.Address) return C_Int;
   pragma Convention (C, Generic_Prev);
   --  This is the generic prev Choice_Function for the low-level fieldtype
   --  representing all the User_Defined_Field_Type derivates. It routes
   --  the call to the Previous implementation for the type.

end Terminal_Interface.Curses.Forms.Field_Types.User.Choice;
