------------------------------------------------------------------------------
--                                                                          --
--                           GNAT ncurses Binding                           --
--                                                                          --
--         Terminal_Interface.Curses.Forms.Field_Types.Enumeration.Ada      --
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
with Ada.Characters.Handling; use Ada.Characters.Handling;

package body Terminal_Interface.Curses.Forms.Field_Types.Enumeration.Ada is

   function Create (Set            : Type_Set := Mixed_Case;
                    Case_Sensitive : Boolean  := False;
                    Must_Be_Unique : Boolean  := False)
                    return Enumeration_Field
   is
      I : Enumeration_Info (T'Pos (T'Last) - T'Pos (T'First) + 1);
      J : Positive := 1;
   begin
      I.Case_Sensitive := Case_Sensitive;
      I.Match_Must_Be_Unique := Must_Be_Unique;

      for E in T'Range loop
         I.Names (J) := new String'(T'Image (T (E)));
         --  The Image attribute defaults to upper case, so we have to handle
         --  only the other ones...
         if Set /= Upper_Case then
            I.Names (J).all := To_Lower (I.Names (J).all);
            if Set = Mixed_Case then
               I.Names (J)(I.Names (J).all'First) :=
                 To_Upper (I.Names (J)(I.Names (J).all'First));
            end if;
         end if;
         J := J + 1;
      end loop;

      return Create (I, True);
   end Create;

   function Value (Fld : Field;
                   Buf : Buffer_Number := Buffer_Number'First) return T
   is
   begin
      return T'Value (Get_Buffer (Fld, Buf));
   end Value;

end Terminal_Interface.Curses.Forms.Field_Types.Enumeration.Ada;
