------------------------------------------------------------------------------
--                                                                          --
--                           GNAT ncurses Binding                           --
--                                                                          --
--         Terminal_Interface.Curses.Forms.Field_Types.Enumeration.Ada      --
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
generic
   type T is (<>);

package Terminal_Interface.Curses.Forms.Field_Types.Enumeration.Ada is

   function Create (Set            : Type_Set := Mixed_Case;
                    Case_Sensitive : Boolean  := False;
                    Must_Be_Unique : Boolean  := False)
                    return Enumeration_Field;

   function Value (Fld : Field;
                   Buf : Buffer_Number := Buffer_Number'First) return T;
   --  Translate the content of the fields buffer - indicated by the
   --  buffer number - into an enumeration value. If the buffer is empty
   --  or the content is invalid, a Constraint_Error is raises.

end Terminal_Interface.Curses.Forms.Field_Types.Enumeration.Ada;
