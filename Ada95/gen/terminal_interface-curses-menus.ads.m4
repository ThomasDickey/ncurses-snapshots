------------------------------------------------------------------------------
--                                                                          --
--                           GNAT ncurses Binding                           --
--                                                                          --
--                      Terminal_Interface.Curses.Menu                      --
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
include(`Menu_Base_Defs')
with System;
with Interfaces.C;
with Terminal_Interface.Curses;

package Terminal_Interface.Curses.Menus is

include(`Menu_Linker_Options')
   use type Terminal_Interface.Curses.Window;
   use type Terminal_Interface.Curses.Line_Position;
   use type Terminal_Interface.Curses.Column_Position;

   type Item is private;
   type Menu is private;

   ---------------------------
   --  Interface constants  --
   ---------------------------
   Null_Item : constant Item;
   Null_Menu : constant Menu;

   subtype Menu_Request_Code is Key_Code
     range (Key_Max + 1) .. (Key_Max + 17);

   --  The prefix M_ stands for "Menu Request"
   M_Left_Item       : constant Menu_Request_Code := Key_Max + 1;
   M_Right_Item      : constant Menu_Request_Code := Key_Max + 2;
   M_Up_Item         : constant Menu_Request_Code := Key_Max + 3;
   M_Down_Item       : constant Menu_Request_Code := Key_Max + 4;
   M_ScrollUp_Line   : constant Menu_Request_Code := Key_Max + 5;
   M_ScrollDown_Line : constant Menu_Request_Code := Key_Max + 6;
   M_ScrollDown_Page : constant Menu_Request_Code := Key_Max + 7;
   M_ScrollUp_Page   : constant Menu_Request_Code := Key_Max + 8;
   M_First_Item      : constant Menu_Request_Code := Key_Max + 9;
   M_Last_Item       : constant Menu_Request_Code := Key_Max + 10;
   M_Next_Item       : constant Menu_Request_Code := Key_Max + 11;
   M_Previous_Item   : constant Menu_Request_Code := Key_Max + 12;
   M_Toggle_Item     : constant Menu_Request_Code := Key_Max + 13;
   M_Clear_Pattern   : constant Menu_Request_Code := Key_Max + 14;
   M_Back_Pattern    : constant Menu_Request_Code := Key_Max + 15;
   M_Next_Match      : constant Menu_Request_Code := Key_Max + 16;
   M_Previous_Match  : constant Menu_Request_Code := Key_Max + 17;

   --  For those who like the old 'C' names for the request codes
   REQ_LEFT_ITEM     : Menu_Request_Code renames M_Left_Item;
   REQ_RIGHT_ITEM    : Menu_Request_Code renames M_Right_Item;
   REQ_UP_ITEM       : Menu_Request_Code renames M_Up_Item;
   REQ_DOWN_ITEM     : Menu_Request_Code renames M_Down_Item;
   REQ_SCR_ULINE     : Menu_Request_Code renames M_ScrollUp_Line;
   REQ_SCR_DLINE     : Menu_Request_Code renames M_ScrollDown_Line;
   REQ_SCR_DPAGE     : Menu_Request_Code renames M_ScrollDown_Page;
   REQ_SCR_UPAGE     : Menu_Request_Code renames M_ScrollUp_Page;
   REQ_FIRST_ITEM    : Menu_Request_Code renames M_First_Item;
   REQ_LAST_ITEM     : Menu_Request_Code renames M_Last_Item;
   REQ_NEXT_ITEM     : Menu_Request_Code renames M_Next_Item;
   REQ_PREV_ITEM     : Menu_Request_Code renames M_Previous_Item;
   REQ_TOGGLE_ITEM   : Menu_Request_Code renames M_Toggle_Item;
   REQ_CLEAR_PATTERN : Menu_Request_Code renames M_Clear_Pattern;
   REQ_BACK_PATTERN  : Menu_Request_Code renames M_Back_Pattern;
   REQ_NEXT_MATCH    : Menu_Request_Code renames M_Next_Match;
   REQ_PREV_MATCH    : Menu_Request_Code renames M_Previous_Match;

   procedure Request_Name (Key  : in Menu_Request_Code;
                           Name : out String);

   ------------------
   --  Exceptions  --
   ------------------

   Menu_Exception : exception;
   --
   --  Menu options
   --

include(`Menu_Opt_Rep')

   Default_Menu_Options : constant Menu_Option_Set;
   --  Initial default options for a menu.

   --
   --  Item options
   --
include(`Item_Rep')

   Default_Item_Options : constant Item_Option_Set;
   --  Initial default options for an item.

   --
   --  Item Array
   --
   type Item_Array is array (Positive range <>) of aliased Item;
   pragma Convention (C, Item_Array);

   type Item_Array_Access is access all Item_Array;
   pragma Controlled (Item_Array_Access);

   --  |=====================================================================
   --  | man page mitem_new.3x
   --  |=====================================================================
   function Create (Name        : String;
                    Description : String := "") return Item;
   --  AKA: new_item()

   function New_Item (Name        : String;
                      Description : String := "") return Item
     renames Create;

   procedure Delete (Itm : in out Item);
   --  AKA: free_item()
   --  Resets Itm to Null_Item

   --  |=====================================================================
   --  | man page mitem_value.3x
   --  |=====================================================================
   procedure Set_Value (Itm   : in Item;
                        Value : in Boolean := True);
   --  AKA: set_item_value()

   function Value (Itm : Item) return Boolean;
   --  AKA: item_value()

   --  |=====================================================================
   --  | man page mitem_visible.3x
   --  |=====================================================================
   function Visible (Itm : Item) return Boolean;
   --  AKA: item_visible()

   --  |=====================================================================
   --  | man page mitem_opts.3x
   --  |=====================================================================
   procedure Set_Options (Itm     : in Item;
                          Options : in Item_Option_Set);
   --  AKA: set_item_opts()

   procedure Switch_Options (Itm     : in Item;
                             Options : in Item_Option_Set;
                             On      : Boolean := True);
   --  AKA: item_opts_on(), item_opts_off()

   procedure Get_Options (Itm     : in  Item;
                          Options : out Item_Option_Set);
   --  AKA: item_opts()

   function Get_Options (Itm : Item := Null_Item) return Item_Option_Set;
   --  AKA: item_opts()

   --  |=====================================================================
   --  | man page mitem_name.3x
   --  |=====================================================================
   procedure Name (Itm  : in Item;
                   Name : out String);
   --  AKA: item_name()

   procedure Description (Itm         : in Item;
                          Description : out String);
   --  AKA: item_description();

   --  |=====================================================================
   --  | man page mitem_current.3x
   --  |=====================================================================
   procedure Set_Current (Men : in Menu;
                          Itm : in Item);
   --  AKA: set_current_item()

   function Current (Men : Menu) return Item;
   --  AKA: current_item()

   procedure Set_Top_Row (Men  : in Menu;
                          Line : in Line_Position);
   --  AKA: set_top_row()

   function Top_Row (Men : Menu) return Line_Position;
   --  AKA: top_row()

   function Get_Index (Itm : Item) return Positive;
   --  AKA: item_index()
   --  Please note that in this binding we start the numbering of items
   --  with 1. So this is number is one more than you get from the low
   --  level call.

   --  |=====================================================================
   --  | man page menu_post.3x
   --  |=====================================================================
   procedure Post (Men  : in Menu;
                   Post : in Boolean := True);
   --  AKA: post_menu(), unpost_menu()

   --  |=====================================================================
   --  | man page menu_opts.3x
   --  |=====================================================================
   procedure Set_Options (Men     : in Menu;
                          Options : in Menu_Option_Set);
   --  AKA: set_menu_opts()

   procedure Switch_Options (Men     : in Menu;
                             Options : in Menu_Option_Set;
                             On      : Boolean := True);
   --  AKA: menu_opts_on(), menu_opts_off()

   procedure Get_Options (Men     : in  Menu;
                          Options : out Menu_Option_Set);
   --  AKA: menu_opts()

   function Get_Options (Men : Menu := Null_Menu) return Menu_Option_Set;
   --  AKA: menu_opts()

   --  |=====================================================================
   --  | man page menu_win.3x
   --  |=====================================================================
   procedure Set_Window (Men : in Menu;
                         Win : in Window);
   --  AKA: set_menu_win()

   function Get_Window (Men : Menu) return Window;
   --  AKA: menu_win()

   procedure Set_Sub_Window (Men : in Menu;
                             Win : in Window);
   --  AKA: set_menu_sub()

   function Get_Sub_Window (Men : Menu) return Window;
   --  AKA: menu_sub()

   procedure Scale (Men     : in Menu;
                    Lines   : out Line_Count;
                    Columns : out Column_Count);
   --  AKA: scale_menu()

   --  |=====================================================================
   --  | man page menu_cursor.3x
   --  |=====================================================================
   procedure Position_Cursor (Men : Menu);
   --  AKA: pos_menu_cursor()

   --  |=====================================================================
   --  | man page menu_mark.3x
   --  |=====================================================================
   procedure Set_Mark (Men  : in Menu;
                       Mark : in String);
   --  AKA: set_meni_mark()

   procedure Mark (Men  : in  Menu;
                   Mark : out String);
   --  AKA: menu_mark()

   --  |=====================================================================
   --  | man page menu_attribs.3x
   --  |=====================================================================
   procedure Set_Foreground (Men  : in Menu;
                             Fore : in Character_Attribute_Set);
   --  AKA: set_menu_fore()

   procedure Foreground (Men  : in  Menu;
                         Fore : out Character_Attribute_Set);
   --  AKA: menu_fore()

   procedure Set_Background (Men  : in Menu;
                             Back : in Character_Attribute_Set);
   --  AKA: set_menu_back()

   procedure Background (Men  : in  Menu;
                         Back : out Character_Attribute_Set);
   --  AKA: menu_back()

   procedure Set_Grey (Men  : in Menu;
                       Grey : in Character_Attribute_Set);
   --  AKA: set_menu_grey()

   procedure Grey (Men  : in  Menu;
                   Grey : out Character_Attribute_Set);
   --  AKA: menu_grey()

   procedure Set_Pad_Character (Men : in Menu;
                                     Pad : in Character := ' ');
   --  AKA: set_menu_pad()

   procedure Pad_Character (Men : in  Menu;
                            Pad : out Character);
   --  AKA: menu_pad()

   --  |=====================================================================
   --  | man page menu_spacing.3x
   --  |=====================================================================
   procedure Set_Spacing (Men  : in Menu;
                          Desc : in Column_Position := 0;
                          Row  : in Line_Position   := 0;
                          Col  : in Column_Position := 0);
   --  AKA: set_menu_spacing()

   procedure Spacing (Men  : in Menu;
                      Desc : out Column_Position;
                      Row  : out Line_Position;
                      Col  : out Column_Position);
   --  AKA: menu_spacing()

   --  |=====================================================================
   --  | man page menu_pattern.3x
   --  |=====================================================================
   function Set_Pattern (Men  : Menu;
                         Text : String) return Boolean;
   --  AKA: set_menu_pattern()
   --  Return TRUE if the pattern matches, FALSE otherwise

   procedure Pattern (Men  : in  Menu;
                      Text : out String);
   --  AKA: menu_pattern()

   --  |=====================================================================
   --  | man page menu_format.3x
   --  |=====================================================================
   procedure Set_Format (Men     : in Menu;
                         Lines   : in Line_Count;
                         Columns : in Column_Count);
   --  AKA: set_menu_format()

   procedure Format (Men     : in  Menu;
                     Lines   : out Line_Count;
                     Columns : out Column_Count);
   --  AKA: menu_format()

   --  |=====================================================================
   --  | man page menu_hook.3x
   --  |=====================================================================
   type Menu_Hook_Function is access procedure (Men : in Menu);
   pragma Convention (C, Menu_Hook_Function);

   procedure Set_Item_Init_Hook (Men  : in Menu;
                                 Proc : in Menu_Hook_Function);
   --  AKA: set_item_init()

   procedure Set_Item_Term_Hook (Men  : in Menu;
                                 Proc : in Menu_Hook_Function);
   --  AKA: set_item_term()

   procedure Set_Menu_Init_Hook (Men  : in Menu;
                                 Proc : in Menu_Hook_Function);
   --  AKA: set_menu_init()

   procedure Set_Menu_Term_Hook (Men  : in Menu;
                                 Proc : in Menu_Hook_Function);
   --  AKA: set_menu_term()

   function Get_Item_Init_Hook (Men : Menu) return Menu_Hook_Function;
   --  AKA: item_init()

   function Get_Item_Term_Hook (Men : Menu) return Menu_Hook_Function;
   --  AKA: item_term()

   function Get_Menu_Init_Hook (Men : Menu) return Menu_Hook_Function;
   --  AKA: menu_init()

   function Get_Menu_Term_Hook (Men : Menu) return Menu_Hook_Function;
   --  AKA: menu_term()

   --  |=====================================================================
   --  | man page menu_items.3x
   --  |=====================================================================
   procedure Redefine (Men   : in Menu;
                       Items : in Item_Array);
   --  AKA: set_menu_items()
   --  With a bit more comfort. You don´t need to terminate the Item_Array
   --  with a null entry. This is handled internally in the binding.

   procedure Set_Items (Men   : in Menu;
                        Items : in Item_Array) renames Redefine;

   function Items (Men : Menu) return Item_Array_Access;
   --  AKA: menu_items()

   function Item_Count (Men : Menu) return Natural;
   --  AKA: item_count()

   --  |=====================================================================
   --  | man page menu_new.3x
   --  |=====================================================================
   function Create (Items : Item_Array) return Menu;
   --  AKA: new_menu()

   function New_Menu (Items : Item_Array) return Menu renames Create;

   procedure Delete (Men : in out Menu);
   --  AKA: free_menu()
   --  Reset Men to Null_Menu

   --  |=====================================================================
   --  | man page menu_driver.3x
   --  |=====================================================================
   type Driver_Result is (Menu_Ok,
                          Request_Denied,
                          Unknown_Request,
                          No_Match);

   function Driver (Men : Menu;
                    Key : Key_Code) return Driver_Result;
   --  AKA: menu_driver()

-------------------------------------------------------------------------------
private
   type Item   is new System.Address;
   type Menu   is new System.Address;

   Null_Item : constant Item := Item (System.Null_Address);
   Null_Menu : constant Menu := Menu (System.Null_Address);

   Default_Menu_Options : constant Menu_Option_Set :=
     Get_Options (Null_Menu);

   Default_Item_Options : constant Item_Option_Set :=
     Get_Options (Null_Item);

   --  This binding uses the original user pointer mechanism of a menu to store
   --  specific informations about a menu. This wrapper record carries this
   --  specifics and contains a field to maintain a new user pointer. Please
   --  note that you must take this into account if you wan't to use the user
   --  pointer mechanism of a menu created with this binding in low-level C
   --  routines.
   type Ada_User_Wrapper is
      record
         U : System.Address;
         I : Item_Array_Access;
      end record;
   pragma Convention (C, Ada_User_Wrapper);
   type Ada_User_Wrapper_Access is access all Ada_User_Wrapper;
   pragma Controlled (Ada_User_Wrapper_Access);

   Generation_Bit_Order : constant System.Bit_Order := System.M4_BIT_ORDER;

end Terminal_Interface.Curses.Menus;
