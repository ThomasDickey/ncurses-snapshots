------------------------------------------------------------------------------
--                                                                          --
--                           GNAT ncurses Binding                           --
--                                                                          --
--                         Terminal_Interface.Curses                        --
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
include(`Base_Defs')
with System;
with Interfaces.C;   --  We need this for some assertions.

package Terminal_Interface.Curses is

include(`Linker_Options')
   type Window is private;
   Null_Window : constant Window;

   type Line_Position   is new Natural; --  line coordinate
   type Column_Position is new Natural; --  column coordinate

   subtype Line_Count   is Line_Position   range 1 .. Line_Position'Last;
   --  Type to count lines. We do not allow null windows, so must be positive
   subtype Column_Count is Column_Position range 1 .. Column_Position'Last;
   --  Type to count columns. We do not allow null windows, so must be positive

   type Key_Code is new Natural;
   --  That is anything including real characters, special keys and logical
   --  request codes.

   subtype Real_Key_Code is Key_Code range 0 .. M4_KEY_MAX;
   --  This are the codes that potentially represent a real keystroke.
   --  Not all codes may be possible on a specific terminal. To check the
   --  availability of a special key, the Has_Key function is provided.

   subtype Special_Key_Code is Real_Key_Code
     range M4_SPECIAL_FIRST .. Real_Key_Code'Last;
   --  Type for a function- or special key number

   subtype Normal_Key_Code is Real_Key_Code range
     Character'Pos (Character'First) .. Character'Pos (Character'Last);
   --  This are the codes for regular (incl. non-graphical) characters.

   --  Constants for function- and special keys
   --
   Key_None                       : constant Special_Key_Code := M4_SPECIAL_FIRST;
include(`Key_Definitions')
   Key_Max                        : constant Special_Key_Code
     := Special_Key_Code'Last;

   subtype User_Key_Code is Key_Code
     range (Key_Max + 129) .. Key_Code'Last;
   --  This is reserved for user defined key codes. The range between Key_Max
   --  and the first user code is reserved for subsystems like menu and forms.

   --  For those who like to use the original key names we produce them were
   --  they differ from the original. Please note that they may differ in
   --  lower/upper case.
include(`Old_Keys')

------------------------------------------------------------------------------

   procedure Key_Name (Key  : in  Real_Key_Code;
                       Name : out String);
   --  The external name for a real keystroke.
   --  AKA: keyname() for some keys.

   type Color_Number is range 0 .. Integer (Interfaces.C.short'Last);
   for Color_Number'Size use Interfaces.C.short'Size;
   --  (n)curses uses a short for the color index
   --  The model is, that a Color_Number is an index into an array of
   --  (potentially) definable colors. Some of those indices are
   --  predefined (see below), although they may not really exist.

include(`Color_Defs')
   type RGB_Value is range 0 .. Integer (Interfaces.C.Short'Last);
   for RGB_Value'Size use Interfaces.C.short'Size;
   --  Some system may allow to redefine a color by setting RGB values.

   type Color_Pair is range 0 .. 255;
   for Color_Pair'Size use 8;
   subtype Redefinable_Color_Pair is Color_Pair range 1 .. 255;
   --  (n)curses reserves 1 Byte for the color-pair number. Color Pair 0
   --  is fixed (Black & White). A color pair is simply a combination of
   --  two colors described by Color_Numbers, one for the foreground and
   --  the other for the background

include(`Character_Attribute_Set_Rep')
   --  (n)curses uses half of an integer for attributes.

   Normal_Video : constant Character_Attribute_Set := (others => False);

   type Attributed_Character is
      record
         Attr  : Character_Attribute_Set := Normal_Video;
         Color : Color_Pair := 0;
         Ch    : Character  := ' ';
      end record;
   pragma Convention (C, Attributed_Character);
   --  This is the counterpart for the chtype in C.

include(`AC_Rep')
   Default_Character : constant Attributed_Character
     := (Ch    => Character'First,
         Color => 0,
         Attr  => Normal_Video);

   type Attributed_String is array (Positive range <>) of Attributed_Character;
   pragma Pack (Attributed_String);
   --  In this binding we allow strings of attributed characters.

   ------------------
   --  Exceptions  --
   ------------------
   Curses_Exception : exception;

   --  Those exceptions are raised by the ETI (Extended Terminal Interface)
   --  subpackets for Menu and Forms handling.
   --
   Eti_System_Error    : exception;
   Eti_Bad_Argument    : exception;
   Eti_Posted          : exception;
   Eti_Connected       : exception;
   Eti_Bad_State       : exception;
   Eti_No_Room         : exception;
   Eti_Not_Posted      : exception;
   Eti_Unknown_Command : exception;
   Eti_No_Match        : exception;
   Eti_Not_Selectable  : exception;
   Eti_Not_Connected   : exception;
   Eti_Request_Denied  : exception;
   Eti_Invalid_Field   : exception;
   Eti_Current         : exception;

   ----------------------------
   --  External C variables  --
   ----------------------------
   --  This is a bit dirty, because we assume C_Int'Size = Integer'Size
   pragma Assert (Interfaces.C.Int'Size = Integer'Size);

   Lines    : Line_Count;
   pragma Import (C, Lines, "LINES");

   Columns  : Column_Count;
   pragma Import (C, Columns, "COLS");

   Tab_Size : Natural;
   pragma Import (C, Tab_Size, "TABSIZE");

   Number_Of_Colors : Natural;
   pragma Import (C, Number_Of_Colors, "COLORS");

   Number_Of_Color_Pairs : Natural;
   pragma Import (C, Number_Of_Color_Pairs, "COLOR_PAIRS");

   ACS_Map : array (Character'Val (0) .. Character'Val (127)) of
     Attributed_Character;
   pragma Import (C, ACS_Map, "acs_map");
   --
   --
   --  Constants for several symbols from the Alternate Character Set
   --  You must use this constants as indices into the ACS_Map array
   --  to get the corresponding attributed character at runtime.
   --
include(`ACS_Map')

   --  We expect that one has to perform some calculations with line- or
   --  column positions.
   function "+" (Left : Column_Position; Right : Natural)
     return Column_Position;
   function "+" (Left : Natural; Right : Column_Position)
     return Column_Position;
   function "+" (Left : Line_Position; Right : Natural)
     return Line_Position;
   function "+" (Left : Natural; Right : Line_Position)
     return Line_Position;
   function "-" (Left : Column_Position; Right : Natural)
     return Column_Position;
   function "-" (Left : Natural; Right : Column_Position)
     return Column_Position;
   function "-" (Left : Line_Position; Right : Natural)
     return Line_Position;
   function "-" (Left : Natural; Right : Line_Position)
     return Line_Position;

   --  |=====================================================================
   --  | man page curs_initscr.3x
   --  | Not implemented: newterm, set_term, delscreen
   --  |=====================================================================
   function Standard_Window return Window;
   --  AKA: stdscr
   pragma Inline (Standard_Window);

   procedure Init_Screen;
   --  AKA: initscr()
   procedure Init_Windows renames Init_Screen;

   procedure End_Windows;
   --  AKA: endwin()
   procedure End_Screen renames End_Windows;

   function Is_End_Window return Boolean;
   --  AKA: isendwin()

   --  |=====================================================================
   --  | man page curs_move.3x
   --  |=====================================================================
   procedure Move_Cursor (Win    : in Window := Standard_Window;
                          Line   : in Line_Position;
                          Column : in Column_Position);
   --  AKA: wmove()

   --  |=====================================================================
   --  | Man page curs_addch.3x
   --  |=====================================================================
   procedure Add (Win :  in Window := Standard_Window;
                  Ch  :  in Attributed_Character);
   --  AKA: waddch()

   procedure Add (Win :  in Window := Standard_Window;
                  Ch  :  in Character);
   --  Add a single character at the current logical cursor position to
   --  the window. Use the current windows attributes.

   procedure Add
     (Win    : in Window := Standard_Window;
      Line   : in Line_Position;
      Column : in Column_Position;
      Ch     : in Attributed_Character);
   --  AKA: mvwaddch()

   procedure Add
     (Win    : in Window := Standard_Window;
      Line   : in Line_Position;
      Column : in Column_Position;
      Ch     : in Character);
   --  Move to the position and add a single character into the window

   procedure Add_With_Immediate_Echo
     (Win : in Window := Standard_Window;
      Ch  : in Attributed_Character);
   --  AKA: wechochar();

   procedure Add_With_Immediate_Echo
     (Win : in Window := Standard_Window;
      Ch  : in Character);
   --  Add a character and do an immediate resfresh of the screen.

   --  |=====================================================================
   --  | man page curs_window.3x
   --  |=====================================================================
   function Create
     (Number_Of_Lines       : Line_Count;
      Number_Of_Columns     : Column_Count;
      First_Line_Position   : Line_Position;
      First_Column_Position : Column_Position) return Window;
   --  AKA: newwin()

   function New_Window
     (Number_Of_Lines       : Line_Count;
      Number_Of_Columns     : Column_Count;
      First_Line_Position   : Line_Position;
      First_Column_Position : Column_Position) return Window
     renames Create;

   procedure Delete (Win : in out Window);
   --  AKA: delwin()
   --  Reset Win to Null_Window

   function Sub_Window
     (Win                   : Window := Standard_Window;
      Number_Of_Lines       : Line_Count;
      Number_Of_Columns     : Column_Count;
      First_Line_Position   : Line_Position;
      First_Column_Position : Column_Position) return Window;
   --  AKA: subwin()

   function Derived_Window
     (Win                   : Window := Standard_Window;
      Number_Of_Lines       : Line_Count;
      Number_Of_Columns     : Column_Count;
      First_Line_Position   : Line_Position;
      First_Column_Position : Column_Position) return Window;
   --  AKA: derwin()

   function Duplicate (Win : Window) return Window;
   --  AKA: dupwin()

   procedure Move_Window (Win    : in Window;
                          Line   : in Line_Position;
                          Column : in Column_Position);
   --  AKA: mvwin()

   procedure Move_Derived_Window (Win    : in Window;
                                  Line   : in Line_Position;
                                  Column : in Column_Position);
   --  AKA: mvderwin()

   procedure Synchronize_Upwards (Win : in Window);
   pragma Import (C, Synchronize_Upwards, "wsyncup");

   procedure Synchronize_Downwards (Win : in Window);
   pragma Import (C, Synchronize_Downwards, "wsyncdown");

   procedure Set_Synch_Mode (Win  : in Window := Standard_Window;
                             Mode : in Boolean := False);
   --  AKA: syncok()

   --  |=====================================================================
   --  | man page curs_addstr.3x
   --  |=====================================================================
   procedure Add (Win : in Window := Standard_Window;
                  Str : in String;
                  Len : in Integer := -1);
   --  AKA: waddnstr() and waddstr()

   procedure Add (Win    : in Window := Standard_Window;
                  Line   : in Line_Position;
                  Column : in Column_Position;
                  Str    : in String;
                  Len    : in Integer := -1);
   --  AKA: mvwaddnstr() and mvwaddstr()

   --  |=====================================================================
   --  | man page curs_addchstr.3x
   --  |=====================================================================
   procedure Add (Win : in Window := Standard_Window;
                  Str : in Attributed_String;
                  Len : in Integer := -1);
   --  AKA: waddchnstr() and waddchstr()

   procedure Add (Win    : in Window := Standard_Window;
                  Line   : in Line_Position;
                  Column : in Column_Position;
                  Str    : in Attributed_String;
                  Len    : in Integer := -1);
   --  AKA: mvwaddchnstr() and mvwaddchstr()

   --  |=====================================================================
   --  | man page curs_border.3x
   --  |=====================================================================
   procedure Border
     (Win                       : in Window := Standard_Window;
      Left_Side_Symbol          : in Attributed_Character := Default_Character;
      Right_Side_Symbol         : in Attributed_Character := Default_Character;
      Top_Side_Symbol           : in Attributed_Character := Default_Character;
      Bottom_Side_Symbol        : in Attributed_Character := Default_Character;
      Upper_Left_Corner_Symbol  : in Attributed_Character := Default_Character;
      Upper_Right_Corner_Symbol : in Attributed_Character := Default_Character;
      Lower_Left_Corner_Symbol  : in Attributed_Character := Default_Character;
      Lower_Right_Corner_Symbol : in Attributed_Character := Default_Character
     );
   --  AKA: wborder()

   procedure Box
     (Win               : in Window := Standard_Window;
      Vertical_Symbol   : in Attributed_Character := Default_Character;
      Horizontal_Symbol : in Attributed_Character := Default_Character);
   --  AKA: box()

   procedure Horizontal_Line
     (Win         : in Window := Standard_Window;
      Line_Size   : in Natural;
      Line_Symbol : in Attributed_Character := Default_Character);
   --  AKA: whline()

   procedure Vertical_Line
     (Win         : in Window := Standard_Window;
      Line_Size   : in Natural;
      Line_Symbol : in Attributed_Character := Default_Character);
   --  AKA: wvline()

   --  |=====================================================================
   --  | man page curs_getch.3x
   --  |=====================================================================
   function Get_Keystroke (Win : Window := Standard_Window)
                           return Real_Key_Code;
   --  AKA: wgetch()
   --  Get a character from the keyboard and echo it - if enabled - to the
   --  window.
   --  If for any reason (i.e. a timeout) we couldn't get a character the
   --  returned keycode is Key_None.

   procedure Undo_Keystroke (Key : in Real_Key_Code);
   --  AKA: ungetch()

   function Has_Key (Key : Special_Key_Code) return Boolean;
   --  AKA: has_key()
   --  |
   --  | Some helper functions
   --  |
   function Is_Function_Key (Key : Special_Key_Code) return Boolean;
   --  Return True if the Key is a function key (i.e. one of F0 .. F63)

   subtype Function_Key_Number is Integer range 0 .. 63;
   --  (n)curses allows for 64 function keys.

   function Function_Key (Key : Real_Key_Code) return Function_Key_Number;
   --  Return the number of the function key. If the code is not a
   --  function key, a CONSTRAINT_ERROR will be raised.

   function Function_Key_Code (Key : Function_Key_Number) return Real_Key_Code;
   --  Return the key code for a given functionkey number.

   --  |=====================================================================
   --  | man page curs_attr.3x
   --  |=====================================================================
   procedure Switch_Character_Attribute
     (Win  : in Window := Standard_Window;
      Attr : in Character_Attribute_Set := Normal_Video;
      On   : in Boolean := True); --  if False we switch Off.
   --  AKA: wattron() and wattroff()

   procedure Set_Character_Attributes
     (Win   : in Window := Standard_Window;
      Attrs : in Character_Attribute_Set := Normal_Video);
   --  AKA: wattrset()

   function Get_Character_Attribute
     (Win : in Window := Standard_Window) return Character_Attribute_Set;
   --  AKA: wattr_get()

   procedure Change_Attributes
     (Win   : in Window := Standard_Window;
      Count : in Integer := -1;
      Attrs : in Character_Attribute_Set;
      Color : in Color_Pair);
   --  AKA: wchgat()

   procedure Change_Attributes
     (Win    : in Window := Standard_Window;
      Line   : in Line_Position;
      Column : in Column_Position;
      Count  : in Integer := -1;
      Attrs  : in Character_Attribute_Set;
      Color  : in Color_Pair);
   --  AKA: mvwchgat()

   --  |=====================================================================
   --  | man page curs_beep.3x
   --  |=====================================================================
   procedure Beep;
   --  AKA: beep()

   procedure Flash_Screen;
   --  AKA: flash()

   --  |=====================================================================
   --  | man page curs_inopts.3x
   --  | Not implemented : typeahead
   --  |=====================================================================
   procedure Set_Cbreak_Mode (SwitchOn : in Boolean := True);
   --  AKA: cbreak(), nocbreak()

   procedure Set_Raw_Mode (SwitchOn : in Boolean := True);
   --  AKA: raw(), noraw()

   procedure Set_Echo_Mode (SwitchOn : in Boolean := True);
   --  AKA: echo(), noecho()

   procedure Set_Meta_Mode (Win      : in Window := Standard_Window;
                            SwitchOn : in Boolean := True);
   --  AKA: meta()

   procedure Set_KeyPad_Mode (Win      : in Window := Standard_Window;
                              SwitchOn : in Boolean := True);
   --  AKA: keypad()

   type Half_Delay_Amount is range 1 .. 255;
   procedure Half_Delay (Amount : in Half_Delay_Amount);
   --  AKA: halfdelay()

   procedure Set_Flush_On_Interrupt_Mode
     (Win  : in Window := Standard_Window;
      Mode : in Boolean := True);
   --  AKA: intrflush()

   procedure Set_Queue_Interrupt_Mode
     (Win   : in Window := Standard_Window;
      Flush : in Boolean := True);
   --  AKA: qiflush(), noqiflush()

   procedure Set_NoDelay_Mode
     (Win  : in Window := Standard_Window;
      Mode : in Boolean := False);
   --  AKA: nodelay()

   type Timeout_Mode is (Blocking, Non_Blocking, Delayed);
   procedure Set_Timeout_Mode (Win    : in Window := Standard_Window;
                               Mode   : in Timeout_Mode;
                               Amount : in Natural); --  in Miliseconds
   --  AKA: wtimeout()
   --  Instead of overloading the semantic of the sign of amount, we
   --  introduce the Timeout_Mode parameter. This should improve
   --  readability. For Blocking and Non_Blocking, the Amount is not
   --  evaluated.

   procedure Set_Escape_Timer_Mode
     (Win       : in Window := Standard_Window;
      Timer_Off : in Boolean := False);
   --  AKA: notimeout()

   --  |=====================================================================
   --  | man page curs_outopts.3x
   --  |=====================================================================
   procedure Set_NL_Mode (SwitchOn : in Boolean := True);
   --  AKA: nl(), nonl()

   procedure Clear_On_Next_Update
     (Win      : in Window := Standard_Window;
      Do_Clear : in Boolean := True);
   --  AKA: clearok()

   procedure Use_Insert_Delete_Line
     (Win    : in Window := Standard_Window;
      Do_Idl : in Boolean := True);
   --  AKA: idlok()

   procedure Use_Insert_Delete_Character
     (Win    : in Window := Standard_Window;
      Do_Idc : in Boolean := True);
   --  AKA: idcok()

   procedure Leave_Cursor_After_Update
     (Win      : in Window := Standard_Window;
      Do_Leave : in Boolean := True);
   --  AKA: leaveok()

   procedure Immediate_Update_Mode
     (Win  : in Window := Standard_Window;
      Mode : in Boolean := False);
   --  AKA: immedok()

   procedure Allow_Scrolling
     (Win  : in Window := Standard_Window;
      Mode : in Boolean := False);
   --  AKA: scrollok()

   function Scrolling_Allowed (Win : Window := Standard_Window) return Boolean;
   --  There is no such function in the C interface.

   procedure Set_Scroll_Region
     (Win         : in Window := Standard_Window;
      Top_Line    : in Line_Position;
      Bottom_Line : in Line_Position);
   --  AKA: wsetscrreg()

   --  |=====================================================================
   --  | man page curs_refresh.3x
   --  |=====================================================================
   procedure Update_Screen;
   --  AKA: doupdate()

   procedure Refresh (Win : in Window := Standard_Window);
   --  AKA: wrefresh()

   procedure Refresh_Without_Update
     (Win : in Window := Standard_Window);
   --  AKA: wnoutrefresh()

   procedure Redraw (Win : in Window := Standard_Window);
   --  AKA: redrawwin()

   procedure Redraw (Win        : in Window := Standard_Window;
                     Begin_Line : in Line_Position;
                     Line_Count : in Positive);
   --  AKA: wredrawln()

   --  |=====================================================================
   --  | man page curs_clear.3x
   --  |=====================================================================
   procedure Erase (Win : in Window := Standard_Window);
   --  AKA: werase()

   procedure Clear
     (Win : in Window := Standard_Window);
   --  AKA: wclear()

   procedure Clear_To_End_Of_Screen
     (Win : in Window := Standard_Window);
   --  AKA: wclrtobot()

   procedure Clear_To_End_Of_Line
     (Win : in Window := Standard_Window);
   --  AKA: wclrtoeol()

   --  |=====================================================================
   --  | man page curs_bkgd.3x
   --  |=====================================================================
   procedure Set_Background
     (Win : in Window := Standard_Window;
      Ch  : in Attributed_Character);
   --  AKA: wbkgdset()

   procedure Change_Background
     (Win : in Window := Standard_Window;
      Ch  : in Attributed_Character);
   --  AKA: wbkgd()

   function Get_Background (Win : Window := Standard_Window)
     return Attributed_Character;
   --  AKA: wbkgdget()

   --  |=====================================================================
   --  | man page curs_touch.3x
   --  |=====================================================================
   procedure Touch (Win : in Window := Standard_Window);
   --  AKA: touchwin()

   procedure Untouch (Win : in Window := Standard_Window);
   --  AKA: untouchwin()

   procedure Touch (Win   : in Window := Standard_Window;
                    Start : in Line_Position;
                    Count : in Positive);
   --  AKA: touchline()

   procedure Change_Lines_Status (Win   : in Window := Standard_Window;
                                  Start : in Line_Position;
                                  Count : in Positive;
                                  State : in Boolean);
   --  AKA: wtouchln()

   function Is_Touched (Win  : Window := Standard_Window;
                        Line : Line_Position) return Boolean;
   --  AKA: is_linetouched()

   function Is_Touched (Win : Window := Standard_Window) return Boolean;
   --  AKA: is_wintouched()

   --  |=====================================================================
   --  | man page curs_overlay.3x
   --  |=====================================================================
   procedure Copy
     (Source_Window            : in Window;
      Destination_Window       : in Window;
      Source_Top_Row           : in Line_Position;
      Source_Left_Column       : in Column_Position;
      Destination_Top_Row      : in Line_Position;
      Destination_Left_Column  : in Column_Position;
      Destination_Bottom_Row   : in Line_Position;
      Destination_Right_Column : in Column_Position;
      Non_Destructive_Mode     : in Boolean := True);
   --  AKA: copywin()

   procedure Overwrite (Source_Window      : in Window;
                        Destination_Window : in Window);
   --  AKA: overwrite()

   procedure Overlay (Source_Window      : in Window;
                      Destination_Window : in Window);
   --  AKA: overlay()

   --  |=====================================================================
   --  | man page curs_deleteln.3x
   --  |=====================================================================
   procedure Insert_Delete_Lines
     (Win   : in Window  := Standard_Window;
      Lines : in Integer := 1); --  default is to insert one line above
   --  AKA: winsdelln()

   procedure Delete_Line (Win : in Window := Standard_Window);
   --  AKA: wdeleteln()

   procedure Insert_Line (Win : in Window := Standard_Window);
   --  AKA: winsertln()

   --  |=====================================================================
   --  | man page curs_getyx
   --  |=====================================================================
   procedure Get_Size
     (Win               : in Window := Standard_Window;
      Number_Of_Lines   : out Line_Count;
      Number_Of_Columns : out Column_Count);
   --  AKA: getmaxyx()

   procedure Get_Window_Position
     (Win             : in Window := Standard_Window;
      Top_Left_Line   : out Line_Position;
      Top_Left_Column : out Column_Position);
   --  AKA: getbegyx()

   procedure Get_Cursor_Position
     (Win    : in  Window := Standard_Window;
      Line   : out Line_Position;
      Column : out Column_Position);
   --  AKA: getyx()

   procedure Get_Origin_Relative_To_Parent
     (Win                : in  Window;
      Top_Left_Line      : out Line_Position;
      Top_Left_Column    : out Column_Position;
      Is_Not_A_Subwindow : out Boolean);
   --  AKA: getparyx()
   --  Instead of placing -1 in the coordinates as return, we use a boolean
   --  to return the info that the window has no parent.

   --  |=====================================================================
   --  | man page curs_pad.3x
   --  |=====================================================================
   function New_Pad (Lines   : Line_Count;
                     Columns : Column_Count) return Window;
   --  AKA: newpad()

   function Sub_Pad
     (Pad                   : Window;
      Number_Of_Lines       : Line_Count;
      Number_Of_Columns     : Column_Count;
      First_Line_Position   : Line_Position;
      First_Column_Position : Column_Position) return Window;
   --  AKA: subpad()

   procedure Refresh
     (Pad                      : in Window;
      Source_Top_Row           : in Line_Position;
      Source_Left_Column       : in Column_Position;
      Destination_Top_Row      : in Line_Position;
      Destination_Left_Column  : in Column_Position;
      Destination_Bottom_Row   : in Line_Position;
      Destination_Right_Column : in Column_Position);
   --  AKA: prefresh()

   procedure Refresh_Without_Update
     (Pad                      : in Window;
      Source_Top_Row           : in Line_Position;
      Source_Left_Column       : in Column_Position;
      Destination_Top_Row      : in Line_Position;
      Destination_Left_Column  : in Column_Position;
      Destination_Bottom_Row   : in Line_Position;
      Destination_Right_Column : in Column_Position);
   --  AKA: pnoutrefresh()

   procedure Add_Character_To_Pad_And_Echo_It
     (Pad : in Window;
      Ch  : in Attributed_Character);
   --  AKA: pechochar()

   procedure Add_Character_To_Pad_And_Echo_It
     (Pad : in Window;
      Ch  : in Character);

   --  |=====================================================================
   --  | man page curs_scroll.3x
   --  |=====================================================================
   procedure Scroll (Win    : in Window  := Standard_Window;
                     Amount : in Integer := 1);
   --  AKA: wscrl()

   --  |=====================================================================
   --  | man page curs_delch.3x
   --  |=====================================================================
   procedure Delete_Character (Win : in Window := Standard_Window);
   --  AKA: wdelch()

   procedure Delete_Character
     (Win    : in Window := Standard_Window;
      Line   : in Line_Position;
      Column : in Column_Position);
   --  AKA: mvwdelch()

   --  |=====================================================================
   --  | man page curs_inch.3x
   --  |=====================================================================
   function Peek (Win : Window := Standard_Window)
     return Attributed_Character;
   --  AKA: winch()

   function Peek
     (Win    : Window := Standard_Window;
      Line   : Line_Position;
      Column : Column_Position) return Attributed_Character;
   --  AKA: mvwinch()

   --  |=====================================================================
   --  | man page curs_winch.3x
   --  |=====================================================================
   procedure Insert (Win : in Window := Standard_Window;
                     Ch  : in Attributed_Character);
   --  AKA: winsch()

   procedure Insert (Win    : in Window := Standard_Window;
                     Line   : in Line_Position;
                     Column : in Column_Position;
                     Ch     : in Attributed_Character);
   --  AKA: mvwinsch()

   --  |=====================================================================
   --  | man page curs_insstr.3x
   --  |=====================================================================
   procedure Insert (Win : in Window := Standard_Window;
                     Str : in String;
                     Len : in Integer := -1);
   --  AKA: winsnstr() and winsstr()

   procedure Insert (Win    : in Window := Standard_Window;
                     Line   : in Line_Position;
                     Column : in Column_Position;
                     Str    : in String;
                     Len    : in Integer := -1);
   --  AKA: mvwinsnstr() and mvwinsstr()

   --  |=====================================================================
   --  | man page curs_instr.3x
   --  |=====================================================================
   procedure Peek (Win : in  Window := Standard_Window;
                   Str : out String;
                   Len : in  Integer := -1);
   --  AKA: winnstr() and winstr()

   procedure Peek (Win    : in  Window := Standard_Window;
                   Line   : in  Line_Position;
                   Column : in  Column_Position;
                   Str    : out String;
                   Len    : in  Integer := -1);
   --  AKA: mvwinnstr() and mvwinstr()

   --  |=====================================================================
   --  | man page curs_inchstr.3x
   --  |=====================================================================
   procedure Peek (Win : in  Window := Standard_Window;
                   Str : out Attributed_String;
                   Len : in  Integer := -1);
   --  AKA: winchnstr() and winchstr()

   procedure Peek (Win    : in  Window := Standard_Window;
                   Line   : in  Line_Position;
                   Column : in  Column_Position;
                   Str    : out Attributed_String;
                   Len    : in  Integer := -1);
   --  AKA: mvwinchnstr() and mvwinchstr()

   --  |=====================================================================
   --  | man page curs_getstr.3x
   --  |=====================================================================
   procedure Get (Win : in  Window := Standard_Window;
                  Str : out String;
                  Len : in  Integer := -1);
   --  AKA: wgetnstr() and wgetstr()

   procedure Get (Win    : in  Window := Standard_Window;
                  Line   : in  Line_Position;
                  Column : in  Column_Position;
                  Str    : out String;
                  Len    : in  Integer := -1);
   --  AKA: not specified in ncurses, should be: mvwgetnstr()
   --       and mvwgetstr() (which exists)

   --  |=====================================================================
   --  | man page curs_slk.3x
   --  |=====================================================================
   type Soft_Label_Key_Format is (Three_Two_Three,
                                  Four_Four,
                                  PC_Style,              --  ncurses specific
                                  PC_Style_With_Index);  --  "
   type Label_Number is new Positive range 1 .. 12;
   type Label_Justification is (Left, Centered, Right);

   procedure Init_Soft_Label_Keys
     (Format : in Soft_Label_Key_Format := Three_Two_Three);
   --  AKA: slk_init()

   procedure Set_Soft_Label_Key (Label : in Label_Number;
                                 Text  : in String;
                                 Fmt   : in Label_Justification := Left);
   --  AKA: slk_set()

   procedure Refresh_Soft_Label_Keys;
   --  AKA: slk_refresh()

   procedure Refresh_Soft_Label_Keys_Without_Update;
   --  AKA: slk_noutrefresh()

   procedure Get_Soft_Label_Key (Label : in Label_Number;
                                 Text  : out String);
   --  AKA: slk_label()

   procedure Clear_Soft_Label_Keys;
   --  AKA: slk_clear()

   procedure Restore_Soft_Label_Keys;
   --  AKA: slk_restore()

   procedure Touch_Soft_Label_Keys;
   --  AKA: slk_touch()

   procedure Switch_Soft_Label_Key_Attributes
     (Attr : in Character_Attribute_Set;
      On   : in Boolean := True);
   --  AKA: slk_attron(), slk_attroff()

   procedure Set_Soft_Label_Key_Attributes
     (Attr : in Character_Attribute_Set);
   --  AKA: slk_attrset()

   function Get_Soft_Label_Key_Attributes return Character_Attribute_Set;
   --  AKA: slk_attr()

   --  |=====================================================================
   --  | man page curs_util.3x
   --  | Not implemented : filter, use_env, putwin, getwin
   --  |=====================================================================
   procedure Un_Control (Ch  : in Attributed_Character;
                         Str : out String);
   --  AKA: unctrl()

   procedure Delay_Output (Msecs : in Natural);
   --  AKA: delay_output()

   procedure Flush_Input;
   --  AKA: flushinp()

   --  |=====================================================================
   --  | man page curs_termattrs.3x
   --  |=====================================================================
   function Baudrate return Natural;
   --  AKA: baudrate()

   function Erase_Character return Character;
   --  AKA: erasechar()

   function Kill_Character return Character;
   --  AKA: killchar()

   function Has_Insert_Character return Boolean;
   --  AKA: has_ic()

   function Has_Insert_Line return Boolean;
   --  AKA: has_il()

   function Supported_Attributes return Character_Attribute_Set;
   --  AKA: termattrs()

   procedure Long_Name (Name : out String);
   --  AKA: longname()

   procedure Terminal_Name (Name : out String);
   --  AKA: termname()

   --  |=====================================================================
   --  | man page curs_color.3x
   --  |=====================================================================
   procedure Start_Color;
   --  AKA: start_clolor()
   pragma Import (C, Start_Color, "start_color");

   procedure Init_Pair (Pair : in Redefinable_Color_Pair;
                        Fore : in Color_Number;
                        Back : in Color_Number);
   --  AKA: init_pair()

   procedure Pair_Content (Pair : in Color_Pair;
                           Fore : out Color_Number;
                           Back : out Color_Number);
   --  AKA: pair_content()

   function Has_Colors return Boolean;
   --  AKA: has_colors()

   procedure Init_Color (Color : in Color_Number;
                         Red   : in RGB_Value;
                         Green : in RGB_Value;
                         Blue  : in RGB_Value);
   --  AKA: init_color()

   function Can_Change_Color return Boolean;
   --  AKA: can_change_color()

   procedure Color_Content (Color : in  Color_Number;
                            Red   : out RGB_Value;
                            Green : out RGB_Value;
                            Blue  : out RGB_Value);
   --  AKA: color_content()

   --  |=====================================================================
   --  | man page curs_kernel.3x
   --  | Not implemented: getsyx, setsyx
   --  |=====================================================================
   type Curses_Mode is (Curses, Shell);

   procedure Save_Curses_Mode (Mode : in Curses_Mode);
   --  AKA: def_prog_mode(), def_shell_mode()

   procedure Reset_Curses_Mode (Mode : in Curses_Mode);
   --  AKA: reset_prog_mode(), reset_shell_mode()

   procedure Save_Terminal_State;
   --  AKA: savetty()

   procedure Reset_Terminal_State;
   --  AKA: resetty();

   type Stdscr_Init_Proc is access
      function (Win     : Window;
                Columns : Column_Count) return Integer;
   pragma Convention (C, Stdscr_Init_Proc);
   --  N.B.: the return value is actually ignored, but it seems to be
   --        a good practice to return 0 if you think all went fine
   --        and -1 otherwise.

   procedure Rip_Off_Lines (Lines : in Integer;
                            Proc  : in Stdscr_Init_Proc);
   --  AKA: ripoffline()
   --  N.B.: to be more precise, this uses a ncurses specific enhancement of
   --        ripoffline(), in which the Lines argument absolute value is the
   --        number of lines to be ripped of. The official ripoffline() only
   --        uses the sign of Lines to rip of a single line from bottom or top.

   type Cursor_Visibility is (Invisible, Normal, Very_Visible);

   procedure Set_Cursor_Visibility (Visibility : in out Cursor_Visibility);
   --  AKA: curs_set()

   procedure Nap_Milli_Seconds (Ms : in Natural);
   --  AKA: napms()

   --  |=====================================================================
   --  | Some usefull helpers.
   --  |=====================================================================
   type Transform_Direction is (From_Screen, To_Screen);
   procedure Transform_Coordinates
     (W      : in Window := Standard_Window;
      Line   : in out Line_Position;
      Column : in out Column_Position;
      Dir    : in Transform_Direction := From_Screen);
   --  This procedure transforms screen coordinates into coordinates relative
   --  to the window and vice versa, depending on the Dir parmeter.
   --  Screen coordinates are the position informations on the physical device.
   --  An Curses_Exception will be raised if Line and Column are not in the
   --  Window or if you pass the Null_Window as argument.

private
   pragma Inline ("+");
   pragma Inline ("-");

   type Window is new System.Address;
   Null_Window : constant Window := Window (System.Null_Address);

   Generation_Bit_Order : constant System.Bit_Order := System.M4_BIT_ORDER;

end Terminal_Interface.Curses;
