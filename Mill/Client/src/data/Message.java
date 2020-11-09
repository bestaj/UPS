package data;

import data.enums.*;

import java.util.HashMap;
import java.util.Map;

public class Message {

    public static final Map<State, String> ALLOWED_MESSAGES = new HashMap<>();
    /** Table with all messages sending to the server */
    public static final Map<Response, String> RESPONSES = new HashMap<>();

    static {
        RESPONSES.put(Response.LOGIN_NEW, "0;LOGIN;NEW;%s\n");
        RESPONSES.put(Response.LOGIN_EXIST, "0;LOGIN;EXIST;%s\n");
        RESPONSES.put(Response.ROOMS, "1;ROOMS\n");
        RESPONSES.put(Response.FIND, "2;FIND\n");
        RESPONSES.put(Response.CREATE, "3;CREATE\n");
        RESPONSES.put(Response.JOIN, "4;JOIN;%d\n");
        RESPONSES.put(Response.LOGOUT, "5;LOGOUT\n");
        RESPONSES.put(Response.LEAVE, "6;LEAVE\n");
        RESPONSES.put(Response.SET_STONE, "7;TURN;SET;%d\n");
        RESPONSES.put(Response.SHIFT_STONE, "7;TURN;SHIFT;%d;%d\n");
        RESPONSES.put(Response.TAKE_STONE, "8;TAKE_STONE;%d\n");
        RESPONSES.put(Response.OPP_CON, "9;OPP_CON_OK\n");
        RESPONSES.put(Response.OPP_TURN_MILL, "10;OPP_TURN_OK;MILL\n");
        RESPONSES.put(Response.OPP_TURN_MILL_NO_TAKE, "10;OPP_TURN_OK;MILL_NO_TAKE\n");
        RESPONSES.put(Response.OPP_TURN_NOT_MILL, "10;OPP_TURN_OK;NOT_MILL\n");
        RESPONSES.put(Response.OPP_TAKE_STONE_LOOSE, "11;OPP_TAKE_STONE_OK;LOOSE\n");
        RESPONSES.put(Response.OPP_TAKE_STONE_CONT, "11;OPP_TAKE_STONE_OK;CONTINUE\n");
        RESPONSES.put(Response.OPP_LEAVE, "12;OPP_LEAVE_OK\n");
        RESPONSES.put(Response.OPP_LOST_CON, "13;OPP_LOST_CON_OK\n");
        RESPONSES.put(Response.OPP_RECON, "14;OPP_RECON_OK\n");
        RESPONSES.put(Response.OPP_DISCON, "15;OPP_DISCON_OK\n");
        RESPONSES.put(Response.UPDATE_ROOM, "16;UPDATE_ROOM_OK;%d\n");
        RESPONSES.put(Response.PING, "17;PING_OK\n");

        ALLOWED_MESSAGES.put(State.CONNECTED, "^(LOGIN_REPLY|PING|UNKNOWN_MESSAGE)$");
        ALLOWED_MESSAGES.put(State.LOBBY, "^(LOGOUT_REPLY|ROOMS_REPLY|CREATE_REPLY|FIND_REPLY|JOIN_REPLY|UPDATE_ROOM|PING|UNKNOWN_MESSAGE)$");
        ALLOWED_MESSAGES.put(State.WAITING_FOR_OPP, "^(OPP_CON|LEAVE_REPLY|PING|UNKNOWN_MESSAGE)$");
        ALLOWED_MESSAGES.put(State.MY_TURN, "^(TURN_REPLY|LEAVE_REPLY|OPP_LEAVE|OPP_LOST_CON|PING|UNKNOWN_MESSAGE)$");
        ALLOWED_MESSAGES.put(State.OPP_TURN, "^(OPP_TURN|OPP_LEAVE|OPP_LOST_CON|LEAVE_REPLY|PING|UNKNOWN_MESSAGE)$");
        ALLOWED_MESSAGES.put(State.TAKING_STONE, "^(TAKE_STONE_REPLY|LEAVE_REPLY|OPP_LEAVE|OPP_LOST_CON|PING|UNKNOWN_MESSAGE)$");
        ALLOWED_MESSAGES.put(State.OPP_TAKING_STONE, "^(OPP_TAKE_STONE|OPP_LEAVE|OPP_LOST_CON|LEAVE_REPLY|PING|UNKNOWN_MESSAGE)$");
        ALLOWED_MESSAGES.put(State.OPP_LOST_CON, "^(OPP_RECON|OPP_DISCON|LEAVE_REPLY|PING|UNKNOWN_MESSAGE)$");
    }
    public String msg_name;
    public String state;
    public String params;
    public int stateID;

    public Message(String msg_name, String state, int stateID, String params) {
        this.msg_name = msg_name;
        this.state = state;
        this.stateID = stateID;
        this.params = params;
    }

    public static Message parseMessage(String data, State clientState) {
        int stateID;
        Message msg;
        String[] parts = data.split(";", 4);
        if (parts.length < 3) {
            System.out.println("Too few parameters in the message.");
            return null;
        }

        if (!parts[0].matches(ALLOWED_MESSAGES.get(clientState))) {
            System.out.println("Unknown message or invalid message in the current state.");
            return null;
        }
        if (!parts[1].matches("^(OK|ERR|INFO)$")) {
            System.out.println("Unknown message state.");
            return null;
        }

        try {
            stateID = Integer.parseInt(parts[2]);
        }
        catch (Exception e) {
            System.out.println("Message state ID cannot convert to number.");
            return null;
        }

        if (parts.length == 4) {
            msg = new Message(parts[0], parts[1], stateID, parts[3]);
        }
        else {
            msg = new Message(parts[0], parts[1], stateID, null);
        }
        
        return msg;
    }

    public static String getMessage(Response resp) {
        return RESPONSES.get(resp);
    }
}
