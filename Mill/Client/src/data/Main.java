package data;

import javafx.application.Application;
import javafx.fxml.FXMLLoader;
import javafx.scene.Parent;
import javafx.scene.Scene;
import javafx.stage.Stage;

/**
 * The {@code Main} is the main class of the application,
 * where app is launched. The application is multiplayer game
 * called Mill.
 * GUI is created with JavaFX 11.0.2.
 *
 * @see Application
 */
public class Main extends Application {

    /** Entry point of the app */
    public static void main(String[] args) {
        launch();
    }

    /** Creates the main window and set the login scene */
    @Override
    public void start(Stage primaryStage) throws Exception {
        Parent root = FXMLLoader.load(getClass().getResource(Mill.loginScene));
        primaryStage.setTitle("Mill");
        primaryStage.setMinWidth(600);
        primaryStage.setMinHeight(600);
        primaryStage.setResizable(false);
        primaryStage.setScene(new Scene(root));
        primaryStage.show();

        Mill.getInstance().window = primaryStage;
    }
}
