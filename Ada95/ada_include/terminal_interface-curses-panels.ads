------------------------------------------------------------------------------
--                                                                          --
--                           GNAT ncurses Binding                           --
--                                                                          --
--                      Terminal_Interface.Curses.Panels                    --
--                                                                          --
--                                 S P E C                                  --
--                                                                          --
--  Version 00.91                                                           --
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
with System;
with Terminal_Interface.Curses;

package Terminal_Interface.Curses.Panels is

   pragma Linker_Options ("-lpanel");

   use type Terminal_Interface.Curses.Window;
   use type Terminal_Interface.Curses.Line_Position;
   use type Terminal_Interface.Curses.Column_Position;

   type Panel is private;

   ---------------------------
   --  Interface constants  --
   ---------------------------
   Null_Panel : constant Panel;

   -------------------
   --  Exceptions   --
   -------------------

   Panel_Exception : exception;

   function Create (Win : Window) return Panel;
   --  AKA: new_panel()

   function New_Panel (Win : Window) return Panel renames Create;

   procedure Bottom (Pan : in Panel);
   --  AKA: bottom_panel()

   procedure Top (Pan : in Panel);
   --  AKA: top_panel()

   procedure Show (Pan : in Panel);
   --  AKA: show_panel()

   procedure Update_Panels;
   --  AKA: update_panels()
   pragma Import (C, Update_Panels, "update_panels");

   procedure Hide (Pan : in Panel);
   --  AKA: hide_panel()

   function Get_Window (Pan : Panel) return Window;
   --  AKA: panel_window()

   function Panel_Window (Pan : Panel) return Window renames Get_Window;

   procedure Replace (Pan : in Panel;
                      Win : in Window);
   --  AKA: replace_panel()

   procedure Move (Pan    : in Panel;
                   Line   : in Line_Position;
                   Column : in Column_Position);
   --  AKA: move_panel()

   function Is_Hidden (Pan : Panel) return Boolean;
   --  AKA: panel_hidden()

   function Above (Pan : Panel) return Panel;
   --  AKA: panel_above()
   pragma Import (C, Above, "panel_above");

   function Below (Pan : Panel) return Panel;
   --  AKA: panel_below()
   pragma Import (C, Below, "panel_below");

   procedure Delete (Pan : in out Panel);
   --  AKA: del_panel()

   private
      type Panel is new System.Address;
      Null_Panel : constant Panel := Panel (System.Null_Address);

end Terminal_Interface.Curses.Panels;
