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

/**
 * Static class responsible for the lifecycle of all game rooms.
 * Handles room creation, player assignment, and rooms cleanup.
 */
class RoomManager {
public:
    static std::vector<std::shared_ptr<Room>> rooms;   /** List of all rooms */     

    /**
     * Attempts to place a user into an available waiting room
     * * @param user Shared pointer to the user who wants to join a game
     * @return Shared pointer to the joined/created room, or nullptr if all rooms are occupied
     */
    static std::shared_ptr<Room> join_waiting_room(std::shared_ptr<User> user);

    /**
     * Searches for a room containing a specific player
     * * @param fd File descriptor (socket) of the player to search for
     * @return Shared pointer to the room where the user is located, or nullptr if not found
     */
    static std::shared_ptr<Room> get_room_by_user_fd(int fd);

    /**
     * Performs a cleanup of the room list by removing all rooms that have no players
     */
    static void cleanup_empty_rooms();

    /**
     * Removes a room based on its Id
     * @param room_id Id of the room to be removed
     */
    static void remove_room(int room_id);

private:
    static int room_id_counter;     /** Counter for new room Id */
};

#endif