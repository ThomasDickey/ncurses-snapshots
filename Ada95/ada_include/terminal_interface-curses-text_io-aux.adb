------------------------------------------------------------------------------
--                                                                          --
--                           GNAT ncurses Binding                           --
--                                                                          --
--                   Terminal_Interface.Curses.Text_IO.Aux                  --
--                                                                          --
--                                 B O D Y                                  --
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
package body Terminal_Interface.Curses.Text_IO.Aux is

   procedure Put_Buf
     (Win    : in Window;
      Buf    : in String;
      Width  : in Field;
      Signal : in Boolean := True;
      Ljust  : in Boolean := False)
   is
      L   : Field;
      Len : Field;
      W   : Field := Width;
   begin
      if Ljust then
         L := 1;
         for I in 1 .. Buf'Length loop
            exit when Buf (L) = ' ';
            L := L + 1;
         end loop;
         Len := L - 1;
         if W = 0 then
            W := Len;
         end if;
         if W > 0 then
            if Len <= W then
               Put (Win, Buf (1 .. Len));
               if Len < W then
                  declare
                     Filler : constant String (1 .. (W - Len))
                       := (others => ' ');
                  begin
                     Put (Win, Filler);
                  end;
               end if;
            else
               if Signal then
                  raise Layout_Error;
               end if;
            end if;
         end if;
      else
         L := Buf'Length;
         for I in 1 .. Buf'Length loop
            exit when Buf (L) = ' ';
            L := L - 1;
         end loop;
         if L < Buf'Length then
            Len := Buf'Length - L;
            if W > 0 then
               if Len < W then
                  declare
                     Filler : constant String (1 .. (W - Len))
                       := (others => ' ');
                  begin
                     Put (Win, Filler);
                  end;
               else
                  if Len > W and then Signal then
                     raise Layout_Error;
                  end if;
               end if;
            end if;
            Put (Win, Buf ((L + 1) .. Buf'Length));
         end if;
      end if;
   end Put_Buf;

end Terminal_Interface.Curses.Text_IO.Aux;
