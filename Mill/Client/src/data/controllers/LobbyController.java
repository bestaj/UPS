package data.controllers;

import data.Message;
import data.Mill;
import data.Room;
import data.enums.Response;
import javafx.beans.Observable;
import javafx.collections.FXCollections;
import javafx.collections.ObservableList;
import javafx.fxml.FXML;
import javafx.fxml.Initializable;
import javafx.scene.control.Button;
import javafx.scene.control.Label;
import javafx.scene.control.TableColumn;
import javafx.scene.control.TableView;
import javafx.scene.control.cell.PropertyValueFactory;

import java.net.URL;
import java.util.ResourceBundle;

public class LobbyController implements Initializable {

    @FXML
    private Button joinRoomBtn;
    @FXML
    private Label selectedRoomLbl, infoLbl;
    @FXML
    private TableView<Room> table;
    @FXML
    private TableColumn<Room, Integer> numberCol;
    @FXML
    private TableColumn<Room, String> player1Col, player2Col;

    private ObservableList<Room> rooms;

    @Override
    public void initialize(URL location, ResourceBundle resources) {
        Mill.getInstance().lobbyGUI = this;
        Mill.getInstance().selectedRoom = null;
        selectedRoomLbl.setText("No selected room.");
        disableJoinButton(true);
        rooms = FXCollections.observableArrayList();

        numberCol.setCellValueFactory(new PropertyValueFactory<Room, Integer>("number"));
        player1Col.setCellValueFactory(new PropertyValueFactory<Room, String>("player1"));
        player2Col.setCellValueFactory(new PropertyValueFactory<Room, String>("player2"));

        table.getSelectionModel().selectedItemProperty().addListener((observable, oldSelection, newSelection) -> {
            if (newSelection != null) {
                Mill.getInstance().selectedRoom = table.getSelectionModel().getSelectedItem();
                setSelectedRoomLbl();
                disableJoinButton(false);
            }
        });

        Mill.getInstance().client.sendMsg(Message.getMessage(Response.ROOMS));
        table.setItems(rooms);
    }

    public void addRoom(boolean isFull, int number, String player1, String player2) {
        Room newRoom;
        if (isFull) {
            newRoom = new Room(number, player1, player2);
        }
        else {
            newRoom = new Room(number, player1);
        }
        rooms.add(newRoom);
        table.refresh();
    }

    public void removeRoom(int roomNumber) {
        for (Room r: rooms) {
            if (r.getNumber() == roomNumber) {
                if (Mill.getInstance().selectedRoom != null && (Mill.getInstance().selectedRoom.getNumber() == roomNumber)) {
                    Mill.getInstance().selectedRoom = null;
                    selectedRoomLbl.setText("No selected room.");
                    disableJoinButton(true);
                }
                rooms.remove(r);
                table.refresh();
                break;
            }
        }

    }

    public void addPlayerIntoRoom(int roomNumber, String player2) {
        for (Room r: rooms) {
            if (r.getNumber() == roomNumber) {
                r.setPlayer2(player2);
                table.refresh();
                break;
            }
        }
    }

    public Room getRoom(int roomNumber) {
        for (Room r: rooms) {
            if (r.getNumber() == roomNumber) {
                return r;
            }
        }
        return null;
    }

    @FXML
    private void findEmptyRoom() {
        Mill.getInstance().client.sendMsg(Message.getMessage(Response.FIND));
        infoLbl.setText("Finding room...");
    }

    @FXML
    private void createRoom() {
        Mill.getInstance().client.sendMsg(Message.getMessage(Response.CREATE));
        infoLbl.setText("Creating room...");
    }

    @FXML
    private void joinRoom() {

        if (Mill.getInstance().selectedRoom == null) { // No room is selected
            infoLbl.setText("You need to select a room to join.");
            return;
        }

        if (Mill.getInstance().selectedRoom.isFull()) {
            infoLbl.setText("The room " + Mill.getInstance().selectedRoom.getNumber() + " is full.");
            return;
        }

        Mill.getInstance().client.sendMsg(String.format(Message.getMessage(Response.JOIN), Mill.getInstance().selectedRoom.getNumber()));
        infoLbl.setText("Joining into the room " + Mill.getInstance().selectedRoom.getNumber() + "...");
    }

    /** Disconnect the client from the server */
    @FXML
    private void disconnect() {
        Mill.getInstance().client.sendMsg(Message.getMessage(Response.LOGOUT));
    }

    private void disableJoinButton(boolean disable) {
        if (disable) {
            joinRoomBtn.setDisable(true);
            joinRoomBtn.setOpacity(0.5);
        }
        else {
            joinRoomBtn.setDisable(false);
            joinRoomBtn.setOpacity(1);
        }
    }

    public void setInfoLbl(String text) {
        infoLbl.setText(text);
    }

    public void setSelectedRoomLbl() {
        selectedRoomLbl.setText("Selected room: " + Mill.getInstance().selectedRoom.getNumber());
    }
}
