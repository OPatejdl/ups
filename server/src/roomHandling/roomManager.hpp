#ifndef ROOM_MANAGER_HPP
#define ROOM_MANAGER_HPP

#include <vector>
#include <memory>
#include <algorithm>
#include "room.hpp"
#include "../userHandling/user.hpp"
#include "../utility/utility.hpp"
#include "../config.hpp"
#include "roomConfig.hpp"

class RoomManager {
public:
    static std::vector<std::shared_ptr<Room>> rooms;   /** List of all rooms */     

    /**
     * Tries to add a user to a room or creates a new one,
     * if all rooms are full and is still space for new room
     * @return pointer to the room, or nullptr
     */
    static std::shared_ptr<Room> join_waiting_room(std::shared_ptr<User> user);

    /**
     * Finds room, where is the user with certain fd
     * @return pointer to the room, or nullptr
     */
    static std::shared_ptr<Room> get_room_by_user_fd(int fd);

    /**
     * Cleans up empty rooms
     */
    static void cleanup_empty_rooms();

private:
    static int room_id_counter;     /** Counter for new room Id */
};

#endif