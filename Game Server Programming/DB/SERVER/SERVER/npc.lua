myid = 99999;
move_count = 0;

function set_uid(x)
   myid = x;
end

function event_player_move(player)
   player_x = API_get_x(player);
   player_y = API_get_y(player);
   my_x = API_get_x(myid);
   my_y = API_get_y(myid);
   if (player_x == my_x) then
      if (player_y == my_y) then
         API_SendMessage(myid, player, "HELLO");
         return 1;
      end
   end
   return 0;
end

function event_npc_move(player)
   API_do_npc_random_move(myid);
   move_count = move_count + 1;
   if (move_count == 3) then
      API_SendMessage(myid, player, "BYE");
      move_count = 0;
      return 1;
   end
   return 0;
end