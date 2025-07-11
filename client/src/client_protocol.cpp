#include "../include/client_protocol.h"

#include <cstdint>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include <arpa/inet.h>

#include "../../common/constants_protocol.h"


ClientProtocol::ClientProtocol(const std::string& hostname, const std::string& servname,
                               const std::string& username):
        Protocol_(hostname, servname), username(username) {
    sendUserName(username);
}


void ClientProtocol::sendUserName(const std::string& username) {
    std::vector<uint8_t> buffer;

    buffer.push_back(BYTE_INIT_MSG);
    insertBigEndian16(username.length(), buffer);
    insertStringBytes(username, buffer);

    socket.sendall(buffer.data(), sizeof(uint8_t) * buffer.size());
}

void ClientProtocol::sendMenuAction(const MenuAction& action) {
    std::vector<uint8_t> buffer;
    int length = action.name_match.length();

    buffer.push_back(encodeMenuActionType(action.type));

    if (action.type == Create || action.type == Join) {
        insertBigEndian16(length, buffer);
        insertStringBytes(action.name_match, buffer);
        if (action.type == Create) {
            insertBigEndian16(action.scenario_name.length(), buffer);
            insertStringBytes(action.scenario_name, buffer);
        }
    }
    socket.sendall(buffer.data(), sizeof(uint8_t) * buffer.size());
}

void ClientProtocol::sendLobbyAction(const LobbyAction& action) {
    std::vector<uint8_t> buffer;

    if (action == LobbyAction::QuitMatch) {
        buffer.push_back(BYTE_QUIT_MATCH);
    } else if (action == LobbyAction::StartMatch) {
        buffer.push_back(BYTE_START_MATCH);
    } else {
        buffer.push_back(BYTE_LIST_PLAYERS);
    }
    socket.sendall(buffer.data(), sizeof(uint8_t) * buffer.size());
}

void ClientProtocol::sendGameAction(const GameAction& gameAction) {
    std::vector<uint8_t> buffer;

    buffer.push_back(encodeGameActionType(gameAction.type));

    if (gameAction.type == BuyWeapon) {
        buffer.push_back(encodeWeapon(gameAction.weapon));
    } else if (gameAction.type == BuyAmmo) {
        buffer.push_back(encodeAmmoType(gameAction.ammoType));
        insertBigEndian16(gameAction.count_ammo, buffer);
    } else if (gameAction.type == ChangeWeapon) {
        buffer.push_back(encodeTypeWeapon(gameAction.typeWeapon));
    } else if (gameAction.type == Attack || gameAction.type == Walk) {
        insertFloatNormalized3Bytes(gameAction.direction.getX(), buffer);
        insertFloatNormalized3Bytes(gameAction.direction.getY(), buffer);
    } else if (gameAction.type == Rotate) {
        insertFloat4Bytes(gameAction.angle, buffer);
    }

    socket.sendall(buffer.data(), sizeof(uint8_t) * buffer.size());
}

bool ClientProtocol::recvConfirmation() {
    uint8_t byte = 0;
    socket.recvall(&byte, sizeof(uint8_t));
    if (byte == BYTE_OK)
        return true;
    else if (byte == BYTE_FAIL)
        return false;

    throw std::runtime_error("Error. Se esperaba un byte OK o FAIL y se recibío otra cosa.");
}

MatchInfo ClientProtocol::recvMatchInfo() {
    uint8_t byte;
    socket.recvall(&byte, sizeof(uint8_t));
    if (byte != BYTE_MATCH_INFO) {
        throw std::runtime_error(
                "Error. Se recibió un byte inesperado en ClientProtocol::recvMatchInfo()");
    }
    // recibo el nombre de la partida.
    std::string name;
    uint16_t length = recvBigEndian16();
    name.resize(length);
    socket.recvall(name.data(), sizeof(uint8_t) * length);
    // recibo el window_config
    int window_width = recvBigEndian16();
    int window_heigth = recvBigEndian16();
    int window_flags = recvBigEndian32();

    // 4) recibo FOVConfig
    socket.recvall(&byte, sizeof(uint8_t));
    bool isActive = decodeBool(byte);
    int screenW = recvBigEndian16();
    int screenH = recvBigEndian16();
    int circleR = recvBigEndian16();
    float fovAngle = recvFloat();
    float visibilityDistance = recvFloat();
    float transparency = recvFloat();

    // recibo el tilemap
    int size_buffer_tilemap = recvBigEndian32();
    std::vector<uint8_t> bytes_tilemap(size_buffer_tilemap);
    socket.recvall(bytes_tilemap.data(), sizeof(uint8_t) * size_buffer_tilemap);

    TileMap tilemap(bytes_tilemap);
    // numero de jugadores
    int numPlayers = recvBigEndian16();

    // playerInfo
    int size_buffer = recvBigEndian16();
    std::vector<uint8_t> playerInfobytes(size_buffer);
    socket.recvall(playerInfobytes.data(), sizeof(uint8_t) * size_buffer);
    LocalPlayerInfo localPlayerInfo(playerInfobytes);

    // ShopInfo
    int size_buffer_shop = recvBigEndian16();
    std::vector<uint8_t> shopInfobytes(size_buffer_shop);
    socket.recvall(shopInfobytes.data(), sizeof(uint8_t) * size_buffer_shop);
    ShopInfo shopInfo(shopInfobytes);

    return MatchInfo(name, WindowConfig(window_width, window_heigth, window_flags),
                     FOVConfig(isActive, screenW, screenH, circleR, fovAngle, visibilityDistance,
                               transparency),
                     tilemap, numPlayers, localPlayerInfo, shopInfo);
}


std::vector<std::string> ClientProtocol::recvListMatchs() {
    std::string message;
    uint8_t byte = 0;

    int r = socket.recvall(&byte, sizeof(uint8_t));
    if (r == 0) {
        throw std::runtime_error(
                "Error. No se recibió ningún byte. Probablemente se cerró el Server.");
    }
    if (byte != BYTE_MATCH_LIST) {
        throw std::runtime_error(
                "Error. Se esperaba recibir la lista de partidas y llegó otro mensaje.");
    }

    // recibo el length
    uint16_t length = recvBigEndian16();

    // recibo el string
    message.resize(length);
    socket.recvall(message.data(), sizeof(uint8_t) * length);

    // Divido el mensaje por '\n' y retorno como vector
    std::vector<std::string> matchNames;
    std::stringstream ss(message);
    std::string line;

    while (std::getline(ss, line, '\n')) {
        if (!line.empty()) {
            matchNames.push_back(line);
        }
    }

    return matchNames;
}

/**
0xC2 <uint16_t cantidad_jugadores> {
    <uint8_t length_nombre>
    <char[nombre]>
    <uint8_t equipo>
} × N jugadores
<uint8t started> (0 false, 1 true)
 */
MatchRoomInfo ClientProtocol::recvUpdateMatchRoom() {
    std::vector<PlayerInfoLobby> list;

    uint8_t byte = 0;
    socket.recvall(&byte, sizeof(uint8_t));
    if (byte != BYTE_PLAYERS_LIST) {
        throw std::runtime_error("Se recibió un byte inesperado");
    }
    uint16_t quantity_players = recvBigEndian16();

    for (int i = 0; i < quantity_players; i++) {
        uint16_t length = recvBigEndian16();
        std::string name;
        name.resize(length);
        socket.recvall(name.data(), sizeof(uint8_t) * length);  // nombre
        socket.recvall(&byte, sizeof(uint8_t));                 // team
        Team team = decodeTeam(byte);
        socket.recvall(&byte, sizeof(uint8_t));
        bool is_host = decodeBool(byte);
        list.emplace_back(PlayerInfoLobby(name, team, is_host));
    }
    // recibo el booleano
    bool started = false;
    socket.recvall(&byte, sizeof(uint8_t));
    if (byte == BYTE_TRUE)
        started = true;

    return MatchRoomInfo(list, started);
}

GameInfo ClientProtocol::recvGameInfo() {
    uint8_t byte = 0;
    socket.recvall(&byte, sizeof(uint8_t));
    if (byte == BYTE_GAME_INFO) {
        int size = recvBigEndian16();

        std::vector<uint8_t> buffer(size);
        socket.recvall(buffer.data(), sizeof(uint8_t) * size);
        GameInfo gameInfo(buffer);

        return gameInfo;
    } else {
        throw std::runtime_error("Error. Byte incorrecto");
    }
}

std::vector<std::string> ClientProtocol::recvListScenaries() {
    std::string message;
    uint8_t byte = 0;

    int r = socket.recvall(&byte, sizeof(uint8_t));
    if (r == 0) {
        throw std::runtime_error(
                "Error. No se recibió ningún byte. Probablemente se cerró el Server.");
    }
    if (byte != BYTE_SCENARIOS_LIST) {
        throw std::runtime_error(
                "Error. Se esperaba recibir la lista de escenarios y llegó otro mensaje.");
    }

    // recibo el length
    uint16_t length = recvBigEndian16();

    // recibo el string
    message.resize(length);
    socket.recvall(message.data(), sizeof(uint8_t) * length);

    // Divido el mensaje por '\n' y retorno como vector
    std::vector<std::string> scenarioNames;
    std::stringstream ss(message);
    std::string line;

    while (std::getline(ss, line, '\n')) {
        if (!line.empty()) {
            scenarioNames.push_back(line);
        }
    }

    return scenarioNames;
}
