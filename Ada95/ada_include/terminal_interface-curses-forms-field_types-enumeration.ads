------------------------------------------------------------------------------
--                                                                          --
--                           GNAT ncurses Binding                           --
--                                                                          --
--           Terminal_Interface.Curses.Forms.Field_Types.Enumeration        --
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
--  $Revision: 1.1 $
------------------------------------------------------------------------------
with Interfaces.C.Strings;

package Terminal_Interface.Curses.Forms.Field_Types.Enumeration is

   type String_Access is access String;

   --  Type_Set is used by the child package Ada
   type Type_Set is (Lower_Case, Upper_Case, Mixed_Case);

   type Enum_Array is array (Positive range <>)
     of String_Access;

   type Enumeration_Info (C : Positive) is
      record
         Names                : Enum_Array (1 .. C);
         Case_Sensitive       : Boolean := False;
         Match_Must_Be_Unique : Boolean := False;
      end record;

   type Enumeration_Field is new Field_Type with private;

   function Create (Info : Enumeration_Info;
                    Auto_Release_Names : Boolean := False)
                    return Enumeration_Field;
   --  Make an fieldtype from the info. Enumerations are special, because
   --  they normally don't copy the enum values into a private store, so
   --  we have to care for the lifetime of the info we provide.
   --  The Auto_Release_Names flag may be used to automatically releases
   --  the strings in the Names array of the Enumeration_Info.

   function Make_Enumeration_Type (Info : Enumeration_Info;
                                   Auto_Release_Names : Boolean := False)
                                   return Enumeration_Field renames Create;

   procedure Release (Enum : in out Enumeration_Field);
   --  But we may want to release the field to release the memory allocated
   --  by it internally. After that the Enumeration field is no longer usable.

   --  The next type defintions are all ncurses extensions. They are typically
   --  not available in other curses implementations.

   procedure Set_Field_Type (Fld : in Field;
                             Typ : in Enumeration_Field);

private
   type CPA_Access is access Interfaces.C.Strings.chars_ptr_array;

   type Enumeration_Field is new Field_Type with
      record
         Case_Sensitive       : Boolean := False;
         Match_Must_Be_Unique : Boolean := False;
         Arr                  : CPA_Access := null;
      end record;

end Terminal_Interface.Curses.Forms.Field_Types.Enumeration;
