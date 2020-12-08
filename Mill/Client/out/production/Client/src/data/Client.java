package data;

import data.enums.*;
import javafx.application.Platform;

import java.io.*;
import java.net.Socket;
import java.net.UnknownHostException;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.function.Consumer;

public class Client {

    private String host; // address of the server
    private int port; // port of the server
    private String nickname; // nickname of the client
    private boolean player1;
    private State state; // current state of the client
    private boolean isConnected = false; // if client is connected to the server
    private boolean reconnect = false; // if client was disconnected and now trying to reconnect
    private final AtomicBoolean running = new AtomicBoolean(false);
    private Consumer<Serializable> onReceiveCallback;
    private ConnectionThread con = new ConnectionThread();
    private Socket socket; // client socket

    public Client(String host, int port, String nickname, Consumer<Serializable> onReceiveCallback) {
        this.host = host;
        this.port = port;
        this.nickname = nickname;
        this.onReceiveCallback = onReceiveCallback;
        con.setDaemon(true);
    }

    public String getNickname() {
        return nickname;
    }

    public String getHost() {
        return host;
    }

    public int getPort() {
        return port;
    }

    public void setPlayer1(boolean isPlayer1) {
        player1 = isPlayer1;
    }

    public boolean isPlayer1() {
        return player1;
    }

    public boolean isConnected() {
        return isConnected;
    }

    public boolean getReconnect() {
        return reconnect;
    }

    public void setState(State st) {
        this.state = st;
    }

    public State getState() {
        return state;
    }

    public void createConnection() {
        running.set(true);
        con.start();
    }

    public void closeConnection() {
        System.out.println("Close connection");
        Mill.getInstance().changeScene(Mill.loginScene);
        this.setState(State.DISCONNECTED);
        this.isConnected = false;
        running.set(false);

        try {
            con.input.close();
            con.output.close();

        }
        catch (Exception e) {
            System.out.println("I/O stream closing error");
        }
        finally {
            try {
                if (con.socket != null)
                    con.socket.close();;
            }
            catch (IOException e) {
                System.out.println("Socket closing error");
            }
        }
    }

    public void sendMsg(Serializable msg) {
        try {
            System.out.print("Send:" + msg);
            con.output.print(msg);
            con.output.flush();
        }
        catch (Exception e) {
            System.out.println("Write error");
        }
    }

    private class ConnectionThread extends Thread {
        private Socket socket;
        private PrintWriter output;
        private BufferedReader input;

        @Override
        public void run() {
            // Create socket
            try {
                socket = new Socket(host, port);
            }
            catch (UnknownHostException e) {
                System.out.println("IP address of the host could not be determined.");
                Platform.runLater(() -> {
                    Mill.getInstance().loginGUI.setInfoLbl("Connection failed.\nInvalid IP address of the host.");
                    Mill.getInstance().loginGUI.setLoginBtn(true);
                });
                return;
            }
            catch (IOException e) {
                System.out.println("Connection to " + host + ":" + port + " failed.");
                Platform.runLater(() -> {
                    Mill.getInstance().loginGUI.setInfoLbl("Connection failed.\nServer is unreachable.");
                    Mill.getInstance().loginGUI.setLoginBtn(true);
                });
                return;
            }
            catch (IllegalArgumentException e){
                System.out.println("Port is not in range 0 - 65535.");
                Platform.runLater(() -> {
                    Mill.getInstance().loginGUI.setInfoLbl("Connection failed.\nPort is not in range 0 - 65535.");
                    Mill.getInstance().loginGUI.setLoginBtn(true);
                });
                return;
            }
            catch (NullPointerException e){
                System.out.println("Hostname not supplied.");
                Platform.runLater(() -> {
                    Mill.getInstance().loginGUI.setInfoLbl("Connection failed.\nAddress is missing.");
                    Mill.getInstance().loginGUI.setLoginBtn(true);
                });
                return;
            }

            // Create input/output streams
            try {
                input = new BufferedReader(new InputStreamReader(socket.getInputStream()));
                output = new PrintWriter(new OutputStreamWriter(socket.getOutputStream()));
            }
            catch (Exception e) {
                System.out.println("Create I/O streams error");
                Platform.runLater(() -> {
                    Mill.getInstance().loginGUI.setInfoLbl("Connection failed.\nServer is unreachable.");
                    Mill.getInstance().loginGUI.setLoginBtn(true);
                });
                return;
            }

            isConnected = true;
            Mill.getInstance().client.setState(data.enums.State.CONNECTED);
            System.out.println("Successfully connected.");
            Mill.getInstance().firstLogin = false;
            sendMsg(String.format(Message.getMessage(Response.LOGIN_NEW), nickname));
            startReading(input);
        }
    }

    private void startReading(BufferedReader input) {
        while (running.get()) {
            try {
                Serializable data = input.readLine();
                System.out.println("Received:" + data);
                if (data == null) {
                    closeConnection();
                    Platform.runLater(()-> {
                        Mill.getInstance().loginGUI.setInfoLbl("Server is unreachable.");
                    });
                    break;
                }
                onReceiveCallback.accept(data);
            }
            catch (IOException e) {
                System.out.println("Reading error");
                return;
            }
        }
    }
}
