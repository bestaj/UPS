package data;

import data.enums.*;
import javafx.application.Platform;
import javafx.scene.input.MouseEvent;
import javafx.scene.shape.Circle;

public class Game {

    private GamePosition[] gamePositions = new GamePosition[24];

    public GamePosition oldVisitedPosition = null;
    public GamePosition newVisitedPosition = null;
    private GamePosition lastVisitedPosition = null;
    private GamePosition pos = null;

    public int myUnusedStoneIndex = 8;
    public int opponentsUnusedStoneIndex = 8;
    public int myStoneCounter = 9; // Number of stones which you have
    public int opponentsStoneCounter = 9; // Number of stones which your opponent have

    public Stone myStone;
    private Stone opponentsStone;
    private GamePosition selectedStone;
    private boolean isSelected = false; // If you have selected stone to shift
    public boolean takingStone = false;
    public State returnedState;
    public boolean paused = false;
    private GamePosition clickedPos;
    public boolean play = false; // If you play or your opponent

    public Action action = Action.NONE;
    private GamePosition setPos = null;
    private GamePosition shiftFrom = null;
    private GamePosition shiftTo = null;
    private GamePosition takePos = null;

    public Game() {
        prepareGame();
    }

    private void prepareGame() {

        int len = 460 / 8; // Length between single gaming positions

        // Create all points of all gaming positions
        /* 0 -------------- 1 --------------- 2
           |                |                 |
           |    8 --------- 9 --------- 10    |
           |    |           |           |     |
           |    |     16 -- 17 -- 18    |     |
           |    |     |           |     |     |
           7 -- 15 -- 23          19 -- 11 -- 3
           |    |     |           |     |     |
           |    |     22 -- 21 -- 20    |     |
           |    |           |           |     |
           |    14 -------- 13 -------- 12    |
           |                |                 |
           6 -------------- 5 --------------- 4
         */
        gamePositions[0] = new GamePosition(len, len, 0);
        gamePositions[1] = new GamePosition(4 * len, len, 1);
        gamePositions[2] = new GamePosition(7 * len, len, 2);
        gamePositions[3] = new GamePosition(7 * len, 4 * len, 3);
        gamePositions[4] = new GamePosition(7 * len, 7 * len, 4);
        gamePositions[5] = new GamePosition(4 * len, 7 * len, 5);
        gamePositions[6] = new GamePosition(len, 7 * len, 6);
        gamePositions[7] = new GamePosition(len, 4 * len, 7);

        gamePositions[8] = new GamePosition(2 * len, 2 * len, 8);
        gamePositions[9] = new GamePosition(4 * len, 2 * len, 9);
        gamePositions[10] = new GamePosition(6 * len, 2 * len, 10);
        gamePositions[11] = new GamePosition(6 * len, 4 * len, 11);
        gamePositions[12] = new GamePosition(6 * len, 6 * len, 12);
        gamePositions[13] = new GamePosition(4 * len, 6 * len, 13);
        gamePositions[14] = new GamePosition(2 * len, 6 * len, 14);
        gamePositions[15] = new GamePosition(2 * len, 4 * len, 15);

        gamePositions[16] = new GamePosition(3 * len, 3 * len, 16);
        gamePositions[17] = new GamePosition(4 * len, 3 * len, 17);
        gamePositions[18] = new GamePosition(5 * len, 3 * len, 18);
        gamePositions[19] = new GamePosition(5 * len, 4 * len, 19);
        gamePositions[20] = new GamePosition(5 * len, 5 * len, 20);
        gamePositions[21] = new GamePosition(4 * len, 5 * len, 21);
        gamePositions[22] = new GamePosition(3 * len, 5 * len, 22);
        gamePositions[23] = new GamePosition(3 * len, 4 * len, 23);

        // Add to every game position all its neighbours
        // Large square - top-left
        gamePositions[0].neighbours.add(gamePositions[1]);
        gamePositions[0].neighbours.add(gamePositions[7]);
        // Large square - top-mid
        gamePositions[1].neighbours.add(gamePositions[0]);
        gamePositions[1].neighbours.add(gamePositions[2]);
        gamePositions[1].neighbours.add(gamePositions[9]);
        // Large square - top-right
        gamePositions[2].neighbours.add(gamePositions[1]);
        gamePositions[2].neighbours.add(gamePositions[3]);
        // Large square - mid-right
        gamePositions[3].neighbours.add(gamePositions[1]);
        gamePositions[3].neighbours.add(gamePositions[4]);
        gamePositions[3].neighbours.add(gamePositions[11]);
        // Large square - bot-right
        gamePositions[4].neighbours.add(gamePositions[3]);
        gamePositions[4].neighbours.add(gamePositions[5]);
        // Large square - bot-mid
        gamePositions[5].neighbours.add(gamePositions[4]);
        gamePositions[5].neighbours.add(gamePositions[6]);
        gamePositions[5].neighbours.add(gamePositions[13]);
        // Large square - bot-left
        gamePositions[6].neighbours.add(gamePositions[5]);
        gamePositions[6].neighbours.add(gamePositions[7]);
        // Large square - mid-left
        gamePositions[7].neighbours.add(gamePositions[6]);
        gamePositions[7].neighbours.add(gamePositions[0]);
        gamePositions[7].neighbours.add(gamePositions[15]);
        // ************************************************************************
        // Medium square - top-left
        gamePositions[8].neighbours.add(gamePositions[9]);
        gamePositions[8].neighbours.add(gamePositions[15]);
        // Medium square - top-mid
        gamePositions[9].neighbours.add(gamePositions[1]);
        gamePositions[9].neighbours.add(gamePositions[8]);
        gamePositions[9].neighbours.add(gamePositions[10]);
        gamePositions[9].neighbours.add(gamePositions[17]);
        // Medium square - top-right
        gamePositions[10].neighbours.add(gamePositions[9]);
        gamePositions[10].neighbours.add(gamePositions[11]);
        // Medium square - mid-right
        gamePositions[11].neighbours.add(gamePositions[3]);
        gamePositions[11].neighbours.add(gamePositions[10]);
        gamePositions[11].neighbours.add(gamePositions[12]);
        gamePositions[11].neighbours.add(gamePositions[19]);
        // Medium square - bot-right
        gamePositions[12].neighbours.add(gamePositions[11]);
        gamePositions[12].neighbours.add(gamePositions[13]);
        // Medium square - bot-mid
        gamePositions[13].neighbours.add(gamePositions[5]);
        gamePositions[13].neighbours.add(gamePositions[12]);
        gamePositions[13].neighbours.add(gamePositions[14]);
        gamePositions[13].neighbours.add(gamePositions[21]);
        // Medium square - bot-left
        gamePositions[14].neighbours.add(gamePositions[13]);
        gamePositions[14].neighbours.add(gamePositions[15]);
        // Medium square - mid-left
        gamePositions[15].neighbours.add(gamePositions[7]);
        gamePositions[15].neighbours.add(gamePositions[8]);
        gamePositions[15].neighbours.add(gamePositions[14]);
        gamePositions[15].neighbours.add(gamePositions[23]);
        // ************************************************************************
        // Small square - top-left
        gamePositions[16].neighbours.add(gamePositions[17]);
        gamePositions[16].neighbours.add(gamePositions[23]);
        // Small square - top-mid
        gamePositions[17].neighbours.add(gamePositions[9]);
        gamePositions[17].neighbours.add(gamePositions[16]);
        gamePositions[17].neighbours.add(gamePositions[18]);
        // Small square - top-right
        gamePositions[18].neighbours.add(gamePositions[17]);
        gamePositions[18].neighbours.add(gamePositions[19]);
        // Small square - mid-right
        gamePositions[19].neighbours.add(gamePositions[11]);
        gamePositions[19].neighbours.add(gamePositions[18]);
        gamePositions[19].neighbours.add(gamePositions[20]);
        // Small square - bot-right
        gamePositions[20].neighbours.add(gamePositions[19]);
        gamePositions[20].neighbours.add(gamePositions[21]);
        // Small square - bot-mid
        gamePositions[21].neighbours.add(gamePositions[13]);
        gamePositions[21].neighbours.add(gamePositions[20]);
        gamePositions[21].neighbours.add(gamePositions[22]);
        // Small square - bot-left
        gamePositions[22].neighbours.add(gamePositions[21]);
        gamePositions[22].neighbours.add(gamePositions[23]);
        // Small square - mid-left
        gamePositions[23].neighbours.add(gamePositions[15]);
        gamePositions[23].neighbours.add(gamePositions[16]);
        gamePositions[23].neighbours.add(gamePositions[22]);
    }

    public void startGame(boolean player1) {
        if (player1) {
            Mill.getInstance().client.setState(State.WAITING_FOR_OPP);
            Mill.getInstance().client.setPlayer1(true);
            myStone = Stone.RED;
            opponentsStone = Stone.BLUE;
            Platform.runLater(() -> {
                Mill.getInstance().gameGUI.setInfoText("Waiting for the opponent...\n");
            });

        }
        else {
            Mill.getInstance().client.setState(State.OPP_TURN);
            Mill.getInstance().client.setPlayer1(false);
            myStone = Stone.BLUE;
            opponentsStone = Stone.RED;
            Platform.runLater(() -> {
                Mill.getInstance().gameGUI.setInfoText("Red begins the game.\nWaiting for the opponent move.\n");
                Mill.getInstance().gameGUI.setVisibleOpponentPlayingLbl();
            });
        }
    }

    public void setYourOpponent(String nickname) {
        Mill.getInstance().selectedRoom.setPlayer2(nickname);
        Platform.runLater(() -> {
            Mill.getInstance().gameGUI.setPlayer2Nick();
            Mill.getInstance().gameGUI.setInfoText("Your opponent is connected.\nRed begins the game.\nPut the stone on a free position.\n");
            Mill.getInstance().gameGUI.setVisibleMyPlayingLbl();
        });
        play = true;
    }

    public void setStone(boolean mystone, int posID) {
        if (mystone) {
            setPos.setStonePosition(myStone);
            action = Action.NONE;
            Platform.runLater(() -> {
                Mill.getInstance().gameGUI.updateCanvas();
                Mill.getInstance().gameGUI.setInfoText("You put the stone.\n");
                Mill.getInstance().gameGUI.setStoneImageInvisible(myStone, myUnusedStoneIndex--);
            });
        }
        else {
            gamePositions[posID].setStonePosition(opponentsStone);
            Platform.runLater(() -> {
                Mill.getInstance().gameGUI.updateCanvas();
                Mill.getInstance().gameGUI.setInfoText("Your opponent put the stone.\n");
                Mill.getInstance().gameGUI.setStoneImageInvisible(opponentsStone, opponentsUnusedStoneIndex--);
            });
        }
    }

    public void shiftStone(boolean mystone, int pos1ID, int pos2ID) {
        if (mystone) {
            shiftFrom.setPositionFree();
            shiftTo.setStonePosition(myStone);
            action = Action.NONE;
            isSelected = false;
            selectedStone = null;
            Platform.runLater(() -> {
                Mill.getInstance().gameGUI.setInfoText("You shifted the stone.\n");
                Mill.getInstance().gameGUI.updateCanvas();
            });
        }
        else {
            gamePositions[pos1ID].setPositionFree();
            gamePositions[pos2ID].setStonePosition(opponentsStone);
            Platform.runLater(() -> {
                Mill.getInstance().gameGUI.setInfoText("Your opponent shifted the stone.\n");
                Mill.getInstance().gameGUI.updateCanvas();
            });
        }
        isSelected = false;
        selectedStone = null;
    }

    public void takeStone(boolean mystone, int posID) {
        lastVisitedPosition = null;
        takingStone = false;

        if (mystone) {
            gamePositions[posID].setPositionFree();
            myStoneCounter--;
            Platform.runLater(() -> {
                Mill.getInstance().gameGUI.setInfoText("Opponent took your stone.\n");
                Mill.getInstance().gameGUI.updateCanvas();
            });
        }
        else {
            takePos.setPositionFree();
            opponentsStoneCounter--;
            Platform.runLater(() -> {
                Mill.getInstance().gameGUI.setInfoText("You took opponent's stone.\n");
                Mill.getInstance().gameGUI.updateCanvas();
            });
        }
    }

    public void processMouseMove(MouseEvent event) {
        if (takingStone) { // Taking the stone
            if ((pos = getOpponentsPosition(event)) != null) {
                if (!testMill(pos, opponentsStone)) {
                    if (lastVisitedPosition != null) {
                        return;
                    }

                    Platform.runLater(() -> {
                        Mill.getInstance().gameGUI.markNewVisitedPosition(pos);
                    });
                    lastVisitedPosition = pos;
                   // updateVisitedPositions(pos);
                }
            }
            else {
                if (lastVisitedPosition != null) {
                    Platform.runLater(() -> {
                        Mill.getInstance().gameGUI.clearLastVisitedPosition(lastVisitedPosition);
                        lastVisitedPosition = null;
                    });

                }
            }
        }
        else {
            if (myUnusedStoneIndex >= 0) {  // Setting the stone
                if ((pos = getFreePosition(event)) != null) {
                    if (lastVisitedPosition != null) {
                        /*
                        if (pos == lastVisitedPosition) { // Mouse move on the same position as before
                            return;
                        }

                         */
                        return;
                    }

                    Platform.runLater(() -> {
                        Mill.getInstance().gameGUI.markNewVisitedPosition(pos);
                    });
                    lastVisitedPosition = pos;
                        //updateVisitedPositions(pos);

                }
                else {
                    if (lastVisitedPosition != null) {
                        Platform.runLater(() -> {
                            Mill.getInstance().gameGUI.clearLastVisitedPosition(lastVisitedPosition);
                            lastVisitedPosition = null;
                        });
                    }
                }
            }
            else {  // Shifting the stone
                if (!isSelected) {
                    if ((pos = getMyPosition(event)) != null) {
                        if (isShiftPossible(pos)) {
                            if (lastVisitedPosition != null) {
                                return;
                            }
                            Platform.runLater(() -> {
                                Mill.getInstance().gameGUI.markNewVisitedPosition(pos);
                            });
                            lastVisitedPosition = pos;
                          //  updateVisitedPositions(pos);
                        }
                    }
                    else {
                        if (lastVisitedPosition != null) {
                            Platform.runLater(() -> {
                                Mill.getInstance().gameGUI.clearLastVisitedPosition(lastVisitedPosition);
                                lastVisitedPosition = null;
                            });

                        }
                    }
                }
                else {
                    if ((pos = getFreePositionToShift(event)) != null) {
                        if (lastVisitedPosition != null) {
                            return;
                        }
                        Platform.runLater(() -> {
                            Mill.getInstance().gameGUI.markNewVisitedPosition(pos);
                        });
                        lastVisitedPosition = pos;
                   //     updateVisitedPositions(pos);
                    }
                    else {
                        /*
                        if (lastVisitedPosition != null) {

                            Platform.runLater(() -> {
                                Mill.getInstance().gameGUI.clearLastVisitedPosition(lastVisitedPosition);
                                lastVisitedPosition = null;
                            });

                        }
*/
                        if ((pos = getMyPosition(event)) != null) {
                            if (pos == selectedStone) {
                                return;
                            }
                            if (isShiftPossible(pos)) {
                                if (lastVisitedPosition != null) {
                                    return;
                                }
                                Platform.runLater(() -> {
                                    Mill.getInstance().gameGUI.markNewVisitedPosition(pos);
                                });
                                lastVisitedPosition = pos;
                               // updateVisitedPositions(pos);
                            }
                        }
                        else {
                            if (lastVisitedPosition != null) {
                                Platform.runLater(() -> {
                                    Mill.getInstance().gameGUI.clearLastVisitedPosition(lastVisitedPosition);
                                    lastVisitedPosition = null;
                                });

                            }
                        }
                    }
                }
            }
        }
    }
/*
    private void updateVisitedPositions(GamePosition pos) {
        oldVisitedPosition = newVisitedPosition;
        newVisitedPosition = pos;
        Platform.runLater(() -> {
            Mill.getInstance().gameGUI.repaintVisitedPositions();
        });
    }
*/
    public void processMouseClick(MouseEvent event) {

        if (Mill.getInstance().client.getState() == State.MY_TURN) {
            if (myUnusedStoneIndex >= 0) {    // Put the stone on the playing field
                if ((clickedPos = getFreePosition(event)) != null) {  // if clicked on free position
                    action = Action.SET;
                    setPos = clickedPos;
                    play = false;
                    Mill.getInstance().client.sendMsg(String.format(Message.getMessage(Response.SET_STONE), setPos.getIndex()));
                }
            }
            else {    // Shift your stone
                if (isSelected) {
                    if ((clickedPos = getFreePositionToShift(event)) != null) { // if clicked on any free position where is possible to shift the stone
                        action = Action.SHIFT;
                        shiftFrom = selectedStone;
                        shiftTo = clickedPos;
                        play = false;
                        Mill.getInstance().client.sendMsg(String.format(Message.getMessage(Response.SHIFT_STONE), shiftFrom.getIndex(), shiftTo.getIndex()));
                    }
                    else {
                        if ((clickedPos = getMyPosition(event)) != null) { // if clicked on my stone - change selected stone
                            if (selectedStone == clickedPos) {
                                return; // Selected the same stone as before
                            }
                            if (!isShiftPossible(clickedPos)) {
                                return;
                            }
                            lastVisitedPosition = null;
                            Platform.runLater(() -> {
                                Mill.getInstance().gameGUI.updateSelectedStone(selectedStone, clickedPos);
                                selectedStone = clickedPos;
                            });

                        }
                    }
                }
                else { // Not selected stone yet
                    if ((clickedPos = getMyPosition(event)) != null) { // if clicked on any my stone
                        if (!isShiftPossible(clickedPos)) {
                            return;
                        }
                        selectedStone = clickedPos;
                        lastVisitedPosition = null;
                     //   newVisitedPosition = null;
                        isSelected = true;
                        Platform.runLater(() -> {
                                Mill.getInstance().gameGUI.updateSelectedStone(null, selectedStone);
                        });
                    }
                }
            }
        }
        else {  // Take some opponent's stone
            if ((clickedPos = getOpponentsPosition(event)) != null) { // Returns the position if clicked on opponent's stone
                if (testMill(clickedPos, opponentsStone)) { // Test if opponent's stone is part of some mill
                    Platform.runLater(() -> {
                        Mill.getInstance().gameGUI.setInfoText("Opponent's stone is inside a mill.\n");
                    });
                }
                else {
                    takePos = clickedPos;
                    play = false;
                    Mill.getInstance().client.sendMsg(String.format(Message.getMessage(Response.TAKE_STONE), takePos.getIndex()));
                }
            }
        }
    }

    private boolean isShiftPossible(GamePosition pos) {
        for (GamePosition p: pos.neighbours) {
            if (p.isFree()) {
                return true;
            }
        }
        return false;
    }

    // Test if mouse clicked on any free game position
    private GamePosition getFreePosition(MouseEvent event) {
        for (int i = 0; i < 24; i++) {
            if (gamePositions[i].isFree() && gamePositions[i].getCircle().contains(event.getX(), event.getY())) {
                return gamePositions[i];
            }
        }

       /*
        for (Map.Entry<Integer, GamePosition> pos: freePositions.entrySet()) {
            if (pos.getValue().getCircle().contains(event.getX(), event.getY())) {
                return pos.getValue();
            }
        }

        */
        return null;
    }

    // Test if mouse clicked on any possible position to shift the stone
    private GamePosition getFreePositionToShift(MouseEvent event) {
        for (GamePosition p: selectedStone.neighbours) {
            if (p.isFree() && p.getCircle().contains(event.getX(), event.getY())) {
                return p;
            }
        }
        return null;
    }

    // Test if mouse clicked on any my stone
    private GamePosition getMyPosition(MouseEvent event) {
        for (int i = 0; i < 24; i++) {
            if (!gamePositions[i].isFree() && gamePositions[i].getStone().equals(myStone) &&
                    gamePositions[i].getCircle().contains(event.getX(), event.getY())) {
                return gamePositions[i];
            }
        }
        return null;
    }

    // Test if mouse clicked on any opponent's stone
    private GamePosition getOpponentsPosition(MouseEvent event) {
        for (int i = 0; i < 24; i++) {
            if (!gamePositions[i].isFree() && gamePositions[i].getStone().equals(opponentsStone) &&
                    gamePositions[i].getCircle().contains(event.getX(), event.getY())) {
                return gamePositions[i];
            }
        }
        return null;
    }

    private boolean testMill(GamePosition pos, Stone stone) {
        int posID = pos.getIndex();
        int restMod8 = posID % 8;

        if (posID % 2 == 0) { // Some corner of the playing field
            if (restMod8 == 0) { // Left upper corner
                if (gamePositions[posID + 1].getStone() == stone) {
                    if (gamePositions[posID + 2].getStone() == stone) {
                        return true;
                    }
                }
                if (gamePositions[posID + 6].getStone() == stone) {
                    if (gamePositions[posID + 7].getStone() == stone) {
                        return true;
                    }
                }
            }
            else if (restMod8 == 6) { // Left bottom corner
                if (gamePositions[posID - 1].getStone() == stone) {
                    if (gamePositions[posID - 2].getStone() == stone) {
                        return true;
                    }
                }
                if (gamePositions[posID + 1].getStone() == stone) {
                    if (gamePositions[posID - 6].getStone() == stone) {
                        return true;
                    }
                }
            }
            else { // All other corners
                if (gamePositions[posID + 1].getStone() == stone) {
                    if (gamePositions[posID + 2].getStone() == stone) {
                        return true;
                    }
                }
                if (gamePositions[posID - 1].getStone() == stone) {
                    if (gamePositions[posID - 2].getStone() == stone) {
                        return true;
                    }
                }
            }
        }
        else { // Some middle game position
            if (restMod8 == 7) {
                if (gamePositions[posID - 1].getStone() == stone) {
                    if (gamePositions[posID - 7].getStone() == stone) {
                        return true;
                    }
                }
            }
            else {
                if (gamePositions[posID + 1].getStone() == stone) {
                    if (gamePositions[posID - 1].getStone() == stone) {
                        return true;
                    }
                }
            }
            if (posID < 8) { // Middle of large square
                if (gamePositions[posID + 8].getStone() == stone) {
                    if (gamePositions[posID + 16].getStone() == stone) {
                        return true;
                    }
                }
            }
            else if (posID > 15) { // Middle of small square
                if (gamePositions[posID - 8].getStone() == stone) {
                    if (gamePositions[posID - 16].getStone() == stone) {
                        return true;
                    }
                }
            }
            else { // Middle of middle square
                if (gamePositions[posID + 8].getStone() == stone) {
                    if (gamePositions[posID - 8].getStone() == stone) {
                        return true;
                    }
                }
            }
        }
        return false;
    }

    /** Return the circle around the game position */
    public Circle getCircle(int positionIndex) {
        return gamePositions[positionIndex].getCircle();
    }

    public GamePosition getGamePosition(int index) {
        return gamePositions[index];
    }
}
