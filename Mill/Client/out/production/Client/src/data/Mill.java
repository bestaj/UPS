package data;

import data.controllers.GameController;
import data.controllers.LobbyController;
import data.controllers.LoginController;
import javafx.application.Platform;
import javafx.fxml.FXMLLoader;
import javafx.scene.Parent;
import javafx.stage.Stage;

public class Mill {

    /** Instance of the Mill */
    private static final Mill instance = new Mill();
    private static final int ERROR = -1;
    private static Processing processing = new Processing();

    /** Paths to the specific scenes of the game */
    public static final String loginScene = "/fxml/Login.fxml";
    public static final String lobbyScene = "/fxml/Lobby.fxml";
    public static final String gameScene = "/fxml/Game.fxml";

    public LoginController loginGUI;
    public LobbyController lobbyGUI;
    public GameController gameGUI;
    /** Window of the app */
    public Stage window;
    /** Instance of the client */
    public Client client;
    /** Instance of the selected room or your playing room */
    public Room selectedRoom;
    /** Instance of the game */
    public Game game;
    public boolean firstLogin = true;

    /** Return the instance of the game */
    public static Mill getInstance() {
        return instance;
    }

    /**
     * Change the scene to another one
     * @param scene     a new scene
     */
    public void changeScene(String scene) {
        Platform.runLater(() -> {
            try {
                 Parent pane = FXMLLoader.load(getClass().getResource(scene));
                 window.getScene().setRoot(pane);

            }
            catch (Exception e) {
                e.printStackTrace();
            }
        });
    }

    public void createClient(String host, int port, String nickname) {
        client = new Client(host, port, nickname, data -> {
            Message msg = Message.parseMessage((String)data, client.getState());
            if (msg == null) {
                client.closeConnection();
                Platform.runLater(()-> {
                    loginGUI.setInfoLbl("Unexpected message from the server.");
                });
            }
            else {
                if (processing.processMessage(msg) == ERROR) {
                    System.out.println("Processing message failed.");
                    client.closeConnection();
                    Platform.runLater(() -> {
                        lobbyGUI.setInfoLbl("Communication with the server failed.");
                    });
                }
            }
        });
        client.createConnection();
    }


    public void joinRoom(Room room) {
        room.joinPlayer2(client.getNickname());
        this.selectedRoom = room;
        game = new Game();
        changeScene(gameScene);
        game.startGame(false);
    }

}
