package data;

import javafx.beans.property.IntegerProperty;
import javafx.beans.property.SimpleIntegerProperty;
import javafx.beans.property.SimpleStringProperty;
import javafx.beans.property.StringProperty;

public class Room {

    private IntegerProperty number; // Number of the room
    private StringProperty player1, player2; // Nicknames of players in the room
    private boolean full;

    public Room(int number, String player1, String player2) {
        this.number = new SimpleIntegerProperty(number);
        this.player1 = new SimpleStringProperty(player1);
        this.player2 = new SimpleStringProperty(player2);
        this.full = true;
    }

    public Room(int number, String player1) {
        this.number = new SimpleIntegerProperty(number);
        this.player1 = new SimpleStringProperty(player1);
        this.player2 = new SimpleStringProperty("");
        this.full = false;
    }

    public IntegerProperty numberProperty() {
        return number;
    }

    public final int getNumber() {
        return number.get();
    }

    public StringProperty player1Property() {
        return player1;
    }

    public final String getPlayer1() {
        return player1.get();
    }

    public final void setPlayer1(String player1) {
        this.player1.set(player1);
    }

    public StringProperty player2Property() {
        return player2;
    }

    public final String getPlayer2() {
        return player2.get();
    }

    public final void setPlayer2(String player2) {
        this.player2.set(player2);
    }

    public boolean isFull() {
        return this.full;
    }

    public void setFull(boolean full) {
        this.full = full;
    }

    public void joinPlayer2(String nickname) {
        player2.set(nickname);
        full = true;
    }
}
