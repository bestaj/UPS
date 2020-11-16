package data;

import data.enums.Action;
import data.enums.Response;
import data.enums.State;
import javafx.application.Platform;

import java.util.HashMap;
import java.util.Map;

public class Processing {

    private static final int ERROR = -1;
    private static final int NO_ERROR = 0;
    private Map<String, Process> processFunctions = new HashMap<>();

    interface Process {
        int processMsg(Message msg);
    }

    public Processing() {
        processFunctions.put("LOGIN_REPLY", new Process() { public int processMsg(Message msg) { return processLoginReply(msg);}});
        processFunctions.put("ROOMS_REPLY", new Process() { public int processMsg(Message msg) { return processRoomsReply(msg);}});
        processFunctions.put("FIND_REPLY", new Process() { public int processMsg(Message msg) { return processFindReply(msg);}});
        processFunctions.put("CREATE_REPLY", new Process() { public int processMsg(Message msg) { return processCreateReply(msg);}});
        processFunctions.put("JOIN_REPLY", new Process() { public int processMsg(Message msg) { return processJoinReply(msg);}});
        processFunctions.put("LOGOUT_REPLY", new Process() { public int processMsg(Message msg) { return processLogoutReply(msg);}});
        processFunctions.put("LEAVE_REPLY", new Process() { public int processMsg(Message msg) { return processLeaveReply(msg);}});
        processFunctions.put("TURN_REPLY", new Process() { public int processMsg(Message msg) { return processTurnReply(msg);}});
        processFunctions.put("TAKE_STONE_REPLY", new Process() { public int processMsg(Message msg) { return processTakeStoneReply(msg);}});
        processFunctions.put("OPP_CON", new Process() { public int processMsg(Message msg) { return processOpponentConnected(msg);}});
        processFunctions.put("OPP_TURN", new Process() { public int processMsg(Message msg) { return processOpponentTurned(msg);}});
        processFunctions.put("OPP_TAKE_STONE", new Process() { public int processMsg(Message msg) { return processOpponentTookStone(msg);}});
        processFunctions.put("OPP_LEAVE", new Process() { public int processMsg(Message msg) { return processOpponentLeaved(msg);}});
        processFunctions.put("OPP_LOST_CON", new Process() { public int processMsg(Message msg) { return processOpponentLostConnection(msg);}});
        processFunctions.put("OPP_RECON", new Process() { public int processMsg(Message msg) { return processOpponentReconnection(msg);}});
        processFunctions.put("OPP_DISCON", new Process() { public int processMsg(Message msg) { return processOpponentDisconnection(msg);}});
        processFunctions.put("UPDATE_ROOM", new Process() { public int processMsg(Message msg) { return processUpdateRoom(msg);}});
        processFunctions.put("PING", new Process() { public int processMsg(Message msg) { return processPing(msg);}});
        processFunctions.put("UNKNOWN_MESSAGE", new Process() { public int processMsg(Message msg) { return processUnknownMessage(msg);}});
    }

    public int processMessage(Message msg) {
        return processFunctions.get(msg.msg_name).processMsg(msg);
    }

    private int processLoginReply(Message msg) {
        String[] params2;
        int roomNumber;

        if (msg.state.equals("OK")) { // log in OK - new client
            if (msg.stateID == 0) {
                if (msg.params != null) {
                    return ERROR;
                }
                Mill.getInstance().changeScene(Mill.lobbyScene);
                Mill.getInstance().client.setState(State.LOBBY);
            }
            else if (msg.stateID == 1) { // log in OK - existing client
                if (msg.params == null) {
                    return ERROR;
                }

                if (msg.params.equals("LOBBY")) {
                    Mill.getInstance().client.setState(State.LOBBY);
                    Mill.getInstance().changeScene(Mill.lobbyScene);
                }
                else {
                    String[] params = msg.params.split(";", 2);
                    if (params.length < 2) {
                        return ERROR;
                    }

                    if (params[0].equals("WAITING_FOR_OPP")) {
                        params2 = params[1].split(";", 2);
                        if (params2.length < 2) {
                            return ERROR;
                        }

                        try {
                            roomNumber = Integer.parseInt(params2[0]);
                        }
                        catch (Exception e) {
                            return ERROR;
                        }

                        
                    }
                }

            }
            else { // message state is wrong
                return ERROR;
            }
        }
        else if (msg.state.equals("ERR")) { // log in ERR - something failed
            if (msg.params == null) {
                return ERROR;
            }
            switch (msg.stateID) {
                case 401:
                    if (msg.params.matches("^(nickname .{1,18} already exists)$")) {
                        loginFailed("This nickname already exists.");
                    }
                    else {
                        return ERROR;
                    }
                    break;
                case 402:
                    if (msg.params.matches("^(nickname contains semicolon)$")) {
                        loginFailed("Nickname contains semicolon.");
                    }
                    else {
                        return ERROR;
                    }
                    break;
                case 403:
                    if (msg.params.matches("^(nickname starts/ends with whitespace)$")) {
                        loginFailed("Nickname starts/ends with whitespace.");
                    }
                    else {
                        return ERROR;
                    }
                    break;
                case 404:
                    if (msg.params.matches("^(nickname is too long)$")) {
                        loginFailed("Nickname is too long.\n(max. 18 characters)");
                    }
                    else {
                        return ERROR;
                    }
                    break;
                case 405:
                    if (msg.params.matches("^(client was not found)$")) {
                        loginFailed("Reconnection failed.\nClient was not found.");
                    }
                    else {
                        return ERROR;
                    }
                    break;
                case 406:
                    if (msg.params.matches("^(game is over)$")) {
                        loginFailed("Reconnection failed.\nThe game in which you have been is over.");
                    }
                    else {
                        return ERROR;
                    }
                    break;
                default:
                    return ERROR;
            }
        }
        else {
            return ERROR;
        }
        return NO_ERROR;
    }

    private void loginFailed(String info) {
        Mill.getInstance().client.closeConnection();
        Platform.runLater(() -> {
            Mill.getInstance().loginGUI.setInfoLbl(info);
            Mill.getInstance().loginGUI.setLoginBtn(true);
        });
    }

    private int processRoomsReply(Message msg) {
        if (msg.params == null) { // no params
            System.out.println("No params.");
            return ERROR;
        }
        String[] params = msg.params.split(";",3);
        int roomNumber;

        if (msg.state.equals("OK")) { // rooms OK - free room
            try {
                roomNumber = Integer.parseInt(params[0]);
            }
            catch (Exception e) {
                System.out.println("Room number converting failed.");
                return ERROR;
            }

            if (msg.stateID == 0) {
                if (params.length < 2) {
                    return ERROR;
                }
                Platform.runLater(()-> {
              //      Mill.getInstance().lobbyGUI.setInfoLbl("");
                    Mill.getInstance().lobbyGUI.addRoom(false, roomNumber, params[1], null);
                });
            }
            else if (msg.stateID == 1) { // rooms OK - full room
                if (params.length < 3) {
                    return ERROR;
                }
                Platform.runLater(()-> {
             //       Mill.getInstance().lobbyGUI.setInfoLbl("");
                    Mill.getInstance().lobbyGUI.addRoom(true, roomNumber, params[1], params[2]);
                });
            }
            else {  // message state is wrong
                return ERROR;
            }
        }
        else if (msg.state.equals("ERR")) { // rooms ERR - no rooms
            if (msg.stateID == 401) {
                if (msg.params.matches("^(no rooms)$")) {
                    Platform.runLater(() -> {
                        Mill.getInstance().lobbyGUI.setInfoLbl("List of rooms is empty.");
                    });
                }
                else {
                    System.out.println("Wrong info");
                    return ERROR;
                }

            }
            else {
                System.out.println("Wrong state ID.");
                return ERROR;
            }
        }
        else {
            System.out.println("Wrong state.");
            return ERROR;
        }
        return NO_ERROR;
    }

    public int processFindReply(Message msg) {
        int roomNumber;

        if (msg.params == null) {
            return ERROR;
        }

        if (msg.state.equals("OK")) { // find OK
            if (msg.stateID == 0) {
                String[] params = msg.params.split(";", 3);
                if (params.length < 3) {
                    return ERROR;
                }
                try {
                    roomNumber = Integer.parseInt(params[0]);
                } catch (Exception e) {
                    return ERROR;
                }

                Room room = Mill.getInstance().lobbyGUI.getRoom(roomNumber);
                if (room == null) {
                    return ERROR;
                }

                if (room.isFull()) {
                    return ERROR;
                }

                if (params[2].matches("^(OPP_READY|OPP_LOST_CON)$")) {
                    Mill.getInstance().selectedRoom = room;
                    room.setPlayer2(Mill.getInstance().client.getNickname());
                    Platform.runLater(() -> {
                        Mill.getInstance().lobbyGUI.setSelectedRoomLbl();
                    });

                    Mill.getInstance().game = new Game();
                    Mill.getInstance().changeScene(Mill.gameScene);
                    if (params[2].equals("OPP_LOST_CON")) {
                        Platform.runLater(() -> {
                            Mill.getInstance().gameGUI.setInfoPlayerLostConnection();
                        });
                    }
                    Mill.getInstance().game.startGame(false);
                }
                else {
                    return ERROR;
                }
            }
            else
                return ERROR;
        }
        else if (msg.state.equals("ERR")) {
            if (msg.stateID == 401) {
                if (msg.params.matches("^(no free room)$")) {
                    Platform.runLater(() -> {
                        Mill.getInstance().lobbyGUI.setInfoLbl("There is no free room to join.");
                    });
                }
                else {
                    return ERROR;
                }
            }
            else
                return ERROR;
        }
        else
            return ERROR;

        return NO_ERROR;
    }

    public int processCreateReply(Message msg) {
        int roomNumber;
        if (msg.params == null) {
            return ERROR;
        }

        if (msg.state.equals("OK")) {
            if (msg.stateID == 0) {
                try {
                    roomNumber = Integer.parseInt(msg.params);
                }
                catch (Exception e) {
                    System.out.println("Room number cannot convert to number.");
                    return ERROR;
                }
                Platform.runLater(() -> {
                    Mill.getInstance().lobbyGUI.addRoom(false, roomNumber, Mill.getInstance().client.getNickname(), null);
                    Mill.getInstance().selectedRoom = Mill.getInstance().lobbyGUI.getRoom(roomNumber);
                    Mill.getInstance().game = new Game();
                    Mill.getInstance().changeScene(Mill.gameScene);
                    Mill.getInstance().game.startGame(true);
                });
            }
            else {
                return ERROR;
            }
        }
        else if (msg.state.equals("ERR")) {
            if (msg.stateID == 401) {
                if (msg.params.matches("^(maximum rooms reached)$")) {
                    Platform.runLater(() -> {
                        Mill.getInstance().lobbyGUI.setInfoLbl("The maximum number of rooms was reached.");
                    });
                }
                else {
                    return ERROR;
                }
            }
            else {
                return ERROR;
            }
        }
        else {
            return ERROR;
        }

        return NO_ERROR;
    }

    public int processJoinReply(Message msg) {
        int roomNumber;
        if (msg.params == null) {
            return ERROR;
        }

        if (msg.state.equals("OK")) {
            if (msg.stateID == 0) {
                String[] params = msg.params.split(";", 3);
                if (params.length < 3) {
                    return ERROR;
                }
                try {
                    roomNumber = Integer.parseInt(params[0]);
                }
                catch (Exception e) {
                    return ERROR;
                }

                Room room = Mill.getInstance().lobbyGUI.getRoom(roomNumber);
                if (room == null) {
                    return ERROR;
                }
                if (!room.getPlayer1().equals(params[1])) {
                    return ERROR;
                }

                if (params[2].matches("^(OPP_READY|OPP_LOST_CON)$")) {
                    room.setPlayer2(Mill.getInstance().client.getNickname());
                    Mill.getInstance().selectedRoom = room;
                    Platform.runLater(() -> {
                        Mill.getInstance().lobbyGUI.setSelectedRoomLbl();
                    });

                    Mill.getInstance().game = new Game();
                    Mill.getInstance().changeScene(Mill.gameScene);
                    if (params[2].equals("OPP_LOST_CON")) {
                        Platform.runLater(() -> {
                            Mill.getInstance().gameGUI.setInfoPlayerLostConnection();
                        });
                    }
                    Mill.getInstance().game.startGame(false);
                }
                else {
                    return ERROR;
                }

            }
            else {
                return ERROR;
            }
        }
        else if (msg.state.equals("ERR")) {
            switch (msg.stateID) {
                case 401:
                    if (msg.params.matches("^(invalid room number)$")) {
                        Platform.runLater(() -> {
                            Mill.getInstance().lobbyGUI.setInfoLbl("Room number is not valid.");
                        });
                    }
                    else {
                        return ERROR;
                    }
                    break;
                case 402:
                    if (msg.params.matches("^(room does not exist)$")) {
                        Platform.runLater(() -> {
                            Mill.getInstance().lobbyGUI.setInfoLbl("The room does not exist.");
                        });
                    }
                    else {
                        return ERROR;
                    }
                    break;
                case 403:
                    if (msg.params.matches("^(room \\d+ is full)$")) {
                        Platform.runLater(() -> {
                            Mill.getInstance().lobbyGUI.setInfoLbl("The room is full.");
                        });
                    }
                    else {
                        return ERROR;
                    }
                    break;
                default:
                    return ERROR;
            }
        }
        else {
            return ERROR;
        }

        return NO_ERROR;
    }

    public int processLogoutReply(Message msg) {
        if (msg.params != null) {
            return ERROR;
        }

        if (msg.state.equals("OK")) { // log out OK
            if (msg.stateID == 0) {
                Mill.getInstance().client.closeConnection();
            }
            else {
                return ERROR;
            }
        }
        else {
            return ERROR;
        }
        return NO_ERROR;
    }

    public int processLeaveReply(Message msg) {
        if (msg.params != null) {
            return ERROR;
        }
        if (msg.state.equals("OK")) {
            if (msg.stateID == 0) {
                Platform.runLater(() -> {
                    Mill.getInstance().lobbyGUI.removeRoom(Mill.getInstance().selectedRoom.getNumber());
                    Mill.getInstance().selectedRoom = null;
                });
                Mill.getInstance().game = null;
                Mill.getInstance().client.setState(State.LOBBY);
                Mill.getInstance().changeScene(Mill.lobbyScene);
            }
            else {
                return ERROR;
            }
        }
        else {
            return ERROR;
        }
        return NO_ERROR;
    }

    public int processTurnReply(Message msg) {
        if (msg.params == null) {
            return ERROR;
        }

        if (msg.state.equals("OK")) {
            if (msg.stateID == 0) {
                if (msg.params.matches("^(MILL|MILL_NO_TAKE|NOT_MILL)$")) {
                    if (Mill.getInstance().game.action == Action.SET) {
                        Mill.getInstance().game.setStone(true, -1);
                    }
                    else if (Mill.getInstance().game.action == Action.SHIFT) {
                        Mill.getInstance().game.shiftStone(true, -1, -1);
                    }
                    else {
                        return ERROR;
                    }
                    if (msg.params.equals("MILL")) {
                        Platform.runLater(() -> {
                            Mill.getInstance().gameGUI.setInfoText("You made a mill.\n");
                        });
                        Mill.getInstance().client.setState(State.TAKING_STONE);
                        Mill.getInstance().game.takingStone = true;
                        Mill.getInstance().game.play = true;
                    }
                    else {
                        Mill.getInstance().client.setState(State.OPP_TURN);
                        Platform.runLater(() -> {
                            Mill.getInstance().gameGUI.setVisibleOpponentPlayingLbl();
                        });
                        if (msg.params.equals("MILL_NO_TAKE")) {
                            Platform.runLater(() -> {
                                Mill.getInstance().gameGUI.setInfoText("You made a mill, but you cannot take any stone.\n");
                            });
                        }
                    }
                }
                else {
                    return ERROR;
                }
            }
            else {
                return ERROR;
            }
        }
        else if (msg.state.equals("ERR")) {
            Mill.getInstance().game.play = true;
            switch (msg.stateID) {
                case 401:
                    if (msg.params.matches("^(no stones to set)$")) {
                        Platform.runLater(() -> {
                            Mill.getInstance().gameGUI.setInfoText("You have already set all your stones.");
                        });
                    }
                    else {
                        return ERROR;
                    }
                    break;
                case 402:
                    if (msg.params.matches("^(cannot shift stone yet)$")) {
                        Platform.runLater(() -> {
                            Mill.getInstance().gameGUI.setInfoText("First you need to set all your stones before shifting.");
                        });
                    }
                    else {
                        return ERROR;
                    }
                    break;
                case 403:
                    if (msg.params.matches("^(invalid position [12])$")) {
                        Platform.runLater(() -> {
                            Mill.getInstance().gameGUI.setInfoText("A wrong position to set a stone.");
                        });
                    }
                    else {
                        return ERROR;
                    }
                    break;
                case 404:
                    if (msg.params.matches("^(position \\d+ is full)$")) {
                        Platform.runLater(() -> {
                            Mill.getInstance().gameGUI.setInfoText("Cannot set the stone. Position is full.");
                        });
                    }
                    else {
                        return ERROR;
                    }
                    break;
                case 405:
                    if (msg.params.matches("^(missing stone on position 1)$")) {
                        Platform.runLater(() -> {
                            Mill.getInstance().gameGUI.setInfoText("Cannot shift the stone. Missing selected stone.");
                        });
                    }
                    else {
                        return ERROR;
                    }
                    break;
                case 406:
                    if (msg.params.matches("^(cannot shift stone to position 2)$")) {
                        Platform.runLater(() -> {
                            Mill.getInstance().gameGUI.setInfoText("Cannot shift the stone to the position.");
                        });
                    }
                    else {
                        return ERROR;
                    }
                    break;
                default:
                    return ERROR;
            }
        }
        else {
            return ERROR;
        }
        return NO_ERROR;
    }

    public int processTakeStoneReply(Message msg) {
        if (msg.params == null) {
            return ERROR;
        }

        if (msg.state.equals("OK")) {
            if (msg.stateID == 0) {
                if (msg.params.matches("^(WIN|CONTINUE)$")) {
                    Mill.getInstance().game.takeStone(false, -1);
                    if (msg.params.equals("WIN")) {
                        Mill.getInstance().client.setState(State.LOBBY);
                        Platform.runLater(() -> {
                            Mill.getInstance().gameGUI.gameOver(true);
                        });
                    }
                    else {
                        Mill.getInstance().client.setState(State.OPP_TURN);

                        Platform.runLater(() -> {
                            Mill.getInstance().gameGUI.setVisibleOpponentPlayingLbl();
                        });
                    }
                }
                else {
                    return ERROR;
                }
            }
            else {
                return ERROR;
            }
        }
        else if (msg.state.equals("ERR")) {
            Mill.getInstance().game.play = true;
            switch (msg.stateID) {
                case 401:
                    if (msg.params.matches("^(invalid position)$")) {
                        Platform.runLater(() -> {
                            Mill.getInstance().gameGUI.setInfoText("A wrong position to take a stone.");
                        });
                    }
                    else {
                        return ERROR;
                    }
                    break;
                case 402:
                    if (msg.params.matches("^(missing opponents stone)$")) {
                        Platform.runLater(() -> {
                            Mill.getInstance().gameGUI.setInfoText("On the position is not opponent's stone.");
                        });
                    }
                    else {
                        return ERROR;
                    }
                    break;
                case 403:
                    if (msg.params.matches("^(taking stone is in mill)$")) {
                        Platform.runLater(() -> {
                            Mill.getInstance().gameGUI.setInfoText("Cannot take a stone inside of a mill.");
                        });
                    }
                    else {
                        return ERROR;
                    }
                    break;
                default:
                    return ERROR;
            }
        }
        else {
            return ERROR;
        }

        return NO_ERROR;
    }

    public int processOpponentConnected(Message msg) {
        if (msg.params == null) {
            return ERROR;
        }

        if (msg.state.equals("INFO")) {
            if (msg.stateID == 100) {
                if (msg.params.length() > 18) {
                    return ERROR;
                }
                Mill.getInstance().client.setState(State.MY_TURN);
                Mill.getInstance().game.setYourOpponent(msg.params);
                Mill.getInstance().client.sendMsg(Message.getMessage(Response.OPP_CON));
            }
            else {
                return ERROR;
            }
        }
        else {
            return ERROR;
        }
        return NO_ERROR;
    }

    public int processOpponentTurned(Message msg) {
        String[] params;
        String[] params2;
        int posID, pos1ID, pos2ID;
        if (msg.params == null) {
            return ERROR;
        }

        if (msg.state.equals("INFO")) {
            if (msg.stateID == 101) {
                params = msg.params.split(";", 2);
                if (params.length < 2) {
                    System.out.println("Missing parameters 1");
                    return ERROR;
                }

                if (params[0].equals("SET")) {
                    params2 = params[1].split(";", 2);
                    if (params2.length < 2) {
                        System.out.println("Missing parameters 2");
                        return ERROR;
                    }
                    if (params2[1].matches("^(MILL|MILL_NO_TAKE|NOT_MILL)$")) {
                        try {
                            posID = Integer.parseInt(params2[0]);
                            if ((posID < 0) || (posID > 23)) {
                                System.out.println("Invalid number");
                                return ERROR;
                            }
                        }
                        catch (Exception e) {
                            return ERROR;
                        }
                        Mill.getInstance().game.setStone(false, posID);

                        if (params2[1].equals("MILL")) {
                            opponentTurned(0);
                        }
                        else if (params2[1].equals("MILL_NO_TAKE")){
                            opponentTurned(1);
                        }
                        else {
                            opponentTurned(2);
                        }
                    }
                    else {
                        return ERROR;
                    }
                }
                else if (params[0].equals("SHIFT")) {
                    params2 = params[1].split(";", 3);
                    if (params2.length < 3) {
                        System.out.println("Missing parameters 3");
                        return ERROR;
                    }
                    if (params2[2].matches("^(MILL|MILL_NO_TAKE|NOT_MILL)$")) {
                        try {
                            pos1ID = Integer.parseInt(params2[0]);
                            pos2ID = Integer.parseInt(params2[1]);
                        }
                        catch (Exception e) {
                            System.out.println("Cannot convert number");
                            return ERROR;
                        }
                        if ((pos1ID < 0) || (pos1ID > 23)) {
                            return ERROR;
                        }
                        if ((pos2ID < 0) || (pos2ID > 23)) {
                            return ERROR;
                        }
                        Mill.getInstance().game.shiftStone(false, pos1ID, pos2ID);

                        if (params2[2].equals("MILL")) {
                            opponentTurned(0);
                        }
                        else if (params2[2].equals("MILL_NO_TAKE")){
                            opponentTurned(1);
                        }
                        else {
                            opponentTurned(2);
                        }
                    }
                    else {
                        System.out.println("Wrong last param");
                        return ERROR;
                    }
                }
                else {
                    System.out.println("Wrong action");
                    return ERROR;
                }
            }
            else {
                System.out.println("Wrong state id");
                return ERROR;
            }
        }
        else {
            System.out.println("Wrong state");
            return ERROR;
        }

        return NO_ERROR;
    }

    private void opponentTurned(int action) {
        if (action == 0) { // Made a mill and can take a stone
            Mill.getInstance().client.setState(State.OPP_TAKING_STONE);
            Mill.getInstance().client.sendMsg(Message.getMessage(Response.OPP_TURN_MILL));
            Platform.runLater(() -> {
                Mill.getInstance().gameGUI.setInfoText("Your opponent made a mill.\n");
            });
        }
        else {
            Mill.getInstance().client.setState(State.MY_TURN);
            Mill.getInstance().game.play = true;
            Platform.runLater(() -> {
                Mill.getInstance().gameGUI.setVisibleMyPlayingLbl();
            });
            if (action == 1) { // Made a mill and cannot take a stone
                Mill.getInstance().client.sendMsg(Message.getMessage(Response.OPP_TURN_MILL_NO_TAKE));
                Platform.runLater(() -> {
                    Mill.getInstance().gameGUI.setInfoText("Your opponent made a mill, but he cannot take any stone.\n");
                });
            }
            else { // No mill
                Mill.getInstance().client.sendMsg(Message.getMessage(Response.OPP_TURN_NOT_MILL));
            }
        }
    }

    public int processOpponentTookStone(Message msg) {
        int posID;
        if (msg.params == null) {
            return ERROR;
        }

        if (msg.state.equals("INFO")) {
            if (msg.stateID == 102) {
                String[] params = msg.params.split(";", 2);
                if (params.length < 2) {
                    return ERROR;
                }

                try {
                    posID = Integer.parseInt(params[0]);
                }
                catch (Exception e) {
                    return ERROR;
                }
                if (posID < 0 || posID > 23) {
                    return ERROR;
                }

                if (params[1].matches("^(LOOSE|CONTINUE)$")) {
                    Mill.getInstance().game.takeStone(true, posID);

                    if (params[1].equals("LOOSE")) {
                        System.out.println("Loose");
                        Mill.getInstance().client.setState(State.LOBBY);
                        Mill.getInstance().client.sendMsg(Message.getMessage(Response.OPP_TAKE_STONE_LOOSE));
                        Platform.runLater(() -> {
                            Mill.getInstance().gameGUI.gameOver(false);
                        });
                    }
                    else {
                        Mill.getInstance().client.setState(State.MY_TURN);
                        Mill.getInstance().game.play = true;
                        Mill.getInstance().client.sendMsg(Message.getMessage(Response.OPP_TAKE_STONE_CONT));
                        Platform.runLater(() -> {
                            Mill.getInstance().gameGUI.setVisibleMyPlayingLbl();
                        });
                    }
                }
                else {
                    return ERROR;
                }
            }
            else {
                return ERROR;
            }
        }
        else {
            return ERROR;
        }

        return NO_ERROR;
    }

    public int processOpponentLeaved(Message msg) {
        if (msg.params != null) {
            return ERROR;
        }

        if (msg.state.equals("INFO")) {
            if (msg.stateID == 103) {
                Mill.getInstance().game.play = false;

                Platform.runLater(() -> {
                    Mill.getInstance().gameGUI.opponentLeaved();
                    Mill.getInstance().lobbyGUI.removeRoom(Mill.getInstance().selectedRoom.getNumber());
                    Mill.getInstance().selectedRoom = null;
                    Mill.getInstance().game = null;
                });
                Mill.getInstance().client.setState(State.LOBBY);
                Mill.getInstance().client.sendMsg(Message.getMessage(Response.OPP_LEAVE));

            }
            else {
                return ERROR;
            }
        }
        else {
            return ERROR;
        }
        return NO_ERROR;
    }

    public int processOpponentLostConnection(Message msg) {
        if (msg.params != null) {
            return ERROR;
        }

        if (msg.state.equals("INFO")) {
            if (msg.stateID == 104) {
                Mill.getInstance().game.returnedState = Mill.getInstance().client.getState();
                Mill.getInstance().client.setState(State.OPP_LOST_CON);
                Platform.runLater(() -> {
                   Mill.getInstance().gameGUI.opponentLostConnection();
                });
                Mill.getInstance().client.sendMsg(Message.getMessage(Response.OPP_LOST_CON));
            }
            else {
                return ERROR;
            }
        }
        else {
            return ERROR;
        }
        return NO_ERROR;
    }

    public int processOpponentReconnection(Message msg) {
        if (msg.params != null) {
            return ERROR;
        }

        if (msg.state.equals("INFO")) {
            if (msg.stateID == 105) {
                Platform.runLater(() -> {
                    Mill.getInstance().gameGUI.opponentReconnect();
                });
                Mill.getInstance().client.sendMsg(Message.getMessage(Response.OPP_RECON));
            }
            else {
                return ERROR;
            }
        }
        else {
            return ERROR;
        }
        return NO_ERROR;
    }

    public int processOpponentDisconnection(Message msg) {
        if (msg.params != null) {
            return ERROR;
        }

        if (msg.state.equals("INFO")) {
            if (msg.stateID == 106) {

                Platform.runLater(() -> {
                    Mill.getInstance().gameGUI.opponentDisconnected();
                    Mill.getInstance().lobbyGUI.removeRoom(Mill.getInstance().selectedRoom.getNumber());
                    Mill.getInstance().selectedRoom = null;
                    Mill.getInstance().game = null;
                });
                Mill.getInstance().client.setState(State.LOBBY);
                Mill.getInstance().client.sendMsg(Message.getMessage(Response.OPP_DISCON));

            }
            else {
                return ERROR;
            }
        }
        else {
            return ERROR;
        }
        return NO_ERROR;
    }

    public int processUpdateRoom(Message msg) {
        int roomNumber;
        if (msg.params == null) {
            return ERROR;
        }
        if (msg.state.equals("INFO")) {
            if (msg.stateID == 107) {
                String[] params = msg.params.split(";", 3);
                if (params.length < 2) {
                    return ERROR;
                }

                try {
                    roomNumber = Integer.parseInt(params[0]);
                }
                catch (Exception e) {
                    return ERROR;
                }

                if (params[1].equals("CLR")) {
                    if (params.length > 2) {
                        return ERROR;
                    }
                    Platform.runLater(() -> {
                        Mill.getInstance().lobbyGUI.removeRoom(roomNumber);
                    });
                }
                else {
                    if (params.length < 3) {
                        return ERROR;
                    }
                    if (params[1].equals("ADD")) {
                        Platform.runLater(() -> {
                            Mill.getInstance().lobbyGUI.addRoom(false, roomNumber, params[2], null);
                        });
                    }
                    else if (params[1].equals("UPDATE")) {
                        Platform.runLater(() -> {
                            Mill.getInstance().lobbyGUI.addPlayerIntoRoom(roomNumber, params[2]);
                        });
                    }
                    else {
                        return ERROR;
                    }
                }
            }
            else {
                return ERROR;
            }
        }
        else {
            return ERROR;
        }
        Mill.getInstance().client.sendMsg(String.format(Message.getMessage(Response.UPDATE_ROOM), roomNumber));

        return NO_ERROR;
    }

    public int processPing(Message msg) {

        return NO_ERROR;
    }

    public int processUnknownMessage(Message msg) {
        if (msg.state.equals("ERR")) {
            if (msg.stateID == 400) {
                Mill.getInstance().client.closeConnection();
                Platform.runLater(() -> {
                    Mill.getInstance().lobbyGUI.setInfoLbl("Communication with the server failed.");
                });
            }
            else {
                return ERROR;
            }
        }
        else {
            return ERROR;
        }
        return NO_ERROR;
    }
}
