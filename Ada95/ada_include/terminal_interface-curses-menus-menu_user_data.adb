------------------------------------------------------------------------------
--                                                                          --
--                           GNAT ncurses Binding                           --
--                                                                          --
--               Terminal_Interface.Curses.Menus.Menu_User_Data             --
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
with Terminal_Interface.Curses.Aux; use Terminal_Interface.Curses.Aux;

package body Terminal_Interface.Curses.Menus.Menu_User_Data is

   use type Interfaces.C.Int;

   procedure Set_User_Data (Men  : in Menu;
                            Data : in User_Access)
   is
      function Set_Menu_Userptr (Men  : Menu;
                                 Data : User_Access)  return C_Int;
      pragma Import (C, Set_Menu_Userptr, "set_menu_userptr");

      Res : constant Eti_Error := Set_Menu_Userptr (Men, Data);
   begin
      if  Res /= E_Ok then
         Eti_Exception (Res);
      end if;
   end Set_User_Data;

   function Get_User_Data (Men  : in  Menu) return User_Access
   is
      function Menu_Userptr (Men : Menu) return User_Access;
      pragma Import (C, Menu_Userptr, "menu_userptr");
   begin
      return Menu_Userptr (Men);
   end Get_User_Data;

   procedure Get_User_Data (Men  : in  Menu;
                            Data : out User_Access)
   is
   begin
      Data := Get_User_Data (Men);
   end Get_User_Data;

end Terminal_Interface.Curses.Menus.Menu_User_Data;
