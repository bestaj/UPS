package data.controllers;

import data.Mill;
import javafx.application.Platform;
import javafx.fxml.FXML;
import javafx.fxml.Initializable;
import javafx.scene.control.Button;
import javafx.scene.control.Label;
import javafx.scene.control.TextField;

import java.net.URL;
import java.util.ResourceBundle;

/**
 * The {@code LoginController} takes care about functionality
 * of the login scene.
 */
public class LoginController implements Initializable {

    @FXML
    private TextField nicknameTF, ipTF, portTF;
    @FXML
    private Label infoLbl;
    @FXML
    private Button loginBtn;

    /** Maximum length of the nickname */
    private final short MAX_L_NICK = 18;

    /** Initializes the login scene */
    @Override
    public void initialize(URL location, ResourceBundle resources) {
        Mill.getInstance().loginGUI = this;

        // Set the maximum length of the nickname
        nicknameTF.textProperty().addListener((observable, oldValue, newValue) -> {
            if (newValue.length() > MAX_L_NICK) {
                nicknameTF.setText(oldValue);
            }
        });

        // Fill the same values into textfields as with previous log in
        if (!Mill.getInstance().firstLogin) {
            nicknameTF.setText(Mill.getInstance().client.getNickname());
            ipTF.setText(Mill.getInstance().client.getHost());
            portTF.setText(String.valueOf(Mill.getInstance().client.getPort()));
        }
    }

    /** Create a new client */
    @FXML
    private void connect() {
        int port;

        if (nicknameTF.getText().equals("")) { // Nickname cannot be empty
            infoLbl.setText("Enter your nickname!");
        }
        else {
            if (nicknameTF.getText().matches("^(\\s.*|.*\\s)$")) { // If nickname starts/ends with whitespaces
                infoLbl.setText("Invalid nickname.\nNickname starts/ends with whitespace.");
            }
            else if (nicknameTF.getText().contains(";")) {   // Nickname cannot contains semicolon
                infoLbl.setText("Invalid nickname.\nNickname contains semicolon.");
            }
            else {
                try {
                    port = Integer.parseInt(portTF.getText());
                }
                catch (Exception e) {
                    infoLbl.setText("Invalid port.\nPort is number in range 0 - 65536.");
                    return;
                }
                infoLbl.setText("Connecting...");
                setLoginBtn(false);
                Mill.getInstance().createClient(ipTF.getText(), port, nicknameTF.getText());
            }
        }
    }

    /** Exit the app */
    @FXML
    private void exit() {
        if (Mill.getInstance().client.isConnected())
            Mill.getInstance().client.closeConnection();

        Platform.exit();
    }

    public void setInfoLbl(String text) {
        infoLbl.setText(text);
    }

    public void setLoginBtn(boolean visible) {
        if (visible) {
            loginBtn.setDisable(false);
            loginBtn.setOpacity(1);
        }
        else {
            loginBtn.setDisable(true);
            loginBtn.setOpacity(0.5);
        }
    }
}
