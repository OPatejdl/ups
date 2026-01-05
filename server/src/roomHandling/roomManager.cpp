#include "roomManager.hpp"

// Init of static parts
std::vector<std::shared_ptr<Room>> RoomManager::rooms;
int RoomManager::room_id_counter = RoomConfig::INIT_ROOM_ID;

std::shared_ptr<Room> RoomManager::join_waiting_room(std::shared_ptr<User> user) {
    // Try to find a room
    for (auto& room : rooms) {
        if (room->state == ROOM_STATE::WAITING_FOR_PLAYER && !room->is_full()) {
            // Make sure that player doesn't play with itself
            if (!room->has_player(user->fd_socket)) {
                room->add_player(user);
                LOG_INFO("User " + user->nickname + " joined existing room " + std::to_string(room->id));
                return room;
            }
        }
    }

    // Try to create a new room
    if (rooms.size() >= Utility::ROOMS_COUNT) {
        LOG_WARNING("All rooms are occupied. Cannot create new room.");
        return nullptr;
    }

    // Create new room
    std::shared_ptr<Room> new_room = std::make_shared<Room>(room_id_counter++);
    new_room->add_player(user);
    rooms.push_back(new_room);
    
    LOG_INFO("Created new room " + std::to_string(new_room->id) + " for user " + user->nickname);
    return new_room;
}

std::shared_ptr<Room> RoomManager::get_room_by_user_fd(int fd) {
    for (auto& room : rooms) {
        if (room->has_player(fd)) {
            return room;
        }
    }
    return nullptr;
}

void RoomManager::cleanup_empty_rooms() {
    auto i = rooms.begin();
    while (i != rooms.end()) {
        if ((*i)->is_empty()) {
            LOG_INFO("Removing empty room ID: " + std::to_string((*i )->id));
            i = rooms.erase(i);
        } else {
            ++i;
        }
    }
}

void RoomManager::remove_room(int room_id) {
    for (auto it = rooms.begin(); it != rooms.end(); ++it) {
        // Pokud najdeme místnost se shodným ID
        if ((*it)->id == room_id) {
            LOG_INFO("Removing room ID: " + std::to_string(room_id));
            
            // Remove room
            rooms.erase(it);

            return;
        }
    }
    
    LOG_WARNING("Unsuccessful attempt to remove room with ID: " + std::to_string(room_id));
}