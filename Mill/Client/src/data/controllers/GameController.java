package data.controllers;

import data.*;
import data.enums.*;
import javafx.fxml.FXML;
import javafx.fxml.Initializable;
import javafx.scene.canvas.Canvas;
import javafx.scene.canvas.GraphicsContext;
import javafx.scene.control.*;
import javafx.scene.image.ImageView;
import javafx.scene.input.MouseEvent;
import javafx.scene.paint.Color;

import java.net.URL;
import java.util.Optional;
import java.util.ResourceBundle;

import static java.lang.Thread.sleep;

public class GameController implements Initializable {

    @FXML
    private Button leaveGameBtn;
    @FXML
    private TextArea infoTA;
    @FXML
    private Label nickname1Lbl, nickname2Lbl, roomLbl, stateLblP1, stateLblP2;
    @FXML
    private Canvas canvas;

    @FXML   // Images of red stones
    private ImageView img_red_1, img_red_2, img_red_3, img_red_4, img_red_5, img_red_6, img_red_7, img_red_8, img_red_9;
    @FXML   // Images of blue stones
    private ImageView img_blue_1, img_blue_2, img_blue_3, img_blue_4, img_blue_5, img_blue_6, img_blue_7, img_blue_8, img_blue_9;

    private final int STONE_SIZE = 20;
    private final int STONE_RADIUS = 10;
    private final int POS_SIZE = 10;
    private final int POS_RADIUS = 5;

    private final Color red = Color.rgb(240, 0, 0);
    private final Color blue = Color.rgb(0, 90, 230);
    private final Color green = Color.rgb(0, 255, 0);
    private final Color bg = Color.rgb(230, 230, 230);

    // Array of the red stones
    private ImageView[] redStones = new ImageView[] {img_red_1, img_red_2, img_red_3, img_red_4, img_red_5, img_red_6, img_red_7, img_red_8, img_red_9};
    // Array of the blue stones
    private ImageView[] blueStones = new ImageView[] {img_blue_1, img_blue_2, img_blue_3, img_blue_4, img_blue_5, img_blue_6, img_blue_7, img_blue_8, img_blue_9};

    private GraphicsContext gc;
    private Game game;
    private Room room;

    /** Initialize an environment of the game */
    @Override
    public void initialize(URL location, ResourceBundle resources) {
        Mill.getInstance().gameGUI = this;
        redStones = new ImageView[] {img_red_1, img_red_2, img_red_3, img_red_4, img_red_5, img_red_6, img_red_7, img_red_8, img_red_9};
        blueStones = new ImageView[] {img_blue_1, img_blue_2, img_blue_3, img_blue_4, img_blue_5, img_blue_6, img_blue_7, img_blue_8, img_blue_9};
        this.game = Mill.getInstance().game;
        this.room = Mill.getInstance().selectedRoom;
        roomLbl.setText("Room " + room.getNumber());
        nickname1Lbl.setText(room.getPlayer1());
        nickname2Lbl.setText(room.getPlayer2());
        gc = canvas.getGraphicsContext2D();

        updateCanvas();
    }

    /** Redraw playing field */
    public void updateCanvas() {

        // Set grey background
        gc.setFill(bg);
        gc.fillRect(0,0,canvas.getWidth(), canvas.getHeight());

        // Draw all lines
        gc.setStroke(Color.BLACK);
        gc.setLineWidth(3);

        // Large square
        gc.strokeLine(game.getCircle(0).getCenterX(), game.getCircle(0).getCenterY(), game.getCircle(2).getCenterX(), game.getCircle(2).getCenterY());
        gc.strokeLine(game.getCircle(2).getCenterX(), game.getCircle(2).getCenterY(), game.getCircle(4).getCenterX(), game.getCircle(4).getCenterY());
        gc.strokeLine(game.getCircle(4).getCenterX(), game.getCircle(4).getCenterY(), game.getCircle(6).getCenterX(), game.getCircle(6).getCenterY());
        gc.strokeLine(game.getCircle(6).getCenterX(), game.getCircle(6).getCenterY(), game.getCircle(0).getCenterX(), game.getCircle(0).getCenterY());
        // Medium square
        gc.strokeLine(game.getCircle(8).getCenterX(), game.getCircle(8).getCenterY(), game.getCircle(10).getCenterX(), game.getCircle(10).getCenterY());
        gc.strokeLine(game.getCircle(10).getCenterX(), game.getCircle(10).getCenterY(), game.getCircle(12).getCenterX(), game.getCircle(12).getCenterY());
        gc.strokeLine(game.getCircle(12).getCenterX(), game.getCircle(12).getCenterY(), game.getCircle(14).getCenterX(), game.getCircle(14).getCenterY());
        gc.strokeLine(game.getCircle(14).getCenterX(), game.getCircle(14).getCenterY(), game.getCircle(8).getCenterX(), game.getCircle(8).getCenterY());
        // Small square
        gc.strokeLine(game.getCircle(16).getCenterX(), game.getCircle(16).getCenterY(), game.getCircle(18).getCenterX(), game.getCircle(18).getCenterY());
        gc.strokeLine(game.getCircle(18).getCenterX(), game.getCircle(18).getCenterY(), game.getCircle(20).getCenterX(), game.getCircle(20).getCenterY());
        gc.strokeLine(game.getCircle(20).getCenterX(), game.getCircle(20).getCenterY(), game.getCircle(22).getCenterX(), game.getCircle(22).getCenterY());
        gc.strokeLine(game.getCircle(22).getCenterX(), game.getCircle(22).getCenterY(), game.getCircle(16).getCenterX(), game.getCircle(16).getCenterY());
        // Interconnection of squares
        gc.strokeLine(game.getCircle(1).getCenterX(), game.getCircle(1).getCenterY(), game.getCircle(17).getCenterX(), game.getCircle(17).getCenterY());
        gc.strokeLine(game.getCircle(3).getCenterX(), game.getCircle(3).getCenterY(), game.getCircle(19).getCenterX(), game.getCircle(19).getCenterY());
        gc.strokeLine(game.getCircle(5).getCenterX(), game.getCircle(5).getCenterY(), game.getCircle(21).getCenterX(), game.getCircle(21).getCenterY());
        gc.strokeLine(game.getCircle(7).getCenterX(), game.getCircle(7).getCenterY(), game.getCircle(23).getCenterX(), game.getCircle(23).getCenterY());

        for (int i = 0; i < 24; i++) {
            drawPosition(game.getGamePosition(i));
        }
    }

    /** Draw circle on the gaming position */
    private void drawPosition(GamePosition pos) {
        // Free position = black circle
        if (pos.isFree()) {
            fillCircle(pos.getLeftUpperCornerX(), pos.getLeftUpperCornerY(), POS_SIZE, Color.BLACK);
        }
        else {
            // Position with blue stone = blue circle
            if (pos.getStone().equals(Stone.BLUE)) {
                fillCircle(pos.getLeftUpperCornerX(), pos.getLeftUpperCornerY(), STONE_SIZE, blue);
            }
            // Position with red stone = red circle
            else {
                fillCircle(pos.getLeftUpperCornerX(), pos.getLeftUpperCornerY(), STONE_SIZE, red);
            }
        }
    }

    /** Draw particular circle */
    private void fillCircle(double x, double y, double size, Color color) {
        gc.setFill(color);
        gc.fillOval(x, y, size, size);
    }

    private void strokeCircle(double x, double y, double size, Color color) {
        gc.setStroke(color);
        gc.setLineWidth(2);
        gc.strokeOval(x, y, size, size);
    }

    @FXML
    private void mouseMoved(MouseEvent event) {
        if (!game.play) return;
        game.processMouseMove(event);
    }

    /** Process mouse click when it is my turn */
    @FXML
    private void mouseClicked(MouseEvent event) {
        if (!game.play) return;  // Opponent's turn
        game.processMouseClick(event);
    }

    @FXML
    private void leaveGame() {
        Alert alert = new Alert(Alert.AlertType.CONFIRMATION);
        alert.setTitle("LEAVING THE GAME");
        alert.setHeaderText("Are you sure you want to leave this room?");
        alert.initOwner(Mill.getInstance().window);
        ButtonType btn1 = new ButtonType("Yes");
        ButtonType btn2 = new ButtonType("No");

        alert.getButtonTypes().setAll(btn1, btn2);

        Optional<ButtonType> result = alert.showAndWait();

        if (result.get() == btn1){
            Mill.getInstance().client.sendMsg(Message.getMessage(Response.LEAVE));
        }
    }

    public void gameOver(boolean winner) {
        if (winner) {
            Alert alert = new Alert(Alert.AlertType.INFORMATION);
            alert.setTitle("VICTORY");
            alert.setHeaderText("Congratulations, you are winner.");
            alert.initOwner(Mill.getInstance().window);
            alert.showAndWait();
            Mill.getInstance().changeScene(Mill.lobbyScene);
        }
        else {
            Alert alert = new Alert(Alert.AlertType.INFORMATION);
            alert.setTitle("LOOSE");
            alert.setHeaderText("Sorry, your opponent was better.");
            alert.setContentText("Better thinking next time.");
            alert.initOwner(Mill.getInstance().window);
            alert.showAndWait();
            Mill.getInstance().changeScene(Mill.lobbyScene);
        }
    }

    public void repaintVisitedPositions() {
        GamePosition oldPos = game.oldVisitedPosition;
        GamePosition newPos = game.newVisitedPosition;
        // Clear the previous mark
        if (oldPos != null) {
            if (oldPos.isFree()) {
                fillCircle(oldPos.getLeftUpperCornerX(), oldPos.getLeftUpperCornerY(), POS_SIZE, Color.BLACK);
            }
            else {
                if (oldPos.getStone().equals(Stone.RED)) {
                    fillCircle(oldPos.getLeftUpperCornerX(), oldPos.getLeftUpperCornerY(), STONE_SIZE, red);
                }
                else {
                    fillCircle(oldPos.getLeftUpperCornerX(), oldPos.getLeftUpperCornerY(), STONE_SIZE, blue);
                }
            }
        }
        // Mark a new position
        if (newPos.isFree()) {
            strokeCircle(newPos.getLeftUpperCornerX() + 1, newPos.getLeftUpperCornerY() + 1, POS_SIZE - 2, green);
        }
        else {
            strokeCircle(newPos.getLeftUpperCornerX() +1, newPos.getLeftUpperCornerY() + 1, STONE_SIZE - 2, green);
        }
    }

    /**
     * Set the image invisible.
     * @param stone
     * @param index of the image to set invisible
     */
    public void setStoneImageInvisible(Stone stone, int index) {
        if (stone.equals(Stone.RED)) {
            redStones[index].setVisible(false);
        }
        else {
            blueStones[index].setVisible(false);
        }
    }

    public void setInfoText(String text) {
        infoTA.appendText(text);
    }

    public void setVisibleMyPlayingLbl() {
        if (Mill.getInstance().client.isPlayer1()) {
            stateLblP1.setVisible(true);
            stateLblP2.setVisible(false);
        }
        else {
            stateLblP1.setVisible(false);
            stateLblP2.setVisible(true);
        }
    }

    public void setVisibleOpponentPlayingLbl() {
        if (Mill.getInstance().client.isPlayer1()) {
            stateLblP1.setVisible(false);
            stateLblP2.setVisible(true);
        }
        else {
            stateLblP1.setVisible(true);
            stateLblP2.setVisible(false);
        }
    }

    public void setInfoPlayerLostConnection(boolean player1) {
        setInfoText("Your opponent lost connection.");
        if (player1) {
            stateLblP1.setText("Reconnecting...");
            stateLblP1.setVisible(true);
            stateLblP2.setVisible(false);
        }
        else {
            stateLblP2.setText("Reconnecting...");
            stateLblP2.setVisible(true);
            stateLblP1.setVisible(false);
        }
    }

    public void setPlayer2Nick() {
        nickname2Lbl.setText(room.getPlayer2());
    }

    public void updateSelectedStone(GamePosition oldPos, GamePosition newPos) {
        if (oldPos != null) {
            if (game.myStone.equals(Stone.RED)) {
                fillCircle(oldPos.getLeftUpperCornerX(), oldPos.getLeftUpperCornerY(), STONE_SIZE, red);
            }
            else {
                fillCircle(oldPos.getLeftUpperCornerX(), oldPos.getLeftUpperCornerY(), STONE_SIZE, blue);
            }
            strokeCircle(newPos.getLeftUpperCornerX() + 1, newPos.getLeftUpperCornerY() + 1, STONE_SIZE - 2, Color.ORANGE);
        }
        else {
            strokeCircle(newPos.getLeftUpperCornerX() + 1, newPos.getLeftUpperCornerY() + 1, STONE_SIZE - 2, Color.ORANGE);
        }
    }
}
